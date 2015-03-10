/*
 * main.c	Main part of the portslave.
 *
 * Version:	@(#)main.c  1.35  08-Nov-1997  MvS.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <signal.h>
#include <ctype.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <utmp.h>	//[RK]Dec/13/02 Bug#129

#include "radius.h"

#include "server.h"

//function prototypes
extern int do_login(struct auth *ai);
extern int clear_utmp_entry(pid_t pid);

struct auth ai;
struct auth *pai = &ai;

//[RK]May/10/06 - event notification
extern void writeevt(int, char *, ...);
int type = 4; //session type TS 

static void close_all(void)
{

#ifdef USE_PAM
	do_pam_close();
#endif

	//[RK]May/10/06 - event notification
	if (ai.authenticated) // authenticated 
		writeevt(6,"ssii",ai.login, lineconf[ai.nasport].alias, (ai.nasport + 1), type);

	close(1);
	close(2);
	serial_close(0);

	//[RK]Dec/13/02 Bug#129 clear utmp entry
	clear_utmp_entry(getpid());

	sleep(1);
}

/*
 *	Main loop for 1 login channel.
 */
int main(int argc, char **argv)
{
	char *tty = NULL;
	int i, r = 0;
	int thisport = -1;
	int printed = 0;
	int is_ppp = 0;
	char ipbuf[INET6_ADDRSTRLEN];

	/*
	 *    Check the syntax.
	 */
	if (argc < 2 || (argv[1][0] != '-'  && argv[1][0] != '+' &&
		  (thisport = atoi(argv[1])) == 0 && argv[1][0] != '0')) {
		    nsyslog(LOG_ERR, "Usage: portslave <port>\n");
		    fprintf(stderr,	"Portslave RADIUS client v%s\tGPL2\n"
				"(c) 1997-1999 Miquel van Smoorenburg, "
				"Dave Cinege, Vesselin Atanasov, et al.\n\n"
				"Usage: portslave <port>\n", PSVERSION);
	exit(1);
	}

	if (argv[1][0] == '-' || argv[1][0] == '+') {
		    thisport = -2;
		    tty = ttyname(0);
		    if (tty == NULL) {
		            fprintf(stderr,
		      	      "portslave: standard input is not a tty\n");
		            exit(1);
		    }
	}

	if (argc >= 3 && argv[2] && argv[1][0] == '+') {
		    strncpy(ai.login, argv[2], sizeof(ai.login));
		    ai.login[sizeof(ai.login)-1] = '\0';
	}

	if (argc >= 3 && argv[2] && argv[1][0] != '+') {
		set_xprintf(atoi(argv[2]));
	}

	/*
	 *    Read the config file.
	 */
#ifdef IPv6enable
	if ((thisport = rad_init(thisport, &ai, tty, 0, 0)) < 0)
#else
	if ((thisport = rad_init(thisport, &ai, tty, 0)) < 0)
#endif
		    return 1;

	if (lineconf[thisport].tty == NULL) {
#ifdef CYCLADES
		    nsyslog(LOG_ERR, "port %d has no tty defined", thisport+1);
#else
		    nsyslog(LOG_ERR, "port %d has no tty defined", thisport);
#endif
		    exit(1);
	}

#ifdef CYCLADES
	nsyslog(LOG_INFO, "portslave started on port %d (%s)", thisport+1,
		            lineconf[thisport].tty);
#else
	nsyslog(LOG_INFO, "portslave started on port %d (%s)", thisport,
		            lineconf[thisport].tty);
#endif

	signal(SIGHUP, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
	unblock(SIGHUP);
	unblock(SIGTERM);
	unblock(SIGINT);
	unblock(SIGQUIT);

	/*
	 *    Now put a getty on the port.
	 */
#ifndef CYCLADES
	if (lineconf[thisport].emumodem)
		    r = emumodem(&ai);
	else
#endif
		    r = getty(&ai, 0);

	if (r < 0) {
		exit(1);
	}

	atexit(close_all);

	//[RK]Dec/13/02 Bug#129
	if ((lineconf[thisport].protocol == P_PPP_ONLY) ||
		(lineconf[thisport].protocol == P_SLIP) ||
		(lineconf[thisport].protocol == P_CSLIP) ||
		(lineconf[thisport].protocol == P_PPP)) {
#ifdef IPv6enable
		// Support IPv6 addr per port 
		if (lineconf[ai.nasport].ipno6.ss_family) {
			memcpy(&ai.address, &lineconf[ai.nasport].ipno6, sizeof(ai.address));
		} else
#endif
		{
			((struct sockaddr_in *)&ai.address)->sin_family = AF_INET;
			((struct sockaddr_in *)&ai.address)->sin_addr.s_addr = lineconf[ai.nasport].ipno;
		}
		type = 5; // session type is dial-in [RK]May/10/06 event notification
	} else {
#ifdef IPv6enable
		//ai.address = lineconf[ai.nasport].host; IPv6 support
		memcpy(&ai.address, &lineconf[ai.nasport].host, sizeof(ai.address));
#else
			((struct sockaddr_in *)&ai.address)->sin_family = AF_INET;
			((struct sockaddr_in *)&ai.address)->sin_addr.s_addr = lineconf[ai.nasport].host;
#endif
	}
	update_utmp("%L", lineconf [ai.nasport].utmp_from, &ai, lineconf[ai.nasport].syswtmp);

	if (lineconf[thisport].protocol == P_PPP_ONLY) {
		goto ppp_only;
	}

//  SOCKET_CLIENT: test it as TELNET or RLOGIN
	nsyslog(LOG_INFO, "Connected - waiting for login");

	/*
	 *	Do login/password if needed.
	 */
//  SOCKET_CLIENT: test it as TELNET or RLOGIN

	if ((lineconf[thisport].protocol == P_LOCAL ||
		 lineconf[thisport].protocol == P_RLOGIN ||
		 lineconf[thisport].protocol == P_TELNET ||
		 lineconf[thisport].protocol == P_SSH ||
		 lineconf[thisport].protocol == P_SSH2) &&
		lineconf[thisport].userauto &&
		*lineconf[thisport].userauto &&
		lineconf[thisport].authtype == AUTH_NONE) {
		strcpy(ai.login, lineconf[thisport].userauto);
		ai.authenticated = 1;
		goto ppp_only;
	}

	if (lineconf[thisport].authtype) {
		//is_ppp = ai.proto == P_PPP;
		for(i = 0; i < 3; i++) {
#ifndef USE_PAM
			int r = 0;
			
			r = do_login(&ai);
			
			if (r == 2)	continue;

			if (r < 0) {
				XPRINTF("Do_login error : %d\r\n", r);
				sleep(3);
				exit(1);
			}
			/*
			 *	Do authentication if needed.
			 */
			if (mainconf.locallogins && ai.login[0] == '!') {
				memmove (ai.login,ai.login+1,strlen (ai.login));
				ai.passwd[0] = '\0';
				ai.proto = P_LOCAL;
				break;
			}
			if (strcmp(ai.login, "AutoPPP") == noerror) {
				ai.proto = P_AUTOPPP;
				break;
			}
#endif
			if (!do_local_or_server_authentication(&ai, is_ppp)) {
				break;
			}

			//[RK]May/10/06 - event notification
			writeevt(7,"ssii",ai.login, lineconf[ai.nasport].alias, (ai.nasport + 1), type);
		}

		if (i == 3) {
			/*
			 *	3 login failures - exit!
			 */

			//[RK]May/10/06 - event notification
			type = -1;	// not authenticated

			fflush(stdout);
			usleep(250000);
			maketermsane(0);
			exit(1);
		}

#ifndef USE_PAM
		if (ai.next_token) {
			int r;
			if (ai.message[0][0]) {
				for (r = 0; r < ai.msn; r++) {
					write(STDOUT_FILENO, ai.message[r], strlen(ai.message[r]));
					ai.message[r][0] = 0;
				}
				write(STDOUT_FILENO, "\r\n", 2);
				ai.msn = 0;
			} else {
				write(STDOUT_FILENO, "\r\nWait for the next token !!! \r\n\r\n", 34);
			}
			do_next_token(&ai, 0);
			if (do_local_or_server_authentication(&ai, is_ppp|2)) {
				exit(1);
			}
		}
#endif
		setenv("LOGNAME", ai.login, 1);

#ifndef USE_PAM
		if (ai.do_acct) {
			/*
			 *	While messing around with accounting, we have to
			 *	block, but not ignore, SIGHUP and friends.
			 */
			block(SIGHUP);
			block(SIGTERM);

			rad_acct(&ai, 1);

			unblock(SIGHUP);
			unblock(SIGTERM);

#if 0 /* XXX - DEBUG */
			printf("Protocol:     %c\n", ai.proto);
			printf("address    :  %s\n", dotted(ai.address));
			printf("netmask:      %s\n", dotted(ai.netmask));
			printf("mtu:          %d\n", ai.mtu);
#endif
		}
#endif
	}
	/*
	 *	Okay, we are logged in here (or authentication comes later).
	 */
//  SOCKET_CLIENT: test it as TELNET or RLOGIN
	if (ai.message[0][0]) {
		int x;
		for (x = 0; x < ai.msn; x++)
			printf("%s\r\n", ai.message[x]);

		printed = 1;
	}

ppp_only:

	switch(ai.proto) {
		case P_AUTOPPP:
			title(0, argv, "-AUTO %d", thisport);
			nsyslog(LOG_INFO, "PPP frames detected - switching to PPP mode");
			break;
		case P_SLIP:
			title(0, argv, "-SLIP %d", thisport);
			nsyslog(LOG_INFO, "%s/SLIP session (%s)", ai.login,
				dotted6((struct sockaddr *)&ai.address, ipbuf, sizeof(ipbuf)));
			break;
		case P_CSLIP:
			title(0, argv, "-CSLIP %d", thisport);
			nsyslog(LOG_INFO, "%s/CSLIP session (%s)", ai.login,
				dotted6((struct sockaddr *)&ai.address, ipbuf, sizeof(ipbuf)));
			break;
		case P_PPP:
		case P_PPP_ONLY:
			title(0, argv, "-PPP %d", thisport);
			if (ai.login && strcmp(ai.login, "NONE")) {
				nsyslog(LOG_INFO, "%s/PPP session (%s)", ai.login,
					dotted6((struct sockaddr *)&ai.address, ipbuf, sizeof(ipbuf)));
			} else {
				nsyslog(LOG_INFO, "PPP session (%s)",
					dotted6((struct sockaddr *)&ai.address, ipbuf, sizeof(ipbuf)));
			}
			break;
		case P_SSH:
			title(0, argv, "-ssh %d", thisport);
			nsyslog(LOG_INFO, "%s/ssh to %s", ai.login,
# ifdef CYCLADES
				dotted6((struct sockaddr *)&ai.host, ipbuf, sizeof(ipbuf)));
# else
				dotted6((struct sockaddr *)&ai.address, ipbuf, sizeof(ipbuf)));
# endif
			break;
		case P_SSH2:
			title(0, argv, "-ssh2 %d", thisport);
			nsyslog(LOG_INFO, "%s/ssh2 to %s", ai.login,
# ifdef CYCLADES
				dotted6((struct sockaddr *)&ai.host, ipbuf, sizeof(ipbuf)));
# else
				dotted6((struct sockaddr *)&ai.address, ipbuf, sizeof(ipbuf)));
# endif
			break;
		case P_TELNET:
			title(0, argv, "-telnet %d", thisport);
			nsyslog(LOG_INFO, "%s/telnet to %s", ai.login,
# ifdef CYCLADES
				dotted6((struct sockaddr *)&ai.host, ipbuf, sizeof(ipbuf)));
# else
				dotted6((struct sockaddr *)&ai.address, ipbuf, sizeof(ipbuf)));
# endif
			break;
		case P_RLOGIN:
			title(0, argv, "-rlogin %d", thisport);
			nsyslog(LOG_INFO, "%s/rlogin to %s", ai.login,
# ifdef CYCLADES
				dotted6((struct sockaddr *)&ai.host, ipbuf, sizeof(ipbuf)));
# else
				dotted6((struct sockaddr *)&ai.address, ipbuf, sizeof(ipbuf)));
# endif
			break;
		default:
			nsyslog(LOG_INFO, "%s/type %c starting", ai.login, ai.proto);
			break;
	}


	/*
	 *	Print a portmaster compatible SLIP/PPP banner.
	 */
	if (strchr("PCS", ai.proto)) {
		printf("%s session from (%s) ",
			ai.proto == 'P' ? "PPP" : "SLIP", dotted(mainconf.ipno));
		printf("to %s beginning....\n", dotted6((struct sockaddr *)&ai.address, ipbuf, sizeof(ipbuf)));
		printed = 1;
	}

	/*
	 *	Wait for output to drain.
	 */
	fflush(stdout);
	if (printed) {
		alarm(3);
		tcdrain(1);
		alarm(0);
	}

	//[RK]Dec/13/02 Bug#129 update after authentication
	//update_utmp("%L", lineconf [ai.nasport].utmp_from, &ai, lineconf[ai.nasport].syswtmp);
	
	//[RK]May/10/06 - event notification
	if (ai.authenticated) {
		writeevt(5,"ssii",ai.login, lineconf[ai.nasport].alias, (ai.nasport + 1), type);
		cy_shm->tty_user[ai.nasport] = shm_strdup(ai.login);
	}

	/*
	 *	Start the protocol.
	 */
	r = spawnit(&ai);

	/*
	 *	If the protocol returned, tell the accounting server we're done.
	 */
#ifdef USE_PAM
	do_pam_close();
#else
	if (ai.do_acct) {
		rad_acct(&ai, 0);
		ai.do_acct = 0;
	}
#endif

    if (ai.proto != P_PPP &&
		ai.proto != P_PPP_ONLY)
		nsyslog(LOG_INFO, "Session done.");

	return (r == 0 ? 0 : 1);
}
