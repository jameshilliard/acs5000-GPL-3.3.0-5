/*****************************************************************
 * File: rot_buffer.c
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
 * Description: Used by cy_buffering. Contains 
 *              functions used for buffering.
 *
*****************************************************************/

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
#include <time.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

# define __ROT_BUFFER__
#include "server.h"
#include "xwras.h" /* for XML functions */

#ifdef IPv6enable
#define EVENT_CONF_FILE   "/etc/event_notif.conf"
#endif

LOG_DATA_BUFFER *curr_log = NULL;

int fd_buf = -1;
#ifdef DLSenable
int pipe_fd[MAXLINES];
#endif

void trunc_file(int fd, int lpos, int rpos, struct line_cfg *pconf)
{
	int cnt, wpos = 0, w;
	char buffer[1024];

	while (lpos > rpos) {
		w = lseek(fd, rpos, SEEK_SET);
		if ((cnt = read(fd, buffer, sizeof(buffer))) <= 0) {
			break;
		}
		w = lseek(fd, wpos, SEEK_SET);
		if (write(fd, buffer, cnt) != cnt) {
			nsyslog(LOG_ERR,"Unable to rewrite data buffering file for %s, ignoring : %m", pconf->tty);
		}
		rpos += cnt;
		wpos += cnt;
	}
	ftruncate(fd, wpos);
}

static int check_write(int fd, char *buffer, int cnt, char *ident, int port)
{
	int npos;

	if (fd != -1) {
		npos = write(fd, buffer, cnt);

		if (npos != cnt) {
			if (cy_shm->DB_status & DB_STATUS_NFS_ACTIVE) {
				nsyslog(LOG_ERR,
						"Data buffering (%s), NFS writing error : %m", ident);
				if (npos < 0) npos = 0;
				if (curr_log->read_buf)
					curr_log->read_buf += npos;
				else
					curr_log->read_buf = npos + 1;
				shm_signal(CY_SEM_NFS);
				cy_shm->DB_status |= DB_STATUS_NFS_ERR;
				shm_unsignal(CY_SEM_NFS);

				//if (npos) handle_dls_buffer(buffer, npos, port);
				//we don't need this because if DLS, then we are in "line" mode and
				//check_write() will be called starting from &curr_log->buffer[0] again...

				return(1);
			}
			nsyslog(LOG_ERR,
					"Couldn't write data buffering file (%s), ignoring : %m", ident);
		}
	}

#ifdef DLSenable
	if (pipe_fd[port] != -1)
		handle_dls_buffer(buffer, cnt, port);
#endif

	return(0);
}

void check_file_size(int fd, char *buffer, int cnt, int flag, struct line_cfg *pconf, int port)
{
	int fpos=0, npos, str, str_sz, msg_sz, max_msg_sz = 254;  //[GY]2006/May/04
	char *p, *p1 = NULL, tbuf[40], ident[64], aux[max_msg_sz+1];  //[GY]2006/May/08
	time_t now;
	struct tm *tz;

	int fac;	//[RK] ALARM facility
	CY_SESS *sess = cy_shm->sess_list[port];

	fac = LOG_FAC(LOG_LOCAL0) + mainconf.DB_facility;


	if (fd != -1) {
		if ((fpos = lseek(fd, 0, SEEK_END)) < 0) {
			return;
		}
	} else { //[RK]Dec/26/02 Bug#156 - 
		if (!pconf->syslog_buffering && !pconf->alarm
#ifdef WINEMSenable
			&& !pconf->xml_monitor
#endif
#ifdef DLSenable
			&& !(lineconf[port].DB_mode == DLS_mode)
#endif
		) {
			return;
		}
	}

	if (curr_log == NULL) {
		return;
	}

	if (pconf->alias && *pconf->alias)
		snprintf(ident, sizeof (ident), "S%d.%s",
			pconf->this+1, pconf->alias);
	else
		snprintf(ident, sizeof (ident), "S%d.%s",
			pconf->this+1, pconf->tty);

	if (buffer && cnt) {
		memcpy(&curr_log->buffer[curr_log->tot_buf], buffer, cnt);
	}

	p = &curr_log->buffer[0];

#ifdef WINEMSenable
	if (pconf->xml_monitor) {
		if (find_xw(&curr_log->buffer[curr_log->tot_buf],cnt)) {
			syslog(LOG_MAKEPRI(fac,LOG_INFO),"XML_MONITOR(%s)\r\n%s", 
				ident,curr_log->xml_buffer);
		}
	}
#endif

	if (curr_log->tot_buf && curr_log->read_buf) {
		buffer = &curr_log->buffer[curr_log->read_buf - 1];
	} else {
		buffer = &curr_log->buffer[curr_log->tot_buf];
		curr_log->tot_buf += cnt;
	}

	if (!pconf->data_buffering || (cy_shm->DB_status & DB_STATUS_NFS_ACTIVE) ||
#ifdef DB_ENH
	    (pconf->data_buffering_sess && sess ) || //[RK]Feb/13/06 - disable data_buffering
#endif
		(flag & 2) || (fd == -1)) { //[RK]Dec/26/02 Bug#156
		goto do_flush;
	}

	if ((cnt + fpos) > pconf->data_buffering) {
		if (cnt > pconf->data_buffering) {
			buffer += cnt - pconf->data_buffering;
			cnt = pconf->data_buffering;
		}
		npos = cnt + fpos - pconf->data_buffering * 9 / 10;

		if (cnt > npos) {
			npos = cnt;
		}
		if (flag && !pconf->syslog_buffering) {
			nsyslog(LOG_WARNING, "%s %d data bytes discarded", ident,
				(npos < fpos ? npos : fpos));
		}
		trunc_file(fd, fpos, npos, pconf); 
	}

do_flush:

	if (curr_log->tot_buf == 0) {
		return;
	}

	if (!pconf->DB_timestamp && !pconf->alarm 
#ifdef WINEMSenable
		&& !pconf->xml_monitor
#endif
#ifdef DLSenable
		&& !(lineconf[port].DB_mode & DLS_mode)
#endif
		) { //[RK]ALARM
		                              //check DLS and DLS_LOCAL
		if (cnt && pconf->data_buffering && (fd != -1)  
#ifdef DB_ENH
		    //[RK]Feb/13/06 - disable data_buffering when there is one session
                    && ((pconf->data_buffering_sess==0) || 
                     (pconf->data_buffering_sess==1 && !sess ))
#endif
		   ) {
			if (check_write(fd, buffer, cnt, ident, port)) return;
		}

		if (pconf->syslog_buffering && ((flag & 2) ||
			curr_log->tot_buf >= pconf->syslog_buffering)) {
			curr_log->buffer[curr_log->tot_buf] = 0;
			// Do not generate syslog messages when syslog_sess=1 and
			// there is no session to the port sending data.
			if (pconf->syslog_sess==0 || (pconf->syslog_sess==1 && !sess)) {
				nsyslog(LOG_NOTICE, "%s [%s]", ident, p);
			}
			curr_log->tot_buf = 0;
		}

		if (!pconf->syslog_buffering)
			curr_log->tot_buf = 0;

		curr_log->read_buf = 0;
		return;
	}

	time(&now);
	tz = localtime(&now);
	snprintf(tbuf, sizeof(tbuf), "<%.3s/%02d %02d:%02d %s>",
		&"JanFebMarAprMayJunJulAugSepOctNovDec"[tz->tm_mon*3], tz->tm_mday,
		tz->tm_hour > 12 ? tz->tm_hour - 12 : tz->tm_hour, tz->tm_min,
		tz->tm_hour >= 12 ? "pm" : "am");

	for( ; cnt; ) {
		p1 = buffer;
		cnt--;
		buffer++;

		if (*p1 != '\n' && *p1 != '\r') {
			continue;
		}

		while (cnt && (*buffer == '\n' || *buffer == '\r')) {
			++buffer;
			--cnt;
		}

		npos = buffer - p;

		if ((fd != -1)
#ifdef DLSenable
			 || (lineconf[port].DB_mode == DLS_mode)
#endif
		){ //[RK]ALARM
#ifdef DB_ENH
			//[RK]Feb/13/06 - disable data_buffering when there is one session
			if ((pconf->data_buffering_sess == 0) ||
 			    (pconf->data_buffering_sess ==1 && !sess)) {
#endif
				if (pconf->DB_timestamp &&
					check_write(fd, tbuf, strlen(tbuf), ident, port)) {
					return;
				}
				if (check_write(fd, p, npos, ident, port)) {
					return;
				}
#ifdef DB_ENH
			}
#endif
		}
		if( (pconf->alarm == 1 || pconf->alarm == 3) && (npos) ) { //[RK]ALARM  [GY]2006/Apr/26
			*p1 = 0;
			if (p != p1) {
				syslog(LOG_MAKEPRI(fac,LOG_INFO),"ALARM(%s) [%s]", ident,p);
			}
		}
		if (pconf->syslog_buffering && (p1 - p)) {
			*p1 = 0;
			// Do not generate syslog messages when syslog_sess=1 and
			// there is no session to the port sending data.
			if (pconf->syslog_sess==0 || (pconf->syslog_sess==1 && !sess)) {
				nsyslog(LOG_NOTICE, "%s [%s]", ident, p);
			}
		}
                //[GY]2006/May/03
#ifdef ALERTSenable
                if( (pconf->alarm == 2 || pconf->alarm == 3) && ((p1 - p) >= ALERT_STRINGS_MIN_CHAR) && snmp_pipe_flag ) {
                   *p1 = 0;
                   aux[0] = 0;
                   for( str = 0; str < ALERT_STRINGS_QUANTITY; str++ ) {
                      if( (str_sz = strlen(curr_log->alert_strings[str])) ) {
                         if( strstr(p, curr_log->alert_strings[str]) ) {
                            if( (msg_sz = (strlen(p))) > max_msg_sz ) {
                               strncpy(aux, p, max_msg_sz);
                               aux[max_msg_sz] = 0;
                               msg_sz = max_msg_sz;
                            }
                            //mount buffer and write to pipe
                            fprintf(snmp_pipe_FP, "EVT8 %ld %d %s %d %d %d %s %d %s \n",
                                                 (long int)now,
                                                 ((pconf->alias && *pconf->alias)?strlen(pconf->alias):strlen(pconf->tty)),
                                                 ((pconf->alias && *pconf->alias)?pconf->alias:pconf->tty),
                                                 pconf->this+1,
                                                 str+1,
                                                 str_sz,
                                                 curr_log->alert_strings[str],
                                                 msg_sz,
                                                 (strlen(aux))?aux:p);
                            aux[0] = 0;
                         }
                      }
                   }
                }
#endif

		p = buffer;
		curr_log->tot_buf -= npos;
		curr_log->read_buf = 0;

	}

	if (curr_log->tot_buf) {
		if ((flag & 2) || //[RK] ALARM
		    (curr_log->tot_buf >= (MAX_BUFFER - 1))) {
			if ((fd != -1)
#ifdef DLSenable
				|| (lineconf[port].DB_mode == DLS_mode)
#endif
			) { //[RK]ALARM
#ifdef DB_ENH
				//[RK]Feb/13/06 - disable data_buffering when there is one session
				if ((pconf->data_buffering_sess == 0) ||
 				    (pconf->data_buffering_sess ==1 && !sess)) {
#endif
					if (pconf->DB_timestamp &&
						check_write(fd, tbuf, strlen(tbuf), ident, port)) {
						return;
					}
					if (check_write(fd, p, curr_log->tot_buf, ident, port)) {
						return;
					}
#ifdef DB_ENH
				}
#endif
			}

			if( pconf->alarm == 1 || pconf->alarm == 3 ) { //[RK]ALARM  [GY]2006/Apr/26
				p[curr_log->tot_buf] = 0;
				syslog(LOG_MAKEPRI(fac,LOG_INFO),"ALARM(%s) [%s]", ident,p);
			}

			if (pconf->syslog_buffering) {
				p[curr_log->tot_buf] = 0;
				// Do not generate syslog messages when syslog_sess=1 and
				// there is no session to the port sending data.
				if (pconf->syslog_sess==0 || (pconf->syslog_sess==1 && !sess)) {
					nsyslog(LOG_NOTICE, "%s [%s]", ident, p);
				}
			}
                        //[GY]2006/May/04
#ifdef ALERTSenable
                        if( (pconf->alarm == 2 || pconf->alarm == 3) && ((p1 - p) >= ALERT_STRINGS_MIN_CHAR) && snmp_pipe_flag ) {
                           p[curr_log->tot_buf] = 0;
                           aux[0] = 0;
                           for( str = 0; str < ALERT_STRINGS_QUANTITY; str++ ) {
                              if( (str_sz = strlen(curr_log->alert_strings[str])) ) {
                                 if( strstr(p, curr_log->alert_strings[str]) ) {
                                    if( (msg_sz = (strlen(p))) >= max_msg_sz ) {
                                       strncpy(aux, p, max_msg_sz);
                                       aux[max_msg_sz] = 0;
                                       msg_sz = max_msg_sz;
                                    }
                                    //mount buffer and write to pipe
                                    fprintf(snmp_pipe_FP, "EVT8 %ld %d %s %d %d %d %s %d %s \n",
                                                         (long int)now,
                                                         ((pconf->alias && *pconf->alias)?strlen(pconf->alias):strlen(pconf->tty)),
                                                         ((pconf->alias && *pconf->alias)?pconf->alias:pconf->tty),
                                                         pconf->this+1,
                                                         str+1,
                                                         str_sz,
                                                         curr_log->alert_strings[str],
                                                         msg_sz,
                                                         (strlen(aux))?aux:p);
                                    aux[0] = 0;
                                 }
                              }
                           }
                        }
#endif
			curr_log->tot_buf = 0;
		} else {
			if (p != curr_log->buffer)
				memcpy(curr_log->buffer, p, curr_log->tot_buf);
		}
		curr_log->read_buf = 0;
	}
}

//[RK]Nov/18/02 Bug#77 - write in data_buffering file doesn't block.
#define OPEN_FLAG (O_RDWR|O_CREAT|O_NONBLOCK)	
#define PERM_FLAG (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)

int open_data_buffering(struct line_cfg *pconf)
{
	int fd = -1;
	char FileName[64];

#ifdef DLSenable
	if (!(pconf->DB_mode == DLS_mode)) 
#endif
	{
		if (pconf->alias && *pconf->alias) {
			snprintf(FileName, sizeof(FileName), "%s/%s.data", cy_shm->DB_path,
					 pconf->alias);
		} else {
			snprintf(FileName, sizeof(FileName), "%s/%s.data", cy_shm->DB_path,
					 pconf->tty);
		}

		umask(0);

		if ((fd = open(FileName, OPEN_FLAG, PERM_FLAG)) < 0) {
			nsyslog(LOG_ERR,"Unable to create/open file (%s) %m", FileName);
			return(-1);
		}

		lseek(fd, 0, SEEK_END);
	}

#ifdef DB_ENH
	//[RK]Feb/23/06 - when dont_show_dbmenu_op is 4, more is not used
	if (pconf->dont_show_db_op == 4) {
		terminal_lines = -1;
	}
#endif

	return(fd);
} 

int check_buffering_retention_mode(struct line_cfg *pconf, LOG_DATA_BUFFER *lbuf, CY_SESS *sess)
{
	int rtn = 0;

	if ((pconf->protocol != P_SOCKET_SERVER &&
	     pconf->protocol != P_SOCKET_SERVER_SSH &&
#ifdef BIDIRECT
	     pconf->protocol != P_TELNET_BIDIRECT &&
#endif
	     pconf->protocol != P_SOCKET_SERVER_RAW &&
		 pconf->protocol != P_SOCKET_SSH) ||
		(!pconf->data_buffering && !pconf->alarm)) { //[RK]ALARM
		fd_buf = -1;
		goto exit_rot;
	}

	if (pconf->data_buffering) { //[RK]ALARM
		if ((fd_buf = open_data_buffering(pconf)) < 0) {
			fd_buf = -1;
			goto exit_rot;
		}
		rtn = lseek(fd_buf, 0, SEEK_END);
	}

	curr_log = lbuf;

	lbuf->tot_buf = 0;

exit_rot:
	if (sess) {
		if (rtn)
			sess->sess_flag |= CY_SESS_FLAG_DBMENU;
		else
			sess->sess_flag &= ~CY_SESS_FLAG_DBMENU;
	}

	return(rtn);
}


/*
 * More: display the buffered data under user control
 * The two method parameters are
 * 	rmethod: routine that gets one character from the user
 * 	wmethod: routine that writes one line to the user
 */	

# define LINE_SIZE	512
# define DEFAULT_LINES	24

void
more (int filefd, int devfd, int (*rmethod)(), void (*wmethod)())
{
	int 	linecnt = 2;	/* Initial header */
	int		opt;
	char	*linebuf;
	int		quit = 0;

	if ((linebuf = (char *)malloc (LINE_SIZE)) == (char *)NULL) {
			nsyslog(LOG_ERR,"More: not enough memory available");
			return;
	}

	if (!terminal_lines) terminal_lines = DEFAULT_LINES;

	while (!quit && get_a_line(filefd, linebuf, 512) > 0) {
		(*wmethod)(linebuf);	/* write to the connection */

#ifdef DB_ENH
		if (terminal_lines == -1) //[RK]Feb/23/06
			continue;
#endif

		if (++linecnt == terminal_lines - 1) { /* [more] */
			(*wmethod)("\r\n[more]");
			do {
				/* read a char from the connection */
				switch (opt = (*rmethod)(0)) {
				case -1:
				case 'q':
				case 'Q':
					quit = 1;
					break;
				case ' ':
					linecnt = 1;
					break;
				case '\r':
					linecnt--;
					break;
				default:
					opt = 0;
					break;
				}
			} while (!opt);
			(*wmethod)("\r      \r");
		}
	}
	free ((void *)linebuf);
}

# define FILEBUF_SIZE	512
static struct minibuf {
	char	*buf;
	int		hold;
	int		off;
} mb;

int
get_a_line (int fd, char *area, int size)
{
	char *	cp;
	int	linesize;

	if (!mb.buf) {
			if ((mb.buf = (char *) malloc (FILEBUF_SIZE+4)) == (char *)NULL) {
				nsyslog(LOG_ERR,"More: not enough memory available");
					return (0);
			}
	}
    cp = &mb.buf[mb.off];
	linesize = 0;
	while (size--) {
		if (mb.off == mb.hold) {
			if ((mb.hold = read (fd, mb.buf, FILEBUF_SIZE)) <= 0) {
				mb.hold = mb.off = 0;
				break;
			}
			mb.off = 0;
       		cp = mb.buf;
		}
		*area++ = *cp;
		linesize++;
		mb.off++; 
		if (*cp == '\n') break;
		cp++;
	}
	*area = 0;
	return (linesize? 1: 0);
}

#ifdef IPv6enable
//---------------------------------------------------------------------
//  FUNCTION: readLocalAddress
//
//  DESCRIPTION: Read the local address to connect to DSVIEW from
//               EVENT_CONF_FILE.
//
//  PARAMETERS: local_address
//
//  RETURNS:  0 = local_address read
//           -1 = error reading local_address
//
//  NOTES:
//
//---------------------------------------------------------------------
static int readLocalAddress(char *local_address)
{
	FILE *fd;
	char *line, *pli, *pe;
	int i, ret = -1;

	// default value
	local_address[0] = 0;

	// open file
	if ((fd = fopen(EVENT_CONF_FILE, "r")) == NULL) {
		return(ret);
	}

	// allocate memory for one line with 80 chars
	line = calloc(81, 1);

	// read a line
	while (fgets(line, 81, fd) != NULL) {

		if (line[0] == '#') continue;

		if (strncmp("localAddress", line, 12) == 0) {
			pe = strstr(line,"=");
			if (!pe) continue;
			pe++;
			while (*pe && (isspace(*pe))) pe++;
			pli=pe;
			while (*pe && !(isspace(*pe))) pe++;
			if (*pe) *pe=0x00;
			strcpy(local_address, pli);
			ret = 0;
			break;
		}
	}

	free(line);
	fclose (fd);
	
	return(ret);
}
#endif


#ifdef DLSenable
void handle_dls_buffer(char* buffer, int cnt, int port)
{
	char header[DLS_MAX_HLEN], *boxip, ipbuf[INET6_ADDRSTRLEN];
	struct tm *t;
	int header_len = 0, pipe_cnt, orig_cnt, i;
	//struct in_addr bip; IPv6
	struct sockaddr_storage bip;
	time_t timenow;

	if (!cnt) { //called due timeout
		if (!curr_log->dls_buffer_pos)
			return;
		else
			goto write_to_pipe;
	}
	if (!buffer) return;

#ifdef IPv6enable
	if (!readLocalAddress(ipbuf)) {
		boxip = ipbuf;
	} else if (mainconf.ipno6.ss_family == AF_INET6) {
		memcpy(&bip, &mainconf.ipno6, sizeof(bip));
		boxip = dotted6((struct sockaddr *)&bip, ipbuf, sizeof(ipbuf));
	} else
#endif
	{
		((struct sockaddr_in *)&bip)->sin_family = AF_INET;
		((struct sockaddr_in *)&bip)->sin_addr.s_addr = mainconf.ipno;
		boxip = dotted6((struct sockaddr *)&bip, ipbuf, sizeof(ipbuf));
	}
	
	timenow = time(NULL);
	t = gmtime(&timenow);

	if ((header_len = snprintf(header, DLS_MAX_HLEN,
							   "<%d> %04d-%02d-%02dT%02d:%02d:%02dZ %s DLS[%d]: ",
							   DLS_PRI, 1900+t->tm_year, t->tm_mon+1, t->tm_mday,
							   t->tm_hour, t->tm_min, t->tm_sec, boxip, port+1))
		>= DLS_MAX_HLEN)
		return; //shouldn't ever occur

	//replace sequences of "\r\n..." by a single "\n"
	while (cnt-2 >= 0 && (buffer[cnt-2]=='\r' || buffer[cnt-2]=='\n'))
		cnt--;
	if (buffer[cnt-1] == '\r')
		buffer[cnt-1] = '\n';

	//skip sequences of "\r\n..." at the beginning of the buffer
	orig_cnt = cnt;
	for (i=0; (cnt && (buffer[i]=='\r' || buffer[i]=='\n')); i++, cnt--);
	if (i == orig_cnt)
		return;
	if (i)
		buffer += i;

	//check if dls_buffer should be consided as full
	if ((MAX_DLS_BUFFER - curr_log->dls_buffer_pos) < (cnt + header_len + 1/*possible '\n'*/)) {
write_to_pipe:
		pipe_cnt = write(pipe_fd[port], curr_log->dls_buffer, curr_log->dls_buffer_pos);
		if (pipe_cnt < 0) {
//			nsyslog(LOG_ERR, "Error writing to dls_pipe%d: %m", port+1);
			pipe_cnt = 0;
		}
		curr_log->dls_buffer_pos -= pipe_cnt;

		if (curr_log->dls_buffer_pos) { //couldn't write all dls_buffer
			curr_log->dls_status |= DLS_ST_PIPEFULL;

			//shifting what remains to the beginning of dls_buffer
			if (pipe_cnt)
				memcpy(curr_log->dls_buffer,
					   &curr_log->dls_buffer[pipe_cnt], curr_log->dls_buffer_pos);

			if (!cnt) return;

			if ((MAX_DLS_BUFFER - curr_log->dls_buffer_pos) < (cnt + header_len + 1/*possible '\n'*/)) {
				if (!curr_log->msgtimer) {
					nsyslog(LOG_ERR, "Couldn't save data log to dls_buffer%d, ignoring", port+1);
					curr_log->msgtimer = time(NULL);
				} else if ((time(NULL) - curr_log->msgtimer) >= 300) {
					curr_log->msgtimer = 0;
				}
				return;
			}
		}

		if (!cnt) {
			curr_log->dls_status &= ~DLS_ST_PIPEFULL; //signals that dls_pipe is OK
			return;
		}
	}

	//save new data to be logged to dls_buffer
	memcpy(&curr_log->dls_buffer[curr_log->dls_buffer_pos], header, header_len);
	curr_log->dls_buffer_pos += header_len;
	memcpy(&curr_log->dls_buffer[curr_log->dls_buffer_pos], buffer, cnt);
	curr_log->dls_buffer_pos += cnt;
	if (buffer[cnt-1] != '\n') {
		//sorry, serial port line will be split into
		//two log lines
		curr_log->dls_buffer[curr_log->dls_buffer_pos] = '\n';
		curr_log->dls_buffer_pos ++;
	}

	return;		
}
#endif
