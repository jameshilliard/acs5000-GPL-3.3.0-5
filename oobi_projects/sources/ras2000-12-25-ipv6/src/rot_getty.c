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
#include <asm/prxk.h>

#include <cyc.h>

struct MachineType mt;

static int baud_table[] = {
	0, 50, 75, 110, 134, 150, 200, 300, 600, 1200, 1800, 2400, 4800,
	9600, 14400, 19200, 28800, 38400, 57600, 76800, 115200, 230400, 
	460800, 500000, 576000, 921600, 1000000, 1152000, 1500000, 2000000, 
	2500000, 3000000, 3500000, 4000000, 0 };

int get_baud_table_index(int BaudRate)
{
	int Speed;

	for (Speed=1; Speed < sizeof(baud_table)/sizeof(int); Speed++) {
		if (BaudRate == baud_table[Speed]) break;
	}

	if (Speed >= sizeof(baud_table)/sizeof(int)) {
		nsyslog(LOG_WARNING,"Unknown baud rate requested. Setting to 9600");
		Speed = B9600;
	}

	return(Speed);
}
 
int get_baud_table_speed(int Speed)
{
	if (Speed && Speed < sizeof(baud_table)/sizeof(int)) {
		return(baud_table[Speed]);
	}

	return(9600UL);
}

/*
 *	Reset the terminal to a reasonably sane state.
 */
void maketermsane(int fd) {
	struct termios tty;

	tcgetattr(fd, &tty);
	tty.c_iflag &= ~IGNCR;
	tty.c_oflag |= OPOST|ONLCR;
	tty.c_lflag |= ISIG|IEXTEN|ICANON|ECHO|ECHOE|ECHOK|ECHOCTL|ECHOKE;
	tcsetattr(fd, TCSANOW, &tty);
}

#ifndef CYCLADES
/*
 *	See if a device is locked.
 */
static int islocked(char *lock) {
	FILE *fp;
	int pid;

	if (!*lock)
		return 0;
	if ((fp = fopen(lock, "r")) == NULL)
		return 0;
	if (fscanf(fp, "%d", &pid) != 1 || (kill(pid, 0) < 0 && errno == ESRCH)) {
		fclose(fp);
		unlink(lock);
		return 0;
	}
	fclose(fp);
	return pid;
}

/*
 *	Create a lockfile.
 *	Returns: -1   some error
 *			0   success
 *			1   already locked.
 */
static int dolock(char *lock) {
	char tmp[MAXPATHLEN+1];
	FILE *fp;
	int i;

	if (!*lock)
		return 0;

	snprintf (tmp, sizeof (tmp), "%s.%d", lock, getpid ());
	unlink(tmp);

	/*
	 *	Already locked?
	 */
	if (islocked(lock))
		return 1;
	/*
	 *	Try to lock.
	 */
	umask(0);
	if ((fp = fopen(tmp, "w")) == NULL) {
		nsyslog(LOG_ERR, "%s: %m", tmp);
		return -1;
	}
	fprintf(fp, "%10d portslave root\n", getpid());
	fclose(fp);

	for(i = 0; i < 5; i++) {
		if (i > 0) sleep(5);
		islocked(lock);
		if (link(tmp, lock) == 0) break;
	}
	unlink(tmp);

	return (i == 5) ? 1 : 0;
}
#endif

static char buf_tty[32];

char *get_tty_text(struct line_cfg *lc)
{
	if (lc->virtual && strchr(lc->tty, ':') == NULL) {
		snprintf(buf_tty, sizeof(buf_tty), "%s:%d", lc->tty, lc->socket_port);
	} else {
		strncpy(buf_tty, lc->tty, sizeof(buf_tty));
	}
	return(buf_tty);
}

struct termios old_tio;

void serial_close(int fd)
{
	//tcflush(fd, TCIOFLUSH);
	//tcsetattr(fd, TCSANOW, &old_tio);
	tcsetattr(fd, TCSADRAIN, &old_tio);
	close(fd);
}

void makeRawOrStty(int devFd, struct line_cfg *pconf)
{
	int rtn;
	char cmd[256];
	struct termios PortSettings;

	tcgetattr(devFd, &PortSettings);
	/* Set the serial port to raw mode */
	//cfmakeraw(&PortSettings);
	//cfmakeraw sets the terminal attributes as follows:
	PortSettings.c_iflag &=
		~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);
	PortSettings.c_oflag &= ~OPOST;
	PortSettings.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
	// We can not change parity and datasize here !!!
	//PortSettings.c_cflag &= ~(CSIZE|PARENB);
	//PortSettings.c_cflag |= CS8;

	/* Enable HANGUP on close */ //VER!!
	if (!pconf->data_buffering && !pconf->alarm) PortSettings.c_cflag |= HUPCL;

	/* Disable break handling */
	PortSettings.c_iflag = (PortSettings.c_iflag & ~BRKINT) | IGNBRK;

	/* Write the port settings to device */
	tcsetattr(devFd, TCSANOW, &PortSettings);

	if(pconf->sttyCmd) {//VER!!
		/* Create the stty command to be executed */
		sprintf(cmd, "/bin/stty -F /dev/%s %s", pconf->tty, pconf->sttyCmd);
		rtn = system(cmd);
		if(rtn != 0) {
			nsyslog(LOG_ERR, "Error in executing the stty command by the system function");
		}
    }
}

/*
 *	Open the serial line and put it into raw/no echo mode.
 */
int serial_open(struct line_cfg *pconf, int location) {
	int fd, fl;
	struct termios tio;
	char tty[64];
	tcflag_t PDataSize;
	speed_t Speed;
    int MLines;

	if (pconf->tty == NULL || pconf->tty[0] == 0) {
		nsyslog(LOG_ERR, "no tty specified");
		return -1;
	}
	if (pconf->tty[0] != '/')
		strcpy(tty, "/dev/");
	else
		tty[0] = 0;
	strcat(tty, pconf->tty);

	if ((fd = open(tty, O_RDWR|O_NOCTTY|O_NONBLOCK)) < 0) {
		nsyslog(LOG_ERR, "%s: %m", tty);
		return -1;
	}

	tcgetattr(fd, &old_tio);
	tio = old_tio;

	if (!location) {
//Toggle DTR in case of modem hang
		cfsetospeed(&tio, B0);
		cfsetispeed(&tio, B0);
		tcsetattr(fd, TCSANOW, &tio);
		sleep(1);
//REALLY Make sure, toggle it again
		cfsetospeed(&tio, B0);
		cfsetispeed(&tio, B0);
		tcsetattr(fd, TCSANOW, &tio);
		sleep(1);
		fcntl(fd,F_SETFL,fcntl(fd,F_GETFL) & ~(O_NDELAY));
	}

	ioctl(fd,TIOCMGET,&MLines);
	MLines = MLines | TIOCM_DTR;
	ioctl(fd,TIOCMSET,&MLines);

	//get_machine_type(&mt);
	MLines = pconf->media | CYC_SET_MEDIA;
	if (ioctl(fd, TIOCSERCYC, &MLines) < 0) {
		nsyslog(LOG_ERR, "TIOCSERCYC %s: %m", tty);
	}

	tio = old_tio;
#ifdef PMD
	if (pconf->protocol == P_IPDU) {
		tio.c_iflag = IGNPAR|IGNBRK;
	} else 
#endif
	{
		tio.c_iflag = IGNPAR|ICRNL|IGNBRK;
	}
	tio.c_oflag = OPOST|ONLCR;
	tio.c_lflag = 0; /* ICANON|ECHO|ECHONL*/

	if (location) {
		tio.c_cflag = CREAD|CS8;
	} else {
		tio.c_cflag = CREAD|CS8|HUPCL;
	}

	tio.c_cflag |= CLOCAL;

	switch (pconf->flow) {
		case FLOW_NONE:
			break;
		case FLOW_HARD:
			tio.c_cflag |= CRTSCTS;
			break;
		case FLOW_SOFT:
			//tio.c_iflag |= IXON|IXOFF;
			tio.c_iflag |= IXON|IXOFF|IXANY;
			tio.c_cc[VSTART] = 17;
			tio.c_cc[VSTOP]  = 19;
			break;
	}

	Speed = get_baud_table_index(pconf->speed);

	switch (pconf->StopBits) {
		case 2:
			tio.c_cflag = tio.c_cflag | CSTOPB;
			break;
		case 1:
		default:
			/* there is no support to 1.5 stop bits defaults to 1 */
			tio.c_cflag = tio.c_cflag & ~CSTOPB;
			break;
	}

	switch (pconf->Parity) {
		case 2:
			tio.c_cflag = tio.c_cflag | PARENB | PARODD;
			break;
		case 3:
			tio.c_cflag = (tio.c_cflag | PARENB) & ~PARODD;
			break;
		case 1:
		default:
			/* There's no support for MARK/SPACE parity so sets no parity */
			tio.c_cflag = tio.c_cflag & ~PARENB;
			break;
	}

	switch (pconf->DataSize) {
		case 5:
			PDataSize = CS5;
			break;
		case 6:
			PDataSize = CS6;
			break;
		case 7:
			PDataSize = CS7;
			break;
		case 8:
		default:
			PDataSize = CS8;
			break;
	}
	tio.c_cflag = tio.c_cflag & ((tio.c_cflag & ~CSIZE) | PDataSize);

	// Setting speed above 38400 correctly
	cfsetospeed(&tio, Speed);
	cfsetispeed(&tio, Speed);

	for (fl=0; fl < 3; fl++) {
		if (tcsetattr(fd, TCSANOW, &tio) != 0) {
			if (fl == 2) {
				nsyslog(LOG_ERR, "tcsetattr %s: %m", tty);
				return -1;
			}
			sleep(2);
		} else {
			break;
		}
	}

	if ((pconf->protocol != P_SOCKET_SERVER) &&
		(pconf->protocol != P_SOCKET_SERVER_SSH) &&
		(pconf->protocol != P_SOCKET_SERVER_RAW) &&
		(pconf->protocol != P_SOCKET_SSH)
#ifdef BIDIRECT
		&& NOT_BIDIR_TCP(pconf->protocol, pconf->conntype, pconf->this) 
#endif
#ifdef PMD
		&& (pconf->protocol != P_IPDU)
#endif
		) {
		(void)dup2(fd, 0);
		(void)dup2(fd, 1);
		(void)dup2(fd, 2);

		if (fd > 2) {
			close(fd);
			fd = 0;
		}
	}

	tcflush(fd, TCIOFLUSH);

	makeRawOrStty(fd, pconf); 

	return fd;
}
