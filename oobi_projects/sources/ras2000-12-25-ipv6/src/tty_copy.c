#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>

/* Copy data across a TCP connection. */
static int _tty_copy(int crnl, int infd, int outfd)
{
	int i, cnt, nfound;
	struct pollfd ufds [2];
	int input_blocked = 0,  output_blocked = 0;
	int rcv_bytes = 0,      send_bytes = 0;
	char rcv_buffer[8192],  send_buffer[8192];
	char *rcv_bufp = NULL, *send_bufp = NULL;
	char *s;

	/* Copy between socket and tty */
	while (1) {
		do {
			ufds[0].fd = infd;
			ufds[0].events = ufds[0].revents = 0;
			ufds[1].fd = outfd;
			ufds[1].events = ufds[1].revents = 0;
			if (input_blocked) {
				ufds[0].events |= POLLOUT;
			} else {
				ufds [1].events |= POLLIN;
			}
			if (output_blocked) {
				ufds [1].events |= POLLOUT;
			} else {
				ufds [0].events |= POLLIN;
			}
			nfound = poll (ufds, 2, -1);
		} while (nfound == -1 && errno == EINTR);

		/* Data is ready on the inputdevice. */
		if (ufds [0].revents & POLLIN) {
			send_bytes = read(infd, send_buffer, sizeof(send_buffer));
			send_bufp = send_buffer;
			if (crnl > 0) {
				for(i = 0, s = send_bufp; i < send_bytes; i++,s++)
					if (*s == '\r') *s = '\n';
			}
		}

		/* New data has become available or there is new room in the output buffer. */
		if ((ufds[0].revents & POLLIN) || (output_blocked && (ufds[1].revents & POLLOUT))) {
			output_blocked = 0;
			if (send_bytes <= 0) return(0);
			cnt = 0;
			while (!output_blocked && send_bytes > 0) {
				cnt = write(outfd, send_bufp, send_bytes);
				if (cnt < 0) switch (errno) {
					case EWOULDBLOCK:
						output_blocked = 1;
						break;
					case EINTR:
						continue;
					default:
						return(-1);
				}
				send_bytes -= cnt;
				send_bufp += cnt;
			}
		}

		/* Data is ready on the outputdevice. */
		if (ufds[1].revents & POLLIN) {
			rcv_bytes = read(outfd, rcv_buffer, sizeof(rcv_buffer));
			rcv_bufp = rcv_buffer;
		}

		/* New data has become available or there is new room in the output buffer. */
		if ((ufds[1].revents & POLLIN) || (input_blocked && ufds[0].revents & POLLOUT)) {
			input_blocked = 0;
			if (rcv_bytes <= 0) return(0);
			cnt = 0;
			while (!input_blocked && rcv_bytes > 0) {
				cnt = write(infd, rcv_bufp, rcv_bytes);
				if (cnt < 0) switch (errno) {
					case EWOULDBLOCK:
						input_blocked = 1;
						break;
					case EINTR:
						continue;
					default:
						return(-1);
				}
				rcv_bytes -= cnt;
				rcv_bufp += cnt;
			}
		}
	 }
}

/* Copy data across a TCP connection. */
int tty_copy(int crnl, int infd, int outfd)
{
	int flags, iflags, oflags;
	void (*osig)(int);
	int ret;

	/* We don't need SIGIO */
	osig = signal(SIGIO, SIG_IGN);

	/* Set file descriptors in non-blocking mode. */
	iflags = fcntl(infd, F_GETFL);
	oflags = fcntl(outfd, F_GETFL);
	flags = iflags | O_NDELAY;
	fcntl(infd, F_SETFL, flags);
	flags = oflags | O_NDELAY;
	fcntl(outfd, F_SETFL, flags);

	ret = _tty_copy(crnl, infd, outfd);

	fcntl(infd, F_SETFL, iflags);
	fcntl(outfd, F_SETFL, oflags);
	signal(SIGIO, osig);

	return ret;
}
