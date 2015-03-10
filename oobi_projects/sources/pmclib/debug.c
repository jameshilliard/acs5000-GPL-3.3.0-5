/*****************************************************************
* File: debug.c
*
* Copyright (C) 2006 Avocent Corporation
*
* www.avocent.com
*
* Description:
*     This file describes the routines used to trace, log and debug
*     the PMD-NG packet.
*
* Date: 05/31/2006
* Author: Helio Fujimoto <helio.fujimoto@avocent.com>
*
****************************************************************/
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <syslog.h>
#include <fcntl.h>

#include <debug.h>

static int debugFd = -1;

#ifdef DEBUG
void pmesg(unsigned long level, int line, const char *func, char* format, ...) 
{
        va_list args;
	char string_to_print[4096];
        if ( level == debugLevel || debugLevel == 0xFF) { 
       		va_start(args, format);
       		vsprintf(string_to_print, format, args);
       		va_end(args);
		if (level == TRACE) {
			debugprintf(string_to_print);
		} else {
			printf("(%04X)%s[%d] - %s\n", (unsigned int)level, func, line, string_to_print);
			//syslog(LOG_DEBUG, "(%04X)%s[%d] - %s", level, func, line, string_to_print);
		}
	}
}
#endif

/****************************************************************************
 * debugprintf
 * Parameters: 
 *     fmt - string format + arguments
 * Returns: 
 *     1 if message was logged; 0 if it wasn't.
 * Description:
 *     This routine sends a log to the console.
 ****************************************************************************/
void debugprintf(const char *fmt, ...)
{
	char buf[1024];
	va_list ap;

	if (debugFd < 0) {
		debugFd = open("/dev/console",O_WRONLY,0);
		if (debugFd < 0) {
			return;
		}
	}
	va_start(ap,fmt);
	vsnprintf(buf,sizeof(buf),fmt,ap);
//	write(debugFd,buf,strlen(buf));
	printf(buf);
	va_end(ap);
}

/****************************************************************************
 * traceprintf
 * Parameters: 
 *     header - header string
 *     name - IPDU/device name string
 *     buf - pointer to buffer
 *     size - size of buffer
 * Description:
 *     This routine sends trace message to the console.
 ****************************************************************************/
void traceprintf (const char *header, char *name, char *buf, int size)
{
	unsigned char ch;
	int i, j;

	debugprintf("%s[%s]\n", header, name);

	i = 0;
	while (1) {
		for (j = 0; j < 16; j ++) {
			if (i * 16 + j < size) {
				debugprintf("%02x ", buf[i * 16 + j]);
			} else {
				debugprintf("   ");
			}
		}
		debugprintf("   ");
		for (j = 0; j < 16; j ++) {
			if (i * 16 + j >= size) {
				break;
			}
			ch = buf[i * 16 + j];
			if (ch >= 0x20 && ch <= 0x7e) {
				debugprintf("%c", ch);
			} else {
				debugprintf(".");
			}
		}
		debugprintf("\n");
		i ++;
		if (i * 16 >= size) {
			break;
		}
	}
}
