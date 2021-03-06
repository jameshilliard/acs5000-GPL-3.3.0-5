/*
 * Copyright (c) 1983, 1990, 1993
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

#ifndef lint
#ifndef __linux__
static
#endif
char copyright[] =
"@(#) Copyright (c) 1983, 1990, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
#ifndef __linux__
static
#endif
char sccsid[] = "@(#)rlogin.c	8.1 (Berkeley) 6/6/93 (Linux 10-Jul-1995)";
#endif /* not lint */

/*
 * rlogin - remote login
 */
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <pwd.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef TERMIOS
#include <sys/ioctl.h>
#include <termios.h>
#else
#include <sgtty.h>
#endif

#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#ifdef KERBEROS
#include <kerberosIV/des.h>
#include <kerberosIV/krb.h>

#include "krb.h"

CREDENTIALS cred;
Key_schedule schedule;
int use_kerberos = 1, doencrypt;
char dst_realm_buf[REALM_SZ], *dest_realm = NULL;
#endif

#ifndef TIOCPKT_WINDOW
#define	TIOCPKT_WINDOW	0x80
#endif

/* concession to Sun */
#ifndef SIGUSR1
#define	SIGUSR1	30
#endif

int eight, litout, rem;

int noescape;
#ifndef TERMIOS
u_char escapechar = '~';
#else
char escapechar = '~';
#endif

#ifndef TERMIOS
char *speeds[] = {
	"0", "50", "75", "110", "134", "150", "200", "300", "600", "1200",
	"1800", "2400", "4800", "9600", "19200", "38400"
};
#endif

#ifdef OLDSUN
struct winsize {
	unsigned short ws_row, ws_col;
	unsigned short ws_xpixel, ws_ypixel;
};
#else
#define	get_window_size(fd, wp)	ioctl(fd, TIOCGWINSZ, wp)
#endif
struct	winsize winsize;

#ifdef TERMIOS
char *		getspeed __P((speed_t));
#endif
void		catch_child __P((int));
void		copytochild __P((int));
void	doit __P((long));
void	done __P((int));
void		echo __P((char));
u_int		getescape __P((char *));
void		lostpeer __P((int));
void		mode __P((int));
void		msg __P((char *));
void		oob __P((int));
int		reader __P((int));
void		sendwindow __P((void));
void		setsignal __P((int));
void		sigwinch __P((int));
void		stop __P((char));
void	usage __P((void));
void		writer __P((void));
void		chunk_writer __P((void));
void		writeroob __P((int));

#ifdef	KERBEROS
void		warning __P((const char *, ...));
#endif
#ifdef OLDSUN
int		get_window_size __P((int, struct winsize *));
#endif

int
main(argc, argv)
	int argc;
	char *argv[];
{
	extern char *optarg;
	extern int optind;
	struct passwd *pw;
	struct servent *sp;
#ifdef TERMIOS
	struct termios ttyb;
#else
	struct sgttyb ttyb;
#endif
	long omask;
	int argoff, ch, dflag, one, uid;
	char *host, *p, *user, term[1024];
	char *localname = NULL;

	struct addrinfo *ai;
	struct addrinfo hints;

	argoff = dflag = 0;
	one = 1;
	host = user = NULL;

	if ((p = rindex(argv[0], '/')) != NULL)
		++p;
	else
		p = argv[0];

	if (strcmp(p, "rlogin"))
		host = p;

	/* handle "rlogin host flags" */
	if (!host && argc > 2 && argv[1][0] != '-') {
		host = argv[1];
		argoff = 1;
	}

#ifdef KERBEROS
#define	OPTIONS	"8EKLde:i:k:l:x"
#else
#define	OPTIONS	"8EKLde:i:l:"
#endif
	while ((ch = getopt(argc - argoff, argv + argoff, OPTIONS)) != EOF)
		switch(ch) {
		case '8':
			eight = 1;
			break;
		case 'E':
			noescape = 1;
			break;
		case 'K':
#ifdef KERBEROS
			use_kerberos = 0;
#endif
			break;
		case 'L':
			litout = 1;
			break;
		case 'd':
			dflag = 1;
			break;
		case 'e':
			noescape = 0;
			escapechar = getescape(optarg);
			break;
		case 'i':
			if (getuid() != 0) {
				fprintf(stderr, "rlogin: -i user: permission denied\n");
				exit(1);
			}
			localname = optarg;
			break;
#ifdef KERBEROS
		case 'k':
			dest_realm = dst_realm_buf;
			(void)strncpy(dest_realm, optarg, REALM_SZ);
			break;
#endif
		case 'l':
			user = optarg;
			break;
#ifdef CRYPT
#ifdef KERBEROS
		case 'x':
			doencrypt = 1;
			des_set_key(cred.session, schedule);
			break;
#endif
#endif
		case '?':
		default:
			usage();
		}
	optind += argoff;
	argc -= optind;
	argv += optind;

	/* if haven't gotten a host yet, do so */
	if (!host && !(host = *argv++))
		usage();

	if (*argv)
		usage();

	if (!(pw = getpwuid(uid = getuid()))) {
		(void)fprintf(stderr, "rlogin: unknown user id.\n");
		exit(1);
	}
	if (!user)
		user = pw->pw_name;

	if (!localname)
		localname = pw->pw_name;
        
	sp = NULL;
#ifdef KERBEROS
	if (use_kerberos) {
		sp = getservbyname((doencrypt ? "eklogin" : "klogin"), "tcp");
		if (sp == NULL) {
			use_kerberos = 0;
			warning("can't get entry for %s/tcp service",
			    doencrypt ? "eklogin" : "klogin");
		}
	}
#endif
	if (sp == NULL)
		sp = getservbyname("login", "tcp");
	if (sp == NULL) {
		(void)fprintf(stderr, "rlogin: login/tcp: unknown service.\n");
		exit(1);
	}

	// getaddrinfo
	memset(&hints, '\0', sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	if (getaddrinfo (host, NULL, &hints, &ai) != 0) {
		(void)fprintf(stderr, "rlogin: error in getaddrinfo\n");
		exit(1);
	}

	(void)strncpy(term, (p = getenv("TERM")) ? p : "network", sizeof(term));
#ifdef TERMIOS
	if (tcgetattr(0, &ttyb) == 0) {
		(void)strcat(term, "/");
		(void)strcat(term, getspeed(ttyb.c_cflag & CBAUD));
	}
#else
	if (ioctl(0, TIOCGETP, &ttyb) == 0) {
		(void)strcat(term, "/");
		(void)strcat(term, speeds[(int)ttyb.sg_ospeed]);
	}
#endif
	(void)get_window_size(0, &winsize);

	(void)signal(SIGPIPE, lostpeer);
	/* will use SIGUSR1 for window size hack, so hold it off */
	omask = sigblock(sigmask(SIGURG) | sigmask(SIGUSR1));
	/*
	 * We set SIGURG and SIGUSR1 below so that an
	 * incoming signal will be held pending rather than being
	 * discarded. Note that these routines will be ready to get
	 * a signal by the time that they are unblocked below.
	 */
	(void)signal(SIGURG, copytochild);
	(void)signal(SIGUSR1, writeroob);

#ifdef KERBEROS
try_connect:
	if (use_kerberos) {
		struct hostent *hp;

		/* Fully qualify hostname (needed for krb_realmofhost). */
		hp = gethostbyname(host);
		if (hp != NULL && !(host = strdup(hp->h_name))) {
			(void)fprintf(stderr, "rlogin: %s\n",
			    strerror(ENOMEM));
			exit(1);
		}

		rem = KSUCCESS;
		errno = 0;
		if (dest_realm == NULL)
			dest_realm = krb_realmofhost(host);

#ifdef CRYPT
		if (doencrypt)
			rem = krcmd_mutual(&host, sp->s_port, user, term, 0,
			    dest_realm, &cred, schedule);
		else
#endif /* CRYPT */
			rem = krcmd(&host, sp->s_port, user, term, 0,
			    dest_realm);
		if (rem < 0) {
			use_kerberos = 0;
			sp = getservbyname("login", "tcp");
			if (sp == NULL) {
				(void)fprintf(stderr,
				    "rlogin: unknown service login/tcp.\n");
				exit(1);
			}
			if (errno == ECONNREFUSED)
				warning("remote host doesn't support Kerberos");
			if (errno == ENOENT)
				warning("can't provide Kerberos auth data");
			goto try_connect;
		}
	} else {
#ifdef CRYPT
		if (doencrypt) {
			(void)fprintf(stderr,
			    "rlogin: the -x flag requires Kerberos authentication.\n");
			exit(1);
		}
#endif /* CRYPT */
		rem = rcmd(&host, sp->s_port, localname, user, term, 0);
	}
#else

#ifdef IPv6enable
	rem = rcmd_af(&host, sp->s_port, localname, user, term, 0, ai->ai_family);
#else
	rem = rcmd(&host, sp->s_port, localname, user, term, 0);
#endif

#endif /* KERBEROS */

	if (rem < 0)
		exit(1);

	if (dflag &&
	    setsockopt(rem, SOL_SOCKET, SO_DEBUG, &one, sizeof(one)) < 0)
		(void)fprintf(stderr, "rlogin: setsockopt: %s.\n",
		    strerror(errno));
	one = IPTOS_LOWDELAY;
	if (setsockopt(rem, IPPROTO_IP, IP_TOS, (char *)&one, sizeof(int)) < 0)
		perror("rlogin: setsockopt TOS (ignored)");

	(void)setuid(uid);
	doit(omask);
	/*NOTREACHED*/
	return(0);
}

int child;
char deferase, defkill;
#ifdef TERMIOS
struct termios deftty;
#else
int defflags, deflflags, tabflag;
struct tchars deftc;
struct ltchars defltc;
struct tchars notc = { -1, -1, -1, -1, -1, -1 };
struct ltchars noltc = { -1, -1, -1, -1, -1, -1 };
#endif

#ifdef TERMIOS
struct _speeds {
  speed_t code;
  int speed;
} speeds[] = {
  { B0,		0,	},
  { B50,	50,	},
  { B110,	110,	},
  { B134,	134,	},
  { B150,	150,	},
  { B200,	200,	},
  { B300,	300,	},
  { B600,	600,	},
  { B1200,	1200,	},
  { B1800,	1800,	},
  { B2400,	2400,	},
  { B4800,	4800,	},
  { B9600,	9600,	},
#ifdef B14400
  { B14400,	14400,	},
#endif
#ifdef B19200
  { B19200,	19200,	},
#endif
#ifdef B28800
  { B28800,	28800,	},
#endif
#ifdef B38400
  { B38400,	38400,	},
#endif
#ifdef B57600
  { B57600,	57600,	},
#endif
#ifdef B76800
  { B76800,	76800,	},
#endif
#ifdef B115200
  { B115200,	115200,	},
#endif
#ifdef B230400
  { B230400,	230400,	},
#endif
};

char *
getspeed(speed)
	speed_t speed;
{
	int i;
	int ret = 9600;
	static char buf[16];

	for (i = 0; i < (sizeof(speeds) / sizeof(struct _speeds)); i++)
		if (speeds[i].code == speed) {
			ret = speeds[i].speed;
			break;
		}
	sprintf(buf, "%d", ret);
	return(buf);
}
#endif /* TERMIOS */

void
doit(omask)
	long omask;
{
#ifdef TERMIOS

	(void)tcgetattr(0, &deftty);
	deferase = deftty.c_cc[VERASE];
	defkill  = deftty.c_cc[VKILL];
#else
	struct sgttyb sb;

	(void)ioctl(0, TIOCGETP, (char *)&sb);
	defflags = sb.sg_flags;
	tabflag = defflags & TBDELAY;
	defflags &= ECHO | CRMOD;
	deferase = sb.sg_erase;
	defkill = sb.sg_kill;
	(void)ioctl(0, TIOCLGET, &deflflags);
	(void)ioctl(0, TIOCGETC, &deftc);
	notc.t_startc = deftc.t_startc;
	notc.t_stopc = deftc.t_stopc;
	(void)ioctl(0, TIOCGLTC, &defltc);
#endif
	(void)signal(SIGINT, SIG_IGN);
	setsignal(SIGHUP);
	setsignal(SIGQUIT);
	child = fork();
	if (child == -1) {
		(void)fprintf(stderr, "rlogin: fork: %s.\n", strerror(errno));
		done(1);
	}
	if (child == 0) {
		mode(1);
		if (reader(omask) == 0) {
			msg("connection closed.");
			exit(0);
		}
		sleep(1);
		msg("\007connection closed.");
		exit(1);
	}

	/*
	 * We may still own the socket, and may have a pending SIGURG (or might
	 * receive one soon) that we really want to send to the reader.  When
	 * one of these comes in, the trap copytochild simply copies such
	 * signals to the child. We can now unblock SIGURG and SIGUSR1
	 * that were set above.
	 */
	(void)sigsetmask(omask);
	(void)signal(SIGCHLD, catch_child);
	if (noescape)
		chunk_writer();
	else
		writer();
	msg("closed connection.");
	done(0);
}

/* trap a signal, unless it is being ignored. */
void
setsignal(sig)
	int sig;
{
	int omask = sigblock(sigmask(sig));

	if (signal(sig, exit) == SIG_IGN)
		(void)signal(sig, SIG_IGN);
	(void)sigsetmask(omask);
}

void
done(status)
	int status;
{
	int w, wstatus;

	mode(0);
	if (child > 0) {
		/* make sure catch_child does not snap it up */
		(void)signal(SIGCHLD, SIG_DFL);
		if (kill(child, SIGKILL) >= 0)
			while ((w = wait(&wstatus)) > 0 && w != child);
	}
	exit(status);
}

int dosigwinch;

/*
 * This is called when the reader process gets the out-of-band (urgent)
 * request to turn on the window-changing protocol.
 */
void
writeroob(signo)
	int signo;
{
	if (dosigwinch == 0) {
		sendwindow();
		(void)signal(SIGWINCH, sigwinch);
	}
	dosigwinch = 1;
}

void
catch_child(signo)
	int signo;
{
	union wait status;
	int pid;

	for (;;) {
		pid = wait3((int *)&status, WNOHANG|WUNTRACED, NULL);
		if (pid == 0)
			return;
		/* if the child (reader) dies, just quit */
		if (pid < 0 || (pid == child && !WIFSTOPPED(status)))
			done((int)(status.w_termsig | status.w_retcode));
	}
	/* NOTREACHED */
}

/*
 * chunk_writer: write to remote: 0 -> line.
 * This function does not look for the escape character
 * and reads and writes in big chunks.
 */
void
chunk_writer()
{
	int i = 0, n;
	char buf[1024];
	char *bufp;

	for (;;) {
		n = read(STDIN_FILENO, buf, sizeof(buf));
		if (n <= 0) {
			if (n < 0 && errno == EINTR)
				continue;
			break;
		}
#ifdef CRYPT
#ifdef KERBEROS
		if (doencrypt) {
			if (des_write(rem, buf, n) == 0) {
				msg("line gone");
				break;
			}
			continue;
		}
#endif
#endif
		bufp = buf;
		while (n > 0) {
			i = write(rem, bufp, n);
			if (i <= 0) {
				if (errno == EINTR) continue;
				msg("line gone");
				break;
			}
			n -= i;
			bufp += i;
		}
		if (i <= 0) break;
	}
}

/*
 * writer: write to remote: 0 -> line.
 * ~.				terminate
 * ~^Z				suspend rlogin process.
 * ~<delayed-suspend char>	suspend rlogin process, but leave reader alone.
 */
void
writer()
{
	register int bol, local, n;
	char c;

	bol = 1;			/* beginning of line */
	local = 0;
	for (;;) {
		n = read(STDIN_FILENO, &c, 1);
		if (n <= 0) {
			if (n < 0 && errno == EINTR)
				continue;
			break;
		}
		/*
		 * If we're at the beginning of the line and recognize a
		 * command character, then we echo locally.  Otherwise,
		 * characters are echo'd remotely.  If the command character
		 * is doubled, this acts as a force and local echo is
		 * suppressed.
		 */
		if (bol) {
			bol = 0;
			if (!noescape && c == escapechar) {
				local = 1;
				continue;
			}
		} else if (local) {
			local = 0;
#ifdef TERMIOS
			if (c == '.' || c == (char)deftty.c_cc[VEOF]) {
#else
			if (c == '.' || c == deftc.t_eofc) {
#endif
				echo(c);
				break;
			}
#ifdef TERMIOS
			if (c == (char)deftty.c_cc[VSUSP]) {
#else
			if (c == defltc.t_suspc || c == defltc.t_dsuspc) {
#endif
				bol = 1;
				echo(c);
				stop(c);
				continue;
			}
			if (c != escapechar)
#ifdef CRYPT
#ifdef KERBEROS
				if (doencrypt)
					(void)des_write(rem,
					    (char *)&escapechar, 1);
				else
#endif
#endif
					(void)write(rem, &escapechar, 1);
		}

#ifdef CRYPT
#ifdef KERBEROS
		if (doencrypt) {
			if (des_write(rem, &c, 1) == 0) {
				msg("line gone");
				break;
			}
		} else
#endif
#endif
			if (write(rem, &c, 1) == 0) {
				msg("line gone");
				break;
			}
#ifdef TERMIOS
		bol = c == defkill || c == (char)deftty.c_cc[VEOF] ||
		    c == (char)deftty.c_cc[VINTR] ||
		    c == (char)deftty.c_cc[VSUSP] ||
		    c == '\r' || c == '\n';
#else
		bol = c == defkill || c == deftc.t_eofc ||
		    c == deftc.t_intrc || c == defltc.t_suspc ||
		    c == '\r' || c == '\n';
#endif
	}
}

void
#if __STDC__
echo(register char c)
#else
echo(c)
	register char c;
#endif
{
	register char *p;
	char buf[8];

	p = buf;
	c &= 0177;
	*p++ = escapechar;
	if (c < ' ') {
		*p++ = '^';
		*p++ = c + '@';
	} else if (c == 0177) {
		*p++ = '^';
		*p++ = '?';
	} else
		*p++ = c;
	*p++ = '\r';
	*p++ = '\n';
	(void)write(STDOUT_FILENO, buf, p - buf);
}

void
#if __STDC__
stop(char cmdc)
#else
stop(cmdc)
	char cmdc;
#endif
{
	mode(0);
	(void)signal(SIGCHLD, SIG_IGN);
#ifdef TERMIOS
	(void)kill(cmdc == (char)deftty.c_cc[VSUSP] ? 0 : getpid(), SIGTSTP);
#else
	(void)kill(cmdc == (char)defltc.t_suspc ? 0 : getpid(), SIGTSTP);
#endif
	(void)signal(SIGCHLD, catch_child);
	mode(1);
	sigwinch(0);			/* check for size changes */
}

void
sigwinch(signo)
	int signo;
{
	struct winsize ws;

	if (dosigwinch && get_window_size(0, &ws) == 0 &&
	    bcmp(&ws, &winsize, sizeof(ws))) {
		winsize = ws;
		sendwindow();
	}
}

/*
 * Send the window size to the server via the magic escape
 */
void
sendwindow()
{
	struct winsize *wp;
	char obuf[4 + sizeof (struct winsize)];

	wp = (struct winsize *)(obuf+4);
	obuf[0] = 0377;
	obuf[1] = 0377;
	obuf[2] = 's';
	obuf[3] = 's';
	wp->ws_row = htons(winsize.ws_row);
	wp->ws_col = htons(winsize.ws_col);
	wp->ws_xpixel = htons(winsize.ws_xpixel);
	wp->ws_ypixel = htons(winsize.ws_ypixel);

#ifdef CRYPT
#ifdef KERBEROS
	if(doencrypt)
		(void)des_write(rem, obuf, sizeof(obuf));
	else
#endif
#endif
		(void)write(rem, obuf, sizeof(obuf));
}

/*
 * reader: read from remote: line -> 1
 */
#define	READING	1
#define	WRITING	2

jmp_buf rcvtop;
int ppid, rcvcnt, rcvstate;
char rcvbuf[8 * 1024];

void
oob(signo)
	int signo;
{
#ifdef TERMIOS
	struct termios tty;
#else
	struct sgttyb sb;
#endif
	int atmark, n, out, rcvd;
	char waste[BUFSIZ], mark;

	out = O_RDWR;
	rcvd = 0;
	while (recv(rem, &mark, 1, MSG_OOB) < 0) {
		switch (errno) {
		case EWOULDBLOCK:
			/*
			 * Urgent data not here yet.  It may not be possible
			 * to send it yet if we are blocked for output and
			 * our input buffer is full.
			 */
			if (rcvcnt < sizeof(rcvbuf)) {
				n = read(rem, rcvbuf + rcvcnt,
				    sizeof(rcvbuf) - rcvcnt);
				if (n <= 0)
					return;
				rcvd += n;
			} else {
				n = read(rem, waste, sizeof(waste));
				if (n <= 0)
					return;
			}
			continue;
		default:
			return;
		}
	}
	if (mark & TIOCPKT_WINDOW) {
		/* Let server know about window size changes */
		(void)kill(ppid, SIGUSR1);
	}
	if (!eight && (mark & TIOCPKT_NOSTOP)) {
#ifdef TERMIOS
		tcgetattr(0, &tty);
		tty.c_iflag &= ~(IXON|IXANY|IXOFF);
		tcsetattr(0, TCSANOW, &tty);
#else
		(void)ioctl(0, TIOCGETP, (char *)&sb);
		sb.sg_flags &= ~CBREAK;
		sb.sg_flags |= RAW;
		(void)ioctl(0, TIOCSETN, (char *)&sb);
		notc.t_stopc = -1;
		notc.t_startc = -1;
		(void)ioctl(0, TIOCSETC, (char *)&notc);
#endif
	}
	if (!eight && (mark & TIOCPKT_DOSTOP)) {
#ifdef TERMIOS
		tcgetattr(0, &tty);
		tty.c_iflag &= ~(IXON|IXANY|IXOFF);
		tty.c_iflag |= (deftty.c_iflag & (IXON|IXANY|IXOFF));
		tcsetattr(0, TCSANOW, &tty);
#else
		(void)ioctl(0, TIOCGETP, (char *)&sb);
		sb.sg_flags &= ~RAW;
		sb.sg_flags |= CBREAK;
		(void)ioctl(0, TIOCSETN, (char *)&sb);
		notc.t_stopc = deftc.t_stopc;
		notc.t_startc = deftc.t_startc;
		(void)ioctl(0, TIOCSETC, (char *)&notc);
#endif
	}
	if (mark & TIOCPKT_FLUSHWRITE) {
#ifdef TERMIOS
		tcflush(0, TCOFLUSH);
#else
		(void)ioctl(1, TIOCFLUSH, (char *)&out);
#endif
		for (;;) {
			if (ioctl(rem, SIOCATMARK, &atmark) < 0) {
				(void)fprintf(stderr, "rlogin: ioctl: %s.\n",
				    strerror(errno));
				break;
			}
			if (atmark)
				break;
			n = read(rem, waste, sizeof (waste));
			if (n <= 0)
				break;
		}
		/*
		 * Don't want any pending data to be output, so clear the recv
		 * buffer.  If we were hanging on a write when interrupted,
		 * don't want it to restart.  If we were reading, restart
		 * anyway.
		 */
		rcvcnt = 0;
		longjmp(rcvtop, 1);
	}

	/* oob does not do FLUSHREAD (alas!) */

	/*
	 * If we filled the receive buffer while a read was pending, longjmp
	 * to the top to restart appropriately.  Don't abort a pending write,
	 * however, or we won't know how much was written.
	 */
	if (rcvd && rcvstate == READING)
		longjmp(rcvtop, 1);
}

/* reader: read from remote: line -> 1 */
int
reader(omask)
	int omask;
{
	int pid, n, remaining;
	char *bufp;

#if BSD >= 43 || defined(SUNOS4)
	pid = getpid();		/* modern systems use positives for pid */
#else
	pid = -getpid();	/* old broken systems use negatives */
#endif
	(void)signal(SIGTTOU, SIG_IGN);
	(void)signal(SIGURG, oob);
	ppid = getppid();
	(void)fcntl(rem, F_SETOWN, pid);
	(void)setjmp(rcvtop);
	(void)sigsetmask(omask);
	bufp = rcvbuf;
	for (;;) {
		while ((remaining = rcvcnt - (bufp - rcvbuf)) > 0) {
			rcvstate = WRITING;
			n = write(STDOUT_FILENO, bufp, remaining);
			if (n < 0) {
				if (errno != EINTR)
					return (-1);
				continue;
			}
			bufp += n;
		}
		bufp = rcvbuf;
		rcvcnt = 0;
		rcvstate = READING;

#ifdef CRYPT
#ifdef KERBEROS
		if (doencrypt)
			rcvcnt = des_read(rem, rcvbuf, sizeof(rcvbuf));
		else
#endif
#endif
			rcvcnt = read(rem, rcvbuf, sizeof (rcvbuf));
		if (rcvcnt == 0)
			return (0);
		if (rcvcnt < 0) {
			if (errno == EINTR)
				continue;
			(void)fprintf(stderr, "rlogin: read: %s.\n",
			    strerror(errno));
			return (-1);
		}
	}
}

void
mode(f)
	int f;
{
#ifdef TERMIOS
	struct termios tty;
#else
	struct ltchars *ltc;
	struct sgttyb sb;
	struct tchars *tc;
	int lflags;
#endif

#ifndef TERMIOS
	(void)ioctl(0, TIOCGETP, (char *)&sb);
	(void)ioctl(0, TIOCLGET, (char *)&lflags);
#endif
	switch(f) {
	case 0:
#ifdef TERMIOS
		tty = deftty;
#else
		sb.sg_flags &= ~(CBREAK|RAW|TBDELAY);
		sb.sg_flags |= defflags|tabflag;
		tc = &deftc;
		ltc = &defltc;
		sb.sg_kill = defkill;
		sb.sg_erase = deferase;
		lflags = deflflags;
#endif
		break;
	case 1:
#ifdef TERMIOS
		tty = deftty;
		if (eight) {
			tty.c_iflag = 0;
			tty.c_oflag = 0;
			tty.c_lflag = 0;
		} else {
			tty.c_iflag &= ~(INLCR|IGNCR|ICRNL|IUCLC|IMAXBEL);
			tty.c_oflag &= ~(XTABS|OLCUC|ONLCR|OCRNL|ONOCR|ONLRET);
			tty.c_lflag &= ~(ICANON|ISIG|ECHO|TOSTOP);
		}
		tty.c_cc[VMIN] = 1;
		tty.c_cc[VTIME] = 0;
#else
		sb.sg_flags |= (eight ? RAW : CBREAK);
		sb.sg_flags &= ~defflags;
		/* preserve tab delays, but turn off XTABS */
		if ((sb.sg_flags & TBDELAY) == XTABS)
			sb.sg_flags &= ~TBDELAY;
		tc = &notc;
		ltc = &noltc;
		sb.sg_kill = sb.sg_erase = -1;
		if (litout)
			lflags |= LLITOUT;
#endif
		break;
	default:
		return;
	}
#ifdef TERMIOS
	tcsetattr(0, TCSANOW, &tty);
#else
	(void)ioctl(0, TIOCSLTC, (char *)ltc);
	(void)ioctl(0, TIOCSETC, (char *)tc);
	(void)ioctl(0, TIOCSETN, (char *)&sb);
	(void)ioctl(0, TIOCLSET, (char *)&lflags);
#endif
}

void
lostpeer(signo)
	int signo;
{
	(void)signal(SIGPIPE, SIG_IGN);
	msg("\007connection closed.");
	done(1);
}

/* copy SIGURGs to the child process. */
void
copytochild(signo)
	int signo;
{
	(void)kill(child, SIGURG);
}

void
msg(str)
	char *str;
{
	(void)fprintf(stderr, "rlogin: %s\r\n", str);
}

#ifdef KERBEROS
/* VARARGS */
void
#if __STDC__
warning(const char *fmt, ...)
#else
warning(fmt, va_alist)
	char *fmt;
	va_dcl
#endif
{
	va_list ap;

	(void)fprintf(stderr, "rlogin: warning, using standard rlogin: ");
#ifdef __STDC__
	va_start(ap, fmt);
#else
	va_start(ap);
#endif
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	(void)fprintf(stderr, ".\n");
}
#endif

void
usage()
{
	(void)fprintf(stderr,
	    "usage: rlogin [ -%s]%s[-e char] [ -l username ] host\n",
#ifdef KERBEROS
#ifdef CRYPT
	    "8EKLx", " [-k realm] ");
#else
	    "8EKL", " [-k realm] ");
#endif
#else
	    "8EL", " ");
#endif
	exit(1);
}

/*
 * The following routine provides compatibility (such as it is) between older
 * Suns and others.  Suns have only a `ttysize', so we convert it to a winsize.
 */
#ifdef OLDSUN
int
get_window_size(fd, wp)
	int fd;
	struct winsize *wp;
{
	struct ttysize ts;
	int error;

	if ((error = ioctl(0, TIOCGSIZE, &ts)) != 0)
		return (error);
	wp->ws_row = ts.ts_lines;
	wp->ws_col = ts.ts_cols;
	wp->ws_xpixel = 0;
	wp->ws_ypixel = 0;
	return (0);
}
#endif

u_int
getescape(p)
	register char *p;
{
	long val;
	int len;

	if ((len = strlen(p)) == 1)	/* use any single char, including '\' */
		return ((u_int)*p);
					/* otherwise, \nnn */
	if (*p == '\\' && len >= 2 && len <= 4) {
		val = strtol(++p, NULL, 8);
		for (;;) {
			if (!*++p)
				return ((u_int)val);
			if (*p < '0' || *p > '8')
				break;
		}
	}
	msg("illegal option value -- e");
	usage();
	/* NOTREACHED */
	return(0);
}
