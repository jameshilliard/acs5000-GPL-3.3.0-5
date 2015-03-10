/*
 * Copyright (c) 1983, 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)syslog.c	8.4 (Berkeley) 3/18/94";
#endif /* LIBC_SCCS and not lint */

/*
 * SYSLOG -- print message on log file
 *
 * This routine looks a lot like printf, except that it outputs to the
 * log file instead of the standard output.  Also:
 *	adds a timestamp,
 *	prints the module name in front of the message,
 *	has some other formatting types (or will sometime),
 *	adds a newline on the end of the message.
 *
 * The output of this routine is intended to be read by syslogd(8).
 *
 * Author: Eric Allman
 * Modified to use UNIX domain IPC by Ralph Campbell
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/signal.h>

#include <sys/syslog.h>

#include <sys/uio.h>
#include <sys/wait.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <paths.h>
#include <stdio.h>

/* On libc5 writev() is not defined in sys/uio.h */

#ifndef _SYS_UIO_H
extern int writev(int fd, const struct iovec *vector, size_t count);
#endif


static int	LogFile = -1;		/* fd for log */
static int	connected;		/* have done connect */
static int	LogStat = 0;		/* status bits, set by openlog() */
static const char *LogTag = "syslog";	/* string to tag the entry with */
static int	LogFacility = LOG_USER;	/* default facility code */
static int	LogMask = 0xff;		/* mask of priorities to be logged */
static int	ConMask = 0xff;		/* mask of priorities to be logged */
static int	BlockOpen = 0;		/* Block 2nd openlog()	*/

#ifndef USE_SYSLOG_SERVER
static struct sockaddr SyslogAddr;	/* AF_UNIX address of local logger */
static unsigned int RemSyslogAddr;	/* Remote IP address of logger. */
#endif

void nsyslog(int pri, const char *fmt, ...);

#ifndef USE_SYSLOG_SERVER
void nvsyslog(int pri, const char *fmt, va_list ap);
int nsetlogmask(int pmask);
#endif
void nopenlog(const char *ident, int logstat, int logfac);
void ncloselog(void);

# ifndef USE_SYSLOG_SERVER
void
syslog(int pri, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	nvsyslog(pri, fmt, ap);
	va_end(ap);
}

void
vsyslog(int pri, const char *fmt, va_list ap)
{
	nvsyslog(pri, fmt, ap);
}

void
openlog(const char *ident, int logstat, int logfac)
{
	nopenlog(ident, logstat, logfac);
}

void
closelog(void)
{
	ncloselog();
}

int
setlogmask(int pmask)
{
	return (nsetlogmask(pmask));
}
# endif // USE_SYSLOG_SERVER

static void 
closelog_intern(int to_default)
{
# ifndef USE_SYSLOG_SERVER
	if (LogFile != -1)
		(void) close(LogFile);
#else
	closelog();
#endif
	LogFile = -1;
	connected = 0;
	if (to_default)
	{
		LogStat = 0;
		LogTag = "syslog";
		LogFacility = LOG_USER;
		LogMask = 0xff;
		ConMask = 0xff;
	}
}

/*
 * syslog, vsyslog --
 *     print message on log file; output is intended for syslogd(8).
 */
void nsyslog(int pri, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	openlog(LogTag, 0 /*LOG_PID*/, LogFacility); 
	vsyslog(pri, fmt, ap);
	va_end(ap);
}

#ifndef USE_SYSLOG_SERVER
void
nvsyslog(pri, fmt, ap)
	int pri;
	const char *fmt;
	va_list ap;
{
	register int cnt;
	register char *p, *p1;
	time_t now;
	int fd, saved_errno;
	char tbuf[2048], fmt_cpy[1024],*stdp = 0;

	saved_errno = errno;

	/* See if we should just throw out this message. */
#ifndef CYCLADES
	if (!(LogMask & LOG_MASK(LOG_PRI(pri))) ||
		(pri &~ (LOG_PRIMASK|LOG_FACMASK)))
#else
	if (!((LogMask & LOG_MASK(LOG_PRI(pri)))  ||
		  (ConMask & LOG_MASK(LOG_PRI(pri)))) || 
		(pri &~ (LOG_PRIMASK|LOG_FACMASK)))
#endif
	{
		return;
	}

	if (LogFile < 0 || !connected)
		nopenlog(LogTag, LogStat | LOG_NDELAY, 0);

	/* Set default facility if none specified. */
	if ((pri & LOG_FACMASK) == 0)
		pri |= LogFacility;

	/* Build the message. */
	(void)time(&now);
	(void)snprintf(tbuf, sizeof(tbuf), "<%d>%.15s ", pri, ctime(&now) + 4);
	for (p = tbuf; *p; ++p);
	if (LogStat & LOG_PERROR)
		stdp = p;
	if (LogTag) {
		(void)strcpy(p, LogTag);
		for (; *p; ++p);
	}
	if (LogStat & LOG_PID) {
		(void)snprintf(p, sizeof(tbuf)-(p-tbuf), "[%d]", getpid());
		for (; *p; ++p);
	}
	if (LogTag) {
		*p++ = ':';
		*p++ = ' ';
	}

	/* Substitute error message for %m. */
	{
		register char ch, *t1;
		char *strerror();

		/* We have to make sure we don't overrun fmt_cpy. */
		for (t1 = fmt_cpy; (ch = *fmt) != '\0' && t1<fmt_cpy+sizeof(fmt_cpy); ++fmt)
			if (ch == '%' && fmt[1] == 'm') {
				++fmt;
				t1 += snprintf(t1, sizeof(fmt_cpy)-(t1-fmt_cpy),
					       "%s", strerror(saved_errno));
			}
			else
				*t1++ = ch;
		*t1 = '\0';
	}

	p += vsnprintf(p, sizeof(tbuf)-(p-tbuf), fmt_cpy, ap);
	cnt = p - tbuf;

	/* Output to stderr if requested. */
	if (LogStat & LOG_PERROR) {
		struct iovec iov[2];
		register struct iovec *v = iov;

		v->iov_base = stdp;
		v->iov_len = cnt - (stdp - tbuf);
		++v;
		v->iov_base = "\n";
		v->iov_len = 1;
		(void)writev(STDERR_FILENO, iov, 2);
	}

#ifndef CYCLADES
	/* Output the message to the local logger. */
	/* Use NUL as a message delimiter. */
	if (write(LogFile, tbuf, cnt + 1) >= 1)
	{
		return;
	}
	else if (!RemSyslogAddr)
	{
		/* If the write fails, we try to reconnect it next
		 * time. */
		closelog_intern (0);
	}

	/*
	 * Output the message to the console; don't worry about blocking,
	 * if console blocks everything will.  Make sure the error reported
	 * is the one from the syslogd failure.
	 */
	/* should mode be `O_WRONLY | O_NOCTTY' ? -- Uli */
	if (LogStat & LOG_CONS &&
	    (fd = open(_PATH_CONSOLE, O_WRONLY, 0)) >= 0) {
		(void)strcat(tbuf, "\r\n");
		cnt += 2;
		p = index(tbuf, '>') + 1;
		(void)write(fd, p, cnt - (p - tbuf));
		(void)close(fd);
	}
#else
	/* Output the message to the local logger. */
	/* Use NUL as a message delimiter. */
	if (LogMask & LOG_MASK(LOG_PRI(pri))) {
		if (write(LogFile, tbuf, cnt + 1) <= 0) {
			if (!RemSyslogAddr) {
				/* If the write fails, we try to reconnect it next time. */
				closelog_intern (0);
			}
		}
	}
	/*
	 * Output the message to the console; don't worry about blocking,
	 * if console blocks everything will.  Make sure the error reported
	 * is the one from the syslogd failure.
	 */
	/* should mode be `O_WRONLY | O_NOCTTY' ? -- Uli */
	if ((LogStat & LOG_CONS) &&
	    (ConMask & LOG_MASK(LOG_PRI(pri))) &&
	    (fd = open(_PATH_CONSOLE, O_WRONLY, 0)) >= 0) {
		(void)strcat(tbuf, "\r\n");
		cnt += 2;
		p = index(tbuf, '>') + 1;
		(void)write(fd, p, cnt - (p - tbuf));
		(void)close(fd);
	}
#endif
}

#endif

/*
 * OPENLOG -- open system log
 */
void
nopenlog(ident, logstat, logfac)
	const char *ident;
	int logstat, logfac;
{
# ifndef USE_SYSLOG_SERVER
	struct sockaddr_in *sin = (struct sockaddr_in *)&SyslogAddr;
#endif

	if (LogFile >= 0 && BlockOpen)
		return;

	if (ident != NULL)
		LogTag = ident;
	LogStat = logstat;
	if (logfac != 0 && (logfac &~ LOG_FACMASK) == 0)
		LogFacility = logfac;

# ifndef USE_SYSLOG_SERVER
	if (LogFile == -1) {
		if (RemSyslogAddr) {
			sin->sin_family = AF_INET;
			sin->sin_port   = htons(514);
			sin->sin_addr.s_addr = RemSyslogAddr;
			if (LogStat & LOG_NDELAY) {
		        	if ((LogFile = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
					return;
			}
		} else {
			SyslogAddr.sa_family = AF_UNIX;
			(void)strncpy(SyslogAddr.sa_data, _PATH_LOG,
			    sizeof(SyslogAddr.sa_data));
			if (LogStat & LOG_NDELAY) {
		        	if ((LogFile = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
					return;
			}
		}
	}
	if (LogFile != -1 && !connected) {
		if (SyslogAddr.sa_family == AF_UNIX &&
		    connect(LogFile, &SyslogAddr, sizeof(SyslogAddr) -
			sizeof(SyslogAddr.sa_data) +
			strlen(SyslogAddr.sa_data)) != -1)
				connected = 1;
		if (SyslogAddr.sa_family == AF_INET &&
		    connect(LogFile, &SyslogAddr, sizeof(SyslogAddr)) != -1)
				connected = 1;
	}
#else
	LogFile = 1;
	connected = 1;
	openlog(LogTag, 0 /*LOG_PID*/, LogFacility); 
#endif
}

/*
 * CLOSELOG -- close the system log
 */
void
ncloselog()
{
	closelog_intern(1);
}

#ifndef USE_SYSLOG_SERVER
/*
 * SETLOGMASK -- set the log mask level
 */
int
nsetlogmask(pmask)
	int pmask;
{
#ifdef CYCLADES
	LogMask = pmask;
#else
	int omask;

	omask = LogMask;
	if (pmask != 0)
		LogMask = pmask;
	return (omask);
#endif
}

#ifdef CYCLADES
int
nsetconmask(cmask)
    int cmask;
{
    ConMask = cmask;
}
#endif

/*
 * SETLOGREMOTE -- set the remote system we log at.
 */
void setlogremote(unsigned int ipaddr, int block_open)
{
	closelog_intern(0);
	BlockOpen = block_open;
	RemSyslogAddr = ipaddr;
}

#endif
