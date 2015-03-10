/*
 * peekfd.c
 * 
 * Copyright (C) January, 1998 Sergei Viznyuk <sv@phystech.com>
 * 
 * This is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
/*****************************************************************************/
int peekfd(s,tv_usec)
int s;
time_t tv_usec;
{
  fd_set fs;
  struct timeval tv;
  extern int DebugFlag;

  FD_ZERO(&fs);
  FD_SET(s,&fs);
  tv.tv_sec=tv_usec/1000000;
  tv.tv_usec=tv_usec%1000000;
  if ( select(s+1,&fs,NULL,NULL,&tv) == -1 ) {
  	if (DebugFlag) syslog(4,"peekfd s=%d time=%d %m",s, tv_usec/1000000);
	return -1;
  }	  
  if ( FD_ISSET(s,&fs) ) {
  	if (DebugFlag) syslog(4,"peekfd s=%d time=%d OK",s, tv_usec/1000000);
        return 0;
  }
  if (DebugFlag) syslog(4,"peekfd s=%d time=%d timeout",s, tv_usec/1000000);
  return 1;
}
