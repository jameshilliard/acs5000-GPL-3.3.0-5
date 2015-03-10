/*
 * ctlp-subs.c	Here's where most SIOC* calls for ctlportslave live.
 *
 * Version:	@(#)ctlp-subs.c  1.10  13-Jan-1998  miquels@cistron.nl
 *
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <time.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef unsigned short u16;
typedef unsigned char u8;


#if defined(__GLIBC__)
#  include <net/if_ppp.h>
#else
#  include <linux/if.h>
#  include <linux/ppp_defs.h>
#  include <linux/if_ppp.h>
#endif

#include "../pslave_cfg.h"

#define MAX_IFS 128

/*
 *	Find the idle time of a certain device.
 */
int idle_stat(char *dev, int *idle)
{
	char buf[128];
	int fd;
	struct termios tty;
	struct ppp_idle ppp_idle;
	struct stat st;
	time_t now;

	*idle = 0;

	if (dev[0] != '/') {
		snprintf(buf, sizeof (buf), "/dev/%s", dev);
		dev = buf;
	}
	if ((fd = open(dev, O_RDONLY|O_NONBLOCK)) < 0) {
		perror(dev);
		return -1;
	}
	if (tcgetattr(fd, &tty) < 0) {
		perror(dev);
		close(fd);
		return -1;
	}
	switch(tty.c_line) {
		case N_TTY:
			if (fstat(fd, &st) < 0) break;
			now = time(NULL);
			*idle = now - st.st_mtime;
			break;
		case N_PPP:
			if (ioctl(fd, PPPIOCGIDLE, &ppp_idle) == 0) {
				if (ppp_idle.recv_idle < ppp_idle.xmit_idle)
					*idle = ppp_idle.recv_idle;
				else
					*idle = ppp_idle.xmit_idle;
			}
			break;
		default:
			break;
	}

	close(fd);

	return 0;
}


/*
 *	Find the device with a certain IP address.
 *	Return packets in/out if we can retrieve such information.
 */
//int traffic_stats(int sockfd, struct in_addr saddr, int *in, int *out) - IPv6
int traffic_stats(int sockfd, struct sockaddr *saddr, int *in, int *out)
{
	int ifs_len;
	struct ifreq *ifr, *ifs;
	struct ifconf ifc;
	struct in_addr ina, sinaddr;
	int ret = -1;
	char buf[256];
	char *p;
	FILE *f;

	*in = *out = 0;

	sinaddr.s_addr = ((struct sockaddr_in *)saddr)->sin_addr.s_addr; //TODO - check how to deal with IPv6

	ifs_len = MAX_IFS * sizeof (struct ifreq);
	ifs = (struct ifreq *) malloc (ifs_len);
	ifc.ifc_len = ifs_len;
	ifc.ifc_req = ifs;

	if (ioctl(sockfd, SIOCGIFCONF, &ifc) < 0) {
		perror("ioctl(SIOCGIFCONF)");
		free(ifs);
		return -1;
	}
	ifs_len = ifc.ifc_len;

	for (ifr = ifc.ifc_req; ifs_len > 0; ifr++) {
		ifs_len -= sizeof(struct ifreq);
		ifr->ifr_dstaddr.sa_family = AF_INET;
		if (ioctl(sockfd, SIOCGIFDSTADDR, ifr) < 0)
			continue;
		if (ifr->ifr_dstaddr.sa_family != AF_INET)
			continue;
		ina = ((struct sockaddr_in *) &ifr->ifr_dstaddr)->sin_addr;
		/*printf("CHECK: %s %s\n", ifr->ifr_name, inet_ntoa(ina));*/
		if (ina.s_addr == sinaddr.s_addr) {
			ret = 0;
			break;
		}
	}
		
	if (ret) {
		free(ifs);
		return ret;
	}


#ifdef LINUX22	
	if ((f = fopen ("/proc/net/dev", "r"))) {
		while (fgets (buf, sizeof(buf), f)) {
			p = strtok (buf, ":");
			while (*p == ' ')	p++;	//Trim leading spaces
			if (strcmp(ifr->ifr_name, p) == 0) {
				int x;
				p = strtok ((char *) 0, " \t\r\n");
				*in = strtoul(p, NULL, 10);
				for (x = 2; x < 10; x++) p = strtok ((char *) 0, " \t\r\n");
				*out = strtoul(p, NULL, 10);
				break;
			}
		}
		fclose (f);
	}

#else	
	//This code must DIIIIIIIIIIIEEEE! DIE BITCH! DIE NOW! ROT YOU FUCKING WEASEL CODE!!!!
	//GO STRAIGHT TO HELL! DO NOT PASS GO! DIIIIIEEEE!!!!!!!!
	if (strncmp(ifr->ifr_name, "ppp", 3) != 0) {

		if (! (f = fopen ("/proc/net/dev", "r"))) {
			fclose (f);
			return 0;
		}

		while (fgets (buf, sizeof(buf), f)) {
			p = strtok (buf, ":");
			while (*p == ' ')	p++;	//Trim leading spaces
			if (strcmp(ifr->ifr_name, p) == 0) {
				int x;
				p = strtok ((char *) 0, " \t\r\n");
				*in = strtoul(p, NULL, 10) * 308;	//Same fucked up results as CMU-SNMP
				for (x = 2; x < 7; x++) p = strtok ((char *) 0, " \t\r\n");
				*out = strtoul(p, NULL, 10) * 308;
				
				break;
			}
		}

		fclose (f);
	} else {
	
		/*
		 *	Get bytes transfered
		 *	for a PPP interface.
		 */
		struct ifpppstatsreq req; 
		char *name = ifr->ifr_name;
		
		memset (&req, 0, sizeof (req));
#ifdef __linux__
		req.stats_ptr = (caddr_t) &req.stats;
#		undef ifr_name
#		define ifr_name ifr__name
#endif
		strcpy(req.ifr_name, name);
		if (ioctl(sockfd, SIOCGPPPSTATS, &req) == 0) {
			*in  = req.stats.p.ppp_ibytes;
			*out = req.stats.p.ppp_obytes;
		}
	    }
#endif

	free(ifs);

	return 0;
}



#ifdef STANDALONE //Not used in ACS - no need to fix for IPv6 now

int main(int argc, char **argv)
{
	struct in_addr ina;
	int r, in, out;
	int sockfd;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	inet_aton(argv[1], &ina);
	r = traffic_stats(sockfd, ina, &in, &out);
	printf("r = %d, in = %d, out = %d\n", r, in, out);

	return 0;
}
#endif

