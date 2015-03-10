/*
 * Copyright 1997-2000 by Pawel Krawczyk <kravietz@ceti.pl>
 *
 * See http://www.ceti.com.pl/~kravietz/progs/tacacs.html
 * for details.
 *
 * connect.c  Open connection to server.
 */

#include <netinet/in.h>
#include <netdb.h>
#include <syslog.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#include "tacplus.h"
#include "libtac.h"

// [GY]  2005/aug/25  clean warnings
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#define        GOT_SIGALRM      14 

#include "../../support.h"  //[GY] 2005/Aug/31  clean warnings

int got_sig;
extern int tac_timeout;
extern int tac_retries;

void unblock(int sig)
{
	sigset_t set;

	sigemptyset(&set);
	sigaddset(&set, sig);
	sigprocmask(SIG_UNBLOCK, &set, NULL);
}

void treatSignals( int sig ) {
	struct sigaction act;
	memset((char*)&act,0,sizeof(struct sigaction));
	switch (sig) {
		case SIGALRM:
			got_sig |= GOT_SIGALRM;
			break;
	}
}

void setSignals() 
{
	struct sigaction act;

	memset((char*)&act,0,sizeof(struct sigaction));
	
	// Setting up the act structure
	act.sa_handler = treatSignals;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	
	sigaction(SIGALRM, &act, 0);
	unblock(SIGALRM);
} 

/* Returns file descriptor of open connection
   to the first available server from list passed
   in server table.
*/
int tac_connect(struct sockaddr_storage *server, int servers) {
	struct sockaddr_storage serv_addr;
	struct sockaddr_in *serv_addr4 = (struct sockaddr_in *)&serv_addr;
	struct sockaddr_in6 *serv_addr6 = (struct sockaddr_in6 *)&serv_addr;
	
	int serv_addr_len = sizeof(struct sockaddr_storage);
	struct servent *s;
	int fd;
	int tries = 0,nro_tries=0; //nro_tries = # tries to connect
	//tries = # server begin connect
	if (!servers) {
		syslog(LOG_ERR, "%s: no TACACS+ servers defined", __FUNCTION__);
		return(-1);
	}

	setSignals();
	while (tries < servers) {

		int port = htons(TAC_PLUS_PORT);
		s = getservbyname("tacacs", "tcp");
		if (s != NULL) { 
			port = s->s_port;
		}

		bzero((char *) &serv_addr, sizeof(serv_addr));
		if (server[tries].ss_family == AF_INET) {
			serv_addr_len = sizeof(struct sockaddr_in);
			memcpy(&serv_addr,&server[tries],serv_addr_len);
			serv_addr4->sin_port = port;
		} else if (server[tries].ss_family == AF_INET6) {
			serv_addr_len = sizeof(struct sockaddr_in6);
			memcpy(&serv_addr,&server[tries],serv_addr_len);
			serv_addr6->sin6_port = port;
		}

		if ((fd=socket(server[tries].ss_family, SOCK_STREAM, 0)) < 0) {
			char str[INET6_ADDRSTRLEN] = "";
			syslog(LOG_WARNING, 
					"%s: socket creation error for %s: %m", __FUNCTION__, \
					xinet_ntoa((struct sockaddr *)&server[tries],str,sizeof(str)));
			tries++;
			continue;
		}
		do {
			alarm(tac_timeout);
			if(connect(fd, (struct sockaddr *) &serv_addr, 
					serv_addr_len) < 0)
			{
				if(errno==EINTR) {
					if(got_sig==SIGALRM) {
						char str[INET6_ADDRSTRLEN] = "";
						syslog(LOG_WARNING, 
								"%s: connection to %s failed: Connection timed out", \
								__FUNCTION__, xinet_ntoa((struct sockaddr *)&server[tries],str,sizeof(str)));
					}
				} else { 
					char str[INET6_ADDRSTRLEN] = "";
					syslog(LOG_WARNING, 
							"%s: connection to %s failed: %m", __FUNCTION__, \
							xinet_ntoa((struct sockaddr *)&server[tries],str,sizeof(str)));
				}
				nro_tries++;
			}
			else {
				/* connected ok */
				char str[INET6_ADDRSTRLEN] = "";
				alarm(0);
				TACDEBUG((LOG_DEBUG, "%s: connected to %s", __FUNCTION__, \
						xinet_ntoa((struct sockaddr *)&server[tries],str,sizeof(str))));

				return(fd);
			}
		} while (nro_tries < tac_retries);
		tries++;
	}

	/* all attempts failed */
	syslog(LOG_ERR, "%s: all possible TACACS+ servers failed", __FUNCTION__); 
	// disable the alarm
	alarm(0);
	return(-1);

} /* tac_connect */


int tac_connect_single(struct sockaddr_storage server) {
	return(tac_connect(&server, 1));
} /* tac_connect_single */
