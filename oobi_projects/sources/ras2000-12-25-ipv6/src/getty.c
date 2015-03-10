/*
 * getty.c	This is the part that talks with the modem,
 *		does the login stuff etc.
 *
 * Version:	@(#)getty.c  1.34  13-Jan-1998  MvS.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <termios.h>
#include <errno.h>

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "server.h"
#include "server1.h"

/*
 *	Turn CR translation on or off.
 */
void crtrans(int fd, int inon, int outon) {
	struct termios tty;

	tcgetattr(fd, &tty);
	if (inon) {
		tty.c_iflag |= ICRNL|IGNCR;
	} else {
		tty.c_iflag &= ~(ICRNL|IGNCR);
	}
	if (outon) {
		tty.c_oflag |= ONLCR|OPOST;
	} else {
		tty.c_oflag &= ~(ONLCR|OPOST);
	}
	tcsetattr(fd, TCSANOW, &tty);
}

/*
 *	Disassociate from our controlling tty.
 */
void disass_ctty() {
	pid_t pid;
	int fl;

	/*
	 *	First let go of our old tty.
	 *	This looks like black magic. It is ;)
	 */
	pid = getpid();
#ifndef __ELF__
	/*
	 *	a.out systems generally don't have getsid()
	 */
	(void)setpgid(0, getpgid(getppid()));
	(void)setsid();
#else
	if (getsid(pid) != pid) {
		if (setpgid(0, getpgid(getppid())) < 0)
			nsyslog(LOG_WARNING, "setpgid: %m");
		setsid();
		//if (setsid() < 0)
		//	nsyslog(LOG_WARNING, "setsid: %m");
	}
#endif
	if ((fl = open("/dev/tty", O_RDWR)) >= 0) {
		signal(SIGHUP, SIG_IGN);
		if (ioctl(fl, TIOCNOTTY, (char *)1) < 0)
			nsyslog(LOG_WARNING, "ioctl TIOCNOTTY: %m");
		signal(SIGHUP, SIG_DFL);
		close(fl);
	}
}

static void set_clocal(int fd, int flag)
{
	struct termios tio;

	tcgetattr(fd, &tio);

	if (flag) {
		tio.c_cflag |= CLOCAL;  
	} else {
		tio.c_cflag &= ~CLOCAL;  
	}
	tcsetattr(fd, TCSANOW, &tio);
}

int wait_dcd_on(int fd, int port)
{
	int signals, flag = 0;

	ioctl(fd,TIOCMGET,&signals);
	signals = signals | TIOCM_DTR;
	ioctl(fd,TIOCMSET,&signals);

	while(TRUE) {
		ioctl(fd, TIOCMGET, &signals);
		if (signals & TIOCM_CD) { // DCD is ON
			if (flag) {
				return(0);
			} else {
				flag = 1;
			}
		} else {
			flag = 0;
		}
		sleep(1);
	}
	return(1);
}

/*
 *	Run a getty on a port.
 */
int getty(struct auth *ai, int location)
{
	struct line_cfg *lc = &lineconf[ai->nasport];
	char banner [1024];
	int fd, fl;

	disass_ctty();

	if (location == 0) {
		close(0);
		close(1);
		close(2);
	}

	/*
	 *	First lock or wait if the port is busy.
	 */

	if ((fd = serial_open(lc, location)) < 0) {
		nsyslog(LOG_ERR, "Serial Open error");
		return -1;
	}

	if ((location != 2) && ioctl(fd, TIOCSCTTY, 1) < 0) {
		nsyslog(LOG_WARNING, "TIOCSCTTY: %m");
	}

   	fl = fcntl(fd, F_GETFL, 0);
   	fl &= ~O_NONBLOCK;
   	fcntl(fd, F_SETFL, fl);

	if (!location && lc->dcd &&
		(lc->initchat == NULL || *lc->initchat == 0)) {
		if (wait_dcd_on(fd, lc->this)) {
			return(-1);
		}
	}

	if (lc->dcd) {
		set_clocal(fd, 0);
	}

	crtrans(fd,0,0 );

	if (chat(fd, lc->initchat, ai) < 0) {
		crtrans(fd, 1, 1);
		tcflush(fd,TCIOFLUSH); //[RK]Mar/08/05
		nsyslog(LOG_ERR, "initchat failed.");
		return -1;
	}
	crtrans(fd, 1, 1);

	if (location) {
    	fl = fcntl(fd, F_GETFL, 0);
    	fl |= O_NONBLOCK;
    	fcntl(fd, F_SETFL, fl);
	}

	/*
	 *	Delay just a little bit and flush junk.
	 */

	usleep(250000);
	tcflush(0, TCIOFLUSH);

	if (location) {
		return(fd);
	}

	if (lc->authtype == AUTH_NONE || lc->protocol == P_PPP_ONLY) {
		return(0);
	}	

	/*
	 *	Show the banner and say hello.
	 */
	// SOCKET_CLIENT: test it as TELNET or RLOGIN
	if (lc->issue && lc->issue[0]) {
		expand_format (banner, sizeof (banner), lc->issue, ai);
		fprintf(stdout, "%s\n", banner);
		fflush(stdout);
	}

	return(fd);
}
/* 
 * routines for dynamic mode support
 * wait "enter" from the terminal to
 * start the bidirect session
 *
 */
int get_bidirect(struct auth *ai)
{
	struct line_cfg *lc = &lineconf[ai->nasport];
	char banner [1024];
	char c;
	int fd, fl;

	disass_ctty();

	close(0);
	close(1);
	close(2);

	if ((fd = serial_open(lc, 0)) < 0) {
		nsyslog(LOG_ERR, "Serial Open error");
		return -1;
	}

	/*
	 *	Now force the new tty into becoming our
	 *	controlling tty. This will also kill other
	 *	processes hanging on this tty.
	 */
	if (ioctl(fd, TIOCSCTTY, 1) < 0) {
		nsyslog(LOG_WARNING, "TIOCSCTTY: %m");
	}

    fl = fcntl(fd, F_GETFL, 0);
    fl &= ~O_NONBLOCK;
    fcntl(fd, F_SETFL, fl);

	if (lc->dcd && (lc->initchat == NULL || *lc->initchat == 0)) {
		if (wait_dcd_on(fd, lc->this)) {
			return(-1);
		}
	}

	if (lc->dcd) {
		set_clocal(fd, 0);
	}

    crtrans(fd,0,0 );

    if (chat(fd, lc->initchat, ai) < 0) {
        crtrans(fd, 1, 1);
        tcflush(fd,TCIOFLUSH); //[RK]Mar/08/05
        nsyslog(LOG_ERR, "initchat failed.");
        return -1;
    }

	crtrans(fd, 0, 1);
	/*
	 *	Loop, and wait for ENTER.
	 */
	while(1) {
		if (read(fd, &c, 1) != 1) {
			nsyslog(LOG_INFO, "error in read");
			return -1;
		}
		if (c == 0x0D) {
			// detect the enter
			SET_BIDIR_LOGIN(lc->protocol, lc->conntype, lc->this);
			break;
		}
	}

	crtrans(fd, 1, 1);

	/*
	 *	Delay just a little bit and flush junk.
	 */
	usleep(250000);
	tcflush(0, TCIOFLUSH);

	/*
	 *	Show the banner and say hello.
	 */
	if (lc->issue && lc->issue[0]) {
		expand_format (banner, sizeof (banner), lc->issue, ai);
		fprintf(stdout, "%s\n", banner);
		fflush(stdout);
	}

	return 0;
}
