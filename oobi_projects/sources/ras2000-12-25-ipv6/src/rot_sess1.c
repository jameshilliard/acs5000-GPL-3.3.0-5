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

//function prototypes
extern int inline SpaceInBuffer(BufferType * B, int cushion);
extern void inline PutBlockInBuffer(BufferType * B, char *Cp, int count);
extern Boolean inline CharsInBuffer(BufferType * B);
extern void inline GetBlockFromBuffer(char *Cp, BufferType * B, int count);

//static struct MsgBuf msg_buf;
static int sniff_direction = 0;

//[RK]Jan/13/06 
static char flag_msg_overflow[MAXLINES];

int send_msg_rw_sess(char *buf, int cnt, int direction, int port)
{
	int i, rtn = 0;
	CY_SESS *sess = cy_shm->sess_list[port];
	int my_pid = getpid();
	int smsg=0;	//[RK] Jan/13/06 
	while (sess) {
		if (direction == 2 && (sess->sess_flag & CY_SESS_FLAG_DBMENU)) {
			sess = sess->next;
			continue;
		}
		if ((direction == 3 && sess->vet_pid == my_pid) || //Don't send msg
			(direction != 3 && sess->sess_flag == 0) ||
			(sess->vet_pid == 0) || ((sess->sess_flag & 4) && direction != 3)) {
			sess = sess->next;
			continue;
		}

		//direction: 1 - out (tcp -> tty)  sniff - IN
		//			 2 - in  (tty -> tcp)  sniff - OUT
		if ((direction == 1) && (sess->sess_flag & CY_SESS_FLAG_W)) {
			sess = sess->next;
			continue;
		}
		if ((direction == 2) && (sess->sess_flag & CY_SESS_FLAG_PMD)) {
			sess = sess->next;
			continue;
		}
        switch (lineconf[port].sniff_mode) {
        case 0:	//out
            if ((direction == 1) && !(sess->sess_flag & CY_SESS_FLAG_W)) {
                sess = sess->next;
                continue;
            }
            break;
        case 1: //in
            if ((direction == 2) && !(sess->sess_flag & CY_SESS_FLAG_W)) {
                sess = sess->next;
                continue;
            }
            break;
        }

		i = 0;
		shm_signal(port);  /* semaphore number: 0 = first port */

		if(!IsBufferFull(&sess->OutBuffer, 2) &&
			lineconf[port].sniff_mode == 2 && direction != 3 && 
			!(sess->sess_flag & CY_SESS_FLAG_W) &&
            (sniff_direction != direction)) {
            
			sniff_direction = direction;
           	PutInBuffer(&sess->OutBuffer,(sniff_direction==1 ? '>' : '<'));
		}

/*  		while (i < cnt) { */
/*  			if (IsBufferFull(&sess->OutBuffer, 2)) { */
/*  				rtn = 1; */
/*  				sess->flow_flag = 1; */
/*  				break; */
/*  			} */
/*  			PutInBuffer(&sess->OutBuffer, buf[i]); */
/*  			i++; */
/*  		} */
//mp: May, 2003 - Improve copy to Shared Memory
		if ((i = SpaceInBuffer(&sess->OutBuffer, 2)) < cnt) {
			//[RK]Jan/04/06 rtn = 1;
			sess->flow_flag = 1;
			smsg = 1; //[RK]Jan/13/06 
		} else 
			i = cnt;
		PutBlockInBuffer(&sess->OutBuffer, buf, i);
///////////

		shm_unsignal(port);  /* semaphore number: 0 = first port */

		sess = sess->next;
	}
	//[RK]Jan/13/06 
	if (smsg) {
		if (!flag_msg_overflow[port]) {
			flag_msg_overflow[port] = 1;
			nsyslog(LOG_ERR,"Session on ttyS%d has connection problem - data overflow ", port+1);
		}
	} else {
		flag_msg_overflow[port] = 0;
	}
	return(rtn);
}

void send_msg_this_sess(char *buf, int cnt, CY_SESS *sess)
{
	int i;

	if (!sess || sess->vet_pid == 0) {
		return;
	}

	i = 0;
	shm_signal(sess->port);  /* semaphore number: 0 = first port */
	
	while (i < cnt) {
		if (IsBufferFull(&sess->OutBuffer, 2)) {
			sess->flow_flag = 1;
			break;
		}
		PutInBuffer(&sess->OutBuffer, buf[i]);
		i++;
	}
	
	shm_unsignal(sess->port);  /* semaphore number: 0 = first port */

	//[RK]Jan/13/06 
	if ((i < cnt) && !flag_msg_overflow[sess->port]) {
		flag_msg_overflow[sess->port] = 1;
		nsyslog(LOG_ERR," **** Session on ttyS%d has connection problem - data overflow ", sess->port+1);
	} else {
		flag_msg_overflow[sess->port] = 0;
	}
}

int read_rw_msg(CY_SESS *sess, char *buf, int count)
{
	int cnt = 0;

	if (sess->sess_flag & 4) {
		return(0);
	}
 
	shm_signal(sess->port);  /* semaphore number: 0 = first port */

/*  	while (!IsBufferEmpty(&sess->OutBuffer) && (cnt < count)) { */
/*  		buf[cnt++] = GetFromBuffer(&sess->OutBuffer); */
/*  	} */
//mp: May, 2003 - Improve copy from Shared Memory
	if ((cnt = CharsInBuffer(&sess->OutBuffer))) {
		cnt = MIN(cnt, count);
		GetBlockFromBuffer(buf, &sess->OutBuffer, cnt);
	}

	shm_unsignal(sess->port);  /* semaphore number: 0 = first port */

	//if (sess->flow_flag && !IsBufferFull(&sess->OutBuffer, BufferSize/2)) 
	if (sess->flow_flag && !IsBufferFull(&sess->OutBuffer, BufferSize/20)) {
		sess->flow_flag = 2;
	}

	return(cnt);
}

int max_sess_free_buf(int cnt, int port)
{
	int aux;
	CY_SESS *sess = cy_shm->sess_list[port];

	while (sess) {
//		aux = GetFreeBufferCushion(&sess->OutBuffer, BufferSize/10);
		aux = GetFreeBufferCushion(&sess->OutBuffer, 10);
		//[RK]Jan/04/06 - if the sess does not have free space it will not be used.
		if (aux && (cnt > aux)) { 
			cnt = aux;
		}
		sess = sess->next;
	}

	return(cnt);

}

void set_flow_off_all_session(int port)
{
	CY_SESS *sess = cy_shm->sess_list[port];

	while (sess) {
		if (sess->flow_flag == 0) sess->flow_flag = 1;
		sess = sess->next;
	}

}

int write_rw_msg(CY_SESS * sess, char *buf, int len)
{
    int count;
    
    BufferType *pInBuffer = &sess->InBuffer;

    if ((count = MaxContiguosFree(pInBuffer)) > 0) {
        if (count > len) count = len;
        memcpy(&pInBuffer->Buffer[pInBuffer->WrPos], buf, count);
        pInBuffer->WrPos = (pInBuffer->WrPos + count) % BufferSize;
	if (cy_shm->DB_pid) { //[RK]Nov/24/03 Bugf#1075
	        kill(cy_shm->DB_pid,SIGIO);
	}
        return (count);
    }
    return (0);
}

