#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <netconfig.h>
#include <errno.h>

#include "bootparam_prot.h"

/* #define bp_address_u bp_address */
#include <stdio.h>

#include "../version.h"
const char callbootd_rcsid[] = 
  "$Id: callbootd.c,v 1.1.1.1 2007/10/10 11:58:27 sergio Exp $";

static int printgetfile(bp_getfile_res *res);
static int printwhoami(bp_whoami_res *res);

int broadcast;

char cln[MAX_MACHINE_NAME+1];
char dmn[MAX_MACHINE_NAME+1];
char path[MAX_PATH_LEN+1];


static int eachres_whoami(bp_whoami_res *resultp, struct sockaddr_in *raddr)
{
    struct hostent *he;

    he = gethostbyaddr((char*)&raddr->sin_addr.s_addr,4,AF_INET);
    printf("%s answered:\n", he ? he->h_name : inet_ntoa(raddr->sin_addr));
    printwhoami(resultp);
    printf("\n");
    return 0;
}


static int eachres_getfile(bp_getfile_res *resultp, struct sockaddr_in *raddr)
{
    struct hostent *he;

    he = gethostbyaddr((char*)&raddr->sin_addr.s_addr,4,AF_INET);
    printf("%s answered:\n", he ? he->h_name : inet_ntoa(raddr->sin_addr));
    printgetfile(resultp);
    printf("\n");
    return 0;
}


static CLIENT *createClientRPC(const char *host, const char *nettype)
{
	struct netconfig *nconf;
	struct netconfig *getnetconfigent();
	struct netbuf svcaddr;
	CLIENT *cl;

	/* initialize the struct netbuf space */
	svcaddr.maxlen = 200;
	svcaddr.buf = (char *)malloc(svcaddr.maxlen);
        if (svcaddr.buf == (char *)NULL) {
		fprintf(stderr, "Error calling malloc() for struct netbuf\n");
		fprintf(stderr, "errno: %d\n", errno);
		exit(1);
	}

	// get net configuration 
	nconf = getnetconfigent(nettype);
	if (nconf == (struct netconfig *) NULL) {
		fprintf(stderr, "getnetconfigent for udp failed\n");
		exit(1);
	}

	printf("Getting information from server...\n");
	
	// get address from rpcbind
	if (rpcb_getaddr(BOOTPARAMPROG, BOOTPARAMVERS, nconf, &svcaddr, host) == FALSE) {
		fprintf(stderr, "Error calling rpcb_getaddr()\n");
		fprintf(stderr, "PROG: %lu\ VERS: %lu NET: %s\n",
			BOOTPARAMPROG, BOOTPARAMVERS, nettype);
		if (errno) perror("Error");
		clnt_pcreateerror("Clnt error");
		exit(1);
	}

	// create rpc connection
	cl = clnt_tli_create(RPC_ANYFD, nconf, &svcaddr, BOOTPARAMPROG,
		BOOTPARAMVERS, 0, 0);

	// check to make sure clnt_tli_create() didn't fail
	if (cl == (CLIENT *)NULL) {
		fprintf(stderr, "Error calling clnt_tli_create()\n");
		perror("Error");
		clnt_pcreateerror("Clnt error: ");
		exit(1);
	}

	// Return
	return(cl);
}


int main(int argc, char **argv)
{
    char *server, nettype[50];
  
    bp_whoami_arg whoami_arg;
    bp_whoami_res *whoami_res, stat_whoami_res;
    bp_getfile_arg getfile_arg;
    bp_getfile_res *getfile_res, stat_getfile_res;
      
    CLIENT *clnt = NULL;

    struct addrinfo *ai;
    struct addrinfo hints;

    stat_whoami_res.client_name = cln;
    stat_whoami_res.domain_name = dmn;

    stat_getfile_res.server_name = cln;
    stat_getfile_res.server_path = path;

    // Check parameters
    if (argc < 3) {
	fprintf(stderr,
		"Usage: %s server procnum (IP-addr | host fileid)\n", argv[0]);
	exit(1);
    }

    // Init vars  
    server = argv[1];

    // Init broadcast
    if (!strcmp(server , "all")) {
        fprintf(stderr, "Broadcast not supported.\n");
        exit(1);
        //broadcast = 1;
    }

    // Init IP address structure
    memset(&hints, '\0', sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo (argv[2], NULL, &hints, &ai) != 0) {
	fprintf(stderr, "Wrong procnum address: %s\n", argv[2]);
	exit(1);
    }

    //check nettype
    if (ai->ai_addr->sa_family == AF_INET6) {
	    strcpy(nettype, NETTYPE6);
    }
    else {
	    strcpy(nettype, NETTYPE);
    }

    // Create RPC connection
    if (!broadcast) {
	clnt = createClientRPC(server, nettype);
    }

    switch (argc) {

     case 3:

	// configure address parameters
	whoami_arg.client_address.address_type = IP_ADDR_TYPE;

	// Check addr len
        if (ai->ai_addrlen > sizeof(whoami_arg.client_address.bp_address_u.ip_addr.sa_data)) {
		fprintf(stderr, "Error in addrlen: %d\n", ai->ai_addrlen);
		exit(1);
	}
		
	// set address family
	whoami_arg.client_address.bp_address_u.ip_addr.sa_family =
		ai->ai_addr->sa_family;

	// set ip address
	memset(&whoami_arg.client_address.bp_address_u.ip_addr.sa_data[0], 0,
		sizeof(whoami_arg.client_address.bp_address_u.ip_addr.sa_data));
	memcpy(&whoami_arg.client_address.bp_address_u.ip_addr.sa_data[0],
		ai->ai_addr->sa_data, ai->ai_addrlen);
        
	if (!broadcast) {

             // send request
             whoami_res = bootparamproc_whoami_1(&whoami_arg, clnt);

             // handle answer
	     if (printwhoami(whoami_res)) {
		     fprintf(stderr, "Bad answer returned from server %s\n",
			     server);
	             exit(1);
	     } 
	     else {
		     exit(0);
	     }
	 } 
	 else {

	     /* 
	     clnt_stat = clnt_broadcast(BOOTPARAMPROG, BOOTPARAMVERS, 
				      BOOTPARAMPROC_WHOAMI,
				      (xdrproc_t) xdr_bp_whoami_arg, 
				      (void *) &whoami_arg, 
				      (xdrproc_t) xdr_bp_whoami_res, 
				      (void *) &stat_whoami_res,
				      (resultproc_t) eachres_whoami);
	     exit(0);
	     */
	 }
	 
     case 4:

	 // set parameters
	 getfile_arg.client_name = argv[2];
	 getfile_arg.file_id = argv[3];
	 
	 if (!broadcast) {

	     // send request
	     getfile_res = bootparamproc_getfile_1(&getfile_arg, clnt);
	     
	     // handle answer
	     if (printgetfile(getfile_res)) {
		 fprintf(stderr, "Bad answer returned from server %s\n", 
			 server);
		 exit(1);
	     } 
	     else {
		 exit(0);
	     }
	 } 
	 else {
		 /*
	     clnt_stat=clnt_broadcast(BOOTPARAMPROG, BOOTPARAMVERS, 
				      BOOTPARAMPROC_GETFILE,
				      (xdrproc_t) xdr_bp_getfile_arg, 
				      (void *) &getfile_arg, 
				      (xdrproc_t) xdr_bp_getfile_res, 
				      (void *) &stat_getfile_res,
				      (resultproc_t) eachres_getfile);
	     exit(0);
	     */
	 }
	 
     default:
	 fprintf(stderr, "Usage: %s server procnum (IP-addr | host fileid)\n", 
		 argv[0]);
	 exit(1);
    }
}


int printwhoami(bp_whoami_res *res)
{
	char str_addr[50];
	int len;

	struct sockaddr *addr = (struct sockaddr *)
		&res->router_address.bp_address_u.ip_addr;

        if (!res) {
		fprintf(stderr,"Null answer!!!\n");
		return(1);
	}

	switch (addr->sa_family) {
		
		case AF_INET6:
			len = sizeof(struct sockaddr_in6);
			break;
			
		case AF_INET:
		default:
			len = sizeof(struct sockaddr_in);
			break;
	}

	printf("client_name:\t%s\ndomain_name:\t%s\n",
	       res->client_name, res->domain_name);

	if ((getnameinfo(addr, len, str_addr, sizeof(str_addr),
			NULL, 0, NI_NUMERICHOST)) == 0) {
		printf("Router:\t\t%s\n", str_addr);
	}
	else {
		perror("Error getting router IP address");
	}

	return(0);
}


static int printgetfile(bp_getfile_res *res)
{
	char str_addr[50];
	int len;

	struct sockaddr *addr = (struct sockaddr *)
		&res->server_address.bp_address_u.ip_addr;

	if (!res) {
		fprintf(stderr,"Null answer!!!\n");
		return(1);
	}
        
	switch (addr->sa_family) {

		case AF_INET6:
			len = sizeof(struct sockaddr_in6);
			break;

		case AF_INET:
		default:
			len = sizeof(struct sockaddr_in);
			break;
	}

	if (getnameinfo(addr, len, str_addr, sizeof(str_addr),
			NULL, 0, NI_NUMERICHOST)) {
		perror("Error getting server IP address");
		str_addr[0] = '\0';
	}

	printf("server_name:\t%s\nserver_address:\t%s\npath:\t\t%s\n",
	       res->server_name, str_addr, res->server_path);

	// return OK
	return(0);
}
