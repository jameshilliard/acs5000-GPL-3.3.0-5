/*
 * spawnit.c	Spawn the appropriate protocol.
 *
 * Version:	@(#)spawnit.c  1.30  24-Oct-1997  MvS.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <utmp.h>
#include <time.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <sys/wait.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pwd.h> 
#include <sys/types.h>
#include <sys/socket.h>

#include "server.h"

extern int traffic_stats(int sockfd, struct sockaddr *ina, int *in, int *out);

#ifndef LINUX22
bool got_usr1 = FALSE;
#endif

/*
 *	Get an utmp entry by process id.
 *	This should be a standard function IMHO.
 */
struct utmp *getutpid(struct utmp *in)
{
	struct utmp *u;

	while((u = getutent()) != NULL) {
		if (u->ut_type != INIT_PROCESS &&
				u->ut_type != LOGIN_PROCESS &&
				u->ut_type != USER_PROCESS) continue;
		if (u->ut_pid == in->ut_pid) break;
	}
	return u;
}

/*
 *	Relay the SIGHUP to our process group.
 */
static void got_hup(int sig)
{
	struct sigaction sa = {{0}};
	pid_t pgrp = getpgrp();

	signal(sig, SIG_IGN);
	unblock(sig);

	if (sig == SIGALRM)	alarm(0);

	if (pgrp > 0) {
		if (sig == SIGALRM) {
			kill(-pgrp, SIGTERM);
		} else {
			kill(-pgrp, sig);
		}
	}

	sa.sa_handler = got_hup;
	sa.sa_flags   = SA_RESTART;
	sigaction(sig, &sa, NULL);
}

/*
 *	Strip the realm/passthrough from the login name, if any.
 */
static void striprealm(struct auth *ai)
{
	struct realm_def *r;
	char *s;

	if ((r = ckrealm(ai)) == NULL || !r->strip)
		return;
	if (r->prefix)
		strcpy(ai->login, ai->login + strlen(r->name));
	else
		*(s = ai->login + strlen(ai->login) - strlen(r->name)) = '\0';
}

/*
 *	Execute but split command line args first.
 */
static int doexec(char *cmd, char *argv0, char *args)
{
	char *argv[32];
	int argc = 0;
	char *s;

	argv[argc++] = argv0;
	s = strtok(args, " \t\n");
	while(s && argc < 31) {
		argv[argc++] = s;
		s = strtok(NULL, " \t\n");
	}
	argv[argc] = NULL;
	return execv(cmd, argv);
}

/*
 *	Execute a rlogin session.
 */
static int do_rlogin(struct auth *ai)
{
	char ipbuf[INET6_ADDRSTRLEN];
#ifdef CYCLADES
	char *host = dotted6((struct sockaddr *)&ai->host, ipbuf, sizeof(ipbuf));
#else
	char *host = dotted6((struct sockaddr *)&ai->address, ipbuf, sizeof(ipbuf));
#endif

	maketermsane(0);
#ifndef CYCLADES
    if (ai->login && ai->login[0] && strcmp(ai->login, "NONE")) {
        execl(mainconf.rlogin, "rlogin", "-E", "-i",
            ai->login, "-l", ai->login, host, NULL);
    } else {
        execl(mainconf.rlogin, "rlogin", "-E", host, NULL);
    }
#else
    if (ai->login && ai->login[0] && strcmp(ai->login, "NONE")) {
        execl(mainconf.rlogin, "rlogin", "-i",
            ai->login, "-l", ai->login, host, NULL);
    } else {
        execl(mainconf.rlogin, "rlogin", host, NULL);
    }
#endif
	nsyslog(LOG_ERR, "%s: %m", mainconf.rlogin);
	return -1;
}

/*
 *	Execute a telnet session.
 */
static int do_telnet(struct auth *ai)
{
	int i = 0;
	char *args[10];
	char ipbuf[INET6_ADDRSTRLEN];

	args[i++] = "telnet";

#ifdef CYCLADES
	args[i++] = "-E"; //[RK]Nov/20/02 Bug#88 - doesn't display the menu
	//[RK]Nov/20/02 Bug#88 
	if (lineconf[ai->nasport].telnet_client_mode) {
		args[i++] = "-8";	// start binary mode
	}
    if (ai->login && ai->login[0] && strcmp(ai->login, "NONE")) {
		args[i++] = "-l";
		args[i++] = ai->login;
	}
	args[i++]= dotted6((struct sockaddr *)&ai->host, ipbuf, sizeof(ipbuf));
	if (ai->loginport) {
		args[i++] = num(ai->loginport);
	} else {
		if (lineconf[ai->nasport].socket_port) {
			args[i++] = num(lineconf[ai->nasport].socket_port);
		}
	}
	args[i] = NULL;
#else
	args[i++] = "-8";
	args[i++] = "-E";
	args[i++] = dotted6((struct sockaddr *)&ai->address, ipbuf, sizeof(ipbuf));
	args[i] = NULL;
#endif

	maketermsane(0);

	execv(mainconf.telnet, args);
	nsyslog(LOG_ERR, "%s: %m", mainconf.telnet);
	return -1;
}

/*
 *	Execute a secure shell session.
 */
static int do_ssh(struct auth *ai)
{
	static char tmp[20];
	int i = 0;
	char *args[10];
	char ipbuf[INET6_ADDRSTRLEN];

	args[i++] = "ssh";

#ifndef CYCLADES
	args[i++] = "-e";
	args[i++] = "none";
#endif

	if (ai->proto != P_SSH)
		args[i++] = "-2";

    if (ai->login && ai->login[0] && strcmp(ai->login, "NONE")) {
		args[i++] = "-l";
		args[i++] = ai->login;
	}

	args[i++] =
#ifdef CYCLADES
		dotted6((struct sockaddr *)&ai->host, ipbuf, sizeof(ipbuf));
#else
		dotted6((struct sockaddr *)&ai->address, ipbuf, sizeof(ipbuf));
#endif

#ifdef CYCLADES
	if (lineconf[ai->nasport].break_interval) {
		args[i] = tmp;
		sprintf(args[i],"-B %d",lineconf[ai->nasport].break_interval);
		i++;
	}
#endif
		
	args[i] = NULL;

	maketermsane(0);

	execv(mainconf.ssh, args);
	nsyslog(LOG_ERR, "%s: %m", mainconf.ssh);
	return -1;
}

/*
 *	Execute a PPP session.
 */
static int do_ppp(struct auth *ai)
{
	char *s, *x;
	char options [1024];

	if (ai->conn_info[0]) setenv("CONNECT_INFO", ai->conn_info, 1);

	if (ai->proto == P_AUTOPPP) {
		cy_shm->vet_ppp[ai->nasport] = 1;
		s = lineconf[ai->nasport].autoppp;
		x = lineconf[ai->nasport].pppopt;
	} else {
		s = lineconf[ai->nasport].pppopt;
		x = lineconf[ai->nasport].autoppp;
	}

	if (s == NULL && x != NULL) s = x;

	expand_format (options, sizeof(options), s, ai);
	doexec(mainconf.pppd, "pppd", options);
	nsyslog(LOG_ERR, "%s: %m", mainconf.pppd);
	return -1;
}

/*
 *	Execute a local login.
 */
static int do_local(struct auth *ai, char *id, pid_t parent)
{
	struct utmp ut, *u;
	char *tty, *full_tty;
	char lhost[UT_HOSTSIZE];
	char ipbuf[INET6_ADDRSTRLEN];

	/*
	 *	Prevent hacking.
	 */
	if (ai->login[0] == '-') return -1;

	/*
	 *	If we forked we have to pull some tricks to
	 *	become process group leader etc.
	 */
	if (parent) {
		setsid();
		ioctl(0, TIOCSCTTY, (char *)1);
	}

	/*
	 *	First fill out an utmp struct.
	 */
	full_tty = tty = ttyname(0);
	if (tty[0] == '/') tty += 5;
	memset(&ut, 0, sizeof(ut));
	if (ai->authenticated)
		ut.ut_type = USER_PROCESS;
	else
		ut.ut_type = LOGIN_PROCESS;
	ut.ut_pid = getpid();
	ut.ut_time = time(NULL);
	strncpy(ut.ut_line, tty, sizeof(ut.ut_line));
	strncpy(ut.ut_user, ai->login, sizeof(ut.ut_user));
	strncpy(ut.ut_id, id, sizeof(ut.ut_id));

	/*
	 *	Now try to find the existing struct..
	 */
	if (parent) ut.ut_pid = parent;
	if ((u = getutpid(&ut)) != NULL)
		strncpy(ut.ut_id, u->ut_id, sizeof(ut.ut_id));
	if (parent) ut.ut_pid = getpid();
	setutent();
	pututline(&ut);

	if (ai->address.ss_family) {
		char *p = dotted6((struct sockaddr *)&ai->address, ipbuf, sizeof(ipbuf));
		int i = 0;

		while(*p && i < 2)
			if (*p++ == '.') i++;

		snprintf(lhost, sizeof (lhost), "%03d:%c.%s", ai->nasport, ai->proto, p);
	} else {
		snprintf(lhost, sizeof (lhost), "%03d:", ai->nasport);
	}

	striprealm(ai);
	maketermsane(0);

	/*
	 *	If we are forked it also means we are already authenticated -
	 *	so don't ask for a password.
	 */

    if (ai->authenticated == 2) {
		struct passwd *pw;

		if (!(pw = getpwnam(ai->login))) {  
        	nsyslog(LOG_ERR, "Invalid username [%s]", ai->login);
        	return -1;
		}

		/* Try these, but don't complain if they fail
		 * (for example when the root fs is read only) */
		chown(full_tty, pw->pw_uid, pw->pw_gid);
		chmod(full_tty, 0600);

		if (setgid(pw->pw_gid) < 0)
			nsyslog(LOG_ERR, "setgid %m:");
		if (setreuid(pw->pw_uid, pw->pw_uid) < 0)
			nsyslog(LOG_ERR, "setreuid: %m");
		if (setuid(pw->pw_uid) < 0)
			nsyslog(LOG_ERR, "setuid %m:");
		//[RK]Mar/28/06 -- 
		if ( chdir ( pw->pw_dir )) {
			setenv("HOME","/home" , 1);
			chdir("/home");
		} else {
			setenv("HOME",pw->pw_dir , 1);
		}
		
		setenv("USER", ai->login, 1);

		if(!pw->pw_shell || !*pw->pw_shell) {
			execl("/bin/sh", "-sh", NULL);
			nsyslog(LOG_ERR, "/bin/sh: %m");
		} else { //restrict shell
			char arg[20];
			sprintf(arg,"-%s",pw->pw_shell);
			execl(pw->pw_shell,arg,NULL);
			nsyslog(LOG_ERR, "/bin/sh: %m");
		}
		return -1;
	}
	if (ai->authenticated)
//TESTE		execl("/bin/login", "login", "-f", ai->login, "-h", lhost, NULL);
		execl("/bin/login", "login", "-f", ai->login, NULL);
	else
		execl("/bin/login", "login", "-h", lhost, ai->login, NULL);

	nsyslog(LOG_ERR, "/bin/login: %m");
	return -1;
}

int get_traffic_stats(struct auth *ai)
{
	int recv_bytes = 0, sent_bytes = 0; 
	int sockfd;
	struct sockaddr_storage spawned_ip;

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		// IPv6 - this socket is used to talk to the kernel (protocol stack)
		// IPv6 - it doesn't matter the socket type, as the IOCTL sent by
		// traffic_stats is protocol independent
		nsyslog(LOG_ERR,"socket(sockfd): %m");
		return 1;
	}
		
	//spawned_ip.s_addr=ai->address; IPv6
	memcpy(&spawned_ip, &ai->address, sizeof(spawned_ip));

	traffic_stats(sockfd, (struct sockaddr *)&spawned_ip, &recv_bytes, &sent_bytes);
	if (recv_bytes)	ai->recv_bytes = recv_bytes;
	if (sent_bytes)	ai->sent_bytes = sent_bytes;
	return 0;
}


#ifndef LINUX22

static void usr1_handler(int sig)
{
	got_usr1 = TRUE;
}

#endif

static char aux_speed[16];

/*
 *	Spawn an extra process to do the work, if needed.
 */
int spawnit(struct auth *ai)
{
	pid_t pid;
	int st;
	struct sigaction sa = {{0}};
	char id[8];
	struct utmp ut, *u;

	cy_shm->vet_ppp[ai->nasport] = 0;

	/*
	 *    Make up an id field if needed.
	 */
	snprintf(id, sizeof (id), "T%d", ai->nasport);

	/*
	 *    We only fork if we are going to exec another program
	 *    to do the work and we need to send a RADIUS logout
	 *    packets afterwards.
	 */

	pid = -1;
	// SOCKET_CLIENT: test it as TELNET or RLOGIN
	if (ai->proto != P_LOCAL &&
		ai->proto != P_SHELL &&
		ai->proto != P_AUTOPPP) {
		if ((pid = fork()) < 0) {
			nsyslog(LOG_ERR, "fork: %m");
			return -1;
		}
		if (pid > 0) {
			/*
			 *      Catch SIGHUP.
			 */
			sa.sa_handler = got_hup;
			sa.sa_flags   = SA_RESTART;
			sigaction(SIGHUP,  &sa, NULL);
			sigaction(SIGTERM, &sa, NULL);
			sigaction(SIGINT,  &sa, NULL);
			unblock(SIGHUP);
			unblock(SIGTERM);
			unblock(SIGINT);
	    	       
			if (ai->sessiontime > 0) {
				alarm(ai->sessiontime);
				sigaction(SIGALRM, &sa, NULL);
				unblock(SIGALRM);
			}
			signal(SIGUSR1, SIG_IGN);       // Ignore stray SIGUSR1

			/*
			 *      Wait for the child to exit. Query traffic stats.
			 */
#ifndef LINUX22		//Yeah it's ugly but optimized. You got a problem with that PUNK?!?!?
			if (ai->proto == P_PPP || ai->proto == P_PPP_ONLY) {
				signal(SIGUSR1, usr1_handler);
				while(waitpid(pid, &st, WNOHANG) != pid) {
					sleep(1);
					if (got_usr1 == TRUE) {
						get_traffic_stats(ai);
						got_usr1 = FALSE;
					}
					errno = 0;	//FIX ME ???
				}
				get_traffic_stats(ai);
			} else // An Illinois hanging else. I hate Illinois hanging elses.
#endif
				if (ai->proto == P_SLIP && ai->proto == P_CSLIP) {
					while(waitpid(pid, &st, WNOHANG) != pid) {
						sleep(STATS_SLEEP);
						get_traffic_stats(ai);
						errno = 0;      //FIX ME ???
					}
					get_traffic_stats(ai);  //Just maybe
				} else {
					while(waitpid(pid, &st, 0) != pid && errno == EINTR)
						errno = 0;
						if (ai->proto == P_PPP || ai->proto == P_PPP_ONLY) {
							get_traffic_stats(ai);
						}
					}

	    	       /*
	    		*      Readjust the utmp entry for P_SHELL.
	    		*/
	    	       if (ai->proto == P_SHELL) {
	    		       setutent();
	    		       ut.ut_pid = pid;
	    		       if ((u = getutpid(&ut)) != NULL) {
	    			       u->ut_pid = getpid();
	    			       setutent();
	    			       pututline(u);
	    		       }
	    	       }
	    	       return 0;
	           }
	           /*
	    	*      If a signal was pending, it will be delivered now.
	    	*/
	           unblock(SIGHUP);
	           unblock(SIGTERM);
	           unblock(SIGINT);

	   }

	   /*
	    *    Set the TERM environment variable.
	    */
		if (lineconf[ai->nasport].term && lineconf[ai->nasport].term[0]) {
			setenv("TERM", lineconf[ai->nasport].term, 1);
			sprintf(aux_speed, "%d", lineconf[ai->nasport].speed);
			setenv("TSPEED", aux_speed, 1);

		}

	   /*
	    *    Catch sighup so that we can be killed.
	    */
	   signal(SIGHUP, SIG_DFL);
       signal(SIGTERM, SIG_DFL);
       signal(SIGINT, SIG_DFL);

	   /*
	    *      Find the protocol.
	    */

	update_utmp("%L", lineconf [ai->nasport].utmp_from, ai, lineconf[ai->nasport].syswtmp);
	   switch(ai->proto) {
		case P_TCPCLEAR:
#if 0 		// XXX - unimplemented protocols
		case P_TCPLOGIN:
			do_tcp(ai);
			break;
		case P_CONSOLE:
			do_console(ai);
			break;
#endif
		case P_SLIP:
		case P_CSLIP:
			do_slip(ai);
			break;
		case P_PPP:
		case P_PPP_ONLY:
		case P_AUTOPPP:
			do_ppp(ai);
			break;
		case P_RLOGIN:
			striprealm(ai);
			do_rlogin(ai);
			break;
		case P_TELNET:
			striprealm(ai);
			do_telnet(ai);
			break;		       
		case P_SSH:
		case P_SSH2:
			striprealm(ai);
			do_ssh(ai);
			break;		       
		case P_LOCAL:
		case P_SHELL:
			do_local(ai, id, pid == 0 ? getppid() : 0);
			break;
		default:
			printf("protocol `%c' unsupported\n", ai->proto);
			fflush(stdout);
			usleep(500000);
			break;
	   }
	   if (pid == 0) exit(0);
	   return 0;
}
