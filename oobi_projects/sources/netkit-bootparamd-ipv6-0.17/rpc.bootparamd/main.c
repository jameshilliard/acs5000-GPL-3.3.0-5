#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>
#include <getopt.h>
#include <unistd.h>
#include "netconfig.h"
#include <errno.h>
#include "bootparam_prot.h"

int debug = 0;
int dolog = 0;

struct addrinfo *route_addr;
struct addrinfo hints;

const char *bootpfile = "/etc/bootparams";

static char *progname;

int createServerRPC(const char *nettype)
{
	struct netconfig *nconf; /* pointer to nettype data */
	SVCXPRT *svc;            /* pointer to service handle */
	bool_t rslt;             /* return value for svc_reg() */

	/* unregister any existing copy of this service */
	svc_unreg(BOOTPARAMPROG, BOOTPARAMVERS);

	/* (struct netconfig *)getnetconfigent(nettype) */
	nconf = getnetconfigent(nettype);

	/* check for errors calling getnetconfigent() */
	if (nconf == (struct netconfig *)NULL) {
		fprintf(stderr, "Error calling getnetconfigent(%s)\n", nettype);
		if (errno) perror("Error");
		exit(1);
	}

	/* (SVCXPRT *)svc_tli_create(filedes, netconfig, bindaddr, sendsz, recvsz) */
	svc = svc_tli_create(RPC_ANYFD, nconf, NULL, 0, 0);

	/* check for errors calling svc_tli_create() */
	if (svc == (SVCXPRT *)NULL) {
		fprintf(stderr, "Error calling %s.\n", "svc_tli_create");
		fprintf(stderr, "errno: %d\n", errno);
		exit(1);
	}
	
	/* (bool_t)svc_reg(svcxprt, prognum, versnum, dispatch, netconf) */
	rslt = svc_reg(svc, BOOTPARAMPROG, BOOTPARAMVERS, bootparamprog_1, nconf);

	/* check for errors calling svc_reg() */
	if (rslt == FALSE) {
		fprintf(stderr, "Error calling svc_reg\n");
		fprintf(stderr, "PROG: %lu\nVERS: %lu\tNET: %s\n",
				BOOTPARAMPROG, BOOTPARAMVERS, nettype);
		fprintf(stderr, "errno: %d\n", errno);
		exit(1);
	}

        /* return OK */
	return(0);
}


int main(int argc, char **argv)
{
    int s, pid;
    struct stat buf;
    char nettype[50];
    char router[100];
    int c;

    // init program variable
    progname = rindex(argv[0],'/');
    if (progname) progname++;
    else progname = argv[0];

    // init router variable
    strcpy(router, "localhost");

    /* default nettype is IPv4 */
    strcpy(nettype, NETTYPE);

    while ((c = getopt(argc, argv,"ds6r:f:")) != EOF) {
	    
	switch (c) {
		
	case 'd':
	    debug = 1;
	    break;
	    
	case 'r':
	    strncpy(router, optarg, sizeof(router));
	    router[sizeof(router) - 1] = '\0';
            break;
	              
	 case 'f':
	    bootpfile = optarg;
	    break;
	    
	 case 's':
	    dolog = 1;
#ifndef LOG_DAEMON 
	    openlog(progname, 0 , 0);
#else
	    openlog(progname, 0 , LOG_DAEMON);
	    setlogmask(LOG_UPTO(LOG_NOTICE));
#endif
	    break;
	    
         case '6':
            strcpy(nettype, NETTYPE6);
            break;
                   
	 default:
		fprintf(stderr, 
			 "Usage: %s [-d] [-s] [-4] [-6] [-r router] [-f bootparmsfile]\n", 
			 argv[0]);
		exit(1);
	}
    }
    
    if (stat(bootpfile, &buf)) {
	fprintf(stderr,"%s: ", progname);
	perror(bootpfile);
	exit(1);
    }

    // Get router address
    if (getaddrinfo(router, NULL, &hints, &route_addr)) {
        fprintf(stderr, "%s: No such host <%s> as router.\n",
		progname, router);
	fprintf(stderr, "Please use option -r to especify router ou create "
		"localhost entry in /etc/hosts.\n");
        exit(1);
    }

    if (!debug) {
	pid = fork();
	if (pid < 0) {
	    perror("bootparamd: fork");
	    exit(1);
	}
	if (pid) exit(0); /* parent */
	
	/* child */
	for (s=0; s<20; s++) close(s);
	open("/", 0);
	dup2(0, 1);
	dup2(0, 2);
	s = open("/dev/tty",2);
	if ( s >= 0 ) {
	    ioctl(s, TIOCNOTTY, 0);
	    close(s);
	}
    }

    /* Init Server RPC */
    createServerRPC(nettype);

    /* this should loop indefinitely waiting for client connections */
    svc_run();

    /* if we get here, svc_run() returned */
    fprintf(stderr, "svc_run() returned\n");
    fprintf(stderr, "errno: %d\n", errno);

    /* clean up by unregistering.  then, exit */
    svc_unreg(BOOTPARAMPROG, BOOTPARAMVERS);

    // Exit
    exit(1);
}
