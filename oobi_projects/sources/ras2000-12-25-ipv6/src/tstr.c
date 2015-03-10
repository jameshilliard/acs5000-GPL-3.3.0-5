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

/*
 *	Parse string and replace escape sequences.
 */
void tstr(int sending, char *s)
{
	char *p = s;

	while(*s) {

		/* Skip quotes. */
		if (*s == '"') {
			s++;
			continue;
		}

		/* ^ for control character. */
		if (*s == '^') {
			*p++ = *++s & 0x1f;
			if (*s) s++;

		/* Some escaped character. */
		} else if (*s == '\\') {
			  switch(*++s) {
			case '\n':
				break;
			case 'r':
				*p++ = '\r';
				break;
			case 'n':
				*p++ = '\n';
				break;
			case 'b':
				*p++ = '\b';
				break;
			case 's':
				*p++ = ' ';
				break;
			case 't':
				*p++ = '\t';
				break;
			case '-':
				/* Keep minus sign escaped. */
				*p++ = '\\';
				*p++ = *s;
				break;
			case '\\':
			case 'd':
			case 'p':
			case 'l':
			case 'c':
			case 'K':
				/* These are double escaped for chat_send(). */
				if (sending) {
					*p++ = '\\';
					*p++ = *s;
					break;
				}
				/*FALLTHRU*/
			default:
				/* See if its an octal number. */
				if (*s >= '0' && *s <= '7') {
					int val = *s - '0';
					while(*++s >= '0' && *s <= '7')
						val = 8*val + (*s - '0');
					s--;
					/* '\0' and '\\' are special. */
					if (val == 0 || val == '\\') {
						*p++ = '\\';
						*p++ = val ? '\\' : 'N';
					} else
						*p++ = val;
				} else
					*p++ = *s;
			  }
			  if (*s) s++;
		} else
			/* Just a normal character. */
			*p++ = *s++;
	}
	*p = 0;
}

