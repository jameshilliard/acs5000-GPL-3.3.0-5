/*
 * lib1.c         Generic functions.
 *
 * Version:      @(#)lib1.c  1.36  13-Jan-1998  miquels@cistron.nl
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <errno.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <values.h>
#include "server.h"

//function prototypes
extern void nopenlog(const char *ident, int logstat, int logfac); //defined in syslog.c
extern int count_sessions_tst (int port_num); //defined in rot_sess2.c

extern struct line_cfg *pconf;

static int rad_portno(char *tty, int port, int fproto, struct sockaddr *ipaddr);

void fill_auth_struct(struct auth *ai, struct line_cfg *pconf);

/*
 *	Initialize struct and read config file.
 */
#ifdef IPv6enable
int rad_init(int port, struct auth *ai, char *tty, int fproto, struct sockaddr *ipaddr)
#else
int rad_init(int port, struct auth *ai, char *tty, int fproto)
#endif
{
	static char ident[64] = "portslave";
	int fac, oport, ppp = 0;
#ifndef IPv6enable
	struct sockaddr *ipaddr = NULL;
#endif
	/*
	 *	attach shared memory
	 */
	if (shm_init(-1) < 0) {
		return -1;
	}

	while (shm_get_set_val(0, &cy_shm->st_cy_ras, 0, CY_SEM_CONFIG) != 1) {
		sleep(1);
		continue;
	}

	/*
	 *	Find out port number if needed.
	 */
	oport = port;
	if ((tty || ipaddr) && (port <= -2 && port >= -6)) /* look for TTY */
	{
		port = rad_portno(tty, port, fproto, ipaddr);
		if (oport == -2) /* PPP daemon */
			ppp = 1;
	}

	if (port < 0) {
		return(port);
	}

	/*
	 *	Open the logfile.
	 */

#ifdef CYCLADES
	fac = LOG_FAC(LOG_LOCAL0) + mainconf.facility;
	if (lineconf[port].alias && *lineconf[port].alias)
#if defined(ONS) || defined(KVM)
		snprintf(ident, sizeof (ident), "%1s%d.%s",
			lineconf[port].auxflg ? "A" : "S",
			lineconf[port].auxflg ? (port+1-AUX_OFFSET) : (port+1),
			lineconf[port].alias);
#else
		snprintf(ident, sizeof (ident), "S%d.%s",
			port+1, lineconf[port].alias);
#endif
	else
#if defined(ONS) || defined(KVM)
		snprintf(ident, sizeof (ident), "%1s%d.%s",
			lineconf[port].auxflg ? "A" : "S",
			lineconf[port].auxflg ? (port+1-AUX_OFFSET) : (port+1),
			lineconf[port].tty);
#else
		snprintf(ident, sizeof (ident), "S%d.%s",
			port+1, lineconf[port].tty);
#endif
	nopenlog(ident, LOG_NDELAY, LOG_MAKEPRI(fac, 0)); //[RK]SYSLOG-NG
#else
	if (mainconf.syslog) setlogremote(mainconf.syslog, 1);
	if (ppp != 1)
	{
		fac = LOG_FAC(LOG_LOCAL0) + mainconf.facility;
		snprintf(ident, sizeof (ident), "port[S%d]", (port < 0) ? 9999 : port);
		nopenlog(ident, LOG_CONS|LOG_NDELAY, LOG_MAKEPRI(fac, 0));
	}
#endif

#ifndef CYCLADES
	if (port >= 0 && lineconf[port].debug == 0) {
		if (oport == -2) /* PPP daemon */
			nsetlogmask(LOG_UPTO(LOG_NOTICE));
		else
			nsetlogmask(LOG_UPTO(LOG_INFO));
	}
#endif

	/*
	 *	Fill in the auth struct for now.
	 */
	fill_auth_struct(ai, &lineconf[port]);

	if (ppp && ai->proto != P_PPP && ai->proto == P_PPP_ONLY) {
		ai->proto = P_AUTOPPP;
	}

	pconf = &lineconf[port];

	return port;
}


/*
 *	Fill in the auth struct for now.
 */
void fill_auth_struct(struct auth *ai, struct line_cfg *pconf)
{
	memset(ai, 0, sizeof(struct auth));
	strcpy(ai->login, "NONE");
	ai->nasport  = pconf->this;
	ai->proto    = pconf->protocol;
//	ai->address  = pconf->ipno; IPv6 - changed IPaddrs format
//	ai->host     = pconf->host; IPv6 - changed IPaddrs format
	ai->netmask  = pconf->netmask;
	ai->mtu      = pconf->mtu;
	ai->mru      = pconf->mru;
	ai->porttype = pconf->porttype;
	ai->localip  = mainconf.ipno;

#ifdef IPv6enable
	if (pconf->ipno6.ss_family) {
		/* Deal with IPv6 addrs for serial ports */
		memcpy(&ai->address, &pconf->ipno6, sizeof(ai->address));
	} else 
#endif
	{
		((struct sockaddr_in *)&ai->address)->sin_family = AF_INET;
		((struct sockaddr_in *)&ai->address)->sin_addr.s_addr = pconf->ipno;
	}

#ifdef IPv6enable
	memcpy(&ai->host, &pconf->host, sizeof(ai->host));
#else
	((struct sockaddr_in *)&ai->host)->sin_family = AF_INET;
	((struct sockaddr_in *)&ai->host)->sin_addr.s_addr = pconf->host;
#endif

#ifdef IPv6enable
	memcpy(&ai->localip6, &mainconf.ipno6, sizeof(ai->localip6));
#endif

	if (ai->netmask == 0) ai->netmask = 0xFFFFFFFF;
#ifdef BIDIRECT
	strncpy(ai->termsh, pconf->termsh, sizeof(ai->termsh));
#endif

}

/*
 * get the first port free from the pool,
 * lc points to the first port of the pool.
 */

static int get_port_free(struct line_cfg *lc)
{
	struct line_cfg *lp = lc;
	int i = lc->this;

	for( ; i < MAXVLINES; i++, lp++) {
		if (!lp->tty)
			continue;
		if (lp->protocol != lc->protocol)
			continue;
		if ((!lp->pool_alias && lc->pool_alias) ||
			(lp->pool_alias && !lc->pool_alias))
			continue;
		if (lc->pool_alias &&
			strcmp(lp->pool_alias, lc->pool_alias))
			continue;
		if (lp->pool_socket_port != lc->pool_socket_port)
			continue;
		if (lp->pool_ipno != lc->pool_ipno)
			continue;
#ifdef IPv6enable
		if ((!lp->pool_ipno6.ss_family && lc->pool_ipno6.ss_family) ||
		    (lp->pool_ipno6.ss_family && !lc->pool_ipno6.ss_family)) {
			continue;
		}
		if (lp->pool_ipno6.ss_family && ipaddr_cmp(&lp->pool_ipno6, &lc->pool_ipno6))
			continue;
#endif
		if (count_sessions_tst(lp->this))
			continue;		/* port already in use */
		if (!cy_shm->vet_pool[lp->this]) {
			cy_shm->vet_pool[lp->this] = getpid();
			return(lp->this);
		}
	}

	return(-2);
}

/*
 *	Get the port number belonging to this tty
 *	(the pppd uses this).
 */
static int rad_portno(char *tty, int port, int fproto, struct sockaddr *ipaddr)
{
	int i;

	if (port == -2 || port == -6)
		if (strncmp(tty, "/dev/", 5) == 0) tty += 5;

	for(i = 0; i < MAXVLINES; i++) {
		if (!lineconf[i].tty) continue;

		if (fproto == 1) {
#ifdef PMD
			//[RK]June/03/04 - SSH/Telnet to IPDU
			if ((lineconf[i].protocol == P_IPDU) && 
			    (lineconf[i].pmsessions & PMSESS_TELNET)) {
			} else 
#endif
			{	
				if (lineconf[i].protocol != P_SOCKET_SERVER &&
					lineconf[i].protocol != P_SOCKET_SERVER_SSH &&
#ifdef BIDIRECT
					lineconf[i].protocol != P_TELNET_BIDIRECT &&
#endif
				    lineconf[i].protocol != P_SOCKET_SERVER_RAW)
					continue;
			}
		} else if (fproto == 2) {
#ifdef PMD
			//[RK]June/03/04 - SSH/Telnet to IPDU
			if ((lineconf[i].protocol == P_IPDU) && 
			    (lineconf[i].pmsessions & PMSESS_SSH)) {
			} else 
#endif
			{	
				if (lineconf[i].protocol != P_SOCKET_SSH &&
                                    lineconf[i].protocol != P_SOCKET_SERVER_SSH) {
					//[RK]Dez/23/04 Bug 2906 - Fixed problem with invalid ipno
					if (port == -4) {
						if ((tty && (lineconf[i].ipno == (int)tty)) 
#ifdef IPv6enable
							|| (ipaddr && !ipaddr_cmp(&lineconf[i].ipno6, ipaddr))
#endif
						) {
							return -3;
						}
					}
					continue;
				}
			}
		}

		switch(port) {
		case -5:
			if (lineconf[i].alias &&
			    !strcmp(lineconf[i].alias, tty))
				return i;
			if (lineconf[i].pool_alias &&
			    !strcmp(lineconf[i].pool_alias, tty))
				return (get_port_free(&lineconf[i]));
			break;
		case -3:
			if (lineconf[i].socket_port == (int)tty)
				return i;
			if (lineconf[i].pool_socket_port == (int)tty)
				return (get_port_free(&lineconf[i]));
			break;
		case -4:
			if (tty && (lineconf[i].ipno == (int)tty))
				return i;
			if (tty && (lineconf[i].pool_ipno == (int)tty))
				return (get_port_free(&lineconf[i]));
#ifdef IPv6enable
			if (ipaddr && 
				!ipaddr_cmp(&lineconf[i].ipno6, ipaddr))
				return i;
			if (ipaddr &&
				!ipaddr_cmp(&lineconf[i].pool_ipno6, ipaddr))
				return (get_port_free(&lineconf[i]));
#endif
			break;
		case -2:
		case -6:
			if (strcmp(lineconf[i].tty, tty) == 0)
				return i;
			break;
		}
	}
	return -1;
}

