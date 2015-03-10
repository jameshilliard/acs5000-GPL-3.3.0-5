/*
 * lib.c         Generic functions.
 *
 * Version:      @(#)lib.c  1.36  13-Jan-1998  miquels@cistron.nl
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <errno.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <values.h>
#include "server.h"
#include <regex.h>
#include "ras_parse.h"

/*
 *	Change argv[0] to reflect what we're doing.
 */

static char **pargv = NULL;

void
title(int all, char **argv, char *fmt, ...)
{
	va_list ap;
    int len=0, i;
	char *s;

	va_start(ap, fmt);

	if (argv) pargv = argv;

	if (!pargv) goto exit_rot;

	switch (all) {
	case 0:
	case 2:
		for(len=0, i=0; pargv[i]; i++) {
			if (i == 0 || all == 2)
				len += strlen(pargv[i]) + 1;		// should count '\0'
			for(s = pargv[i]; *s; s++)
				*s = ' ';
			pargv[i][0] = 0;
		}
		break;
	case 1:
		len = strlen(pargv[0]) + 1;		// should count '\0'
		break;
	}

	vsnprintf(pargv[0], len, fmt, ap);
exit_rot:
	va_end(ap);
}

#if 0
/*
 * Malloc and die if failed.
 */
void *xmalloc(int size)
{
	void *p;

	if ((p = malloc(size)) == NULL) {
		nsyslog(LOG_ERR, "Virtual memory exhausted.\n");
		exit(1);
	}
	memset(p, 0, size);
	return p;
}
#endif

/*
 * Signal functions.
 */
void block(int sig)
{
	sigset_t set;

	sigemptyset(&set);
	sigaddset(&set, sig);
	sigprocmask(SIG_BLOCK, &set, NULL);
}
void unblock(int sig)
{
	sigset_t set;

	sigemptyset(&set);
	sigaddset(&set, sig);
	sigprocmask(SIG_UNBLOCK, &set, NULL);
}

/*
 *	Print number in static buffer (itoa).
 */
char *num(int i)
{
	static char buf[16];

	snprintf(buf, sizeof (buf), "%d", i);
	return buf;
}

/*
 *	Print dotted IP address.
 */
char *dotted(unsigned int a)
{
		struct in_addr ia;

		ia.s_addr = a;
		return inet_ntoa(ia);
}

/*
 *	Print dotted IPv4 or IPv6 address.
 */
char *dotted6(struct sockaddr *ipaddr, char *buffer, int b_len)
{
	struct sockaddr_storage addr;
	int addr_len, n;

	if (ipaddr->sa_family == AF_INET) {
		addr_len = sizeof(struct sockaddr_in);
	} else {
		addr_len = sizeof(struct sockaddr_in6);
	}

	memcpy(&addr, ipaddr, addr_len);

	n = getnameinfo((struct sockaddr *)&addr, addr_len, buffer,
			b_len, NULL, 0, NI_NUMERICHOST);

	if (n != 0) {
		nsyslog(LOG_ERR, "getnameinfo error : %d - %s", n, strerror(errno));
	}

	return buffer;
}

/*
 *      Converts dotted IPv4 or IPv6 address into binary data
 */
int asc_to_binary6(char *ipstring, struct sockaddr *ipaddr)
{
	struct addrinfo hints, *res = NULL;
	int i;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_flags = AI_NUMERICHOST;

	i = getaddrinfo(ipstring, NULL, &hints, &res);

	if (i) {
		nsyslog (LOG_ERR, "\nbinary6: ERROR = %s \n", gai_strerror(i));
		//freeaddrinfo(res); not necessary?
		return (i);
	}
	memcpy(ipaddr, res->ai_addr, res->ai_addrlen);
        freeaddrinfo(res);
	return 0;
}

/*
 *	Print 1/4 of IPv4 or IPv6 address.
 */
char *print_quarter(struct sockaddr *ipaddr, char *buffer, int b_len, int index)
{
	struct sockaddr_in  *ip4 = (struct sockaddr_in *)ipaddr;
	struct sockaddr_in6 *ip6 = (struct sockaddr_in6 *)ipaddr;
	unsigned long int byte_val;
	
	if (ipaddr->sa_family == AF_INET) {
		byte_val = (ntohl(ip4->sin_addr.s_addr) >> (index * BITSPERBYTE)) & 0xFF;
	} else {
		byte_val = ntohl(ip6->sin6_addr.s6_addr32[index]);
	}
	
	snprintf (buffer, b_len, "%ld", byte_val);
	
	return buffer;
}

/*
 * This function compares 2 IP addresses from sockaddr structs pointed 
 * by \a sa1 and \a sa2. It returns 0 if they match, and 1 otherwise.
 */
int ipaddr_cmp (struct sockaddr_storage *sa1, struct sockaddr_storage *sa2)
{
	struct sockaddr_in *sinA, *sinB;
	struct sockaddr_in6 *sin6A, *sin6B;
	
	if (sa1->ss_family != sa2->ss_family) {
		return (1);
	}
	if (sa1->ss_family == AF_INET) {
		sinA = (struct sockaddr_in *)sa1;
		sinB = (struct sockaddr_in *)sa2;
		if (sinA->sin_addr.s_addr == sinB->sin_addr.s_addr) {
			return (0);
		} else {
			return (1);
		}
	}
	if (sa1->ss_family == AF_INET6) {
		sin6A = (struct sockaddr_in6 *)sa1;
		sin6B = (struct sockaddr_in6 *)sa2;
		if (!memcmp(&sin6A->sin6_addr, &sin6B->sin6_addr, sizeof(sin6A->sin6_addr))) {
			return (0);
		} else {
			return (1);
		}
	}
	return (1);
}
/*
 * Strips login name and writes it to buffer.
 */

void strip_login (char *buffer, int buf_size, char *login) {
	char *ptr;

	if (strchr("PCS!L", login [0])) {
		snprintf (buffer, buf_size, "%s", login+1);
	} else {
		snprintf (buffer, buf_size, "%s", login);
		ptr = strrchr (buffer, '.');
		if (ptr) {
			if ((strcmp(ptr, ".slip")  == 0) ||
				(strcmp(ptr, ".cslip") == 0) ||
				(strcmp(ptr, ".ppp")   == 0)) {
				*ptr = 0;
			}
		}
	}
}

/*
 *
 * Expands a format string
 *
 * %l: login name
 * %L: stripped login name
 * %p: NAS port number
 * %P: protocol
 * %b: port speed
 * %i: local IP
 * %j: remote IP
 * %J: remote TCP Port
 * %1: first byte (MSB) of remote IP
 * %2: second byte of remote IP
 * %3: third byte of remote IP
 * %4: fourth (LSB) byte of remote IP
 * %c: connect-info
 * %m: netmask
 * %t: MTU
 * %r: MRU
 * %I: idle timeout
 * %T: session timeout
 * %h: hostname
 * %%: %
 *
 * Everything else is copied unchanged.
 *
 */

void expand_format(char *buffer, size_t size, char *format, struct auth *ai) {
	char ch;
	int done;
	char str [256];
//	unsigned long int byte_val;

	if (size-- == 0) {
		return;
	}
	while (size) {
		if ((ch = *format++) == '%') {
			switch (*format) {
				case 'l':
					snprintf (str, sizeof(str), "%s", ai->login);
					break;
				case 'L':
					strip_login (str, sizeof (str), ai->login);
					break;
				case 'p':
#ifndef CYCLADES
					snprintf (str, sizeof(str), "%03d", ai->nasport);
#else
					snprintf (str, sizeof(str), "%03d", ai->nasport+1);
#endif
					break;
				case 'P':
					snprintf (str, sizeof(str), "%c", (char) ai->proto);
					break;
				case 'b':
					snprintf (str, sizeof(str), "%d", lineconf[ai->nasport].speed);
					break;
				case 'H':
					// IPv6 - changed addrs format
					if (ai->host.ss_family) {
						dotted6((struct sockaddr *)(&ai->host),str,sizeof(str));
						break;
					}
				case 'i':
					snprintf (str, sizeof(str), "%s", dotted(mainconf.ipno));
					break;
				case 'J':
					snprintf (str, sizeof(str), "%d", ai->loginport);
					break;
				case 'j':
					// IPv6 - changed addrs format
					//snprintf (str, sizeof(str), "%s", dotted(ai->address));
					dotted6((struct sockaddr *)(&ai->address),str,sizeof(str));
					break;
				case '1':
					// IPv6 - changed addrs format
					//byte_val = (ntohl (ai->address) >> (3*BITSPERBYTE)) & 0xFF;
					//snprintf (str, sizeof(str), "%ld", byte_val);
					print_quarter((struct sockaddr *)&ai->address, str, sizeof(str), 3);
					break;
				case '2':
					// IPv6 - changed addrs format
					//byte_val = (ntohl (ai->address) >> (2*BITSPERBYTE)) & 0xFF;
					//snprintf (str, sizeof(str), "%ld", byte_val);
					print_quarter((struct sockaddr *)&ai->address, str, sizeof(str), 2);
					break;
				case '3':
					// IPv6 - changed addrs format
					//byte_val = (ntohl (ai->address) >> BITSPERBYTE) & 0xFF;
					//snprintf (str, sizeof(str), "%ld", byte_val);
					print_quarter((struct sockaddr *)&ai->address, str, sizeof(str), 1);
					break;
				case '4':
					// IPv6 - changed addrs format
					//byte_val = ntohl (ai->address) & 0xFF;
					//snprintf (str, sizeof(str), "%ld", byte_val);
					print_quarter((struct sockaddr *)&ai->address, str, sizeof(str), 0);
					break;
				case 'c':
					snprintf (str, sizeof(str), "%s", ai->conn_info);
					break;
				case 'm':
					snprintf (str, sizeof(str), "%s", dotted(ai->netmask));
					break;
				case 't':
					snprintf (str, sizeof(str), "%d", ai->mtu);
					break;
				case 'r':
					snprintf (str, sizeof(str), "%d", ai->mru);
					break;
				case 'I':
					snprintf (str, sizeof(str), "%d", ai->idletime);
					break;
				case 'T':
					snprintf (str, sizeof(str), "%d", ai->sessiontime);
					break;
				case 'h':
					snprintf (str, sizeof(str), "%s", mainconf.hostname);
					break;
				case 'g':
					snprintf (str, sizeof(str), "%d", getpid());
					break;
				case '%':
					snprintf (str, sizeof(str), "%c", '%');
					break;
				default:
					*buffer++ = '?';
					--size;
					continue;
			}
			++format;
			if ((done = snprintf (buffer, size+1, "%s", str)) == -1) {
				return;
			}
			buffer += done;
			size -= done;
		} else {
			if ((*buffer = ch) == '\0') {
				return;
			}
			++buffer;
			--size;
		}
	}
	*buffer = '\0';
}

void set_remote_address(int s, struct auth *ai)
{
	struct sockaddr_storage sn;
	size_t size = sizeof(sn);

	ai->loginport = 0;

	if (getpeername(s, (struct sockaddr *)&sn, &size)) {
		//ai->address = 0; IPv6 - changed IP addrs format
		memset(&ai->address, 0, sizeof(ai->address));
		return;
	}

	//ai->address = sn.sin_addr.s_addr; IPv6 - changed IP addrs format
	memcpy(&ai->address, &sn, sizeof(ai->address));
	
	if (sn.ss_family == AF_INET) {
		ai->loginport = ((struct sockaddr_in *)&sn)->sin_port;
	} else if (sn.ss_family == AF_INET6) {
		ai->loginport = ((struct sockaddr_in6 *)&sn)->sin6_port;
	}
}

/*
 * This function should be called before using the parsing routines.
 * It compiles all regular expressions into patterns.
 */
int ras_parse_init (struct ras_parse *parse, char **pattern_table, int total)
{
	int i;

	printf ("ras_parse_init 1\n");	
	for (i = 0; i < total; i++) {
		if (regcomp(&parse->pattern[i], pattern_table[i],
			REG_EXTENDED | REG_ICASE))
		{
			nsyslog (LOG_ERR, "RAS: Failure compiling regexp: %s", pattern_table[i]);
			return 0;
		}
	}
	return 1;
}

/*
 * This function releases all pre-compile regular expression patterns.
 */
void ras_parse_end (struct ras_parse *parse, int total)
{
	int i;
	for (i = 0; i < total; i++) {
		regfree(&parse->pattern[i]);
	}
}

/*
 * This function searches for the regular expression that matches the given
 * string (\a str). The regular expressions are in a vector and only the
 * regular expressions in the range [\a start, \a end] are searched.
 */
int ras_parse (struct ras_parse *parse, char *str, int total)
{
	int i;
	int k;

	for (i = 0; i < total; i++) {
		if (! regexec (&parse->pattern[i],str,RAS_MAX_MATCHES,parse->match,0)) 
		{
			nsyslog (LOG_INFO, "\nMatch for [%s]:\n", str);

			for (k = 0; k < RAS_MAX_MATCHES; k++) {
				if (parse->match[k].rm_so == -1)
					continue;
				nsyslog (LOG_INFO, "$%d: '%.*s'\n", k,
					parse->match[k].rm_eo - parse->match[k].rm_so,
					&str[parse->match[k].rm_so]);
			}
			return i;
		}
	}
	return i;
}

/*
 * Extract a string field from the given \a from_str string and place it in
 * \a to_str limited to a size of \a maxlen bytes. If \a to_str is NULL the
 * string is not stored (this can be used to determine the size of the
 * destination string).
 *
 * \return the length of the string (not counting the terminating null).
 */
int ras_get_str_field (regmatch_t *match, unsigned char *from_str, int field, 
				char *to_str, int maxlen)
{
	int beg, end, siz;

	/* Just return the size of the matched string. */
	if (to_str == NULL) {
		return match[field].rm_eo - match[field].rm_so;
	}

	/* Check if field was not recognized. */
	if (match[field].rm_so == -1) {
		*to_str = '\0';
		return 0;
	}
	
	beg = match[field].rm_so;
	end = match[field].rm_eo;
	siz = end - beg;

	if (siz >= maxlen) {
		siz  = maxlen - 1;
	}

	strncpy(to_str, &from_str[beg], siz);
	to_str[siz] = '\0';

	nsyslog (LOG_INFO, "STR: $%d = [%s]", field, to_str);

	return siz;
}

/*
 * Parse the given \a string and extract its contents. If it contains a tty
 * name, copy it to \a tty. If it contains IP:port, copy the IP address to
 * \a ipaddr and the TCP port to \a tcp_port. All the provided strings must
 * be limited to a size of \a strs_len bytes. 
 *
 * \return nothing. Strings will be null if they are not found in \a string.
 */

void ras_parse_tty (char *string, char *tty, char *ipaddr, char *tcp_port, int strs_len)
{
	struct ras_parse parse;
	int i, j;

	*tty = '\0';
	*ipaddr = '\0';
	*tcp_port = '\0';

	ras_parse_init (&parse, (char **)&g_ttyRegexTable, RAS_TTY_TOTAL);
	i = ras_parse (&parse, string, RAS_TTY_TOTAL);

	if (i == RAS_TTY_NONE) {
		nsyslog (LOG_ERR, "RAS: ras_parse returned RAS_TTY_NONE");
		return;
	}

	switch (i) {
		case RAS_TTY_DEVICE:
			j = ras_get_str_field (&parse.match[0], string, 0, tty, strs_len);
			nsyslog (LOG_INFO, "TTY = %s", tty);
			break;
		case RAS_TTY_VIRTUAL_IPv4_PORT:
		case RAS_TTY_HOSTNAME_PORT:
			j = ras_get_str_field (&parse.match[1], string, 0, ipaddr, strs_len);
			nsyslog (LOG_INFO, "IP = %s", ipaddr);
			j = ras_get_str_field (&parse.match[2], string, 0, tcp_port, strs_len);
			nsyslog (LOG_INFO, "Port = %s", tcp_port);
			break;
		case RAS_TTY_VIRTUAL_IPv6_PORT:
			j = ras_get_str_field (&parse.match[1], string, 0, ipaddr, strs_len);
			nsyslog (LOG_INFO, "IP = %s", ipaddr);
			j = ras_get_str_field (&parse.match[4], string, 0, tcp_port, strs_len);
			nsyslog (LOG_INFO, "Port = %s", tcp_port);
			break;
		case RAS_TTY_VIRTUAL_IPv4:
		case RAS_TTY_VIRTUAL_IPv6:
		case RAS_TTY_HOSTNAME:
			j = ras_get_str_field (&parse.match[0], string, 0, ipaddr, strs_len);
			nsyslog (LOG_INFO, "IP/HOSTNAME = %s", ipaddr);
			break;
		default:
			printf("ERROR\n");
			break;
	}
	
	ras_parse_end (&parse, RAS_TTY_TOTAL);
}
