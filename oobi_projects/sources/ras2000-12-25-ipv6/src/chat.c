/*
 * chat.c        This is a generic chat handler
 *               for initialization of the modems.
 *
 * Version:      (@)#chat.c  1.00  12-Sep-1995  MvS.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <syslog.h>

#include "../pslave_cfg.h"
#include "server.h"

static int timed_out;
int chat_timeout = 10;		/* Seconds */
int chat_send_delay = 1;	/* Tenths  */

enum special_code {
	SPECIAL_NONE = -1,
	SPECIAL_TIMEOUT = 0,
	SPECIAL_WAIT = 1,
	SPECIAL_STATUS = 2
};

struct special_chat {
	char *word;
	enum special_code code;
	int req_words;
};

struct special_chat sw [] = {
	{"TIMEOUT", SPECIAL_TIMEOUT, 2},
	{"WAIT", SPECIAL_WAIT, 2},
	{"STATUS", SPECIAL_STATUS, 3},
	{NULL, SPECIAL_NONE, 0}
};

/*
 *	Send a string to the remote side. Process any
 *	left-over escape sequences.
 */
static void chat_send(int fd, char *p) {
	char *s, *start, *end;
	int addcr = 1;
	int flags, val;
	char c;

	nsyslog (LOG_DEBUG, "chat_send(%s)", p);

	/* Translate escape characters, pass 1 */
	if ((s = (char *)malloc(strlen(p) + 1)) == NULL)
		return;
	strcpy(s, p);
	tstr(1, s);

	/* Delay a bit and then flush the input. */
	usleep(chat_send_delay * 100000);
	tcflush(0, TCIFLUSH);

	/* Now send the string. */
	start = end = s;
	while(1) {
		/* Collect characters till backslash or end of string. */
		while(*end != '\\' && *end)
			end++;
		if (end != start) write(fd, start, end - start);
		if (*end == 0) {
			if (addcr) {
				c = '\r';
				write(fd, &c, 1);
			}
			free(s);
			return;
		}

		/* Special escape sequence. */
		val = -1;
		switch (*++end) {
			case 'd': /* delay */
				sleep(1);
				break;
			case 'p': /* pause */
				usleep(100000);
				break;
			case 'l': /* lower DTR */
				ioctl(fd, TIOCMGET, &flags);
				flags &= ~TIOCM_DTR;
				ioctl(fd, TIOCMSET, &flags);
				sleep(1);
				flags |= TIOCM_DTR;
				ioctl(fd, TIOCMSET, &flags);
				break;
			case 'c': /* Don't add CR */
				addcr = 0;
				break;
			case 'K': /* Send a break */
				tcsendbreak(fd, 0);
				break;
			default: /* Just fill in character. */
				val = *end;
				break;
		}
		/* Write character if needed. */
		if (val >= 0) {
			c = val;
			write(fd, &c, 1);
		}

		/* And continue after escape character. */
		if (*end) end++;
		start = end;
	}
}

/*
 * Skip string until the next '-'.
 */
static char *skip(char *s)
{

	for(; *s; s++) {
		if (*s == '\\' && *(s+1)) s++;
		if (*s == '-') {
			*s++ = 0;
			return s;
		}
	}
	return NULL;
}

/*
 * Compare 2 strings where '@' means ".*[\r\n]"
 */
int chatncmp(char *exp, char *str, int len, char *matchpart)
{
	char *p;
	int r, l;

	nsyslog(LOG_DEBUG, "chatncmp 1: (%s, %s, %d, %s)", exp, str, len,
		matchpart ? "mp" : "--");

	/* No `@': return normal strncmp */
	if (matchpart == NULL || (p = strchr(exp, '@')) == NULL)
		return strncmp(exp, str, len);

	nsyslog(LOG_DEBUG, "chatncmp 2");

	/* Find the `@'. See if we need to look behind the `@' */
	if ((l = p - exp) >= len)
		return -1;

	nsyslog(LOG_DEBUG, "chatncmp 3");

	/* Compare up to the `@' */
	if ((r = strncmp(exp, str, l)) != 0)
		return r;

	nsyslog(LOG_DEBUG, "chatncmp 4");

	/* Now see if the rest of the string contains ".*[\r\n]" */
	for(p = str + l; *p; p++)
		if (*p == '\r' || *p == '\n')
			break;
	if (*p) {
		strncpy(matchpart, str + l, p - (str + l));
		matchpart[p - (str + l)] = 0;
	}

	nsyslog(LOG_DEBUG, "chatncmp 5: *p = %d", *p);

	return *p == 0 ? -1 : 0;
}

/*
 * Wait until fd is readable or timeout/error occurs. If fd is readable, try
 * to read 1 char.
 */

int read_char (char *c, int fd) {
	struct pollfd read_fd;
	time_t poll_timeout;
	struct itimerval left;
	int ret;

	/*
	 * We don't enclose all poll code in if(chat_timeout) {...},
	 * because we want to be able to specify infinite timeouts on
	 * non-blocking sockets too.
	 */

	if (chat_timeout) {
		if (timed_out) {
			return (-1);
		}
		if (getitimer (ITIMER_REAL, &left)) {
			nsyslog (LOG_DEBUG, "read_char: getitimer: %d(%m)", errno);
			return (-1);
		}
		if ((! left.it_value.tv_sec) && (! left.it_value.tv_usec)) {
			nsyslog (LOG_DEBUG, "read_char: timed out outside of poll");
			timed_out = 1;
			return (-1);
		}
		poll_timeout = left.it_value.tv_sec*1000+(left.it_value.tv_usec ? 1 : 0);
	} else {
		poll_timeout = -1;
	}
	read_fd.fd = fd;
	read_fd.events = POLLIN;
	read_fd.revents = 0;
	switch (ret = poll (&read_fd, 1, poll_timeout)) {
		case -1:
			nsyslog (LOG_DEBUG, "read_char: poll: %d(%m)", errno);
			return (-1);
		case 0:
			if (chat_timeout) {
				nsyslog (LOG_DEBUG, "read_char: timed out in poll");
				timed_out = 1;
			} else {
				nsyslog (LOG_DEBUG, "read_char: bad thing"
					"happened: timed out in poll after"
					"infinite timeout had been specified");
			}
			return (-1);
		case 1:
			break;
		default:
			nsyslog (LOG_DEBUG, "read_char: poll returned: %d", ret);
			return (-1);
	}
	if ((read_fd.revents & POLLIN) == 0) {
		nsyslog (LOG_DEBUG, "read_char: bad poll mask: %d", read_fd.revents);
		return (-1);
	}
	switch (ret = read (fd, c, 1)) {
		case -1:
			nsyslog (LOG_DEBUG, "read_char: read: %d(%m)", errno);
			return (-1);
		case 1:
			break;
		default:
			nsyslog (LOG_DEBUG, "read_char: read %d chars", ret);
			return (-1);
	}
	return (0);
}

/*
 * Expect a string.
 */
int chat_expect(int fd, char *p, char *matchpart, int matchlen) {
	char *s, *q;
	char *send, *expect;
	char buf[64];
	int len, conn_hack;
	char c;
	int retries = 0;

	/* Copy argument. */
	if ((s = (char *)malloc(strlen(p) + 1)) == NULL) {
		nsyslog(LOG_CRIT, "malloc: %m");
		return -1;
	}
	strcpy(s, p);
	expect = s;

	while(1) {

		/* Prepare expect string. */
		send = skip(expect);
		nsyslog (LOG_DEBUG, "chat_expect(%s, %d)", expect, chat_timeout);
		tstr(0, expect);
		len = strlen(expect);
		if (len > 63) len = 63;

		/* Empty expects always succeed. */
		if (*expect == 0) {
			nsyslog (LOG_DEBUG, "chat_expect - got it");
			free(s);
			return 0;
		}

		/*
		 *	Hack for Connect-Info (a bit ugly tho')
		 */
		conn_hack = 0;
		if (matchpart && (q = strchr(p, '@')) != NULL && q[1] == 0) {
			len--;
			conn_hack = 1;
		}

		/* Wait for string to arrive, or timeout. */

		if (chat_timeout) {
			signal (SIGALRM, SIG_IGN);
			timed_out = 0;
			alarm(chat_timeout);
		}
		memset(buf, 0, 64);

		/* Put every char in a buffer and shift. */
		while(1) {
			if (read_char (&c, fd)) {
				if (timed_out) {
					break;
				}
				nsyslog(LOG_DEBUG, "chat_expect (%s) - got (%s) with error",
					expect, (buf + 63 - len));
				if (retries++ >= 3) {
					break;
				}
				continue;
			}
			memmove(buf, buf + 1, 62);
			buf[62] = c;
			/* See if we got it. */
			if (strncmp(expect, buf + 63 - len, len) == 0) {
				/*
				 *	Connect-Info hack again.
				 */
				if (conn_hack) {
					len = 0;
					while(1) {
						if (read_char (&c, fd))
							break;
						if (!len && c == ' ')
							continue;
						if (c == '\r' || c == '\n')
							break;
						matchpart[len++] = c;
					}
					if (c != '\r' && c != '\n')
						break;
					matchpart[len] = 0;
				}
				nsyslog (LOG_DEBUG, "chat_expect - got it");
				alarm(0);
				free(s);
				return 0;
			}
		}

		if (!timed_out) {
			nsyslog (LOG_DEBUG, "chat_expect(%s): interrupted", expect);
			alarm(0);
			free(s);
			return -1;
		}
		if (send == NULL) {
			nsyslog (LOG_DEBUG, "chat_expect(%s): timeout", expect);
			alarm(0);
			free(s);
			return -1;
		}
		nsyslog (LOG_DEBUG, "chat_expect(%s): timeout (retry)", expect);
		expect = skip(send);
		chat_send(fd, send);

		/* If there's no expect string now - I guess we succeeded? */
		if (expect == NULL) {
			alarm(0);
			free(s);
			return 0;
		}

	}
	/*NOTREACHED*/
	return -1;
}

/*
 * Return its index in sw array of special chat word or -1 if not special
 */

int get_special_index (char *word) {
	int index;

	for (index=0;sw [index].word;++index) {
		if (! strcmp (word, sw [index].word)) {
			return (index);
		}
	}
	return (-1);
}

/*
 *	Given a string, converts it to integer and sets chat timeout
 */

void set_chat_timeout (char *str) {
	char *end_conv;
	long int val;

	val = strtol (str, &end_conv, 10);
	if (*end_conv != 0) {
		nsyslog (LOG_DEBUG, "set_chat_timeout: bad integer %s", str);
		return;
	}
	if (val < 0) {
		nsyslog (LOG_DEBUG, "set_chat_timeout: negative timeout %ld", val);
		return;
	}
	chat_timeout = (int) val;
}

/*
 * Implements special chat keyword "WAIT"
 */

int special_chat_wait (char *str) {
	struct itimerval left;
	int TIO_flags;

	if (strcmp (str, "DCD")) {
		nsyslog (LOG_DEBUG, "special_chat_wait: bad wait parameter: %s", str);
		return (-1);
	}
	if (chat_timeout == 0) {
		if (ioctl (0, TIOCMIWAIT, TIOCM_CD) == -1) {
			nsyslog (LOG_DEBUG, "special_chat_wait: ioctl"
				"(0, TIOCMIWAIT, TIOCM_CD): %d(%m)", errno);
			return (-1);
		}
	} else {
		signal (SIGALRM, SIG_IGN);
		alarm (chat_timeout);
		for (;;) {
			if (getitimer (ITIMER_REAL, &left)) {
				nsyslog (LOG_DEBUG, "special_chat_wait: "
					"getitimer: %d(%m)", errno);
				alarm (0);
				return (-1);
			}
			if (ioctl (0, TIOCMGET, &TIO_flags)) {
				nsyslog (LOG_DEBUG, "special_chat_wait:"
					"ioctl(0, TIOCMGET, ...): %d(%m)", errno);
				alarm (0);
				return (-1);
			}
			if (TIO_flags & TIOCM_CAR) {
				break;
			}
			if ((left.it_value.tv_sec == 0) && (left.it_value.tv_usec == 0)) {
				nsyslog (LOG_DEBUG, "special_chat_wait: timed out");
				return (-1);
			}
			sleep (1);
		}
	}
	nsyslog (LOG_DEBUG, "Carrier detected");
	alarm (0);
	return (0);
}

/*
 *	This is the main chat loop;
 *	you have to feed it with an argument
 *	vector and a count.
 */
int chatarray(int fd, int argc, char **argv, struct auth *ai) {
	int c;
	int index;

	nsyslog (LOG_DEBUG, "chatarray: %d words", argc);

	/*
	 * Now do alternative expect-sends. Before each expect check if current
	 * expect string is a special word.
	 */
	for (c = 0;c != argc;) {
		if ((index = get_special_index (argv[c])) == -1) {
			if (chat_expect(fd, argv[c++], ai->conn_info, sizeof (ai->conn_info)) < 0) {
				return -1;
			}
			if (c == argc) {
				break;
			}
			chat_send(fd, argv[c++]);
		} else {
			nsyslog (LOG_DEBUG, "chatarray: special word: %s, "
				"index: %d, code: %d, req. words: %d",
				sw [index].word, index, sw [index].code,
				sw [index].req_words);
			if ((argc - c) < sw [index].req_words) {
				nsyslog(LOG_DEBUG, "Special chat word %s is missing parameter(s)",
					argv[c]);
				return -1;
			}
			switch (index) {
				case SPECIAL_TIMEOUT:
					set_chat_timeout (argv[c+1]);
					break;
				case SPECIAL_WAIT:
					if (special_chat_wait (argv[c+1])) {
						return (-1);
					}
					break;
				case SPECIAL_STATUS:
					update_utmp (argv[c+1], argv[c+2], ai, 0);
					break;
				default:
					break;
			}
			c += sw [index].req_words;
		}
	}
	return 0;
}

/*
 *	This is a generic chat; you can
 *	just pass a string to it.
 */
int chat(int fd, char *str, struct auth *ai)
{
	char *args[64];
	char *s;
	int i = 0;

	if (str == NULL || str[0] == 0) return 0;

	if ((s = strdup(str)) == NULL) return -1;

	/* Split string up in expect-send pairs. */
	while(*s) {
		if (*s == ' ' || *s == '\t' || i == 0) {
			if (i) {
				/* End of a string. */
				*s++ = 0;
			}
			/* Skip until next non-space. */
			while (*s == ' ' || *s == '\t')
				s++;
			if (*s == 0) continue;
			args[i++] = s;
		}
		if (*s == '"') {
			while(*s && *s != '"') {
				if (*s == '\\' && *(s+1)) s++;
				s++;
			}
		}
		if (*s) s++;
	}
	args[i] = NULL;

	/* Now call the real routine. */
	return chatarray(fd, i, args, ai);
}

#ifdef CHATPROG
int main(int argc, char **argv)
{
	argv++;
	argc--;

	return chatarray(1, argc, argv);
}
#endif
