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
#include <syslog.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "rot_inetd.h"

static void check_shm(void)
{
	if (cy_shm != (CY_SHM *) -1)
		return;

	if (shm_init(-1) < 0) {
		exit(-1);
	}

}

/**
 *  Check whether the connection should be accepted: 
 *  it returns 0 if so, or a non-zero value otherwise.
 */
int check_telnet_ip(struct sockaddr_storage *sa)
{
	int i;
	int iplocal = 0, tcp_port = 0;

	check_shm();

	if (sa->ss_family == AF_INET6) {
		struct sockaddr_in6 *sa6 = (struct sockaddr_in6 *) sa;

		if (IN6_IS_ADDR_V4MAPPED(sa6->sin6_addr.s6_addr)) {
			syslog(LOG_INFO, "Telnet to [%u.%u.%u.%u] (IPv4-mapped).",
			       (unsigned) sa6->sin6_addr.s6_addr[12],
			       (unsigned) sa6->sin6_addr.s6_addr[13],
			       (unsigned) sa6->sin6_addr.s6_addr[14],
			       (unsigned) sa6->sin6_addr.s6_addr[15]);

			iplocal = sa6->sin6_addr.s6_addr32[3];
		} else {
#ifndef IPv6enable
			/**
			 *  Accept the connection as there is no possibility
			 *  of having an IPv6 address assigned to a port.
			 */
			return 0;
#endif
			/** IF IPv6enable
			 * Search IPv6 address assigned to port. 
			 */
		}
		tcp_port = sa6->sin6_port;
	} else if (sa->ss_family == AF_INET) {
		struct sockaddr_in *sa4 = (struct sockaddr_in *) sa;

		iplocal = sa4->sin_addr.s_addr;
		tcp_port = sa4->sin_port;
	} else {
		/* Unknown type: do not accept the connection. */
		return 1;
	}

	for (i=0; i < MAXVLINES; i++) {
		if (lineconf[i].this != i ||
			lineconf[i].tty == NULL ||
			*lineconf[i].tty == 0) {
			continue;
		}

		if (
#ifdef IPv6enable
			ipaddr_cmp(&lineconf[i].ipno6, sa) &&
			ipaddr_cmp(&lineconf[i].pool_ipno6, sa) &&
			
#endif
			(!iplocal || lineconf[i].ipno != iplocal) &&
			(!iplocal || lineconf[i].pool_ipno != iplocal)) {
			continue;
		}


		if (lineconf[i].protocol == P_SOCKET_SSH) {
			return(1);
		}

		if ((lineconf[i].protocol == P_SOCKET_SERVER ||
                    lineconf[i].protocol == P_SOCKET_SERVER_SSH ||
#ifdef BIDIRECT
                    lineconf[i].protocol == P_TELNET_BIDIRECT ||
#endif
		    lineconf[i].protocol == P_SOCKET_SERVER_RAW) &&
			tcp_port != 23 && tcp_port != lineconf[i].socket_port) {
			return(1);
		}

#ifdef PMD
		//[RK]Jun/22/04 - SSH/Telnet to IPDU
		if ((lineconf[i].protocol == P_IPDU) &&
		    (!(lineconf[i].pmsessions & PMSESS_TELNET) ||
		     (tcp_port != 23 && tcp_port != lineconf[i].socket_port))) {
			return(1);
		}
#endif

		return(0);
	}
	return(0);
}

static char txt_tty[INET6_ADDRSTRLEN];
static char txt_exe[16] = "/bin/socket";

int check_telnet_ip_serial(struct sockaddr_storage *sa, char **server, char **argv)
{
	int i;
	int iplocal = 0, tcp_port = 0;

	check_shm();

	if (sa->ss_family == AF_INET6) {
		struct sockaddr_in6 *sa6 = (struct sockaddr_in6 *) sa;

		if (IN6_IS_ADDR_V4MAPPED(sa6->sin6_addr.s6_addr)) {
			iplocal = sa6->sin6_addr.s6_addr32[3];
		} else {
#ifndef IPv6enable
			return 1;
#endif
			/** IF IPv6enable
			 * Search IPv6 address assigned to port. 
			 */
		}
		tcp_port = sa6->sin6_port;
	} else if (sa->ss_family == AF_INET) {
		struct sockaddr_in *sa4 = (struct sockaddr_in *) sa;

		iplocal = sa4->sin_addr.s_addr;
		tcp_port = sa4->sin_port;
	} else {
		return 1;
	}

	for (i=0; i < MAXVLINES; i++) {
		if (lineconf[i].this != i ||
			lineconf[i].tty == NULL ||
			*lineconf[i].tty == 0) {
			continue;
		}

		if (lineconf[i].protocol != P_SOCKET_SERVER &&
			lineconf[i].protocol != P_SOCKET_SERVER_SSH &&
#ifdef BIDIRECT
			lineconf[i].protocol != P_TELNET_BIDIRECT &&
#endif
			lineconf[i].protocol != P_SOCKET_SERVER_RAW) {
		       //[RK]June/03/04 - SSH/Telnet to IPDU
#ifdef PMD
		    	if ((lineconf[i].protocol != P_IPDU) || !(lineconf[i].pmsessions & PMSESS_TELNET)) 
#endif
				continue;
		}

#ifdef IPv6enable
		if (!ipaddr_cmp(&lineconf[i].ipno6, sa)) {
			sprintf(txt_tty, "%d", i);
			*server = txt_exe;
			argv[0] = txt_exe;
			argv[1] = txt_tty;
			argv[2] = txt_exe;
			argv[3] = NULL;
			return(0);
		}
		
		if (!ipaddr_cmp(&lineconf[i].pool_ipno6, sa)) {
			dotted6((struct sockaddr *)sa, txt_tty, INET6_ADDRSTRLEN);
			*server = txt_exe;
			argv[0] = txt_exe;
			argv[1] = txt_tty;
			argv[2] = txt_exe;
			argv[3] = NULL;
			return(0);
		}
#endif

		if (iplocal && lineconf[i].ipno == iplocal) {
			sprintf(txt_tty, "%d", i);
			*server = txt_exe;
			argv[0] = txt_exe;
			argv[1] = txt_tty;
			argv[2] = txt_exe;
			argv[3] = NULL;
			return(0);
		}

		if (iplocal && lineconf[i].pool_ipno == iplocal) {
			sprintf(txt_tty, "%s", dotted(iplocal));
			*server = txt_exe;
			argv[0] = txt_exe;
			argv[1] = txt_tty;
			argv[2] = txt_exe;
			argv[3] = NULL;
			return(0);
		}
	}

	return(1);
}

static int next_port = MAXVLINES;
static int next_pool = 0;

static char inetd_line[128];

char *get_telnet_port(int max_con, int from)
{
	check_shm();

	for ( ; next_port < MAXVLINES; next_port++) {
		if (lineconf[next_port].this != next_port ||
			lineconf[next_port].tty == NULL ||
			*lineconf[next_port].tty == 0) {
			continue;
		}

		if ((lineconf[next_port].protocol != P_SOCKET_SERVER &&
		     lineconf[next_port].protocol != P_SOCKET_SERVER_SSH &&
#ifdef BIDIRECT
		     lineconf[next_port].protocol != P_TELNET_BIDIRECT &&
#endif
		     lineconf[next_port].protocol != P_SOCKET_SERVER_RAW) ||
		     (lineconf[next_port].virtual && mainconf.nat_clustering_ip)) {
#ifdef PMD
			//[RK]June/03/04
			if ((lineconf[next_port].protocol != P_IPDU) || !(lineconf[next_port].pmsessions & PMSESS_TELNET))
#endif
				continue;
		}

		if (from == 1) {
			if (lineconf[next_port].socket_port == 0) {
				continue;
			}
			snprintf(inetd_line, sizeof(inetd_line),
				"%u stream tcp nowait.%d root /bin/socket /bin/socket %d %s",
				lineconf[next_port].socket_port, max_con,
				next_port,
				lineconf[next_port].tty);

			++next_port;
			return(inetd_line);
		} else {
			if (lineconf[next_port].pool_socket_port == 0) {
				continue;
			}
			if (next_pool == lineconf[next_port].pool_socket_port) {
				continue;
			}
			next_pool = lineconf[next_port].pool_socket_port;
			snprintf(inetd_line, sizeof(inetd_line),
				"%u stream tcp nowait.%d root /bin/socket /bin/socket %d %s",
				lineconf[next_port].pool_socket_port, max_con,
				lineconf[next_port].pool_socket_port,
				lineconf[next_port].tty);

			++next_port;
			return(inetd_line);
		}
	}
	return(NULL);
}

void set_telnet_init_port(void)
{
	next_pool = 0;
	next_port = 0;
}

void set_telnet_end_port(void)
{
	next_port = MAXVLINES;
}

#ifdef CY_DLA_ALERT
// Does not return if DLA logging or event alert configured.
// Will start telnetd with extra parameters from sharewd memory.
void dla_start_telnet(char *se_server, char **se_argv, int max_argc)
{
	int argc, argi;
	int dla_args = 0;
	int alert_len = 0;
	int file_len = 0;

	check_shm();

	if (mainconf.dla) dla_args += 2;	// will add --dla N 
	if (mainconf.alert) dla_args += 2;	// will add --dla N 
	if (mainconf.alert && mainconf.alert_strings &&
		((alert_len = strlen(mainconf.alert_strings)) > 0))
		dla_args += 2;   // will add --dla_alert STR
	if (mainconf.dla_file &&
		((file_len = strlen(mainconf.dla_file)) > 0))
		dla_args += 2;   // will add --dla_alert STR

	if (!dla_args) return;	// DLA not configured. Let original telnet to run.

// Look for room to insert new args.
	for (argc = 0, argi = 0; argc < (max_argc - dla_args); argc++) {

// Keep args already included
		if (se_argv[argc]) {
			argi++;
			continue;
		}

// Add dla (logging) arguments
		if (mainconf.dla) {
			char *s_dla;

			if ((s_dla = calloc(1, 11)) == NULL) break; 
			if (snprintf(s_dla, 10, "%d", mainconf.dla) < 0) break;
			se_argv[argc++] = "--dla";
			se_argv[argc++] = s_dla;
		}

//Add alert enabled/disabled
		if (mainconf.alert) {
			char *s_alert;

			if ((s_alert = calloc(1, 11)) == NULL) break; 
			if (snprintf(s_alert, 10, "%d", mainconf.alert) < 0) break;
			se_argv[argc++] = "--dla_alert";
			se_argv[argc++] = s_alert;
		}

// Add event alerts arguments
		if (mainconf.alert && alert_len) {
			char *s_alert_str;

			if ((s_alert_str = calloc(1, alert_len+1)) == NULL) break; 
			if (snprintf(s_alert_str, alert_len, "%.512s", 
					mainconf.alert_strings) < 0) break;
			se_argv[argc++] = "--dla_alert_str";
			se_argv[argc++] = s_alert_str;
		}

// Redefine file to write log and alerts
		if (file_len) {
			char *s_file_str;

			if ((s_file_str = calloc(1, file_len+1)) == NULL) break; 
			if (snprintf(s_file_str, file_len, "%.512s", 
					mainconf.dla_file) < 0) break;
			se_argv[argc++] = "--dla_file";
			se_argv[argc++] = s_file_str;
		}
		break;
	}
	
	if (argc == argi + dla_args) {
		execv(se_server, se_argv);
	}

// Never return, end process
	exit(1);
}
#endif //CY_DLA_ALERT

