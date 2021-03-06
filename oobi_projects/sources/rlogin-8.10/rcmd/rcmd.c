/*
 * Copyright (c) 1983, 1993, 1994
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
static char sccsid[] = "@(#)rcmd.c	8.3 (Berkeley) 3/26/94";
#endif /* LIBC_SCCS and not lint */

#include "inetprivate.h"
#include <paths.h>
#include <pwd.h>
#include <sys/file.h>
#include <sys/signal.h>
#include <sys/stat.h>

#if NLS
#include "nl_types.h"
#endif

#ifdef YP
#include <rpcsvc/ypclnt.h>
extern int setnetgrent(const char *);
extern void endnetgrent(void);
extern int getnetgrent(char **, char **, char **);
static char *nisdomain = NULL;
static int __ichecknetgrouphost(u_long, const char *);
static int _checknetgroupuser(const char *, const char *);
#endif

int rcmd(ahost, rport, locuser, remuser, cmd, fd2p)
	char **ahost;
	u_short rport;
	const char *locuser, *remuser, *cmd;
	int *fd2p;
{
	struct hostent *hp;
	struct sockaddr_in sin, from;
	fd_set reads;
#ifdef _POSIX_SOURCE
	sigset_t set, oset;
#else
	long oldmask;
#endif
	int s, lport, timo;
#ifdef F_SETOWN
	pid_t pid;
#endif
	char c;
#ifdef __linux__
	int retries = 0;
#endif

#if NLS
	libc_nls_init();
#endif

#ifdef F_SETOWN
	pid = getpid();
#endif
	hp = gethostbyname(*ahost);
	if (hp == 0) {
#if NLS
		fprintf(stderr, "%s: %s\n", *ahost,
                              catgets(_libc_cat, HerrorListSet,
				      2, "unknown host"));
#else
		fprintf(stderr, "%s: unknown host\n", *ahost);
#endif
		return (-1);
	}
	*ahost = hp->h_name;
#ifdef SIGURG
#ifdef _POSIX_SOURCE
	sigemptyset (&set);
	sigaddset (&set, SIGURG);
	sigprocmask (SIG_BLOCK, &set, &oset);
#else
	oldmask = sigblock(sigmask(SIGURG));
#endif
#endif
	for (timo = 1, lport = IPPORT_RESERVED - 1;;) {
		s = rresvport(&lport);
		if (s < 0) {
			if (errno == EAGAIN)
#if NLS
				fprintf(stderr, "socket: %s\n",
					catgets(_libc_cat, NetMiscSet,
						NetMiscAllPortsInUse,
						"All ports in use"));
#else
				fprintf(stderr, "socket: All ports in use\n");
#endif
			else
#if NLS
				perror(catgets(_libc_cat, NetMiscSet,
					       NetMiscRcmdSocket,
					       "rcmd: socket"));
#else
				perror("rcmd: socket");
#endif
#ifdef SIGURG
#ifdef _POSIX_SOURCE
			 sigprocmask (SIG_SETMASK, &oset,
				(sigset_t *)NULL);
#else
			sigsetmask(oldmask);
#endif
#endif
			return (-1);
		}
#ifdef F_SETOWN
		fcntl(s, F_SETOWN, pid);
#endif
		sin.sin_family = hp->h_addrtype;
		bcopy(hp->h_addr_list[0], (caddr_t)&sin.sin_addr, hp->h_length);
		sin.sin_port = rport;
		if (connect(s, (struct sockaddr *)&sin, sizeof (sin)) >= 0)
			break;
		(void) close(s);
		if (errno == EADDRINUSE) {
			lport--;
			continue;
		}
#ifdef __linux__
		/*
		 *	EADDRINUSE on ICMP level is ECONNREFUSED
		 *	 -- miquels@cistron.nl
		 */
		if (errno == ECONNREFUSED && retries < 16) {
			lport--;
			retries++;
			continue;
		}
		retries = 0;
#endif
		if (errno == ECONNREFUSED && timo <= 16) {
			sleep(timo);
			timo *= 2;
			continue;
		}
		if (hp->h_addr_list[1] != NULL) {
			int oerrno = errno;

			fprintf(stderr,
#if NLS
				"%s %s: ", catgets(_libc_cat, NetMiscSet,
						   NetMiscAllPortsInUse,
						   "connect to address"),
					   inet_ntoa(sin.sin_addr));

#else

			    "connect to address %s: ", inet_ntoa(sin.sin_addr));
#endif
			errno = oerrno;
			perror(0);
			hp->h_addr_list++;
			bcopy(hp->h_addr_list[0], (caddr_t)&sin.sin_addr,
			    hp->h_length);

#if NLS
			fprintf(stderr, catgets(_libc_cat, NetMiscSet,
						NetMiscTrying,
						"Trying %s...\n"),
#else
			fprintf(stderr,	"Trying %s...\n",
#endif
				inet_ntoa(sin.sin_addr));
			continue;
		}
		perror(hp->h_name);
#ifdef SIGURG
#ifdef _POSIX_SOURCE
		sigprocmask (SIG_SETMASK, &oset, (sigset_t *)NULL);
#else
		sigsetmask(oldmask);
#endif
#endif
		return (-1);
	}
	lport--;
	if (fd2p == 0) {
		write(s, "", 1);
		lport = 0;
	} else {
		char num[8];
		int s2 = rresvport(&lport), s3;
		int len = sizeof (from);

		if (s2 < 0)
			goto bad;
		listen(s2, 1);
		(void) sprintf(num, "%d", lport);
		/* void) snprintf(num, sizeof(num), "%d", lport); XXX */
		if (write(s, num, strlen(num)+1) != strlen(num)+1) {
#if NLS
			perror(catgets(_libc_cat, NetMiscSet,
				       NetMiscSettingUpStderr,
				       "write: setting up stderr"));
#else
			perror("write: setting up stderr");
#endif
			(void) close(s2);
			goto bad;
		}
		FD_ZERO(&reads);
		FD_SET(s, &reads);
		FD_SET(s2, &reads);
		errno = 0;
		if (select(1 + (s > s2 ? s : s2), &reads, 0, 0, 0) < 1 ||
		    !FD_ISSET(s2, &reads)) {
			if (errno != 0)
#if NLS
				perror(catgets(_libc_cat, NetMiscSet,
					       NetMiscSelectSetup,
					       "rcmd: select (setting up stderr)"));
#else
				perror("rcmd: select (setting up stderr): ");
#endif
			else
				fprintf(stderr,
#if NLS
				        "%s\n",
				        catgets(_libc_cat, NetMiscSet,
						NetMiscRcmdProtocolError,
						"select: protocol failure in circuit setup"));
#else
					"select: protocol failure in circuit setup.\n");
#endif
			(void)close(s2);
			goto bad;
		}
		s3 = accept(s2, (struct sockaddr *)&from, &len);
		(void) close(s2);
		if (s3 < 0) {
#if NLS
			perror(catgets(_libc_cat, NetMiscSet,
				       NetMiscAccept,
				       "accept"));
#else
			perror("accept");
#endif
			lport = 0;
			goto bad;
		}
		*fd2p = s3;
		from.sin_port = ntohs((u_short)from.sin_port);
		if (from.sin_family != AF_INET ||
		    from.sin_port >= IPPORT_RESERVED ||
		    from.sin_port < IPPORT_RESERVED / 2) {
			fprintf(stderr,
#if NLS
				"%s\n",
				catgets(_libc_cat, NetMiscSet,
					NetMiscProtocolFailure,
					"socket: protocol failure in circuit setup."));
#else
			    "socket: protocol failure in circuit setup.\n");
#endif
			goto bad2;
		}
	}
	(void) write(s, locuser, strlen(locuser)+1);
	(void) write(s, remuser, strlen(remuser)+1);
	(void) write(s, cmd, strlen(cmd)+1);
	if (read(s, &c, 1) != 1) {
		perror(*ahost);
		goto bad2;
	}
	if (c != 0) {
		while (read(s, &c, 1) == 1) {
			(void) write(STDERR_FILENO, &c, 1);
			if (c == '\n')
				break;
		}
		goto bad2;
	}
#ifdef SIGURG
#ifdef _POSIX_SOURCE
	sigprocmask (SIG_SETMASK, &oset, (sigset_t *)NULL);
#else
	sigsetmask(oldmask);
#endif
#endif
	return (s);
bad2:
	if (lport)
		(void) close(*fd2p);
bad:
	(void) close(s);
#ifdef SIGURG
#ifdef _POSIX_SOURCE
	sigprocmask (SIG_SETMASK, &oset, (sigset_t *)NULL);
#else
	sigsetmask(oldmask);
#endif
#endif
	return (-1);
}

int
rresvport(alport)
	int *alport;
{
	struct sockaddr_in sin;
	int s;

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0)
		return (-1);
	for (;;) {
		sin.sin_port = htons((u_short)*alport);
		if (bind(s, (struct sockaddr *)&sin, sizeof (sin)) >= 0)
			return (s);
		if (errno != EADDRINUSE) {
			(void) close(s);
			return (-1);
		}
		(*alport)--;
		if (*alport == IPPORT_RESERVED/2) {
			(void) close(s);
			errno = EAGAIN;		/* close */
			return (-1);
		}
	}
}

int     __check_rhosts_file = 1;
char    *__rcmd_errstr;

int
ruserok(const char *rhost, int superuser, const char *ruser,
	const char *luser)
{
	struct hostent *hp;
	u_long addr;
	char **ap;

	if ((hp = gethostbyname(rhost)) == NULL)
		return (-1);
	for (ap = hp->h_addr_list; *ap; ++ap) {
		bcopy(*ap, &addr, sizeof(addr));
		if (iruserok(addr, superuser, ruser, luser) == 0)
			return (0);
	}
	return (-1);
}

/*
 * New .rhosts strategy: We are passed an ip address. We spin through
 * hosts.equiv and .rhosts looking for a match. When the .rhosts only
 * has ip addresses, we don't have to trust a nameserver.  When it
 * contains hostnames, we spin through the list of addresses the nameserver
 * gives us and look for a match.
 *
 * Returns 0 if ok, -1 if not ok.
 */
int
iruserok(u_long raddr, int superuser, const char *ruser, const char *luser)
{
	register char *cp;
	struct stat sbuf;
	struct passwd *pwd;
	FILE *hostf;
	uid_t uid;
	int first = 1;
	char pbuf[MAXPATHLEN];

	first = 1;
	hostf = superuser ? NULL : fopen(_PATH_HEQUIV, "r");
again:
	if (hostf) {
		if (__ivaliduser(hostf, raddr, luser, ruser) == 0) {
			(void) fclose(hostf);
			return(0);
		}
		(void) fclose(hostf);
	}
	if (first == 1 && (__check_rhosts_file || superuser)) {
		first = 0;
		if ((pwd = getpwnam(luser)) == NULL)
			return(-1);
		(void)strcpy(pbuf, pwd->pw_dir);
		(void)strcat(pbuf, "/.rhosts");

		/*
		 * Change effective uid while opening .rhosts.  If root and
		 * reading an NFS mounted file system, can't read files that
		 * are protected read/write owner only.
		 */
		uid = geteuid();
		(void)seteuid(pwd->pw_uid);
		hostf = fopen(pbuf, "r");
		(void)seteuid(uid);

		if (hostf == NULL)
			return(-1);
		/*
		 * If not a regular file, or is owned by someone other than
		 * user or root or if writeable by anyone but the owner, quit.
		 */
		cp = NULL;
		if (lstat(pbuf, &sbuf) < 0)
#if NLS
			cp = catgets(_libc_cat, NetMiscSet,
				     NetMiscRhostLstat,
				     ".rhosts not regular file");
#else
			cp = ".rhosts not regular file";
#endif
		else if (!S_ISREG(sbuf.st_mode))
#if NLS
			cp = catgets(_libc_cat, NetMiscSet,
				     NetMiscRhostNotRegular,
				     ".rhosts not regular file");
#else
			cp = ".rhosts not regular file";
#endif
		else if (fstat(fileno(hostf), &sbuf) < 0)
#if NLS
			cp = catgets(_libc_cat, NetMiscSet,
				     NetMiscRhostFstatFailed,
				     ".rhosts fstat failed");
#else
			cp = ".rhosts fstat failed";
#endif
		else if (sbuf.st_uid && sbuf.st_uid != pwd->pw_uid)
#if NLS
			cp = catgets(_libc_cat, NetMiscSet,
				     NetMiscBadRhostsOwner,
				     "bad .rhosts owner");
#else
			cp = "bad .rhosts owner";
#endif
		else if (sbuf.st_mode & (S_IWGRP|S_IWOTH))
#if NLS
			cp = catgets(_libc_cat, NetMiscSet,
				     NetMiscRhostWritable,
				     ".rhosts writeable by other than owner");
#else
			cp = ".rhosts writeable by other than owner";
#endif
		/* If there were any problems, quit. */
		if (cp) {
			__rcmd_errstr = cp;
			fclose(hostf);
			return(-1);
		}
		goto again;
	}
	return (-1);
}

/*
 * XXX
 * Don't make static, used by lpd(8).
 *
 * Returns 0 if ok, -1 if not ok.
 */
int
__ivaliduser(FILE *hostf, u_long raddr, const char *luser, const char *ruser)
{
	register char *user, *p;
	int ch;
	char buf[MAXHOSTNAMELEN + 128];		/* host + login */
#ifdef YP
	int hostvalid = 0;
	int uservalid = 0;
#endif

	while (fgets(buf, sizeof (buf), hostf)) {
		p = buf;
		/* Skip lines that are too long. */
		if (strchr(p, '\n') == NULL) {
			while ((ch = getc(hostf)) != '\n' && ch != EOF);
			continue;
		}
		while (*p != '\n' && *p != ' ' && *p != '\t' && *p != '\0') {
			/* *p = isupper(*p) ? tolower(*p) : *p;  -- Uli */
			*p = tolower(*p);	/* works for linux libc */
			p++;
		}
		if (*p == ' ' || *p == '\t') {
			*p++ = '\0';
			while (*p == ' ' || *p == '\t')
				p++;
			user = p;
			while (*p != '\n' && *p != ' ' &&
			    *p != '\t' && *p != '\0')
				p++;
		} else
			user = p;
		*p = '\0';
#ifdef YP
            /* disable host from -hostname entry */
        if ('-' == buf[0] && '@' != buf[1]
            && __icheckhost(raddr, &buf[1]))
          return -1;
            /* disable host from -@netgroup entry for host */
        if ('-' == buf[0] && '@' == buf[1] && '\0' != buf[2]
            && __ichecknetgrouphost(raddr, &buf[2]))
          return -1;
            /* disable user from -user entry */
        if ('\0' != *user && user[0] == '-' && user[1] != '@'
            && !strcmp(&user[1], ruser))
          return -1;
            /* disable user from -@netgroup entry for user */
        if ('\0' != *user && user[0] == '-' && user[1] == '@'
            && user[2] != '\0' && _checknetgroupuser(ruser, &user[2]))
          return -1;
            /* enable host from +@netgroup entry for host */
        if ('+' == buf[0] && '@' == buf[1] && '\0' != buf[2])
          hostvalid = __ichecknetgrouphost(raddr, &buf[2]);
        else
          hostvalid = __icheckhost(raddr, buf);
            /* enable user from +@netgroup entry for user */
        if ('\0' != *user && user[0] == '+'
            && user[1] == '@' && user[2] != '\0')
          uservalid = _checknetgroupuser(ruser, &user[2]);
        else
          uservalid = !strcmp(ruser, *user ? user : luser);
        
        if (hostvalid && uservalid)
          return 0;
#else
		if (__icheckhost(raddr, buf) &&
		    !strcmp(ruser, *user ? user : luser)) {
          		return (0);
		}
#endif /* YP */
	}
	return (-1);
}

/*
 * Returns "true" if match, 0 if no match.
 */
int
__icheckhost(u_long raddr, register char *lhost)
{
	register struct hostent *hp;
	register u_long laddr;
	register char **pp;

	/* Try for raw ip address first. */
	if (isdigit(*lhost) && (long)(laddr = inet_addr(lhost)) != -1)
		return (raddr == laddr);

	/* Better be a hostname. */
	if ((hp = gethostbyname(lhost)) == NULL)
		return (0);

	/* Spin through ip addresses. */
	for (pp = hp->h_addr_list; *pp; ++pp)
		if (!bcmp(&raddr, *pp, sizeof(u_long)))
			return (1);

	/* No match. */
	return (0);
}

#ifdef YP
static int
__ichecknetgrouphost(u_long raddr, const char *netgr)
{
  char *host, *user, *domain;
  int status;
  
  if (NULL == nisdomain)
    yp_get_default_domain(&nisdomain);
  
  setnetgrent(netgr);
  while (1)
    {
      while (1 == (status = getnetgrent(&host, &user, &domain))
             && NULL == host
             && NULL != domain
             && 0 != strcmp(domain, nisdomain))
        ;  /* find valid host entry */
      
      if (0 == status || NULL == host)
        {
          endnetgrent();
          return 0;
        }

      if(__icheckhost(raddr, host))
        {
          endnetgrent();
          return 1;
        }
    }
}

static int
_checknetgroupuser(const char *ruser, const char *netgr)
{
  char *host, *user, *domain;
  int status;
  
  if (NULL == nisdomain)
    yp_get_default_domain(&nisdomain);
  
  setnetgrent(netgr);
  while (1)
    {
      while (1 == (status = getnetgrent(&host, &user, &domain))
             && NULL == user
             && NULL != domain
             && 0 != strcmp(domain, nisdomain))
        ;  /* find valid user entry */
      
      if (0 == status || NULL == user)
        {
          endnetgrent();
          return 0;
        }

      if(0 == strcmp(ruser, user))
        {
          endnetgrent();
          return 1;
        }
    }
}
#endif /* YP */
