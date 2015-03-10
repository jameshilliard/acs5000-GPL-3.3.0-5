/*****************************************************************
 * File: config_lock.c			
 *
 * Copyright (C) 2003 Cyclades Corporation
 *
 * Maintainer: www.cyclades.com
 *	
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version. 
 *
 * Description: Lock/Unlock configuration process.
 *
 ****************************************************************/

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
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "server.h"
#include "config_lock.h"

// It must have a text for each value in enum CY_CNF_APPL
static char cnf_sess_text[4][8] = {"none", "CLI", "WEB", "OSD"};

/*----------------------------------------------------------------------*/
/* Unlocks the configuration.											*/
/*----------------------------------------------------------------------*/

void unlock_config_session(int sid)
{
	shm_signal(CY_SEM_CNF_SESSION);
    if (cy_shm->cnf_session.pid == getpid() &&
    	cy_shm->cnf_session.sid == sid) {
		memset(&cy_shm->cnf_session, 0, sizeof(CY_CONFIG_SESSION));
    }
	shm_unsignal(CY_SEM_CNF_SESSION);
}

/*----------------------------------------------------------------------*/
/* Locks the configuration for the user <username>.						*/
/*----------------------------------------------------------------------*/

int lock_config_session(int appl, int sid, char *username)
{
	int rtn=1;

	shm_signal(CY_SEM_CNF_SESSION);

	if (cy_shm->cnf_session.appl) {
		if ((kill(cy_shm->cnf_session.pid,0) == -1) &&
		    (errno == ESRCH)) {
			rtn = 0;
		}
	} else {
		rtn = 0;
	}

	if (rtn == 0) {
		cy_shm->cnf_session.appl = appl;
		cy_shm->cnf_session.sid = sid;
		cy_shm->cnf_session.pid = getpid();
		strcpy(cy_shm->cnf_session.username, username);
	}

	shm_unsignal(CY_SEM_CNF_SESSION);
	return(rtn);
}

/*----------------------------------------------------------------------*/
/* Switches web user configuration session.								*/
/*----------------------------------------------------------------------*/

void change_config_session(int appl, int sid, char *username)
{
	shm_signal(CY_SEM_CNF_SESSION);
	cy_shm->cnf_session.appl = appl;
	cy_shm->cnf_session.sid = sid;
	cy_shm->cnf_session.pid = getpid();
	strcpy(cy_shm->cnf_session.username, username);
	shm_unsignal(CY_SEM_CNF_SESSION);
}

/*----------------------------------------------------------------------*/
/* Sends a signal USR1 to a configuration session and waits <wait> sec	*/
/* for a signal USR2 (answer). If wait is zero it sends a signal KILL	*/
/* and clear the lock session struct.									*/
/*																		*/
/* PS. The caller must handle the signal USR1 and USR2 and and sets		*/
/*     the parameter sig_int when they are received.					*/
/*																		*/
/*----------------------------------------------------------------------*/

int cancel_config_session(int wait, volatile int *sig_int, CY_CONFIG_SESSION *sess)
{
	shm_signal(CY_SEM_CNF_SESSION);
	memcpy(sess, &cy_shm->cnf_session, sizeof(CY_CONFIG_SESSION));

	if (!wait) {
		if (cy_shm->cnf_session.pid) {
			kill(cy_shm->cnf_session.pid, SIGKILL);
		}
		goto exit_ok;
	}

	if (cy_shm->cnf_session.pid) {
		cy_shm->cnf_session.new_pid = getpid();
		if (kill(cy_shm->cnf_session.pid, SIGUSRKILL)  < 0) {
			if (errno != ESRCH) {
				shm_unsignal(CY_SEM_CNF_SESSION);
				return(1);
			}
			goto exit_ok;
		}
	} else {
		goto exit_ok;
	}

	while (wait--) {
		if (*sig_int & 2) {
			goto exit_ok;
		}
		if (*sig_int & 1) {
			shm_unsignal(CY_SEM_CNF_SESSION);
			return(2);
		}
		sleep(1);
	}

	shm_unsignal(CY_SEM_CNF_SESSION);
	return(1);

exit_ok:

	memset(&cy_shm->cnf_session, 0, sizeof(CY_CONFIG_SESSION));
	shm_unsignal(CY_SEM_CNF_SESSION);
	return(0);
}

/*----------------------------------------------------------------------*/
/* Sends a signal SIGUSRACK to the other admin process					*/
/*----------------------------------------------------------------------*/

void send_signal_admin(void)
{
	if (cy_shm->cnf_session.new_pid) {
		kill(cy_shm->cnf_session.new_pid, SIGUSRACK);
	}
}

/*----------------------------------------------------------------------*/
/* Gets the configuration session type description (CLI, WEB, etc)		*/
/*----------------------------------------------------------------------*/

char *get_cnf_session_text(CY_CNF_APPL appl)
{
	if (appl < CNF_APPL_NONE || appl > CNF_APPL_OSD) {
		appl = CNF_APPL_NONE;
	}

	return(cnf_sess_text[appl]);
}

