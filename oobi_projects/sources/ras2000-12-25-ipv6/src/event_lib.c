/*****************************************************************
File: event_library.c

- openevt, closeevt, writeevt  - send messages to the event 
notification (event_gen) daemon.

void openevt(void);
void writeevt(int event_number, char * format, va_list parameters);
void closeevt(void);

Description 

- closeevt() closes the descriptor being sued to write to
  the fifo device (/var/run/snmp_pipe)

- openevt() opens a connection to the fifo device

- writeevt generates a event notification message, which
  will be written to the fifo device.
  Parameters :
	. event_number : number of the event
	. format : type of the parameters : 'i' = interger
                                            's' = string
					    'u' = username
                   Ex. "iss" : interger, string, string
        . va_list : list of the parameters for the evnet
****************************************************************/

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/signal.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <paths.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <sys/types.h>
#include <pwd.h>

#define MAXMSGSIZE 1024

// variables related with /var/run/snmp_pipe
static int EventPipe = -1;	// fd 

int openevt(void)
{
	if (EventPipe != -1) {
		return (0); // OK
	}

	EventPipe = open("/var/run/snmp_pipe",
                        O_RDWR | O_NONBLOCK | O_NOCTTY );
	if (EventPipe == -1) {
		syslog(4,"Event Library: Open error[%d] : %m",getpid());
		return(-1); // error
	}
	return(0); // OK
}

void closeevt(void)
{
	if (EventPipe != -1) {
		(void) fsync(EventPipe);
		(void) close(EventPipe);
		EventPipe = -1;
	}
}

void writeevt(int evtn, char * format,...)
{
	va_list params;
	int pard;	// parameter type digit
	char *pars;	// parameter type string
	int size;	// size of the message	

	struct passwd *pusr; 

	char buffer[MAXMSGSIZE+2];

	if (EventPipe == -1) { // open the connect
		if (openevt() == -1) 
			return;
	}
	
	va_start(params,format);

	// mount the first part of the message
	size = sprintf(buffer,"EVT%d %ld ",evtn, time(NULL));
	
	// add the parameters
	while (*format) {
		switch(*format++) {
			case 's': // string
				pars = va_arg(params,char *);
				size += snprintf(buffer+size,MAXMSGSIZE-size,
						"%d %s ",strlen(pars),pars);
				break;
			case 'i': // integer
				pard = va_arg(params, int);
				size += snprintf(buffer+size,MAXMSGSIZE-size,
					"%d ",pard);
				break;		
			case 'U': // user name from LOGNAME
				if ((pars = getenv("LOGNAME")) || (pars = getenv("USER"))) {
					size += snprintf(buffer+size,MAXMSGSIZE-size,
						"%d %s ",strlen(pars),pars);
					break;
				}
				// fallthrough if NULL
			case 'u': // user name 
				pusr = getpwuid(getuid());
				if (pusr == NULL) {
					pars = strdup("unknow");
				} else {
					pars = strdup(pusr->pw_name);
				}			
				size += snprintf(buffer+size,MAXMSGSIZE-size,
						"%d %s ",strlen(pars),pars);
				free(pars);
		}
		if (size >= MAXMSGSIZE) {
			break;
		}
	}
	va_end(params);
	buffer[size++] = '\n';
	size=write(EventPipe, buffer, size);
	fsync(EventPipe);
}
