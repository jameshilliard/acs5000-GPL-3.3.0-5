#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "xdebug.h"

static int XDEBUG_FLAG = 0;

static int xdebug_fd = -1;

int test_xflag(int bit)
{
	return(XDEBUG_FLAG & bit);
}

int set_xprintf(int flag)
{
	int old = XDEBUG_FLAG;

	XDEBUG_FLAG = flag;

	return(old);
}

void xprintf(int flag, const char *fmt, ...)
{
	char buf[1024];
	va_list ap;

	if (!(flag & XDEBUG_FORCE) && !(flag & XDEBUG_FLAG)) {
		if (xdebug_fd >= 0) {
			close(xdebug_fd);
			xdebug_fd = -1;
		}
		return;
	}

	if (xdebug_fd < 0) {
		if ((xdebug_fd = open("/dev/console", O_WRONLY, 0)) <= 0) {
			return;
		}
	}

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	write(xdebug_fd, buf, strlen(buf));
	va_end(ap);
}
