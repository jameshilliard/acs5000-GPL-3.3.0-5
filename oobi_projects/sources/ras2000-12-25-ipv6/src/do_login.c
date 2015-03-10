/*
 * do_login.c	This is the part that talks with the device. It gets
 *				the username/password for the login.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <termios.h>
#include <errno.h>

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef USE_PAM
#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <security/pam_appdata.h>
#endif

#include "server.h"

HANDLECHAR checkIAC = NULL;

//function prototypes
extern void crtrans(int fd, int inon, int outon);

extern struct line_cfg *pconf;
extern struct auth *pai;


#ifdef USE_PAM

/* the following code is used to get text input */

/* read a line of input string, giving prompt when appropriate */
static char *read_string(int echo, const char *prompt, struct pam_appl_data *pappl)
{
	char line[INPUTSIZE] = {0};
	char *input;
	int r = 0;

    D(("called with echo='%s', prompt='%s'.", echo ? "ON":"OFF" , prompt));

	crtrans(0, 0, 1);
	r = get_line(pappl->ppp, prompt, echo ? 3 : 4,
		line, sizeof(line), pappl->sock);
	crtrans(0, 1, 1);

	pappl->result = r;
	if (r == -2) {
		nsyslog(LOG_INFO, "Login timed out / quit");
		pappl->result = 2;
	}

	if (!pappl->conv_once && r == 0)
		nsyslog(LOG_INFO, "Detected login for %s", line);

	/* A kludge so that a leading '!' is a local login. */
	if (!pappl->conv_once && line[0] == '!') {
		pappl->result = 3;
		strncpy(pappl->name, line+1, sizeof(pappl->name));
		return(NULL);
	}
	pappl->conv_once = 1;   /* we've been here, ignore leading '!' now */

	if (pappl->result)
		return(NULL);

	input = x_strdup(line);
	return input;                  /* return malloc()ed string */
}

/* end of read_string functions */

int socket_tty_conv(int num_msg, const struct pam_message **msgm,
	      struct pam_response **response, void *appdata_ptr)
{
    int count=0;
    int len;
    struct pam_response *reply;
	struct pam_appl_data *pappl = (struct pam_appl_data *)appdata_ptr;

    if (num_msg <= 0)
	return PAM_CONV_ERR;

    D(("allocating empty response structure array."));

    reply = (struct pam_response *) calloc(num_msg,
					   sizeof(struct pam_response));
    if (reply == NULL) {
	D(("no memory for responses"));
	return PAM_CONV_ERR;
    }

    D(("entering conversation function."));

    for (count=0; count < num_msg; ++count) {
	char *string=NULL;

	switch (msgm[count]->msg_style) {
	case PAM_PROMPT_ECHO_OFF:
	    string = read_string(CONV_ECHO_OFF,msgm[count]->msg, pappl);
	    if (string == NULL) {
		goto failed_conversation;
	    }
	    break;
	case PAM_PROMPT_ECHO_ON:
	    string = read_string(CONV_ECHO_ON,msgm[count]->msg, pappl);
	    if (string == NULL) {
		goto failed_conversation;
	    }
	    break;
	case PAM_ERROR_MSG:
	    len = strlen(msgm[count]->msg);
	    if (write(STDIN_FILENO, msgm[count]->msg, len) < len) {
		goto failed_conversation;
	    }
	    //[RK]Mar/27/06 - include \n 
	    write(STDIN_FILENO, "\n", 1);
	    break;
	case PAM_TEXT_INFO:
	    len = strlen(msgm[count]->msg);
	    if (write(STDIN_FILENO, msgm[count]->msg, len) < len){
		goto failed_conversation;
	    }
	    //[RK]Mar/27/06 - include \n 
	    write(STDIN_FILENO, "\n", 1);
	    break;
	default:
	    fprintf(stderr, "erroneous conversation (%d)\n"
		    ,msgm[count]->msg_style);
	    goto failed_conversation;
	}

	if (string) {                         /* must add to reply array */
	    /* add string to list of responses */

	    reply[count].resp_retcode = 0;
	    reply[count].resp = string;
	    string = NULL;
	}
    }

    /* New (0.59+) behavior is to always have a reply - this is
       compatable with the X/Open (March 1997) spec. */
    *response = reply;
    reply = NULL;

    return PAM_SUCCESS;

failed_conversation:

    if (reply) {
	for (count=0; count<num_msg; ++count) {
	    if (reply[count].resp == NULL) {
		continue;
	    }
	    switch (msgm[count]->msg_style) {
	    case PAM_PROMPT_ECHO_ON:
	    case PAM_PROMPT_ECHO_OFF:
		_pam_overwrite(reply[count].resp);
		free(reply[count].resp);
		break;
	    case PAM_ERROR_MSG:
	    case PAM_TEXT_INFO:
		/* should not actually be able to get here... */
		free(reply[count].resp);
	    }                                            
	    reply[count].resp = NULL;
	}
	/* forget reply too */
	free(reply);
	reply = NULL;
    }

    return PAM_CONV_ERR;
}
#endif

/*
 *	Alarm handler for the login timeout.
 */

#ifndef CYCLADES
/*
 *	Imitate a modem on a port.
 */
int emumodem(struct auth *ai)
{
	struct line_cfg *lc = &lineconf[ai->nasport];
	char banner [1024];
	char buf[128];

	disass_ctty();

	close(0);
	close(1);
	close(2);

	/*
	 *	First lock or wait if the port is busy.
	 */

	if (term_in_use(lc, 0)) {
		return(-1);
	}

	if (serial_open(lc, 0) < 0) {
		return -1;
	}

	/*
	 *	Now force the new tty into becoming our
	 *	controlling tty. This will also kill other
	 *	processes hanging on this tty.
	 */
	if (ioctl(0, TIOCSCTTY, 1) < 0)
		nsyslog(LOG_WARNING, "TIOCSCTTY: %m");

	/*
	 *	Loop, and emulate a modem.
	 */
	crtrans(0, 0, 0);
	printf("\r\nNO CARRIER\r\n");
	while(1) {
		fflush(stdout);
		get_line(0, "", 1, buf, sizeof(buf), 0);
		if (strncasecmp(buf, "ATD", 3) == 0) {
			sleep(3);
			printf("CONNECT 115200\r\n");
			snprintf (ai->conn_info, sizeof(ai->conn_info), "115200/DIRECT");
			fflush(stdout);
			break;
		}
		if (strncasecmp(buf, "AT&V", 4) == 0) {
			printf("OK - Portslave Modem emulator\r\n");
			continue;
		}
		if (strncasecmp(buf, "AT", 2) == 0) {
			printf("OK\r\n");
			continue;
		}
		if (buf[0]) printf("ERROR\r\n");
	}
	crtrans(0, 1, 1);

	/*
	 *	Delay just a little bit and flush junk.
	 */
	usleep(250000);
	tcflush(0, TCIOFLUSH);

	if (lc->authtype == AUTH_NONE) {
		return(0);
	}	

	/*
	 *	Show the banner and say hello.
	 */
	if (lc->issue && lc->issue[0]) {
		expand_format (banner, sizeof (banner), lc->issue, ai);
		fprintf(stdout, "%s\n", banner);
		fflush(stdout);
	}

	return 0;
}
#endif

static int got_alrm;

static void alrm_handler()
{
	got_alrm = 1;
}
/*
 * prompt login for authtype: none user
 * do not prompt for password
 *
 */
int do_nonlogin(struct auth *ai)
{
	struct line_cfg *lc = &lineconf[ai->nasport];
	int r = 0;
	char *p = lc->prompt ? lc->prompt : "login: ";
	char prompt [1024];
    int flag = (lc->protocol == P_SOCKET_SERVER || lc->protocol == P_SOCKET_SERVER_RAW ||
                lc->protocol == P_SOCKET_SERVER_SSH
#ifdef BIDIRECT
		 || GET_BIDIR_TCP(lc->protocol, lc->conntype, ai->nasport)
#endif
	);

	expand_format (prompt, sizeof (prompt), p, ai);

	crtrans(0, 0, 1);

	do {
		r = get_line(0, prompt, 3, ai->login, sizeof(ai->login), flag);
	} while(r == 0 && ai->login[0] == '\0');

	crtrans(0, 1, 1);

	if (r == 0 && ai->login[0] == '\0')
		return 2;

	if (r >= 0)
		nsyslog(LOG_INFO, "Detected login for %s", ai->login);
	else {
		if (r == -2) {
			nsyslog(LOG_INFO, "Login timed out / quit");
			return(2);
		}
		return(r);
	}
	ai->passwd[0] = '\0';
	maketermsane(0);
	return 0;
}
		
#ifndef USE_PAM
/*
 *	We've got a connection. Now let the user login.
 */
int do_login(struct auth *ai)
{
	struct line_cfg *lc = &lineconf[ai->nasport];
	int r = 0;
	char *p = lc->prompt ? lc->prompt : "login: ";
	char prompt [1024];
    int flag = (lc->protocol == P_SOCKET_SERVER || lc->protocol == P_SOCKET_SERVER_RAW ||
                lc->protocol == P_SOCKET_SERVER_SSH
#ifdef BIDIRECT
		|| lc->protocol == P_TELNET_BIDIRECT
#endif
	);

	expand_format (prompt, sizeof (prompt), p, ai);

	crtrans(0, 0, 1);

	do {
		r = get_line(!flag, prompt, 3, ai->login, sizeof(ai->login), flag);
	} while(r == 0 && ai->login[0] == '\0');

	crtrans(0, 1, 1);

	if (r == 0 && ai->login[0] == '\0')
		return 2;

	if (r >= 0)
		nsyslog(LOG_INFO, "Detected login for %s", ai->login);
	else {
		if (r == -2) {
			nsyslog(LOG_INFO, "Login timed out / quit");
			return(2);
		}
		return(r);
	}
		
	if (r == 1) {
		strncpy(ai->login, "AutoPPP", sizeof(ai->login)-1);
		ai->login [sizeof(ai->login)-1] = '\0';
		ai->passwd[0] = '\0';
		return 0;
	}	

	if (mainconf.locallogins && ai->login[0] == '!' &&
		(lc->protocol != P_SOCKET_SERVER &&
                 lc->protocol != P_SOCKET_SERVER_SSH &&
		 lc->protocol != P_SOCKET_SERVER_RAW 
#ifdef BIDIRECT
		 && lc->protocol != P_TELNET_BIDIRECT
#endif
		))
		return 0;

	if (lc->authtype == AUTH_REMOTE) {
		ai->passwd[0] = '\0';
		return 0;
	}

	crtrans(0, 0, 1);
	if ((r = get_line(!flag, "Password: ", 4, ai->passwd, sizeof(ai->passwd), flag)) != 0) {
		if (r == 1) {
			strncpy(ai->login, "AutoPPP", sizeof(ai->login)-1);
			ai->login [sizeof(ai->login)-1] = '\0';
			ai->passwd[0] = '\0';
			r = 0;
		}
	}
	crtrans(0, 1, 1);
	 
	return r;
}
#endif

/*
 * This part is for the automatic PPP detection.
 * get_line() can check for it.
 */
#define PPP1 "\x7e\xff\x03\xc0\x21"
#define PPP2 "\x7e\xff\x7d\x23\xc0\x21"
#define PPP3 "\x7e\x7d\xdf\x7d\x23\xc0\x21"
static int pppseen(int ch)
{
		static char buf[16];

		if (ch == -1) {
			  memset(buf, 0, sizeof(buf));
			  return 0;
		}

		memmove(buf, buf + 1, 15);
		buf[14] = ch;
		if (strcmp(buf + 15 - 5, PPP1) == 0 ||
			  strcmp(buf + 15 - 6, PPP2) == 0 ||
			  strcmp(buf + 15 - 7, PPP3) == 0)
			return 1;

		return 0;
}

/*
 * Read and echo a line.
 *
 * Returns -1 on EOF, 0 on success and 1 on PPP detect.
 */
static int get_line(int ppp, const char *prompt, int echo, char *buffer, int size, int flag_telnet)
{
	int pos = 0;
	unsigned char c;
	int echonl = 1;
	static int pppinit = 0;
	static int crseen = 0;
	int len, rtn = 0;
	struct sigaction sa = {{0}};

	if (ppp && !pppinit) pppseen(-1);

	if (prompt && prompt[0]) {
		len = strlen(prompt);
		if (write(STDIN_FILENO, prompt, len) < len)
			goto ret_err;
		if (prompt[len - 1] != ' ')
			if (write(STDIN_FILENO, " ", 1) < 1)
				goto ret_err;
	}
	buffer[0] = 0;

	if (echo&6) {
		got_alrm = 0;
		sa.sa_handler = alrm_handler;
		sigaction(SIGALRM,  &sa, NULL);
		unblock(SIGALRM);
	}

	while(TRUE) {

		if (echo&8) alarm(60);

		if (read(STDIN_FILENO, &c, 1) != 1) {
			rtn = -1;
			if (got_alrm) {
				rtn = -2;
				write(STDOUT_FILENO, "\r\n", 2);
			}
			goto ret_back;
		}

		if (echo&2) echo |= 8;

		if (flag_telnet && checkIAC) {
			if (!(*checkIAC)(c)) {
				continue;
			}
		}

		if (ppp && pppseen(c)) {
			buffer[0] = 0;
			rtn = 1;
			goto ret_back;
		}

		if (c == '\n' && crseen) {
			crseen = 0;
			continue;
		}
		crseen = 0;

		switch(c) {
			case '\n':
			case '\r':
				if (c == '\r') crseen = 1;
				buffer[pos] = 0;
				if (echonl)
					if (write(STDOUT_FILENO, "\r\n", 2) < 2)
						goto ret_err;
				rtn = 0;
				goto ret_back;
			case 4: /* Control-D */
				if (pos == 0) {
					if (echonl) write(STDOUT_FILENO, "\r\n", 2);
					goto ret_err;
				}
				break;
			case 5: /* Control-E */
				echo ^= ~1;
				echonl = 0;
				break;
			case 127:
			case 8: /* Backspace. */
				if (pos == 0) {
					if (write(STDOUT_FILENO, "\007", 1) < 1)
						goto ret_err;
					break;
				}
				if (write(STDOUT_FILENO, "\010 \010", 3) < 3)
					goto ret_err;
				pos--;
				break;
			default:
				if (c < Fascii || c > Lascii || pos >= size - 1) {
					if (write(STDOUT_FILENO, "\007", 1) < 1)
						goto ret_err;
					break;
				}
				if (echo&1) {
					 if (write(STDOUT_FILENO, &c, 1) < 1)
						goto ret_err;
				} else {
					 if (write(STDOUT_FILENO, "*", 1) < 1)
						goto ret_err;
				}
				buffer[pos++] = c;
				break;
		}
	}
ret_err:
	rtn = -1;
ret_back:
	alarm(0);
	if (echo&6) {
		signal(SIGALRM, SIG_DFL); 
	}
	buffer[pos] = 0;
	return(rtn);
}

#ifndef USE_PAM
void do_next_token(struct auth *ai, int flag)
{
	crtrans(0, 0, 1);
	get_line(0, "Next Token: ", 4, ai->passwd, sizeof(ai->passwd), flag);
	crtrans(0, 1, 1);
}
#endif

