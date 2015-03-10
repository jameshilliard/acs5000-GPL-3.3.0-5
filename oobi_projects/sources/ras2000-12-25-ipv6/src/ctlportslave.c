/*
 * ctlportslave.c	Command line interface to running portslaves.
 *			Basically this is to accomodate the pmmon
 *			program, but it might have some more uses too.
 *
 * Version:		ctlportslave	1.01  22-Nov-1997	miquels@cistron.nl
 *		 			1.2.0  1999-01-22	dcinege@psychosis.com
 *
 */
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <utmp.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

#include "server.h"

#define SESS 1
#define WHO 2
#define DETAIL 3
#define QUIET 10

int do_show(int, char **);
int do_reset(int, char **);
int do_quit(int, char **);
int do_help(int, char **);
int do_help_finger(void);
int read_utmp(void);
char* get_passwd(char* p);

struct utmp lines[MAXLINES];
int max_port_ever = 0;
char *eol = "\n";
int finger_prog = 0;
char fpasswd[256];
char rpasswd[256];
char passwd_in[256];

#ifndef BIGUTMP
char eth0_classB[32];
#endif

int sockfd = 0;

extern int traffic_stats(int sockfd, struct in_addr ina, int *in, int *out);
extern int idle_stat(char *dev, int *idle);


/*
 *	Header for the 2 formats: sessions and who.
 */
char *hdr_sess =
"Port User            Host/Inet/Dest   Type    Dir Status         Start   Idle";
char *sep_sess =
"---- --------------- ---------------- ------- --- ------------- ------ ------";
char *idle_sess = 
"S%-3d -               -                Log/Net In  IDLE               0      0%s";
char *hdr_who =
"Port User         Host/Inet/Dest  Type    Stat Start        In       Out  Idle";
char *sep_who =
"---- ------------ --------------- ------- ---- ----- --------- --------- -----";
char *idle_who =
"S%-3d -            -               -       IDLE     0         0         0     0%s%";
char *hdr_detail =
"Port User            Host/Inet/Dest   Type    Status";
char *sep_detail =
"---------------------------------------------------------------------------";
char *idle_detail =
"S%-3d -               -                        IDLE%s%s%s";

/*
 *	Commands we support.
 */
struct commands {
	char *cmd;
	int (*fun)(int, char **);
} commands[] = {
	{ "show",	do_show		},
	{ "reset",	do_reset	},
	{ "help",	do_help		},
	{ "exit",	do_quit		},
	{ "quit",	do_quit		},
	{ NULL,	NULL		},
};


/*
 *	Return extended type.
 */
char *xtype(char t)
{
	if (t == 'x')  return("Network");
	if (t == 'E')  return("Telnet");
	if (t == 'R')  return("Rlogin");
	if (t == 'L')  return("Local");
	if (t == 'X')  return("Local");
	if (t == 'C')  return("CSLIP");
	if (t == 'S')  return("SLIP");
	if (t == 'P')  return("PPP");
	if (t == 'A')  return("APPP");
	if (t == 'H')  return("SSH");	
	if (t == 'I')  return("Logon");

	return ("unknown");
}

/*
 *	Return address.
 */
char *address(struct utmp *ut, struct in_addr *ina)
{
#ifdef __linux__
	ina->s_addr = ut->ut_addr;
	return(inet_ntoa(*ina));
#else

	static char buf[256];
	char *p;
	int i = 0;
	ina->s_addr = 0;

#  ifndef BIGUTMP
	if ((p = strchr(ut->ut_host, ':')) == NULL || p[1] == 0)
		return("");
	p++;
	if (p[0]) p++;
	snprintf(buf, sizeof (buf), "%s%s", eth0_classB, p + 1);
#  else
	p = ut->ut_host + 6;
	while (*p && *p != ':') {
		buf[i++] = *p;
		p++;
	}
	buf[i] = '\0';
	if (*p == 0)
		return("");
#  endif

	ina->s_addr = inet_addr(buf);
	return buf;

#endif

}


/*
 *	Show logged in users. If arg is `sessions', use the
 *	portmaster format. If arg is `who', use our own format.
 */
int do_show(int argc, char **argv)
{
	int style = 0;
	int i;
	char *type = 0, *p;
	char name[256];
	char status[256];
	char *addr = 0;
	struct in_addr ina;
	struct utmp *ut;
	time_t now;
	int tm;
	int in, out, idle;

	if (argc > 1)
		switch (argv[1][0]) {
			case 's':	style = SESS;	break;
			case 'w':	style = WHO;	break;
			case 'd':	style = DETAIL;	break;
		}
	
	if (style == 0) {
		printf("Error: usage: show sessions,who,detailed\n");
		return 0;
	}

	read_utmp();
	now = time(NULL);

#ifdef UPTIME
	if (finger_prog) {
		printf("System: ");
		fflush(stdout);
		system(UPTIME);
		printf("\n");
	}
#endif 
	switch (style) {
		case SESS:	printf("%s%s%s%s", hdr_sess, eol, sep_sess, eol);	break;
		case WHO:	printf("%s%s%s%s", hdr_who, eol, sep_who, eol);		break;
		case DETAIL:	printf("%s%s%s%s", hdr_detail, eol, sep_detail, eol);	break;
	}

	for(i = 0; i <= max_port_ever; i++) {

		ut = lines + i;
		p = strchr(ut->ut_host, ':');
		if (p == NULL || ut->ut_name[0] == 0) {
			if (!finger_prog)
				switch (style) {
					case SESS:	printf(idle_sess, i, eol);	break;
					case WHO:	printf(idle_who, i, eol);	break;
					case DETAIL:	printf(idle_detail, i, eol, sep_detail, eol);	break;
				}
			continue;
		}

		strncpy(name, ut->ut_name, sizeof(ut->ut_name));
		name[sizeof(ut->ut_name)] = 0;
		tm = (now - ut->ut_time) / 60;
		p++;

		ina.s_addr = 0;
		if (strchr("xERSCPLXAI", *p)) {
			
			if (strchr("I", *p)) {
				addr = "";
				strncpy(status, ut->ut_host + 6, sizeof(ut->ut_name)-6);
				status[sizeof(ut->ut_name)-6] = 0;			
				
			} else {
				addr = address(ut, &ina);
				strcpy(status, "ESTABLISHED");	
			}
				
			type = xtype(*p);	//type = (n == 1) ? "Network" : xtype(*p);
#ifdef BIGUTMP
//Override with new values
			if (style != WHO)
				//Is this safe? Grabbing the length for strncpy is a pain.
				strcpy(status, ut->ut_host + (strlen(addr) + 7));
#endif
		}
		

		in = out = idle = 0;
		if (style >= WHO && ina.s_addr) {
			traffic_stats(sockfd, ina, &in, &out);
		}
		idle_stat(ut->ut_line, &idle);
		idle /= 60;

		switch (style) {
			case SESS:
				printf("S%-3d %-15.15s %-16.16s %-7.7s In  %-13.13s %6d %6d%s",
	            			i, name, addr, type, status, tm, idle, eol);
				break;
			case WHO:	
				printf("S%-3d %-12.12s %-15.15s %-7.7s %-4.4s %5d %9d %9d %5d%s",
	    	        		i, name, addr, type, status, tm, in, out, idle, eol);
				break;
			case DETAIL:	
				printf("S%-3d %-15.15s %-16.16s %-7.7s %-28.28s%s",
	            			i, name, addr, type, status, eol);
				printf(" IOT: %10d %10d %7d KB        Start:%6d  Idle:%5d%s",
	    	        		in, out,  (in + out) / 1024, tm, idle, eol);
				printf("%s%s", sep_detail, eol);
				break;
		}
	}
	return 0;
}

/*
 *	Reset a terminal line (send SIGHUP)
 */
int do_reset(int argc, char **argv)
{
	int port;
	
	if (argc < 2 || (argv[1][0] != 's' && argv[1][0] != 'S')) {
		printf("Error: Usage: reset Port_Name\n");
		return 0;
	}
	port = atoi(argv[1] + 1);
	
	read_utmp();
	
	if (lines[port].ut_pid > 0) {
		printf("Resetting port S%d (line %s, pid %d)\n",
			port, lines[port].ut_line, lines[port].ut_pid);
		kill(lines[port].ut_pid, SIGHUP);
	} else {
		printf("Port S%d does not appear to be active. Can not reset...\n", port);
	}
	return 0;
}

/*
 *	Exit from this program.
 */
int do_quit(int argc, char **argv)
{
	return -1;
}

/*
 *	Pretend we're the finger daemon.
 */
int do_finger(void)
{
	char buf[256];
	char port[16];
	char *args[3] = {NULL, "sessions", NULL};	//set default style
	char *p;

	eol = "\r\n";

	memset(passwd_in,'\0',sizeof(passwd_in));
		
	fgets(buf, sizeof(buf) / 2, stdin);	//Just paranoid about an overflow.

	switch (buf[0]) {
		case 'h':
			do_help_finger();
			return 1;
		case 'r':
			
			if (strcmp(rpasswd, "") == noerror ) {
				printf("Reset access disabled. (no password defined)\n");
				break;
			}
			
			p = get_passwd(buf);
			if (strcmp(rpasswd,passwd_in) != noerror) {
				printf("Permission denied.\n");
				break;
			}	

			memset(port,'\0',sizeof(port));
			strncpy(port,++p,3);
			
			args[0] = "reset"; args[1] = port; args[2] = NULL;
			do_reset(2,args);
			break;
		case 's':	args[1] = "sessions";	goto def; break;
		case 'w':	args[1] = "who";	goto def; break;
		case 'd':	args[1] = "detailed";	goto def; break;
		default:
def:		
			if (strcmp(fpasswd,"") != noerror) {
				get_passwd(buf);
				if (strcmp(fpasswd,passwd_in) != noerror) {
					printf("Permission denied.\n");
					return 1;
				}
			}	
				
			args[0] = "show"; args[2] = NULL;
			do_show(2,args);
			break;
	}
	return 0;
}


char* get_passwd(char *p)
{
int len = 0, i = 0;
							//len = strlen(p); 	//This includes escape chars!
	while (*p >= Fascii && *p <= Lascii) {		//strlen up to non-escape chars
		p++;
		len++;
	}
	if (len <= 2) goto bad_usage;
	p -= len;

	while ((*++p != ':') && (--len > 0));
	if (len <= 1) goto bad_usage;
	p++; len--;

	while (*p != ':' &&  len > 1) {
		passwd_in[i++] = *p++;
		len--;
	}
	return (p);

bad_usage:
	//do_help_finger();		//Hmmm...I guess best to just say this:
	printf("Permission denied.\n");
	exit(1);
}



/*
 *	Short help text.
 */
int do_help(int arc, char **argv)
{
	printf("\nCtlportslave v%s help:\n\n", VERSION);
	printf("  show  <sessions|who|detailed>\n");
	printf("  reset Sxx\n");
	printf("  exit\n");
	printf("  quit\n\n");
	return 0;
}

int do_help_finger(void)
{
	printf("\nCtlportslave v%s as finger help:\n\n", VERSION);
	printf("  <sessions|who|detailed>[:password]@	show active ports\n");
	printf("  reset:password:Sxx@			reset port Sxx\n");
	return 0;
}



/*
 *	Decipher portslave port from ut_host.
 */
int ps_port(char *data)
{
	if (isdigit(data[0]) && isdigit(data[1]) &&
		  isdigit(data[2]) && data[3] == ':')
		return (atoi(data));
	return -1;
}

/*
 *	Read the portslave UTMP file..
 */
int read_utmp(void)
{
	struct utmp *ut;
	int port;

	setutent();
	memset(lines, 0, sizeof(lines));
	while ((ut = getutent()) != NULL) {
		if ((port = ps_port(ut->ut_host)) < 0 || port > MAXLINES)
			continue;
		if (port > max_port_ever) max_port_ever = port;
		if (ut->ut_type != LOGIN_PROCESS &&
			  ut->ut_type != USER_PROCESS)
			continue;
		lines[port] = *ut;
	}
	endutent();

	return 0;
}


int main(int argc, char **argv)
{
	char hostname[128];
	char buf[64];
	char *args[16];
	char *p=0;
	int i, n;
	int quit = 0;

#ifndef __linux__ 
#ifndef BIGUTMP	
	struct ifreq ifr;
	struct sockaddr_in *sin;
#endif
#endif


	while (( i = getopt(argc, argv, "+fp:r:")) != -1) {
		switch(i) {
			case 'f':	finger_prog = 1;		break;
			case 'p':	strcpy(fpasswd, optarg);	break;
			case 'r': 	strcpy(rpasswd, optarg);	break;
			case '?':	exit (1);
		}	
	}

	/*
	 *	See if we we're called as `fingerd'.
	 */
	if ((strstr(argv[0], "finger")) || finger_prog) {
		while (argc > 1) {		//Clear displayed command line
			p = argv[--argc];
			while (*p) *p++ = '\0';
		}
		//strcpy(argv[0], "ps-finger");	//Set display name...do this??
		finger_prog = 1;
	}


	/*
	 *	Find out our ethernet address. We only need
	 *	the first two octets. This is because that's missing
	 *	from the information in the utmp file...
	 */
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		exit(1);
	}
	
#ifndef __linux__ 
#ifndef BIGUTMP	
	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, "eth0");
	ifr.ifr_addr.sa_family = AF_INET;
	if (ioctl(sockfd, SIOCGIFADDR, &ifr) < 0) {
		perror("SIOCGIFADDR(eth0)");
		exit(1);
	}
	
	sin = (struct sockaddr_in *)&(ifr.ifr_addr);
	strcpy(eth0_classB, inet_ntoa(sin->sin_addr));
	/*printf("ether is at %s\n", eth0_classB);*/
	p = eth0_classB;
	i = 0;
	while(*p && i < 2)
		if (*p++ == '.') i++;
	*p = 0;
#endif
#endif

	if (finger_prog) {
		exit (do_finger());
	}

	/*
	 *	Do some network/socket initialization.
	 */
	gethostname(hostname, sizeof(hostname));

	/*
	 *	Command Loop.
	 */
	while(!quit) {
		printf("%s> ", hostname);
		fflush(stdout);
		if (fgets(buf, sizeof(buf), stdin) == NULL) {
			printf("\n");
			break;
		}
		i = 0;
		p = strtok(buf, " \t\r\n");
		while(p && i < 15) {
			args[i++] = p;
			p = strtok(NULL, " \t\r\n");
		}
		args[i] = NULL;
		if (args[0] == NULL)
			continue;
		for(n = 0; commands[n].cmd; n++) {
			if (strcmp(args[0], commands[n].cmd) == 0) {
				if (commands[n].fun(i, args) < 0)
					quit = 1;
				break;
			}
		}
		if (commands[n].cmd == NULL)
			printf("Invalid command.\n");
	}
	printf("Goodbye....\n");

	return 0;
}
