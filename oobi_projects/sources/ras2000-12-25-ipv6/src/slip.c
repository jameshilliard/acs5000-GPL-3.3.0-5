/*
 * slip.c	Go into SLIP mode.
 *
 * Version:	@(#)slip.c  1.10  14-Jul-1996  MvS.
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
#include <net/if.h>
#if !defined(__GLIBC__)
#  include <linux/netdevice.h>
#  include <linux/if_ether.h>
#  include <linux/if_arp.h>
#endif
#include <linux/if_slip.h>

#include "server.h"


static int got_hup = 0;

/*
 *	Note that we got the HUP signal.
 */
static void hup_handler(int sig)
{
	got_hup = 1;
	alarm(0);
}

/*
 *	Setup the interface.
 */
static int slip_ifconfig(struct auth *ai, char *slname)
{
	char cmd[260];
	char local[INET6_ADDRSTRLEN];
	char remote[INET6_ADDRSTRLEN];
	char netmask[16];
	char ipbuf[INET6_ADDRSTRLEN];
	int aux;
#ifdef IPv6enable
	int isIPv6 = 0;
#endif

	if (ai->netmask == 0) ai->netmask = 0xFFFFFFFF;
#ifdef CYCLADES
    strcpy(remote, dotted6((struct sockaddr *)&ai->host, ipbuf, sizeof(ipbuf)));
    if (ai->address.ss_family) {
    	strcpy(local, dotted6((struct sockaddr *)&ai->address, ipbuf, sizeof(ipbuf)));
#ifdef IPv6enable
		if (ai->address.ss_family == AF_INET6) {
			isIPv6 = 1;
		}
#endif
    } else {
#ifdef IPv6enable
		if (mainconf.ipno6.ss_family) {
    		strcpy(local, dotted6((struct sockaddr *)&mainconf.ipno6, ipbuf, sizeof(ipbuf)));
			isIPv6 = 1;
		}
		else
#endif
        	strcpy(local, dotted(mainconf.ipno));
    }
#else
	strcpy(local, dotted(mainconf.ipno));
    strcpy(remote, dotted6((struct sockaddr *)&ai->address, ipbuf, sizeof(ipbuf)));
#endif
	strcpy(netmask, dotted(ai->netmask));

	/* Ifconfig the interface. */
//  Changed to support IPv6 addresses
//	if (ai->mtu > 0) {
//		snprintf(cmd, sizeof (cmd), "exec %s %s %s pointopoint %s "
//			"netmask %s mtu %d", PATH_IFCONFIG, slname, local,
//			 remote, netmask, ai->mtu);
//	} else {
//		snprintf(cmd, sizeof (cmd), "exec %s %s %s pointopoint %s netmask %s",
//			PATH_IFCONFIG, slname, local, remote, netmask);
//	}
#ifdef IPv6enable
	if (isIPv6)
		aux = snprintf(cmd, sizeof (cmd), "exec %s %s %s/%d pointopoint %s/%d ", PATH_IFCONFIG, slname, local, ai->prefix6, remote, ai->prefix6);
	else
#endif
		aux = snprintf(cmd, sizeof (cmd), "exec %s %s %s pointopoint %s netmask %s ", PATH_IFCONFIG, slname, local, remote, netmask);

	if (ai->mtu > 0) {
		snprintf(cmd + aux, (sizeof(cmd) - aux), "mtu %d", ai->mtu);
	} 

	return system(cmd);
}

/*
 *	Add a route to this address.
 */
static int slip_addroute(struct auth *ai, char *slname)
{
	char cmd[100];
	char netmask[16];
	char network[16];
	char remote[16];
	unsigned int nw;
	int ret;
	char ipbuf[INET6_ADDRSTRLEN];

	/* Add a host pointopoint route. */
	snprintf(cmd, sizeof (cmd), "exec %s add -host %s dev %s", PATH_ROUTE,
			dotted6((struct sockaddr *)&ai->address, ipbuf, sizeof(ipbuf)), 
			slname); //TODO - check IPv6
	if ((ret = system(cmd)) != 0)
		return ret;

	/* See if we need to add a route to a whole network. */
	if (ai->netmask && ai->netmask != 0xFFFFFFFF) {
		//nw = ai->address & ai->netmask; TODO - check IPv6
		nw = ((struct sockaddr_in *)&ai->address)->sin_addr.s_addr & ai->netmask;
		strcpy(netmask, dotted(ai->netmask));
		strcpy(network, dotted(nw));
		strcpy(remote, dotted6((struct sockaddr *)&ai->address, ipbuf, sizeof(ipbuf))); //TODO - check IPv6
		snprintf(cmd, sizeof (cmd), "exec %s add -net %s netmask %s gw %s",
			PATH_ROUTE, network, netmask, remote);
		ret = system(cmd);
	}

	return ret;
}

/*
 *	Add a proxy arp entry.
 *
 *	FIXME: we are very simplistic here. We should loop over
 *	all interfaces and pick the right one (like pppd does).
 *
 */
static int slip_addproxy(struct auth *ai, char *slname)
{
	struct ifreq ifr;
	struct sockaddr *sa;
	int s;
	char ethaddr[16];
	char cmd[100];
	char ipbuf[INET6_ADDRSTRLEN];

	/*
	 *	First we have to find out our ethernet address.
	 */
	strcpy(ifr.ifr_name, "eth0");
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		nsyslog(LOG_ERR, "slip_addproxy: socket: %m");
		return -1;
	}
	if (ioctl(s, SIOCGIFHWADDR, &ifr) < 0) {
		nsyslog(LOG_ERR, "slip_addproxy: SIOCGIFADDR: %m");
		return -1;
	}
	sa = &(ifr.ifr_hwaddr);
	snprintf(ethaddr, sizeof (ethaddr), "%02x:%02x:%02x:%02x:%02x:%02x",
		(unsigned char)sa->sa_data[0], (unsigned char)sa->sa_data[1],
		(unsigned char)sa->sa_data[2], (unsigned char)sa->sa_data[3],
		(unsigned char)sa->sa_data[4], (unsigned char)sa->sa_data[5]);

	/* Add an proxy arp entry. */
	if (ai->netmask && ai->netmask != 0xFFFFFFFF) {
		snprintf(cmd, sizeof (cmd), "exec %s -s %s %s netmask %s pub",
			PATH_ARP, dotted6((struct sockaddr *)&ai->address, ipbuf, sizeof(ipbuf)), ethaddr, dotted(ai->netmask));
	} else {
		snprintf(cmd, sizeof (cmd), "exec %s -s %s %s pub", PATH_ARP, dotted6((struct sockaddr *)&ai->address, ipbuf, sizeof(ipbuf)), ethaddr);
	}
	return system(cmd);
}

/*
 *	Delete proxy arp entry.
 */
static int slip_delproxy(struct auth *ai, char *slname)
{
	char cmd[100];
	char ipbuf[INET6_ADDRSTRLEN];

	snprintf(cmd, sizeof (cmd), "exec %s -d %s pub", PATH_ARP, dotted6((struct sockaddr *)&ai->address, ipbuf, sizeof(ipbuf)));
	return system(cmd);
}

/*
 *	Put port into SLIP mode.
 */
int do_slip(struct auth *ai) {
	struct termios tty, orig_tty;
	int disc, odisc;
	char slname[16];
	int mode;
	struct sigaction sa;

	/*
	 *	Catch SIGHUP and friends.
	 */
	got_hup = 0;
	sa.sa_handler = hup_handler;
	sa.sa_flags   = SA_RESTART;
	sigaction(SIGHUP, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	if (ai->sessiontime > 0) {
		alarm(ai->sessiontime);
		sigaction(SIGALRM, &sa, NULL);
		unblock(SIGALRM);
	}

	unblock(SIGHUP);
	unblock(SIGINT);
	unblock(SIGTERM);

	/*
	 *	First set terminal into raw mode
	 */
	tcgetattr(0, &tty);
	tcgetattr(0, &orig_tty);
	tty.c_lflag = 0;
	tty.c_iflag = 0;
	tty.c_oflag = 0;
	tcsetattr(0, TCSANOW, &tty);

	/*
	 *	Now set line discipline to SLIP.
	 */
	ioctl(0, TIOCGETD, &odisc);
	disc = N_SLIP;
	if (ioctl(0, TIOCSETD, &disc) < 0) {
		tcsetattr(0, TCSANOW, &orig_tty);
		if (errno == EINVAL)
			printf("Sorry - SLIP is not available on this system\r\n");
		else
			perror("SLIP mode");
		nsyslog(LOG_ERR, "TIOCSETD(SLIP): %m");
		return -1;
	}

	/*
	 *	Now set to either SLIP or CSLIP.
	 */
	mode = (ai->proto == P_SLIP ? SL_MODE_SLIP : SL_MODE_CSLIP);
	(void) ioctl(0, SIOCSIFENCAP, &mode);

	/*
	 *	Find out the name of the interface, ifconfig it,
	 *	add routes, and set up proxyarp.
	 */
	ioctl(0, SIOCGIFNAME, slname);
	slip_ifconfig(ai, slname);
	slip_addroute(ai, slname);
	slip_addproxy(ai, slname);

	/*
	 *	Now keep waiting for a SIGHUP.
	 */
	while(!got_hup)
		pause();
	
	ioctl(0, TIOCSETD, &odisc);
	tcsetattr(0, TCSANOW, &orig_tty);
	slip_delproxy(ai, slname);

	return 0;
}
