/*
 * utmp.c	Write system and radius utmp files.
 *
 * Version:	#(#)utmp.c  1.00  12-Nov-1997  miquels@cistron.nl
 *
 */
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <utmp.h>
#include "server.h"

int update_utmp(char *login_fmt, char *from_fmt, struct auth *ai, int wtmp) {
	struct utmp tmp, *utmptr;
	FILE *fp;
	pid_t pid;
	char tmp_str [256];

	pid = getpid();
	/*
	 *	Init should have setup this for us, so we
	 *	only update the entry if it exists (and only
	 *	at login - init does logout for us).
	 */
	setutent();
	while((utmptr = getutent()) != NULL) {
		if (utmptr->ut_pid == pid) {
			break;
		}
	}
	if (utmptr) {
		tmp = *utmptr;
	} else {
		memset(&tmp, 0, sizeof(tmp));
		tmp.ut_pid = pid;
	}

	expand_format (tmp_str, sizeof (tmp_str), login_fmt, ai);
	strncpy(tmp.ut_user, tmp_str, UT_NAMESIZE);
	expand_format (tmp_str, sizeof (tmp_str), from_fmt, ai);
	strncpy (tmp.ut_host, tmp_str, UT_HOSTSIZE);

	//[RK]Dec/13/02 Bug#129
	if (ai->authenticated)
		tmp.ut_type = USER_PROCESS;
	else
		tmp.ut_type = LOGIN_PROCESS;

	tmp.ut_time = time(NULL);
	strncpy(tmp.ut_line, get_tty_text(&lineconf[ai->nasport]), UT_LINESIZE);

	// IPv6 support
	if (ai->address.ss_family == AF_INET) {
		tmp.ut_addr = ((struct sockaddr_in *)&ai->address)->sin_addr.s_addr;
	} else if (ai->address.ss_family == AF_INET6) {
		memcpy(tmp.ut_addr_v6, ((struct sockaddr_in6 *)&ai->address)->sin6_addr.s6_addr32, sizeof(tmp.ut_addr_v6));
	}

	/*if (utmptr) */ setutent();

	pututline(&tmp);
	endutent();
	if (wtmp && (fp = fopen(WTMP_FILE, "a")) != NULL) {
		fwrite(&tmp, sizeof(tmp), 1, fp);
		fclose(fp);
	}

	return 0;
}

int clear_utmp_entry(pid_t pid) {
	struct utmp tmp, *utmptr;

	/*  busybox init is not doing logout for us.
	 *  That's why I created this function(), to be able
	 *  to set the user as logged out in utmp. This will fix wrong
	 *  number of users in w.  [FW]Oct/25/02 Bug #40
	 */
	setutent();
	while((utmptr = getutent()) != NULL) {
		if (utmptr->ut_pid == pid) {
			break;
		}
	}
	if (utmptr && (utmptr->ut_pid == pid)) {
		tmp = *utmptr;
		tmp.ut_type = DEAD_PROCESS;
		tmp.ut_time = time(NULL);
		memset(tmp.ut_user,0,UT_NAMESIZE);
		memset(tmp.ut_line,0,UT_LINESIZE);
		memset(tmp.ut_host,0,UT_HOSTSIZE);
		pututline(&tmp);
	} 

	endutent();
	return 0;
}
