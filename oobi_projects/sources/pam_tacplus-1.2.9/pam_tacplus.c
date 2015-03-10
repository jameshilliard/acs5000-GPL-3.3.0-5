/* pam_tacplus.c	PAM interface for TACACS+ protocol.
 * 
 * Copyright 1998,1999,2000 by Pawel Krawczyk <kravietz@ceti.pl>
 *
 * See end of this file for copyright information.
 * See file `CHANGES' for revision history.
 */	
#include <unistd.h>
#include <netinet/in.h> //for in_addr
#include <arpa/inet.h>  
#include <netdb.h>      //for NETDB res functions
#include <sys/types.h>  


#include <stdlib.h>		/* malloc */
#include <syslog.h>
#include <netdb.h>	
#include <sys/socket.h>		/* in_addr */
#include <sys/ioctl.h>   //[GY]2006/Jan/24  Group_Auth_Box
#include <net/if.h>      //[GY]2006/Jan/24  Group_Auth_Box

#ifdef IPv6enable
#include <ifaddrs.h>
#include <getifaddrs6.h>
#endif

#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdarg.h>		/* va_ */
#include <signal.h>
#include <string.h> /* strdup */
#include <unistd.h>

// [GY] 2005/aug/25  clean warnings
#include <stdio.h>
#include <ctype.h>
#include <time.h>

#include "tacplus.h"
#include "libtac.h"
#include "pam_tacplus.h"
#include "support.h"

#define PAM_SM_AUTH
#define PAM_SM_ACCOUNT
#define PAM_SM_SESSION
/* #define PAM_SM_PASSWORD */

#include <security/pam_modules.h>
#include <security/pam_appdata.h>

// [GY] 2005/Aug/31  clean warnings
PAM_EXTERN int _pam_account(pam_handle_t *pamh, int argc, const char **argv,  int type);
int _pam_send_account(int tac_fd, int type, char *user, char *tty);

/* support.c */
extern char tac_vers[MAX_TAC_VERS];
extern int max_tac_vers;
extern struct sockaddr_storage tac_srv[TAC_MAX_SERVERS];
extern int tac_srv_no;
extern char *tac_service;
extern char *tac_protocol;
extern unsigned long _getserveraddr (char *serv);
extern int tacacs_get_password (pam_handle_t * pamh, int flags
		     ,int ctrl, char **password);
extern int converse (pam_handle_t * pamh, int nargs
	  ,struct pam_message **message
	  ,struct pam_response **response);
extern void _pam_log (int err, const char *format,...);
extern void *_xcalloc (size_t size);

/* libtac */
extern char *tac_secret;
extern int tac_encryption;

/* address of server discovered by pam_sm_authenticate */
static struct sockaddr_storage active_server = { 0 };

/* accounting task identifier */
static short int task_id = 0;

static const int SIZE_HOSTNAME=256;

static struct pam_appl_data *get_cyc_appl(pam_handle_t *pamh)
{
  struct pam_conv *conv;
  struct pam_appl_data *pappl;

  if (pam_get_item(pamh, PAM_CONV, (const void **)(void *)&conv) != PAM_SUCCESS) {    // [GY] 2005/Sep/19  clean warning
    return(NULL);
  }

  pappl = (struct pam_appl_data *) conv->appdata_ptr;

  if (!pappl || pappl->id != 0x1234fedc) {
    return(NULL);
  }

  return(pappl);
}

/* authenticates user on remote TACACS+ server
 * returns PAM_SUCCESS if the supplied username and password
 * pair is valid 
 */
PAM_EXTERN 
int pam_sm_authenticate (pam_handle_t * pamh, int flags,
		     int argc, const char **argv)
{
  int ctrl, retval;
  const char *user;
  char *pass;
  char *tty;
  int srv_i;
  int tac_fd;
  int status = PAM_AUTHINFO_UNAVAIL;
  char *prompt = NULL;
  struct pam_appl_data *pappl;

  user = pass = tty = NULL;

  pappl = get_cyc_appl(pamh);

  //[RK]Jun/02/05 - Group Authorization Support - removed the get_str
  if (pappl) {
  	ctrl = _pam_parse (0, pappl->port, argc, argv);
  } else {
  	ctrl = _pam_parse (0,0, argc, argv);
  }

  if (ctrl & PAM_TAC_DEBUG)
    syslog (LOG_DEBUG, "%s: called (pam_tacplus v%hu.%hu.%hu)"
	, __FUNCTION__, PAM_TAC_VMAJ, PAM_TAC_VMIN, PAM_TAC_VPAT);

  (void) pam_get_item(pamh,PAM_USER_PROMPT,(const void **)(void *)&prompt);    // [GY] 2005/Sep/19  clean warning
  if (prompt == NULL) prompt = "username: ";
  retval = pam_get_user(pamh, &user, prompt);

  if (retval != PAM_SUCCESS || user == NULL || *user == '\0')
    {
      _pam_log (LOG_ERR, "unable to obtain username");
      return PAM_USER_UNKNOWN;
    }

  if (ctrl & PAM_TAC_DEBUG)
    syslog (LOG_DEBUG, "%s: user [%s] obtained", __FUNCTION__, user);
  
  /* set username */
  /*
  retval = pam_set_item(pamh, PAM_USER, user);
  if(retval != PAM_SUCCESS) {
	  _pam_log(LOG_ERR, "unable to set username item");
	  return PAM_AUTHINFO_UNAVAIL;
  }
  */

  /*
  retval = pam_get_item (pamh, PAM_USER, (const void **) &user);
  if(retval != PAM_SUCCESS) {
	  _pam_log(LOG_ERR, "unable to re-retrieve username item");
	  return PAM_AUTHINFO_UNAVAIL;
  }
  */

  /* XXX uwzgledniac PAM_DISALLOW_NULL_AUTHTOK */

  /* grab the password (if any) from the previous authentication layer */
  retval = pam_get_item(pamh, PAM_AUTHTOK, (const void **)(void *)&pass);    // [GY] 2005/Sep/19  clean warning

  if (retval != PAM_SUCCESS || pass == NULL || *pass == '\0')
    pass = NULL;
  else
    pass = strdup(pass);

  /* no previous password: maybe get one from the user */
  if (!pass && (ctrl & PAM_USE_FIRST_PASS)) {
    _pam_log(LOG_ERR, "unable to re-retrieve password from previous layer");
    return PAM_AUTHINFO_UNAVAIL;
  } /* end of password == NULL */

  if (!pass) {
    retval = tacacs_get_password (pamh, flags, ctrl, &pass);
    if (retval != PAM_SUCCESS || pass == NULL || *pass == '\0') {
      _pam_log (LOG_ERR, "unable to obtain password");
      return PAM_CRED_INSUFFICIENT;
    }
  }

  retval = pam_set_item (pamh, PAM_AUTHTOK, pass);
  if (retval != PAM_SUCCESS)
    {
      _pam_log (LOG_ERR, "unable to set password");
      return PAM_CRED_INSUFFICIENT;
    }

  if (ctrl & PAM_TAC_DEBUG)
    syslog (LOG_DEBUG, "%s: pass [%s] obtained", __FUNCTION__, pass);

  tty = _pam_get_terminal(pamh);

  if (!strncmp (tty, "/dev/", 5))
    {
      tty += 5;
    }

  if (ctrl & PAM_TAC_DEBUG)
    syslog (LOG_DEBUG, "%s: tty [%s] obtained", __FUNCTION__, tty);

  for (srv_i = 0; srv_i < tac_srv_no; srv_i++)
    {
      char *msg = NULL; int st, tac_ver, ixvers = 0;
req_again:
	  tac_ver = tac_vers[ixvers];
  	  if (ctrl & PAM_TAC_DEBUG) {
  		char str[INET6_ADDRSTRLEN] = "";
		syslog (LOG_DEBUG, "%s: trying srv %d tac_ver %d ip %s",
			__FUNCTION__, srv_i, tac_ver, xinet_ntoa((struct sockaddr *)&tac_srv[srv_i],str,sizeof(str)) );
  	  }
      tac_fd = tac_connect_single(tac_srv[srv_i]);
      if (tac_fd < 0)
	{
	  _pam_log (LOG_ERR, "connection failed srv %d: %m", srv_i);
	  if (srv_i == tac_srv_no-1) /* XXX check if OK */
	    {			/* last server tried */
	      _pam_log (LOG_ERR, "no more servers to connect");
	      return status;
	    }
		continue;
	}
      if (tac_authen_pap_send (tac_fd, (char *)user, pass, tty, tac_ver) < 0) //[GY] 2005/Sep/19  clean warning
	{
	  _pam_log (LOG_ERR, "error sending auth req to TACACS+ server");
	}
      else
	{
read_again:
	  st = tac_authen_pap_read (tac_fd, &msg);
_pam_log (LOG_ERR, "tac_authen_pap_read st %d", st);
	  if (st == TAC_PLUS_AUTHEN_STATUS_GETUSER) {
        if (!tac_authen_pap_cont_send (tac_fd, user)) {
	      if (msg) free (msg);
		  goto read_again;
	    } else {
		  goto send_error;
	    }
	  }
	  if (st == TAC_PLUS_AUTHEN_STATUS_GETPASS) {
        if (!tac_authen_pap_cont_send (tac_fd, pass)) {
	      if (msg) free (msg);
		  goto read_again;
	    } else {
		  goto send_error;
	    }
	  }
	  if (st == TAC_PLUS_AUTHEN_STATUS_PASS) {
	    /* OK, we got authenticated; save the server that
	       accepted us for pam_sm_acct_mgmt and exit the loop */
	    status = PAM_SUCCESS;
	    active_server = tac_srv[srv_i];
	    close(tac_fd);
		if (msg) free(msg);
	    break;
	  }
send_error:
		if (++ixvers < max_tac_vers) {
	  		if (msg) free (msg);
	        close(tac_fd);
			goto req_again;
		}
	      _pam_log (LOG_ERR, "auth failed: %s", msg ? msg : "unknown error");
	      status = PAM_AUTH_ERR;
	  if (msg) free (msg);
	}
      close(tac_fd);
      /* if we are here, this means that authentication failed
	 on current server; break if we are not allowed to probe
	 another one, continue otherwise */
      if (!ctrl & PAM_TAC_FIRSTHIT)
	break;
    }
  if (ctrl & PAM_TAC_DEBUG)
    syslog (LOG_DEBUG, "%s: exit", __FUNCTION__);
  bzero (pass, strlen (pass));
  free(pass);
  pass = NULL;
  return status;

}				/* pam_sm_authenticate */

/* no-op function to satisfy PAM authentication module */ 
PAM_EXTERN 
int pam_sm_setcred (pam_handle_t * pamh, int flags,
		int argc, const char **argv)
{
  struct pam_appl_data *pappl;
  int ctrl;

  pappl = get_cyc_appl(pamh);

  //[RK]Jun/02/05 - Group Authorization Support - removed the get_str
  if (pappl) {
  	ctrl = _pam_parse (0, pappl->port, argc, argv);
  } else {
  	ctrl = _pam_parse (0,0, argc, argv);
  }

  if (ctrl & PAM_TAC_DEBUG)
    syslog (LOG_DEBUG, "%s: called (pam_tacplus v%hu.%hu.%hu)"
	, __FUNCTION__, PAM_TAC_VMAJ, PAM_TAC_VMIN, PAM_TAC_VPAT);

  return PAM_SUCCESS;
}				/* pam_sm_setcred */

#ifdef IPv6enable
int isLocalAddress(const char *ipAddress) {
	struct sockaddr_storage addr;
	char ipStr[INET6_ADDRSTRLEN] = "";
	const char *error = xinet_aton(ipAddress,&addr);
	
	if (error != NULL) {
		// invalid address string
		if (PAM_TAC_DEBUG) {
			syslog (LOG_DEBUG,"error: %s - %s",error,ipAddress);
		}
		return 0;
	}
	
	xinet_ntoa((struct sockaddr *)&addr,ipStr,sizeof(ipStr));

	{
		int localAddress = 0;
		struct ifaddrs *addrs;
		char str[INET6_ADDRSTRLEN];
		
		int n = getifaddrs6(&addrs);
		if (n == 0) {
			struct ifaddrs *i;
			for (i = addrs ;i != NULL; i = i->ifa_next) {
				str[0] = '\0';
				xinet_ntoa(i->ifa_addr,str,sizeof(str));
				if (!strcasecmp(str,ipStr)) {
					localAddress = 1;
					break;
				}
			}
			freeifaddrs6(addrs);
		}
		return localAddress;
	}
}
#else
int isLocalAddress(const char *ipAddress) {

	int fd_aux;
	struct ifreq ifr;
	struct sockaddr_in * sin;
	char IPADDRESS[INET6_ADDRSTRLEN] = "";
	
	// GET IP ADDRESS FROM eth0   [GY]2006/Jan/24  Group_Auth_Box
	fd_aux = socket(AF_INET,SOCK_STREAM,0);
	strcpy(ifr.ifr_name, "eth0");
	ifr.ifr_addr.sa_family = AF_INET;
	//[RK]Mar/16/06 - fixed problem with bonding Bug#6730
	if (ioctl(fd_aux, SIOCGIFADDR, &ifr) == -1) {
		strcpy(ifr.ifr_name, "bond0");
		ifr.ifr_addr.sa_family = AF_INET;
		if (ioctl(fd_aux, SIOCGIFADDR, &ifr) == -1) {
			sin = NULL;
		} else {
			sin = (struct sockaddr_in *) &ifr.ifr_addr;
		}
	} else {
		sin = (struct sockaddr_in *) &ifr.ifr_addr;
	}
	
	if (sin) {
		xinet_ntoa((struct sockaddr *)sin,IPADDRESS,sizeof(IPADDRESS));
	}
	close(fd_aux);
	
	return !strcmp(ipAddress,IPADDRESS);
}
#endif

/* authorizes user on remote TACACS+ server, i.e. checks
 * his permission to access requested service
 * returns PAM_SUCCESS if the service is allowed
 */
PAM_EXTERN 
int pam_sm_acct_mgmt (pam_handle_t * pamh, int flags,
		  int argc, const char **argv)
{
  int retval, ctrl, status=PAM_AUTH_ERR, rastatus=PAM_SUCCESS;
  char *user;
  char *tty;
  struct areply arep;
  struct tac_attrib *attr = NULL;
  int tac_fd;
  char *rhostname;
  struct sockaddr_storage rhost = { 0 };
  struct pam_appl_data *pappl;
  int tty_num=-1;
  char *ptr,*ptr2;  //[GY]2006/Jan/24  Group_Auth_Box
  char  HOSTNAME[SIZE_HOSTNAME], ttysn[9];
//struct hostent *ht;
//struct in_addr sAddr;
  //int fd_aux;
  //struct ifreq ifr;
  //struct sockaddr_in * sin;

  user = tty = rhostname = NULL;
  pappl = get_cyc_appl(pamh);
  /* this also obtains service name for authorization
     this should be normally performed by pam_get_item(PAM_SERVICE)
     but since PAM service names are incompatible TACACS+
     we have to pass it via command line argument until a better
     solution is found ;) */
  //[RK]Jun/02/05 - Group Authorization Support - removed the get_str
  if (pappl) {
  	ctrl = _pam_parse (0, pappl->port, argc, argv);
  } else {
  	ctrl = _pam_parse (0,0, argc, argv);
  }
  if (ctrl & PAM_TAC_DEBUG) {
    struct sockaddr_storage addr;
    char str[INET6_ADDRSTRLEN] = "";

    syslog (LOG_DEBUG, "%s: called (pam_tacplus v%hu.%hu.%hu)"
	, __FUNCTION__, PAM_TAC_VMAJ, PAM_TAC_VMIN, PAM_TAC_VPAT);

    bcopy(&active_server, &addr, sizeof(addr));
    syslog (LOG_DEBUG, "%s: active server is [%s]", __FUNCTION__,
		    xinet_ntoa((struct sockaddr *)&addr,str,sizeof(str)));
  }
  
  retval = pam_get_item(pamh, PAM_USER, (const void **)(void *)&user);    // [GY] 2005/Sep/19  clean warning
  if (retval != PAM_SUCCESS || user == NULL || *user == '\0') {
      _pam_log (LOG_ERR, "unable to obtain username");
      return PAM_USER_UNKNOWN;
  }

  if (ctrl & PAM_TAC_DEBUG) {
	  syslog(LOG_DEBUG, "%s: username obtained [%s]", __FUNCTION__, user);
  }
  
  tty = _pam_get_terminal(pamh);
  if(!strncmp(tty, "/dev/", 5)) 
	  tty += 5;

  if (ctrl & PAM_TAC_DEBUG) {
	  syslog(LOG_DEBUG, "%s: tty obtained [%s]", __FUNCTION__, tty);
  }
  
  /* XXX temporarily disabled 
  retval = pam_get_item(pamh, PAM_RHOST, (const void **) &rhostname);
  */
  /* if this fails, this means the remote host name was not supplied
     by the application, and there's no need to report error; if it was,
     then we appreciate this :) */
  /* XXX this is lame */
  /*
  if(retval == PAM_SUCCESS && rhostname != NULL && *rhostname != '\0') {
	  rhost = _resolve_name(rhostname);
	  rhostname = NULL;
   */
	  /* if _resolve_name succeded, rhost now contains IP address
	     in binary form, rhostname is prepared for storing its
	     ASCII representation */
  /*
  	  if (ctrl & PAM_TAC_DEBUG) {
	  	syslog(LOG_DEBUG, "%s: rhost obtained [%lx]", __FUNCTION__,
				rhost);
  	  }
  }
  */
  
  /* checks if user has been successfully authenticated
     by TACACS+; we cannot solely authorize user if it hasn't
     been authenticated or has been authenticated by method other
     than TACACS+ */
  if(!active_server.ss_family) {
	  _pam_log (LOG_ERR, "user not authenticated by TACACS+");
	  return PAM_AUTH_ERR;
  }

  /* checks for specific data required by TACACS+, which should
     be supplied in command line  */
  if(tac_service == NULL || *tac_service == '\0') {
	  _pam_log (LOG_ERR, "TACACS+ service type not configured");
	  return PAM_AUTH_ERR;
  }
  if(tac_protocol == NULL || *tac_protocol == '\0') {
	  _pam_log (LOG_ERR, "TACACS+ protocol type not configured");
	  return PAM_AUTH_ERR;
  }

  tac_add_attrib(&attr, "service", tac_service);
  tac_add_attrib(&attr, "protocol", tac_protocol);

  if (rhost.ss_family) {
	  char str[INET6_ADDRSTRLEN] = "";
	  tac_add_attrib(&attr, "ip", xinet_ntoa((struct sockaddr *)&rhost,str,sizeof(str)));
  }
  
  tac_fd = tac_connect_single(active_server);
  if(tac_fd < 0) {
	  _pam_log (LOG_ERR, "TACACS+ server unavailable");
	  status = PAM_AUTH_ERR;
	  goto ErrExit;
  }

  retval = tac_author_send(tac_fd, user, tty, attr);
  
  /* this is no longer needed */
  tac_free_attrib(&attr);
  
  if(retval < 0) {
	  _pam_log (LOG_ERR, "error getting authorization");
	  status = PAM_AUTH_ERR;
	  goto ErrExit;
  }

  if (ctrl & PAM_TAC_DEBUG) {
	  syslog(LOG_DEBUG, "%s: sent authorization request", __FUNCTION__);
  }
  
  tac_author_read(tac_fd, &arep);

  if(arep.status != AUTHOR_STATUS_PASS_ADD &&
		  arep.status != AUTHOR_STATUS_PASS_REPL) {
	  _pam_log (LOG_ERR, "TACACS+ authorization failed for [%s]", user);
	  status = PAM_PERM_DENIED;
	  goto ErrExit;
  }

  if (ctrl & PAM_TAC_DEBUG) {
	  syslog(LOG_DEBUG, "%s: user [%s] successfully authorized", 
			  __FUNCTION__, user);
  }

  status = PAM_SUCCESS;

  if(pappl > 0) {
	  
	// GET HOSTNAME [LT]
	if(gethostname(HOSTNAME, SIZE_HOSTNAME) == -1) 
      		syslog(LOG_WARNING, "%s: Hostname is invalid.", __FUNCTION__);
  }

  /* set PAM_RHOST if 'addr' or 'raccess' attribute was returned from server */
  attr = arep.attr;
  while (attr != NULL)  {
		  //ADDR
		  if(!strncmp(attr->attr, "addr", 4)) {
			char buff[128];
			char *sep;
			
			sep = index(attr->attr, '=');
			if(sep == NULL)
					sep = index(attr->attr, '*');
			if(sep == NULL) {
					syslog(LOG_WARNING, "%s: invalid attribute `%s', no separator", __FUNCTION__, attr->attr);
					break;
			}
			
			bcopy(++sep, buff, attr->attr_len-5);
			buff[attr->attr_len-5] = '\0';
			
			if(isdigit(*buff)) 
					retval = pam_set_item(pamh, PAM_RHOST, buff);
					if (retval != PAM_SUCCESS)
							syslog(LOG_WARNING, "%s: unable to set remote address for PAM", __FUNCTION__);
					else if(ctrl & PAM_TAC_DEBUG)
							syslog(LOG_DEBUG, "%s: set remote addr to `%s'", __FUNCTION__, buff);
				  
		  	break;
		  }
                //RACCESS [LT]
               	if((!strncmp(attr->attr, "port", 4)) && (pappl > 0) && (pappl->port > 0)) {
               		char host_ip[SIZE_HOSTNAME];
               		host_ip[0] = '\0';
			if(ctrl & PAM_TAC_DEBUG) {
				syslog(LOG_DEBUG, "%s: raccess-port[%s]", __FUNCTION__, attr->attr);
			}
			tty_num = -1;
                       	if ((ptr=strchr(attr->attr, '=')) != NULL) {
                               	ptr++;
                               	if ((ptr2=strstr(ptr,"/")) != NULL) {
                                       	strncpy(host_ip,ptr,(ptr2-ptr));
                                       	host_ip[(ptr2-ptr)] = '\0';  
					//[RK]Sep/04/06 - accept : ttyNN or ttySnn or any
					ptr2++; // skip '/'
					if (!strncmp(ptr2,"tty",3)) {
                                       		ptr2 += 3;
						if (*ptr2 == 'S') ptr2++;
						strcpy(ttysn,ptr2);
						tty_num=(int)atoi(ttysn);
					} else {
						if (!strncmp(ptr2,"any",3)) {
							tty_num = 0; // all ports
						}
					}
                               	} else {
                                       	host_ip[0]='\0';
                               	}
                       	} else {
                               	host_ip[0]='\0';
                       	}

			if (ctrl & PAM_TAC_DEBUG) {
				syslog(LOG_DEBUG, "%s: host_ip[%s] tty_num[%d]", __FUNCTION__, host_ip, tty_num);
			}
			rastatus=PAM_RACCESS_DENIED;

                       	//Check permission
                       	if ((tty_num != -1) &&
                            (!strcmp(host_ip, HOSTNAME) || isLocalAddress(host_ip))) {
				if  ((tty_num == 0) || (pappl->port == tty_num)) {
                               		rastatus=PAM_SUCCESS;
                               		status=PAM_SUCCESS;
					break;
				}
                       	}

               	}

#ifdef GRPAUTHenable
		//[RK]May/19/05 - Group Authorization Support
		if (pappl) {
			char *aux, *aux1; int len;
			if ((aux=strstr(attr->attr,"group_name"))) {
				aux+=10;
				while (isblank(*aux) && *aux) aux++;
				if (*aux == '=') { 
					aux++;
					if ((aux1=strchr(aux,';'))) {
						len = (int) (aux1 - aux);
						pappl->group_name = strndup(aux,len);
					}
				}
			}
		}
#endif

		attr = attr->next;
  }

  /* free returned attributes */
  if(arep.attr != NULL) tac_free_attrib(&arep.attr);

  //[LT] - Raccess allow or denied
  if((rastatus == PAM_RACCESS_DENIED) && (pappl > 0)) {
	syslog(LOG_WARNING, "%s: Tacacs+: Raccess denied for user [%s]",__FUNCTION__,user); 
	status = PAM_RACCESS_DENIED;
	goto ErrExit;
  }

ErrExit:
  
  close(tac_fd);

  return status;
}				/* pam_sm_acct_mgmt */

/* sends START accounting request to the remote TACACS+ server
 * returns PAM error only if the request was refused or there
 * were problems connection to the server
 */
/* accounting packets may be directed to any TACACS+ server,
 * independent from those used for authentication and authorization;
 * it may be also directed to all specified servers
 */  
PAM_EXTERN 
int pam_sm_open_session (pam_handle_t * pamh, int flags,
		     int argc, const char **argv)
{
		task_id=(short int) magic();
		return(_pam_account(pamh, argc, argv,TAC_PLUS_ACCT_FLAG_START)); 
}				/* pam_sm_open_session */

/* sends STOP accounting request to the remote TACACS+ server
 * returns PAM error only if the request was refused or there
 * were problems connection to the server
 */
PAM_EXTERN 
int pam_sm_close_session (pam_handle_t * pamh, int flags,
		      int argc, const char **argv) {
		return(_pam_account(pamh, argc, argv,TAC_PLUS_ACCT_FLAG_STOP)); 

}	/* pam_sm_close_session */

PAM_EXTERN
int _pam_account(pam_handle_t *pamh, int argc, const char **argv,  int type)
{
  int retval;
  static int ctrl;
  char *user, *tty, *typemsg;
  int status = PAM_SESSION_ERR;
  struct pam_appl_data *pappl;
  
  user = tty = NULL;

  pappl = get_cyc_appl(pamh);

  typemsg = (type == TAC_PLUS_ACCT_FLAG_START) ? "START" : "STOP";
  /* debugging
  if(type == TAC_PLUS_ACCT_FLAG_STOP)
		  sleep(60);
		  */
  
  /* when we are sending STOP packet we no longer have uid 0 */
/*  if(type == TAC_PLUS_ACCT_FLAG_START) */
  //[RK]Jun/02/05 - Group Authorization Support - removed the get_str
  if (pappl) {
  	ctrl = _pam_parse (1, pappl->port, argc, argv);
  } else {
  	ctrl = _pam_parse (1,0, argc, argv);
  }
  if (ctrl & PAM_TAC_DEBUG)
    syslog (LOG_DEBUG, "%s: [%s] called (pam_tacplus v%hu.%hu.%hu)"
	, __FUNCTION__, typemsg, PAM_TAC_VMAJ, PAM_TAC_VMIN, PAM_TAC_VPAT);
  if (ctrl & PAM_TAC_DEBUG)
	  syslog(LOG_DEBUG, "%s: tac_srv_no=%d", __FUNCTION__, tac_srv_no);
  
  retval = pam_get_item(pamh, PAM_USER, (const void **)(void *)&user);    // [GY] 2005/Sep/19  clean warning
  if(retval != PAM_SUCCESS || user == NULL || *user == '\0') {
	  _pam_log(LOG_ERR, "%s: unable to obtain username", __FUNCTION__);
	  return PAM_SESSION_ERR;
  }

  if (ctrl & PAM_TAC_DEBUG)
	  syslog(LOG_DEBUG, "%s: username [%s] obtained", __FUNCTION__, user);
  
  tty = _pam_get_terminal(pamh);
  
  if(!strncmp(tty, "/dev/", 5)) 
	  tty += 5;
  
  if (ctrl & PAM_TAC_DEBUG)
  	syslog(LOG_DEBUG, "%s: tty [%s] obtained", __FUNCTION__, tty);

  /* checks for specific data required by TACACS+, which should
     be supplied in command line  */
  if(tac_service == NULL || *tac_service == '\0') {
    _pam_log (LOG_ERR, "TACACS+ service type not configured");
    return PAM_AUTH_ERR;
  }
  if(tac_protocol == NULL || *tac_protocol == '\0') {
    _pam_log (LOG_ERR, "TACACS+ protocol type not configured");
    return PAM_AUTH_ERR;
  }

  /* when this module is called from within pppd or other
	 application dealing with serial lines, it is likely
	 that we will get hit with signal caused by modem hangup;
	 this is important only for STOP packets, it's relatively
	 rare that modem hangs up on accounting start */
  if(type == TAC_PLUS_ACCT_FLAG_STOP) {
  	signal(SIGALRM, SIG_IGN);
  	signal(SIGCHLD, SIG_IGN);
  	signal(SIGHUP, SIG_IGN);
  }

  if(! ctrl & PAM_TAC_ACCT) {
  /* normal mode, send packet to the first available server */
		  int tac_fd;
		  
		  status = PAM_SUCCESS;

		  
		  tac_fd = tac_connect(tac_srv, tac_srv_no);
		  if(tac_fd < 0) {
				  _pam_log(LOG_ERR, "%s: error sending %s - no servers",
								  __FUNCTION__, typemsg);
				  status = PAM_SESSION_ERR;
		  }
  		  if (ctrl & PAM_TAC_DEBUG)
				  syslog(LOG_DEBUG, "%s: connected with fd=%d", __FUNCTION__, tac_fd);

  		  retval = _pam_send_account(tac_fd, type,
						user, tty);
		  if(retval < 0) {
				  _pam_log(LOG_ERR, "%s: error sending %s", 
								  __FUNCTION__, typemsg);
				  status = PAM_SESSION_ERR;
		  }
		  
		  close(tac_fd);
  		  
		  if (ctrl & PAM_TAC_DEBUG) {
				  syslog(LOG_DEBUG, "%s: [%s] for [%s] sent",
								  __FUNCTION__, typemsg,user);
		  }

  } else {
  /* send packet to all servers specified */
		  int srv_i;
		  
		  status = PAM_SESSION_ERR;
		  
		  for(srv_i = 0; srv_i < tac_srv_no; srv_i++) {
				  int tac_fd;
				  
				  tac_fd = tac_connect_single(tac_srv[srv_i]);
				  if(tac_fd < 0) {
						  _pam_log(LOG_WARNING, "%s: error sending %s (fd)",
										  __FUNCTION__, typemsg);
						  continue;
				  }

  		  		  if (ctrl & PAM_TAC_DEBUG) {
  		  			char str[INET6_ADDRSTRLEN] = "";
				  	syslog(LOG_DEBUG, "%s: connected with fd=%d (srv %d ip %s)", __FUNCTION__, tac_fd, srv_i, 
				  			xinet_ntoa((struct sockaddr *)&tac_srv[srv_i],str,sizeof(str)));
  		  		  }


				  retval = _pam_send_account(tac_fd, type,
								  user, tty);

				  
				  /* return code from function in this mode is
					 status of the last server we tried to send
					 packet to */
				  if(retval < 0) 
						  _pam_log(LOG_WARNING, "%s: error sending %s (acct)",
										  __FUNCTION__, typemsg);
				  else {
						  status = PAM_SUCCESS;
		  				  if (ctrl & PAM_TAC_DEBUG) 
				  				  syslog(LOG_DEBUG, "%s: [%s] for [%s] sent",
								  		__FUNCTION__, typemsg,user);
				  }

				  close(tac_fd);

		  }
				  
  }  /* acct mode */

  if(type == TAC_PLUS_ACCT_FLAG_STOP) {
  	signal(SIGALRM, SIG_DFL);
  	signal(SIGCHLD, SIG_DFL);
  	signal(SIGHUP, SIG_DFL);
  }
			  
  return status;
}				

int _pam_send_account(int tac_fd, int type, char *user, char *tty) {
	char buf[40];
	struct tac_attrib *attr;
	int retval, status = -1;
	

	attr=(struct tac_attrib *)_xcalloc(sizeof(struct tac_attrib));
	
	sprintf(buf, "%lu", time(0));
	tac_add_attrib(&attr, 
		(type == TAC_PLUS_ACCT_FLAG_START) ? "start_time" : "stop_time"
			, buf);
	sprintf(buf, "%hu", task_id);
	tac_add_attrib(&attr, "task_id", buf);
	tac_add_attrib(&attr, "service", tac_service);
	tac_add_attrib(&attr, "protocol", tac_protocol);
	/* XXX this requires pppd to give us this data */
	/*
	tac_add_attrib(&attr, "addr", ip_ntoa(ho->hisaddr));
	*/

	retval = tac_account_send(tac_fd, type, user, tty, attr);

	/* this is no longer needed */
	tac_free_attrib(&attr);
	
	if(retval < 0) {
		_pam_log (LOG_WARNING, "%s: send %s accounting failed (task %hu)",
			__FUNCTION__, 
			(type == TAC_PLUS_ACCT_FLAG_START) ? "start" : "stop",
			task_id);
		status = -1;
		goto ErrExit;
	}
	
	if(tac_account_read(tac_fd) != NULL) {
		_pam_log (LOG_WARNING, "%s: accounting %s failed (task %hu)",
			__FUNCTION__, 
			(type == TAC_PLUS_ACCT_FLAG_START) ? "start" : "stop",
			task_id);
		status = -1;
		goto ErrExit;
	}

	status = 0;

ErrExit:
	close(tac_fd);
	return status;
}

#ifdef PAM_SM_PASSWORD
/* no-op function for future use */ 
PAM_EXTERN 
pam_sm_chauthtok (pam_handle_t * pamh, int flags,
		  int argc, const char **argv)
{
  struct pam_appl_data *pappl;
  int ctrl;

  pappl = get_cyc_appl(pamh);

  //[RK]Jun/02/05 - Group Authorization Support - removed the get_str
  if (pappl) {
  	ctrl = _pam_parse (0, pappl->port, argc, argv);
  } else {
  	ctrl = _pam_parse (0,0, argc, argv);
  }

  if (ctrl & PAM_TAC_DEBUG)
    syslog (LOG_DEBUG, "%s: called (pam_tacplus v%hu.%hu.%hu)"
	, __FUNCTION__, PAM_TAC_VMAJ, PAM_TAC_VMIN, PAM_TAC_VPAT);

  return PAM_SUCCESS;
}				/* pam_sm_chauthtok */
#endif


#ifdef PAM_STATIC

struct pam_module _pam_tacplus_modstruct
{
  "pam_tacplus",
  pam_sm_authenticate,
  pam_sm_setcred,
  pam_sm_acct_mgmt,
  pam_sm_open_session,
  pam_sm_close_session,
#ifdef PAM_SM_PASSWORD
  pam_sm_chauthtok
#else
  NULL
#endif
};
#endif

/*
 * Copyright 1998 by Pawel Krawczyk <kravietz@ceti.com.pl>
 *                                              All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, and the entire permission notice in its entirety,
 *    including the disclaimer of warranties.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * ALTERNATIVELY, this product may be distributed under the terms of
 * the GNU Public License, in which case the provisions of the GPL are
 * required INSTEAD OF the above restrictions.  (This clause is
 * necessary due to a potential bad interaction between the GPL and
 * the restrictions contained in a BSD-style copyright.)
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

