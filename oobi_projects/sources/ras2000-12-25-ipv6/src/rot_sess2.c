#include <time.h>
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
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <termios.h>
#include <termio.h>

#include "server.h"

int count_sessions_tst (int port_num)
{
	int sess_count = 0;
	CY_SESS *sess = cy_shm->sess_list[port_num];
	CY_SESS *sess_aux;
	
	while (sess) {
		if (sess->vet_pid) {
			if (kill(sess->vet_pid, 0) < 0 && errno == ESRCH) {
				//this process has been killed;
				sess_aux = sess;
				sess = sess->next;
				free_session_entry(sess_aux);
				continue;
    		} else {
				if (sess->vet_pid != getpid()) {
					sess_count++;
				}
			}
		}
		//this process has not initiated a session yet (1st menu)
		sess = sess->next;
	}
	return (sess_count);
}

CY_SESS *alloc_session_entry (int port, int flag, char *username)
{
	CY_SESS *sess = NULL, *last = NULL;
	char id = 0;

	shm_signal(CY_SEM_SHM); //[RK] lock access to shm buffers lists 

	sess = cy_shm->sess_list[port];
	while (sess) {
		if (id == sess->id) {
			if (++id >= 127) {
				shm_unsignal(CY_SEM_SHM); //[RK]Nov/26/03
				return(NULL);
			}
			sess = cy_shm->sess_list[port];
		} else {
			sess = sess->next;
		}
	}

	sess = cy_shm->sess_list[port];
	while (sess) {
		//Look for the last session in the list
		last = sess;
		sess = sess->next;
	}

	if (!(sess = (CY_SESS *)shm_malloc(sizeof (CY_SESS), TYPE_SHM_SESS))) {
		nsyslog(LOG_WARNING,"No shared memory available for this session");
		shm_unsignal(CY_SEM_SHM);
		return(NULL);
	}
	sess->prev = last;
	sess->next = NULL;

	if (last) {
		last->next = sess;
	} else {
		cy_shm->sess_list[port] = sess;
	}

	sess->vet_pid = getpid();
	sess->tty_user = shm_strdup(username);
	sess->sess_flag = flag;
	sess->sendbreak = 0;
	sess->id = id;
	sess->port = port;
	InitBuffer(&sess->InBuffer);
	InitBuffer(&sess->OutBuffer);

	shm_unsignal(CY_SEM_SHM); //[RK] unlock access to shm buffers lists 

	return(sess);
}

void free_session_entry (CY_SESS *sess)
{
	CY_SESS *next, *prev, *saux;
	
	shm_signal(CY_SEM_SHM); //[RK] lock access to shm buffers lists 
	prev = sess->prev;
	next = sess->next;

    //mp[Sep2003] update multiuser notification status
	if (cy_shm->line_conf[sess->port].multiuser_notif && (prev || next)) {
		if (prev) {
			saux = prev;
			while (saux) {
				saux->sess_flag &= ~CY_SESS_FLAG_MUNTF_M;
				saux->sess_flag |= CY_SESS_FLAG_MUNTF_OUT;
				saux = saux->prev;
			}
		}
		if (next) {
			saux = next;
			while (saux) {
				saux->sess_flag &= ~CY_SESS_FLAG_MUNTF_M;
				saux->sess_flag |= CY_SESS_FLAG_MUNTF_OUT;
				saux = saux->next;
			}
		}
	}
	//mp[Nov2003] reset data buffer user logs control
	if (cy_shm->line_conf[sess->port].DB_user_logs) {
		if (prev || next) {
			if (prev)
				saux = prev;
			else
				saux = next;
			saux->user_away = shm_strdup(sess->tty_user);
			saux->time_user_left = time(0);
		} else {
			cy_shm->tty_user[sess->port] = shm_strdup(sess->tty_user);
			cy_shm->st_time[sess->port] = time(0);
		}
	}

	if (prev) {
		prev->next = next;
	} else {
		cy_shm->sess_list[sess->port] = next;
	}
	if (next) {
		next->prev = prev;
	}
	if (sess->user_away) {
		shm_free_str(sess->user_away);
	}
	if (sess->tty_user) {
		shm_free_str(sess->tty_user);
	}
	shm_free_str((char *)sess);
	shm_unsignal(CY_SEM_SHM); //[RK] unlock access to shm buffers lists 
	
	return;
}
