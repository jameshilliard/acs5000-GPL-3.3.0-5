/*
 *  This is a dummy version of the tcpd program. It only logs a message
 *  to syslog and passes control to the actual server.
 */

#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>

#ifndef MAXPATHNAMELEN
#define MAXPATHNAMELEN 256
#endif

#define DEFAULT_DAEMON_DIR "/usr/sbin"

#define STDIN_FILENO 0

static int xstrcpy(char *tgt, size_t tgtlen, size_t *pos, const char *src)
{
	size_t srclen = strlen(src);

	if (tgtlen < *pos)
		return -1;
	if ((tgtlen - *pos) <= srclen)
		return -1;

	strcpy(tgt + *pos, src);
	*pos += srclen;

	return 0;
}

static const char *get_client(char *host, size_t hlen)
{
	static const char unknown_str[] = "unknown";
	struct sockaddr_storage ss;
	size_t size = sizeof(ss);

	if (getpeername(STDIN_FILENO, (struct sockaddr *) &ss, &size) != 0)
		return unknown_str;

	if (getnameinfo((struct sockaddr *)&ss, sizeof(ss),
			host, hlen, NULL, 0, NI_NUMERICHOST) != 0)
		return unknown_str;

	return host;
}

int main(int argc, char *argv[])
{
	size_t pos = 0;
	char host[128];
	char path[MAXPATHNAMELEN];

	/*
	 * If argv[0] is an absolute path name, ignore DEFAULT_DAEMON_DIR, and strip
	 * argv[0] to its basename.
	 */
	if (argv[0][0] == '/') {
		if(xstrcpy(path, MAXPATHNAMELEN, &pos, argv[0]) < 0)
			return 1;

		argv[0] = strrchr(argv[0], '/') + 1;
	} else {
		/* Format the string "<daemon_dir>/<argv[0]>": */
		xstrcpy(path, MAXPATHNAMELEN, &pos, DEFAULT_DAEMON_DIR);
		xstrcpy(path, MAXPATHNAMELEN, &pos, "/");
		if (xstrcpy(path, MAXPATHNAMELEN, &pos, argv[0]) < 0)
			return 1;
	}

	/* Open a channel to the syslog daemon. */
	openlog(argv[0], LOG_PID, FACILITY);

	/* Invoke the real daemon program. */
	syslog(LOG_INFO, "connect from %s", get_client(host, sizeof(host)));
	closelog();
	execv(path, argv);
	syslog(LOG_ERR, "error: cannot execute %s: %m", path);

	return 1;
}
