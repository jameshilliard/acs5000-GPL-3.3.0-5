/*
 * getty.c	This is the part that talks with the modem,
 *		does the login stuff etc.
 *
 * Version:	@(#)getty.c  1.34  13-Jan-1998  MvS.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <termios.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "server.h"

static char buf[128];

//static struct MsgBuf msg_buf;

#ifndef CYCLADES
/*
 *  Get the lockfile.
 */
static int lockfile(char *lock, size_t lock_size, char *lockdir, char *tty, int
location) {
    struct stat st;

    /*
     *  First some checks to see if we do want to
     *  use a lockdir and if it exists.
     */
    if (lockdir == NULL || lockdir[0] != '/') {
        *lock = 0;
        return -1;
    }
    if (stat(lockdir, &st) < 0 || !S_ISDIR(st.st_mode)) {
        *lock = 0;
        return -1;
    }
    if (tty[0] == '/') tty += 5;
    if (snprintf(lock, lock_size, "%s/%s..%s", lockdir,
        (location == 1 ? "BUF" : "LCK"), tty) == -1) {
        *lock = 0;
        return (-1);
    }
    return 0;
}
#endif

char *term_in_use(struct line_cfg *lc, int location)
{
#ifdef CYCLADES
	int	cnt;
	CY_SESS *first_rw, *first_sniff;
#else
	char lock[MAXPATHLEN+1];
#endif

#ifndef CYCLADES
	lockfile(lock, sizeof (lock),mainconf.lockdir, lc->tty, location);

	while (dolock(lock) == 1) {
		while(islocked(lock)) {
			sleep(10);
		}
		sleep(1);
	}
	return (NULL);
#else
	if (lc->virtual) {
		//always accept connection to a virtual port
		return(NULL);
	}

	if (lc->fast) { //[RK]May/22/03 - Performance improvement
		if (!cy_shm->vet_pid[lc->this]) {
			return(NULL);
		}
		if ((kill(cy_shm->vet_pid[lc->this],0) < 0) && (errno == ESRCH)) {
			return(NULL);
		}
		cnt = snprintf(buf, sizeof(buf), 
			"\r\n*\r\n* * * %s is being used by (%s) ", get_tty_text(lc),
			cy_shm->tty_user[lc->this] ? 
				cy_shm->tty_user[lc->this]:"none");
		snprintf(&buf[cnt], sizeof(buf) - cnt, "!!!\r\n*\r\n");
		return(buf);
	}

	if (!count_sessions(lc->this)) {
		//There is no active session for this port
		return(NULL);
	}

	first_rw = first_rw_session(lc->this);
	first_sniff = first_sniff_session(lc->this);

	//[RK]Oct/25/02 BUG#43
	if (first_rw->tty_user && strcmp(first_rw->tty_user,"NONE")) {
		cnt = snprintf(buf, sizeof(buf), 
			"\r\n*\r\n* * * %s is being used by (%s) ", get_tty_text(lc),
			first_rw->tty_user);
	} else {
		cnt = snprintf(buf, sizeof(buf), 
			"\r\n*\r\n* * * %s is being used by (pid=%d) ", get_tty_text(lc),
			first_rw->vet_pid);
	}

	if (first_sniff) {
		//[RK]Oct/25/02 BUG#43
		if (first_sniff->tty_user && strcmp(first_sniff->tty_user,"NONE")) {
			cnt += snprintf(&buf[cnt], sizeof(buf) - cnt, 
			"and viewed by (%s) ", first_sniff->tty_user);
		} else {
			cnt += snprintf(&buf[cnt], sizeof(buf) - cnt, 
			"and viewed by (pid=%d) ", first_sniff->vet_pid);
		}
	}
	snprintf(&buf[cnt], sizeof(buf) - cnt, "!!!\r\n*\r\n");
	return(buf);
#endif
}
