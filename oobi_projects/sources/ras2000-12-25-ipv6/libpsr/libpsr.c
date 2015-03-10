/*
 * libpsr.c	LibPortSlaveRadius.
 *		Shared library to be linked with pppd-2.2.0f
 *		Takes care of the portslave side of things.
 *
 * Version	@(#)libpsr.c 1.03  09-Jan-1998  miquels@cistron.nl
 *
 */

typedef unsigned short u16;
typedef unsigned char u8;

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

#include <pppd.h>
#include <fsm.h>
#include <lcp.h>
#include <ipcp.h>

#include <../pslave_cfg.h>
#include <server.h>

#ifdef CBCPenable
#ifdef CYCLADES
#define CBCP_SERVER_SUPPORT
#include <cbcp.h>
#undef CBCP_SERVER_SUPPORT
#endif
#endif

// These are necessary to deal with sockaddr structures
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

//[RK]May/12/06 - event configuration
extern void writeevt(int, char* , ...);

static struct auth thisauth;
struct auth *pai = &thisauth;

extern int clear_utmp_entry(pid_t pid);
extern int setipaddr(char *, char **, int);

#ifndef CYCLADES
static int rad_logged_in;
#else
int ul_pap_check_hook (void)	//[RK]Dec/30/02 Bug#163 
{
	return(1);
}

int socket_tty_conv(void)
{
	return(3);
}
#endif

int ul_login(char *, char *, char **, struct wordlist **, struct wordlist **);
void ul_logout (void);
void ul_ppp_ipup (void);
void ul_ppp_ipdown (void);
void rad_make_wordlist (u_int32_t, struct wordlist **);

int plugin_init (void) {

	nsyslog (LOG_DEBUG, "ul_init () called");
	pap_auth_hook = ul_login;
	pap_logout_hook = ul_logout;
#ifdef CYCLADES
	pap_check_hook = ul_pap_check_hook;	//[RK]Dec/30/02 Bug#163
#endif
	ip_up_hook = ul_ppp_ipup;
	ip_down_hook = ul_ppp_ipdown;
#ifndef CYCLADES
	rad_logged_in = 0;
#endif
	return 0;
}

int ul_login(char *user, char *passwd, char **msg, struct wordlist **addrs,
    struct wordlist **opts) {
	char tmp[255];
	char *s;

#ifdef IPv6enable
	if (rad_init (-2, &thisauth, devnam, 0, 0) < 0)
#else
	if (rad_init (-2, &thisauth, devnam, 0) < 0)
#endif
	{
		exit(1);
	}

	nsyslog(LOG_DEBUG, "ul_login(%c) called", thisauth.proto);

	if ((s = getenv ("CONNECT_INFO")) != NULL) {
		strcpy (thisauth.conn_info, s);
	}

	snprintf (thisauth.login, sizeof (thisauth.login), "%s", user);
	snprintf (thisauth.passwd, sizeof (thisauth.passwd), "%s", passwd);
#ifdef CYCLADES
	if (do_local_or_server_authentication(&thisauth, 1)) {
		return (0);
	}
	update_utmp("%L",lineconf[thisauth.nasport].utmp_from, 
		&thisauth, lineconf[thisauth.nasport].syswtmp); //[RK]Dec/26/02 Bug#129 - utmp
	//[RK]Jun/12/096
	writeevt(5,"ssii",thisauth.login, lineconf[thisauth.nasport].alias, (thisauth.nasport + 1), 5);
	if (thisauth.next_token) {
		return(0);
	}
#else
	if (rad_client (&thisauth, 1) < 0) {
		return (0);
	}
#endif
//[RK]Jun/12/06 - port from tslinux_mv21
//	if (thisauth.proto != P_PPP && thisauth.proto == P_PPP_ONLY) {
//		return (0);
//	}

#ifdef CBCPenable
#ifdef CYCLADES
	/* CallBack Server */
	if (lcp_hisoptions[0].neg_cbcp) {	/* callback negociated in LCP */
		cbcp_protent.enabled_flag = 1;
		cbcp[0].us_cbcp = 1;	/* callback server */
		if (thisauth.sess_type) { /* callback session - set by Radius Server */
			if (thisauth.callback_number[0]) {
				/* callback number is defined by administrator */
				cbcp[0].us_type = (1 << CB_CONF_ADMIN);
				cbcp[0].us_number = thisauth.callback_number;
			} else {
				cbcp[0].us_type = (1 << CB_CONF_USER);
			}
		} else {
			/* reset callback, it is a normal session to Radius Server */
			cbcp[0].us_type = (1 << CB_CONF_NO);
		}
	}
#endif
#endif
	
	if (thisauth.netmask && thisauth.netmask != 0xFFFFFFFF) {
		netmask = thisauth.netmask;
	}
	if (thisauth.mtu) {
		lcp_allowoptions[0].mru = thisauth.mtu;
	}
	if (thisauth.mru) {
		lcp_wantoptions[0].mru = thisauth.mru;
		lcp_wantoptions[0].neg_mru = 1;
	}
	strcpy (tmp, dotted (thisauth.localip));
	strcat (tmp, ":");
	//TODO - check how to deal with IPv6
	if (((struct sockaddr_in *)(&thisauth.address))->sin_addr.s_addr != 0xFFFFFFFF) {
		strcat(tmp, dotted (((struct sockaddr_in *)&thisauth.address)->sin_addr.s_addr));
		rad_make_wordlist (((struct sockaddr_in *)&thisauth.address)->sin_addr.s_addr, addrs);
	} else {
		*addrs = NULL;
	}
	if (setipaddr(tmp,NULL,1) < 0) {
		nsyslog (LOG_ERR, "bad IP address %s", tmp);
		return (0);
	}
	*opts = NULL;
	if (thisauth.idletime > 0) {
		idle_time_limit = thisauth.idletime;
	}
	if (thisauth.sessiontime > 0) {
		maxconnect = thisauth.sessiontime;
	}
	setenv ("LOGNAME", user, 1);
#ifndef CYCLADES
	rad_acct (&thisauth, 1);
	rad_logged_in = 1;
#else
	if (thisauth.do_acct) {
		rad_acct (&thisauth, 1);
	}
#endif
	nsyslog (LOG_NOTICE, "user %s logged in", user);
	return (1);
}

void ul_logout (void) {
	nsyslog (LOG_DEBUG, "ul_logout () called");
#ifdef CYCLADES
	do_pam_close();
	nsyslog (LOG_NOTICE, "user %s logged out", thisauth.login);
	clear_utmp_entry(getpid());	//[RK]Dec/26/02 Bug#129 - utmp 
	//[RK]Jun/12/096
	writeevt(6,"ssii",thisauth.login, lineconf[thisauth.nasport].alias, (thisauth.nasport + 1), 5);
#else
	if (rad_logged_in) {
		rad_acct(&thisauth, 0);
		rad_logged_in = 0;
		nsyslog (LOG_NOTICE, "user %s logged out", thisauth.login);
	}
#endif
}

void ul_ppp_ipup (void) {
	nsyslog (LOG_DEBUG, "ul_ipup () called");
	update_framed_route(&thisauth, 1); 
}

void ul_ppp_ipdown (void) {
	struct pppd_stats io_stats;
	
	nsyslog (LOG_DEBUG, "ul_ppp_ifdown () called");
	get_ppp_stats (ifunit, &io_stats);
	if (io_stats.bytes_out) {
		thisauth.sent_bytes = io_stats.bytes_out;
	}
	if (io_stats.bytes_in) {
		thisauth.recv_bytes = io_stats.bytes_in;
	}
}

void rad_make_wordlist (u_int32_t addr, struct wordlist **addrs) {
	char *ipaddr;
	struct wordlist *ap;

	ipaddr = ip_ntoa (addr);
	ap = (struct wordlist *) malloc(sizeof(struct wordlist) + strlen(ipaddr) + 1);
	if (ap == NULL) {
		nsyslog (LOG_DEBUG, "rad_make_wordlist: NOMEM: authorized addresses");
	}
	ap->word = (char *) (ap + 1);   
	ap->next = NULL;
	strcpy(ap->word, ipaddr);
	*addrs = ap;
}
