#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <termios.h>
#include <termio.h>

#include "server.h"

#define CHAR_ESC            27
#define CHAR_CTRL_C         3
#define CHAR_CR             '\r'
#define CHAR_BS             '\b'

extern struct auth *pai;
extern struct line_cfg *pconf;

int (*read_char_method)(int nonblock_flag);
void (*write_line_method)(char *line, int cnt);
int (*main_routine_sess)(struct auth *p);

#ifdef _CONTROL_
static void control(CY_SESS *sess, char *str);
static char menu_esc[] = " (type '^Z' to go back to menu) ...\r\n";
#else
#define control(x,y)  if (pai->proto != P_SOCKET_SERVER_RAW)\
                       write_line_method("\r\n", 0);
#endif

static char sess_buff[512];

//static struct MsgBuf msg_buf;

//[RK]Oct/17/02 BUG#36
static int got_int_flag = 0;
static void got_int(int sig)
{
	got_int_flag = sig;
}

void kill_sniff_session(int sess, int flag, int clean)
{
	CY_SESS *sniff;

	cy_shm->sess_list[pai->nasport]->sess_flag |= flag;

	if (cy_shm->sess_list[pai->nasport]->next) { //there is a sniff session
		sniff = cy_shm->sess_list[pai->nasport]->next;
		sniff->sess_flag |= flag;
		kill_session(sniff, flag);
		if (clean) {
			clean_session_entry (sniff);
		}
	}

	if (sess) {
		cy_shm->sess_list[pai->nasport]->sess_flag &= CY_SESS_FLAG_MASK;
		return;
	}

	//kill the main session
	if (cy_shm->sess_list[pai->nasport]->vet_pid) {
		kill_session(cy_shm->sess_list[pai->nasport], flag);
	}
}

CY_SESS *open_sniff_session(int location)
{
	CY_SESS *sess = NULL;

	if (location != 2)
		title(2, NULL, "-Sniff_srv %s", get_tty_text(pconf));

	//update_utmp("%L", pconf->utmp_from, pai, pconf->syswtmp);

	if (!pai->sess) {
		pai->sess = sess = alloc_session_entry (pai->nasport, CY_SESS_FLAG_R, pai->login);

		if (!sess) {
			nsyslog(LOG_WARNING,"Could not open a sniff session.");
			return(NULL);
		}
	} else {
		sess = pai->sess;
	}

	if (cy_shm->line_conf[sess->port].multiple_sessions == MULTIPLE_RW_AND_SNIFF_SESSIONS) { 
		/* This user can see the menu */
		sess->sess_flag |= CY_SESS_FLAG_MENU;
	}

	//mp[Sep2003] update multiuser notification status
	if (cy_shm->line_conf[sess->port].multiuser_notif && sess->prev) {
		CY_SESS *saux = sess;
		shm_signal(CY_SEM_SHM);
		while (saux) {
			saux->sess_flag &= ~CY_SESS_FLAG_MUNTF_M;
			saux->sess_flag |= CY_SESS_FLAG_MUNTF_IN;
			saux = saux->prev;
		}
		shm_unsignal(CY_SEM_SHM);
	}
	//mp[Nov2003] set data buffer user logs control
	if (cy_shm->line_conf[sess->port].DB_user_logs) {
		shm_signal(CY_SEM_SHM);
		sess->sess_flag |= CY_SESS_FLAG_DB_USRLOG;
		shm_unsignal(CY_SEM_SHM);
	}

	sess->time = time(0); //[RK]Nov/26/02 - W-CAS

	pai->st_sess = 2; // read/only session;

	control(sess, "\r\nSniff session started");
	return(sess);
}

int read_sniff_msg(CY_SESS *sess)
{
	int count = 0;
	char temp[BufferSize + 1];

	if (sess->sess_flag & 4) { //[RK] Aug/07/03 bug #629
		return 0;	// in Sniff Menu mode
	}

	shm_signal(sess->port);  /* semaphore number: 0 = first port */

	while (!IsBufferEmpty(&sess->OutBuffer)) {
        temp[count++] = GetFromBuffer(&sess->OutBuffer);
	}
	temp[count] = 0;

	shm_unsignal(sess->port);  /* semaphore number: 0 = first port */

	//[MP]June/26/03 - Performance Improvement (as for TS...)
	if (sess->flow_flag && !IsBufferFull(&sess->OutBuffer, BufferSize/20)) {
		sess->flow_flag = 2;
	}

	if (count)
		write_line_method(temp, count);
	return(count);
}

static int do_socket_sniff_session(CY_SESS *sess)
{
	fd_set rfds, efds;
	struct timeval tv;
	int cnt;
//[RK]Oct/25/02 BUG #43	char snif_esc = (pconf->escape_char ? pconf->escape_char : SNIF_ESCAPE);
	struct sigaction sa={{0}};

	sa.sa_handler = got_int;

	sigaction(SIGIO,  &sa, NULL);	//[RK]Oct/17/02 BUG#36
	sigaction(SIGPIPE,  &sa, NULL);  //[RK] socket session is stopped (receive RST) 

	while (1) {
		if (got_signal_int == 1) {
			break;
		}

		read_sniff_msg(sess);

		FD_ZERO(&rfds);
		FD_ZERO(&efds);
		FD_SET(STDIN_FILENO,&rfds);
		FD_SET(STDIN_FILENO,&efds);

		tv.tv_usec = 100*1000;
		tv.tv_sec = 0;

		if ((cnt=select(STDIN_FILENO+1,&rfds,NULL,&efds,&tv)) < 0) {
			//[RK]Oct/17/02 BUG#36
			if (got_int_flag == SIGIO) {
				got_int_flag = 0;
				continue;
			}
			break;
		}

		if (cnt == 0) {
			continue;
		}

		if (FD_ISSET(STDIN_FILENO,&rfds)) {
			if ((cnt = read_char_method(0)) == -1) {
				break;
			}
			if ((cnt == pconf->escape_char) &&
			    (sess->sess_flag & CY_SESS_FLAG_MENU)) {
				sess->sess_flag |= 4;
				return(0);
			}
		}

		if (FD_ISSET(STDIN_FILENO,&efds)) {
			//XPRINTF("Connection closed: %s\r\n", strerror(errno));
			break;
		}
	}
	return(1);
}

void close_sniff_session(CY_SESS *sess)
{
	if (sess->sess_flag == 1) {
		write_line_method("\r\nSniff session killed by the administrator...\r\n"
						, 0);
	}

	if (sess->sess_flag == 2) {
		write_line_method("\r\nMain session has been finished...\r\n", 0);
	}

	pai->st_sess = 0;
//	sess->vet_pid = 0;
}

static char menu_snif[] =
	"\r\n1 - Assume the main session"
	"\r\n2 - Initiate a sniff session"
	"\r\n3 - Quit"
	"\r\n\nEnter your option : ";

static char menu_mult_sess[] =
	"\r\n1 - Initiate a regular session"
	"\r\n2 - Initiate a sniff session"
	"\r\n3 - Send messages to another user"
	"\r\n4 - Kill session(s)"
	"\r\n5 - Quit"
	"\r\n\nEnter your option : ";

int check_sniff_options(char opt, CY_SESS *sess)
{
	CY_SESS *first_rw;
	char *p;
	int pid, ch;
//[RK]Oct/25/02 BUG#43	char snif_esc = (pconf->escape_char ? pconf->escape_char : SNIF_ESCAPE);

	if (!sess) {
		return(1);
	}

	if (sess->sess_flag & 4) {
		if (pconf->multiple_sessions != NO_MULTIPLE_SESSIONS) {
			switch (opt) {
			case '1':
				//Initiate a regular session
				if (pai->sess && pai->st_sess == 2) {
					clean_session_entry (pai->sess);
					if (count_sessions (pai->nasport)) {
						sess->sess_flag |= CY_SESS_FLAG_MENU;
					}
				} 
				if (pai->sess && pai->st_sess == 1) {
					control(sess, "\r\nRW session re-started");
				}
				sess->sess_flag &= CY_SESS_FLAG_MASK;
				sess->sess_flag |= CY_SESS_FLAG_RW;
				return(1);
			case '2':
				//Initiate a sniff session
				sess->sess_flag = CY_SESS_FLAG_R;
				if (!(first_rw = first_rw_session(pai->nasport))) {
					write_line_method("\r\nMain session has finished...\r\n", 0);
					got_signal_int = 3;
					break;
				}
				pai->st_sess = 2;
				control(sess, "\r\nSniff session re-started");
				break;
			case '3':
				//Send messages to another user
				write_line_method("\r\n\r\nEnter message and type <Enter>: ", 0);
				memset (sess_buff, 0, sizeof(sess_buff));
				ch = snprintf(sess_buff, sizeof(sess_buff),
					"\r\n[Message from %s / %d] <<", 
					pai->login ? pai->login : "none", getpid());
				ch+= get_param_method("%s", 7, (int *)&sess_buff[ch]);
				ch+= snprintf(&sess_buff[ch], (sizeof(sess_buff) - ch),">>");
				send_msg_rw_sess(sess_buff, ch, 3, pai->nasport);
				write_line_method("\r\n\r\nMessage sent.\r\n", 0);
				control(sess, "\r\nSession re-started");
				sess->sess_flag &= CY_SESS_FLAG_MASK;
				break;
			case '4':
				//Kill session(s)
				if (!count_sessions (pai->nasport)) {
					write_line_method("\r\n\r\nNo session to kill.\r\n", 0);
					break;
				}
				p = sock_show_sess(pai->nasport);
				write_line_method(p, 0);
				switch (get_param_method("%d", 3, &pid)) {
					case 0:
					//Kill all sessions
					kill_session_list (pai->nasport, 1);
					write_line_method("\r\n\r\nAll sessions killed.\r\n", 0);
					break;
					case 1:
					kill_session_pid (pai->nasport, pid);
					break;
					default:
					write_line_method("\r\n\r\nNo session killed.\r\n", 0);
					break;
				}
				//[RK]Oct/17/02 BUG#36
				if (!(first_rw = first_rw_session(pai->nasport))) {
					// R/W session was killed -> change this to R/W
					clean_session_entry (pai->sess);
					sess->sess_flag &= CY_SESS_FLAG_MASK;
					if (count_sessions (pai->nasport)) {
						sess->sess_flag |= CY_SESS_FLAG_MENU;
					} else {
						sess->sess_flag &= ~CY_SESS_FLAG_MENU;
					}
					sess->sess_flag |= CY_SESS_FLAG_RW;
					//[RK]Oct/31/02 - to inform the new state to the user
					write_line_method("\r\n\r\nAssumed the main session.\r\n", 0);
					return(1);
				}
				control(sess, "\r\nSession re-started");
				sess->sess_flag &= CY_SESS_FLAG_MASK;
				break;
			case '5':
				got_signal_int = 3;
				sess->sess_flag &= CY_SESS_FLAG_MASK;
				break;
			default:
				break;
			}
		} else {
			switch (opt) {
			case '1':
				close_sniff_session(sess);
				kill_sniff_session(0, 1, 0);
				write_line_method("\r\nSession killed\r\n", 0);
				clean_session_entry (sess);
				pai->st_sess = 2;
				//sess->sess_flag &= CY_SESS_FLAG_MASK;
				sess->sess_flag = CY_SESS_FLAG_RW;
				return(1);
				break;
			case '2':
				//sess->sess_flag &= CY_SESS_FLAG_MASK;
				sess->sess_flag = CY_SESS_FLAG_R;
				control(sess, "\r\nSniff session re-started");
				break;
			case '3':
				got_signal_int = 3;
				sess->sess_flag &= CY_SESS_FLAG_MASK;
				break;
			default:
				break;
			}
		}
	} else {
		if (opt == pconf->escape_char) {	//[RK]Oct/25/02 BUG#43
			if (!count_sessions (pai->nasport) ||
			    (pconf->authtype == AUTH_NONE && (!pconf->admin_users || !*pconf->admin_users)) ||
			    (pconf->authtype != AUTH_NONE && !pai->flag_admin_user)) {
				sess->sess_flag &= ~CY_SESS_FLAG_MENU;
				return (0);
			}
			sess->sess_flag |= 4;
			if (pconf->multiple_sessions != NO_MULTIPLE_SESSIONS) {
				write_line_method(menu_mult_sess, 0);
			} else {
				write_line_method(menu_snif, 0);
			}
		}
	}
	return (0);
}

int check_mult_session(int location, char *msg,	int rmethod(int nonblock_flag), 
			void wmethod(char *line, int cnt), int main_routine(struct auth *)) 
{
	int ch = 1, pid, main_res;
	int flg_show_menu = 1;
	CY_SESS *sniff_sess, *first_rw;
	char *p;

	write_line_method = wmethod;
	read_char_method = rmethod;

	if ((pconf->sniff_mode == 3) &&
		(pconf->multiple_sessions == NO_MULTIPLE_SESSIONS)) { // NO sniff
		if (msg) { // second conection
			write_line_method(msg, 0);
       		return(1);
		}
		open_rw_session(location, rmethod, wmethod, main_routine);
		return(0);
	}

	if (msg) {
		if ((pconf->authtype == AUTH_NONE &&
		    (!pconf->admin_users || !*pconf->admin_users)) ||
		    (pconf->authtype != AUTH_NONE &&
		     !pai->flag_admin_user)) { //[RK]May/31/05 - Group Authorization Support
			write_line_method(msg, 0);
			return(1);
		}
		switch (pconf->multiple_sessions) {
		case SNIFF_SESSIONS_ONLY:
			goto do_mult_sniff_sess;
		case RW_SESSIONS_ONLY:
			goto do_mult_rw_sess;
		case MULTIPLE_RW_AND_SNIFF_SESSIONS:
		case NO_MULTIPLE_SESSIONS:
		default:
			break;
		}
		write_line_method(msg, 0);
	} else {
		if (pconf->multiple_sessions != NO_MULTIPLE_SESSIONS) {
			goto do_mult_rw_sess;
		}
		open_rw_session(location, rmethod, wmethod, main_routine);
		return(0);
	}

	do {
		if (pconf->multiple_sessions != NO_MULTIPLE_SESSIONS) {
			if (location == 2 && pai->st_sess == 2) {
				//SSH - changing from sniff to RW session
				return(3);
			}
			if (flg_show_menu) write_line_method(menu_mult_sess, 0);
			if (!pai->sess) {
				pai->sess = alloc_session_entry (pai->nasport, 4, pai->login);
				pai->st_sess = 0;
			}
			if (pai->sess) {
				read_sniff_msg(pai->sess);
			}
			switch ((ch = read_char_method(1))) {
			case '1':
				//Initiate a regular session
				if (pai->sess && pai->st_sess != 0 &&
					!(pai->sess->sess_flag & CY_SESS_FLAG_W)) {
					clean_session_entry (pai->sess);
				}
				if (pai->sess) {
					pai->sess->sess_flag = CY_SESS_FLAG_RW;
				}
				if (location == 2)	//SSH
					return(3);
do_mult_rw_sess:
				main_res = open_rw_session(location, rmethod, wmethod, main_routine);
				flg_show_menu = 1;
				if (main_res == 4) {
					if (!count_sessions (pai->nasport) ||
			    		    (pconf->authtype == AUTH_NONE && (!pconf->admin_users || !*pconf->admin_users)) ||
					    (pconf->authtype != AUTH_NONE && !pai->flag_admin_user)) {
						if (!pai->sess) {
							pai->sess = alloc_session_entry (pai->nasport, 4, pai->login);
							pai->st_sess = 0;
						}
						pai->sess->sess_flag = CY_SESS_FLAG_RW;
						goto do_mult_rw_sess;
					}
					ch = 0;
					break;
				} else {
					return(main_res);
				}
				break;
			case '2':
				//Initiate a sniff session
				if (pai->sess && (pai->sess->sess_flag & CY_SESS_FLAG_W)) {
					clean_session_entry (pai->sess);
				}
				if (pai->sess) {
					pai->sess->sess_flag = CY_SESS_FLAG_R;
					pai->st_sess = 2; // read/only session;
				}
				if (!(first_rw = first_rw_session(pai->nasport))) {
					write_line_method("\r\nMain session has finished...\r\n", 0);
					break;
				}
do_mult_sniff_sess:
				if (!(sniff_sess = open_sniff_session(location))) {
					break;
				}
				if (location == 2) {
					return(2);
				}
				ch = do_socket_sniff_session(sniff_sess);
				close_sniff_session(sniff_sess);
				flg_show_menu = 1;
				break;
			case '3':
				//Send messages to another user
				write_line_method("\r\n\r\nEnter message and type <Enter>: ", 0);
				memset (sess_buff, 0, sizeof(sess_buff));
				ch = snprintf(sess_buff, sizeof(sess_buff),
					"\r\n[Message from %s / %d] <<", 
					pai->login ? pai->login : "none", getpid());
				ch+= get_param_method("%s", 7, (int *)&sess_buff[ch]);
				ch+= snprintf(&sess_buff[ch], (sizeof(sess_buff) - ch),">>");
				send_msg_rw_sess(sess_buff, ch, 3, pai->nasport);
				write_line_method("\r\n\r\nMessage sent.\r\n", 0);
				ch = 0;
				flg_show_menu = 1;
				pai->sess->sess_flag &= CY_SESS_FLAG_MASK;
				pai->sess->sess_flag |= 4;
				break;
			case '4':
				//Kill session(s)
				if (!count_sessions (pai->nasport)) {
					write_line_method("\r\n\r\nNo session to kill.\r\n", 0);
					ch = 0;
					pai->sess->sess_flag &= CY_SESS_FLAG_MASK;
					pai->sess->sess_flag |= 4;
					break;
				}
				ch = 0;
				p = sock_show_sess(pai->nasport);
				write_line_method(p, 0);
				switch (get_param_method("%d", 3, &pid)) {
					case 0:
					//Kill all sessions
					kill_session_list (pai->nasport, 1);
					write_line_method("\r\n\r\nAll sessions killed.\r\n", 0);
					break;
					case 1:
					kill_session_pid (pai->nasport, pid);
					break;
					default:
					write_line_method("\r\n\r\nNo session killed.\r\n", 0);
					break;
				}
				//[RK]Oct/17/02 BUG#36
				if (!(first_rw = first_rw_session(pai->nasport))) {
					// R/W session was killed -> change this to R/W
					clean_session_entry (pai->sess);
					//[RK]Oct/31/02 - to inform the new state to the user
					write_line_method("\r\n\r\nAssumed the main session(open_rw_session).\r\n", 0);
					main_res = open_rw_session(location, rmethod, 
							wmethod, main_routine);
					return(main_res);
				}
				flg_show_menu = 1;
				pai->sess->sess_flag &= CY_SESS_FLAG_MASK;
				pai->sess->sess_flag |= 4;
				break;
			case '5':
				//Quit
				break;
			default:
				if (ch != -1) {
					flg_show_menu = 1;
				} else {
					flg_show_menu = 0;
				}
				ch = 0;
				pai->sess->sess_flag &= CY_SESS_FLAG_MASK;
				pai->sess->sess_flag |= 4;
				break;
			}
		} else {
			write_line_method(menu_snif, 0);
			switch ((ch = read_char_method(0))) {
			case '1':
				if (pai->sess) {
					clean_session_entry (pai->sess);
				}
				pai->sess = alloc_session_entry (pai->nasport, CY_SESS_FLAG_RW, pai->login);
				//kill the main session
				kill_sniff_session(0, 1, 0);
				write_line_method("\r\nSession killed\r\n", 0);
//[RK]Dec/05/02 Bug#76		cy_shm->sess_list[pai->nasport] = NULL;
				open_rw_session(location, rmethod, wmethod, main_routine);
				return(0);
				break;
			case '2':
				if (!(cy_shm->sess_list[pai->nasport]->vet_pid)) {
					write_line_method("\r\nMain session has finished...\r\n", 0);
					break;
				}
				if (!pai->sess) {
					kill_sniff_session(1, 1, 1);
				} else {
					clean_session_entry (pai->sess);
				}
				if (!(sniff_sess = open_sniff_session(location))) {
					break;
				}
				if (location == 2) {
					return(2);
				}
				ch = do_socket_sniff_session(sniff_sess);
				close_sniff_session(sniff_sess);
				break;
			case '3':
				break;
			default:
				if (ch != -1) {
					ch = 0;
				}
				break;
			}
		}

	} while (!ch);

	return(1);
}

void do_virtual_session(int location,
			int rmethod(int nonblock_flag), void wmethod(char *line, int cnt))
{
	fd_set rfds, efds;
	int cnt;
	char buf[256];
	struct sigaction sa={{0}};

	sa.sa_handler = got_int;

	sigaction(SIGIO,  &sa, NULL);	//[RK]Oct/17/02 BUG#36
	sigaction(SIGPIPE,  &sa, NULL);  //[RK] socket session is stopped (receive RST) 

	if ((DeviceFd = open_virtual_session(location, rmethod, wmethod)) < 0) {
		return;
	}

	update_utmp("%L", pconf->utmp_from, pai, pconf->syswtmp);

	while (1) {
		if (got_signal_int == 1) {
			break;
		}

		FD_ZERO(&rfds);
		FD_ZERO(&efds);
		FD_SET(STDIN_FILENO,&rfds);
		FD_SET(DeviceFd,&rfds);
		FD_SET(STDIN_FILENO,&efds);
		FD_SET(DeviceFd,&efds);

		if ((cnt=select(DeviceFd+1,&rfds,NULL,&efds,NULL)) < 0) {
			//[RK]Oct/17/02 BUG#36
			if (got_int_flag == SIGIO) {
				got_int_flag = 0;
				continue;
			}
			break;
		}

		if (cnt == 0) {
			continue;
		}

		if (FD_ISSET(STDIN_FILENO,&rfds)) {
			if ((cnt = read(STDIN_FILENO, buf, sizeof(buf))) <= 0) {
				break;
			}
			if (write(DeviceFd, buf, cnt) != cnt) {
				break;
			}
			//send_msg_rw_sess(buf, cnt, 1, pai->nasport);
		}

		if (FD_ISSET(DeviceFd,&rfds)) {
			if ((cnt = read(DeviceFd, buf, sizeof(buf))) <= 0) {
				break;
			}
			if (write(STDIN_FILENO, buf, cnt) != cnt) {
				break;
			}
			//send_msg_rw_sess(buf, cnt, 2, pai->nasport);
		}

		if (FD_ISSET(STDIN_FILENO,&efds)) {
			//XPRINTF("Connection closed: %s\r\n", strerror(errno));
			break;
		}
	}
}

int open_virtual_session(int location,
			int rmethod(int nonblock_flag), void wmethod(char *line, int cnt))
{
	int addr_len, client_fd, cnt, tot, fbin=0;
	char *p, *c, *b, buf[16];
	struct sockaddr_storage sock_client;
	unsigned int host;
	struct hostent *h;
	char tty[INET6_ADDRSTRLEN], ipaddr[INET6_ADDRSTRLEN], tcp_port[INET6_ADDRSTRLEN];

	write_line_method = wmethod;
	read_char_method = rmethod;

#if 0 //IPv6
	c = strdup(pconf->tty);

	if ((p = strchr(c, ':')) != NULL) {
		*p++ = 0;
		if ((b = strchr(p, ':')) != NULL) {
			*b++ = 0;
			if (*b == 'b' || *b =='B') {
				fbin = 1;
			}
		}
		sock_client.sin_port = atoi(p);
	} else {
		sock_client.sin_port = pconf->socket_port;
	}

	if ((int)(host = inet_addr(c)) == -1) {
		if ((h = gethostbyname(c)) == NULL) {
			return -1;
		}
		host = *(unsigned int *)h->h_addr_list[0];
	}

	free(c);

	sock_client.sin_family = AF_INET;
	sock_client.sin_addr.s_addr = host;
	addr_len = sizeof(sock_client);
	if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		nsyslog(LOG_ERR, "Open virtual connection (socket) error: %s",
			strerror(errno));
		return(-1);
	}
#endif //IPv6
	ras_parse_tty (pconf->tty, tty, ipaddr, tcp_port, INET6_ADDRSTRLEN);

	if (ipaddr[0] == NULL) {
		nsyslog(LOG_ERR, "Open virtual connection error: no IP to connect");
		return(-1);
	}
	if (asc_to_binary6(ipaddr, (struct sockaddr *)&sock_client)) {
		nsyslog(LOG_ERR, "Open virtual connection error: wrong IP %s", ipaddr);
		return(-1);
	}
	if (tcp_port[0] != NULL) {
		if(sock_client.ss_family == AF_INET6) {
			((struct sockaddr_in6 *)&sock_client)->sin6_port = atoi(tcp_port);
		} else {
			((struct sockaddr_in *)&sock_client)->sin_port = atoi(tcp_port);
		}
	} else {
		if(sock_client.ss_family == AF_INET6) {
			((struct sockaddr_in6 *)&sock_client)->sin6_port = pconf->socket_port;
		} else {
			((struct sockaddr_in *)&sock_client)->sin_port = pconf->socket_port;
		}
	}

	addr_len = sizeof(sock_client);
	if ((client_fd = socket(sock_client.ss_family, SOCK_STREAM, 0)) < 0) {
		nsyslog(LOG_ERR, "Open virtual connection (socket) error: %s",
			strerror(errno));
		return(-1);
	}
	
	if (connect(client_fd, (struct sockaddr *)&sock_client, addr_len) < 0) {
		nsyslog(LOG_ERR, "Open virtual connection (connect) error: %s",
			strerror(errno));
		return(-1);
	}

	// flush telnet negotiation if ssh
	if (location == 2 && !fbin) {
		tot = 0;
		while (tot < 15) {
			if ((cnt = read(client_fd, buf, 15)) <= 0) {
				nsyslog(LOG_ERR, "Open virtual connection (read) error: %s",
					strerror(errno));
				return(-1);
			}
			tot += cnt;
		}
	}

	return(client_fd);
}

int open_rw_session(int location, int rmethod(int nonblock_flag), 
			void wmethod(char *line, int cnt), int main_routine(struct auth *))
{
	int main_res;
	CY_SESS *sess = NULL, *saux = NULL;

	write_line_method = wmethod;
	read_char_method = rmethod;
	main_routine_sess = main_routine;

	if (location != 2)
		title(2, NULL, "-RW_srv %s", get_tty_text(pconf));

	//update_utmp("%L", pconf->utmp_from, pai, pconf->syswtmp);

	if (!pai->sess) {
		pai->sess = sess = alloc_session_entry (pai->nasport, CY_SESS_FLAG_RW, pai->login);

		if (!sess) {
			nsyslog(LOG_WARNING,"Could not open a read/write session.");
			return(1);
		}
	} else {
		sess = pai->sess;
	}

	if (pconf->virtual) {
		pai->st_sess = 3; // read/write session;
		if (location == 2)	//SSH
			return(3);
		do_virtual_session(3, rmethod, wmethod);
		return(0);
	}

	sess->time = time(0); //[RK]Nov/26/02 - W-CAS

	pai->st_sess = 1; // read/write session;
	//[RK]Oct/25/02 - main session doesn't have the sniff menu
	//[RK]Nov/25/05 - RW multiple session has menu
	// if (pconf->multiple_sessions != NO_MULTIPLE_SESSIONS &&
	saux = first_rw_session(pai->nasport);
	if (pconf->multiple_sessions == MULTIPLE_RW_AND_SNIFF_SESSIONS &&
		(sess != saux) &&
		!((pconf->authtype == AUTH_NONE &&
		(!pconf->admin_users || !*pconf->admin_users)) ||
		    (pconf->authtype != AUTH_NONE &&
		     !pai->flag_admin_user))) { //[RK]May/31/05 - Group Authorization Support
		/* This user can see the menu */
		sess->sess_flag |= CY_SESS_FLAG_MENU;
    }

	if (count_sessions (pai->nasport)) {
		// allow first session to see the menu
		saux->sess_flag |= CY_SESS_FLAG_MENU;
	}

	if (pconf->dcd) { // must check DCD
		cy_shm->flag_dcd[pai->nasport] = 1;
		while (cy_shm->flag_dcd[pai->nasport]) sleep(1);
		if (!cy_shm->dcd_status[pai->nasport]) { // DCD is OFF
			write_line_method("\r\nFailed to connect due to DCD OFF.", 0);
			nsyslog(LOG_WARNING,"Failed to connect due to DCD OFF.");
			return(0);
		}
	}

	//[RK]Nov/26/02 - W-CAS update_utmp("%L", lineconf [pai->nasport].utmp_from, pai,
	//          		lineconf[pai->nasport].syswtmp);
	control(sess, "\r\nRW session started");
	if (main_routine_sess) {

		//mp[Sep2003] update multiuser notification status
		if (cy_shm->line_conf[sess->port].multiuser_notif && sess->prev) {
			saux = sess;
			shm_signal(CY_SEM_SHM);
			while (saux) {
				saux->sess_flag &= ~CY_SESS_FLAG_MUNTF_M;
				saux->sess_flag |= CY_SESS_FLAG_MUNTF_IN;
				saux = saux->prev;
			}
			shm_unsignal(CY_SEM_SHM);
		}
		//mp[Nov2003] set data buffer user logs control
		if (cy_shm->line_conf[sess->port].DB_user_logs) {
			shm_signal(CY_SEM_SHM);
			sess->sess_flag |= CY_SESS_FLAG_DB_USRLOG;
			shm_unsignal(CY_SEM_SHM);
		}

		main_res = main_routine_sess(pai);
//[RK]Nov/26/02 - W-CAS		clear_utmp_entry(getpid()); //[FW]Oct/25/02 Bug #40. Handles
					    //telnet to local serial port.
		if (main_res) {
			return(4);
		}
	}
	return(3);
}

int count_sessions (int port_num)
{
	int sess_count = 0;
	CY_SESS *sess = cy_shm->sess_list[port_num];
	CY_SESS *sess_aux;
	
	while (sess) {
		if (sess->vet_pid) {
			if (kill(sess->vet_pid, 0) < 0 && errno == ESRCH) {
				//this process has been killed; clean this entry
				sess_aux = sess;
				sess = sess->next;
				clean_session_entry(sess_aux);
    		} else {
				if (sess->vet_pid != getpid()) {
					sess_count++;
				}
				sess = sess->next;
			}
		} else {
			//this process has been killed; clean this entry
			sess_aux = sess;
			sess = sess->next;
			clean_session_entry(sess_aux);
		}
	}
	return (sess_count);
}

CY_SESS *first_rw_session (int port_num)
{
	CY_SESS *sess = cy_shm->sess_list[port_num];
	
	while (sess) {
		if ((sess->sess_flag & CY_SESS_FLAG_W) &&
			sess->vet_pid ) {   //[RK]Nov/07/02 Bug #76
			return (sess);
		}
		sess = sess->next;
	}
	return NULL;
}

CY_SESS *first_sniff_session (int port_num)
{
	CY_SESS *sess = cy_shm->sess_list[port_num];
	
	while (sess) {
		if (!(sess->sess_flag & CY_SESS_FLAG_W) && 
			 (sess->sess_flag & CY_SESS_FLAG_R)) {
			return (sess);
		}
		sess = sess->next;
	}
	return NULL;
}

void kill_session(CY_SESS *sess, int flag)
{
	int sec = 0, sess_pid;	//[RK]Dec/05/02 Bug#76
	
	sess->sess_flag |= flag;

	sess_pid = sess->vet_pid; //[RK]Dec/05/02 Bug#76
	
	if (sess_pid && (sess_pid != getpid())) {
		if (kill(sess_pid, SIGINT)) {
			sess->vet_pid = 0;
		}
		while (sess->vet_pid && sec < 6) { 
			sec++;
			sleep(1);
		}
		if (sess->vet_pid == sess_pid) { //[RK]Dec/05/02 Bug#76
			nsyslog(LOG_WARNING,"KILL sess_pid2 = %d", sess->vet_pid);
			// Doesn't wait anymore
			kill(sess->vet_pid, SIGKILL);
			clean_session_entry (sess);
		}
	}

	return;
}

void clean_session_entry (CY_SESS *sess)
{
	free_session_entry(sess);

	if (sess == pai->sess) { //[RK]Nov/04/02 Bug#76	
		pai->sess = NULL;
	}
}

void kill_session_list (int port_num, int flag)
{
	CY_SESS *sess = cy_shm->sess_list[port_num];

	while (sess) {
		kill_session(sess, flag);
		sess = sess->next;
	}
	return;
}

void kill_session_pid (int port_num, int pid)
{
	CY_SESS *sess = cy_shm->sess_list[port_num];

	while (sess) {
		if (sess->vet_pid == pid) {
			kill_session(sess, 1);
			write_line_method("\r\n\r\nSession killed.\r\n", 0);
			return;
		}
		sess = sess->next;
	}
	write_line_method("\r\n\r\nNo session killed.\r\n", 0);
	return;
}

char *sock_show_sess(int ind)
{
	CY_SESS *sess = cy_shm->sess_list[ind];
	int cnt = 0, tot = 0, sess_index = 1;
	int my_pid;
	
	my_pid = getpid();

	while (sess) {
		if (sess->vet_pid && (sess->vet_pid != my_pid)) {
			cnt = snprintf(&sess_buff[tot], (sizeof(sess_buff) - tot - 1),
    	        "\r\n* Session %d:  Pid %d   User %s ", sess_index, 
				sess->vet_pid, sess->tty_user ? sess->tty_user : "none");
			tot += cnt;
		}
		sess = sess->next;
		sess_index++;
    }
    snprintf(&sess_buff[tot], sizeof(sess_buff) - tot, 
			"\r\nEnter session PID or 'all': ");
    return(sess_buff);
}

int get_param_method(char *f, int n, int *p)
{
	char aux[380]; //Must be smaller than sess_buff
	char *r = aux;
	char local[3];
	int	i, ret = 0, cnt = 0;

	memset (aux, 0, sizeof(aux));
	while((cnt < sizeof(aux)) && (*r = read_char_method(1)) != CHAR_CR) {
		if (*r != 0xff) {
			if (*r != CHAR_BS) {
				if (*r >= 32 && *r < 127) {
					write_line_method (r, 0);
					cnt++;
					r++;
				} else {
					nsyslog(LOG_INFO,"Could not print this char: 0x%x", *r);
				}
			} else {
				if (cnt) {
					write_line_method ("\b \b", 0);
					*r = 0;
					--cnt;
					--r;
				}
			}
		} else {
			if (pai->sess) {
				read_sniff_msg(pai->sess);
			}
		}
	}

	if (cnt == 0) {
		if (n !=3) {
			return(0);
		} else {
			p[0] = 0; //no PID was typed
			return(2);
		}
	}

	*r = 0;
	r = aux;
	
	if (n == 6) { // MAC address
		if (r[2] == ':') { // using format NN:NN:NN:NN:NN:NN
			ret = sscanf(r, f, &p[0], &p[1], &p[2], &p[3], &p[4], &p[5]);
		}
		else { // using format NNNNNNNNNNNN
			for (i = 0; i < n; i++) {
				local[0] = r[i*2];
				local[1] = r[i*2+1];
				local[2] = 0;
				ret = ret + sscanf(local, "%x", &p[i]);
			}
		}
	}
	if (n == 4) { // IP address
		ret = sscanf(r, f, &p[0], &p[1], &p[2], &p[3]);
	}
	if (n == 1) { // string or speed
		ret = sscanf(r, f, &p[0]);
	}
	if (n == 2) {
		strcpy((char*)&p[0], r);
		ret = n;
	}
	if (n == 3) { // PID
		if (strcmp("all", r) == 0) {
			//all pids
			p[0] = 0;
			return (0);
		}
		ret = sscanf(r, f, &p[0]);
		return (1);
	}
	if (n == 7) {
		strcpy((char *)&p[0], r);
		for (n = 0; n < sizeof(aux); n++) {
			if (aux[n] == 0) {
				return (n);	//size of the string
			}
		}
		return (n);
	}

	if (ret != n) {
		return(0);
	}

	return(ret);
}

#ifdef _CONTROL_
static void control(CY_SESS *sess, char *str)
{
	char *str_ch = &menu_esc[8];
	char ch = pconf->escape_char;

	if (ch) {
		if (ch == 0xff) {
			*str_ch++ = ' ';
			*str_ch = ' ';
		}
		if (ch >= 0x20) {
			*str_ch++ = ch;
			*str_ch = ' ';
		} else {
			*str_ch++ = '^';
			*str_ch = '@'+ch;
		}
	}
	write_line_method(str, 0);
	if (!(sess->sess_flag & CY_SESS_FLAG_W) || 
		((sess->sess_flag & CY_SESS_FLAG_MENU) && ch)) { 
		write_line_method(menu_esc, 0);
	} else {
		write_line_method(" ...\r\n", 0);
	}
	return;
}
#endif
