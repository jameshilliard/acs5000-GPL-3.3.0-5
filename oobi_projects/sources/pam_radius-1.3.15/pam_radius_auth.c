/*
 *      Authenticate a user via a RADIUS session
 *
 * 0.9.0 - Didn't compile quite right.
 * 0.9.1 - Hands off passwords properly.  Solaris still isn't completely happy
 * 0.9.2 - Solaris now does challenge-response.  Added configuration file
 *         handling, and skip_passwd field
 * 1.0.0 - Added handling of port name/number, and continue on select
 * 1.1.0 - more options, password change requests work now, too.
 * 1.1.1 - Added client_id=foo (NAS-Identifier), defaulting to PAM_SERVICE
 * 1.1.2 - multi-server capability.
 * 1.2.0 - ugly merger of pam_radius.c to get full RADIUS capability
 * 1.3.0 - added my own accounting code.  Simple, clean, and neat.
 * 1.3.1 - Supports accounting port (oops!), and do accounting authentication
 * 1.3.2 - added support again for 'skip_passwd' control flag.
 * 1.3.10 - ALWAYS add Password attribute, to make packets RFC compliant.
 * 1.3.11 - Bug fixes by Jon Nelson <jnelson@securepipe.com>
 * 1.3.12 - miscellanous bug fixes.  Don't add password to accounting
 *          requests; log more errors; add NAS-Port and NAS-Port-Type
 *          attributes to ALL packets.  Some patches based on input from 
 *          Grzegorz Paszka <Grzegorz.Paszka@pik-net.pl>
 * 1.3.13 - Always update the configuration file, even if we're given
 *          no options.  Patch from Jon Nelson <jnelson@securepipe.com>
 * 1.3.14 - Don't use PATH_MAX, so it builds on GNU Hurd.
 * 1.3.15 - Implement retry option, miscellanous bug fixes.
 *
 * See end for Copyright information */

#define PAM_SM_AUTH
#define PAM_SM_PASSWORD
#define PAM_SM_SESSION

#include <limits.h>
#include <errno.h>

#ifdef sun
#include <security/pam_appl.h>
#endif
#include <security/pam_modules.h>

#include <security/pam_appdata.h>
#include "pam_radius_auth.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>


/* internal data */
static CONST char *pam_module_name = "pam_radius_auth";
static char conf_file[BUFFER_SIZE]; /* configuration file */

/* we need to save these from open_session to close_session, since
 * when close_session will be called we won't be root anymore and
 * won't be able to access again the radius server configuration file
 * -- cristiang */
//static radius_server_t *live_server = NULL;
static char valid_live_server = 0;
static radius_server_t live_server = {NULL, {0}, 0, NULL, NULL, 0, 0, 0};
static time_t session_time;

static void copy_server_info (radius_server_t *dest, radius_server_t *orig)
{
  dest->next = NULL;
  dest->ip = orig->ip;
  dest->port = orig->port;
  if (dest->hostname) {
	free (dest->hostname);
  }
  dest->hostname = strdup(orig->hostname);
  if (dest->secret) {
	free (dest->secret);
  }
  dest->secret = strdup(orig->secret);
  dest->timeout = orig->timeout;
  dest->accounting = orig->accounting;
  dest->tries = orig->tries;
}

static struct pam_appl_data *get_cyc_appl(pam_handle_t *pamh)
{
  CONST struct pam_conv *conv;
  struct pam_appl_data *pappl;

  if (pam_get_item(pamh, PAM_CONV, (CONST void **)(void *) &conv) != PAM_SUCCESS) { //[GY] 2005/Sep/19  clean warning
    return(NULL);
  }

  pappl = (struct pam_appl_data *) conv->appdata_ptr;

  if (!pappl || pappl->id != 0x1234fedc) {
    return(NULL);
  }

  return(pappl);
}

/* logging */
static void _pam_log(int err, CONST char *format, ...)
{
    va_list args;
    char buffer[BUFFER_SIZE];

    va_start(args, format);
    vsprintf(buffer, format, args);
    /* don't do openlog or closelog, but put our name in to be friendly */
    syslog(err, "%s: %s", pam_module_name, buffer);
    va_end(args);
}

/* argument parsing */
static int _pam_parse(int argc, CONST char **argv, radius_conf_t *conf)
{
  int ctrl=0;

  memset(conf, 0, sizeof(radius_conf_t)); /* ensure it's initialized */

  conf->retries = 3;
  strcpy(conf_file, CONF_FILE);
  
  /*
   *  If either is not there, then we can't parse anything.
   */
  if ((argc == 0) || (argv == NULL)) {
    return ctrl;
  }
  
  /* step through arguments */
  for (ctrl=0; argc-- > 0; ++argv) {
    
    /* generic options */
    if (!strncmp(*argv,"conf=",5)) {
      strcpy(conf_file,*argv+5);

    } else if (!strcmp(*argv, "use_first_pass")) {
      ctrl |= PAM_USE_FIRST_PASS;

    } else if (!strcmp(*argv, "try_first_pass")) {
      ctrl |= PAM_TRY_FIRST_PASS;

    } else if (!strcmp(*argv, "skip_passwd")) {
      ctrl |= PAM_SKIP_PASSWD;

    } else if (!strncmp(*argv,"retry=",6)) {
      int i = atoi(*argv+6);
      if (i > 1 && i <= 10) {
        conf->retries = i;
      }
    } else if (!strncmp(*argv, "client_id=", 10)) {
      if (conf->client_id) {
	_pam_log(LOG_WARNING, "ignoring duplicate '%s'", *argv);
      } else {
	conf->client_id = (char *) *argv+10; /* point to the client-id */
      }
    } else if (!strcmp(*argv, "accounting_bug")) {
      conf->accounting_bug = TRUE;

    } else if (!strcmp(*argv, "debug")) {
      ctrl |= PAM_DEBUG_ARG;
      conf->debug = 1;
      
#ifdef RADIUS2FACTORenable
    } else if (!strcmp(*argv, "radius2factor")) {
      ctrl |= PAM_RADIUS_TWO_FACTOR;

#endif
    } else {
      _pam_log(LOG_WARNING, "unrecognized option '%s'", *argv);
    }
  }
  
  return ctrl;
}

/* Callback function used to free the saved return value for pam_setcred. */
void _int_free( pam_handle_t * pamh, void *x, int error_status )
{
    free(x);
}

/*************************************************************************
 * SMALL HELPER FUNCTIONS
 *************************************************************************/

/*
 * Return an IP address in host long notation from a host
 * name or address in dot notation.
 */
static struct sockaddr_storage *get_ipaddr(char *host, struct sockaddr_storage *addr) {

    struct addrinfo *res = NULL;
    struct addrinfo hints;
    int n;
  
    memset(addr,0,sizeof(*addr));
  
    memset(&hints,0,sizeof(hints));
    hints.ai_family = PF_UNSPEC;
#ifdef AI_ADDRCONFIG
    hints.ai_flags = AI_ADDRCONFIG;
#endif
    n = getaddrinfo(host,NULL,&hints,&res);

    if (res != NULL) {
        if (n == 0) {
            // TODO: need to filter address family according to appliance configuration
            memcpy(addr,res->ai_addr,res->ai_addrlen);
        }
        freeaddrinfo(res);
    }
    
    return n == 0 ? addr : NULL;
}

/*
 * take server->hostname, and convert it to server->ip and server->port
 */
static int
host2server(radius_server_t *server)
{
  char *p;
  char *sb = NULL;

  p = strchr(server->hostname, ']');
  if (p == NULL) {
      p = server->hostname;
  } else {
      sb = p;
  }
  
  if ((p = strchr(p, ':')) != NULL) {
    *(p++) = '\0';		/* split the port off from the host name */
  }
  
  if (sb != NULL) {
      char name[strlen(server->hostname) + 1];
      *sb = '\0';
      sb = strchr(server->hostname,'[');
      if (sb != NULL) {
          strcpy(name,sb+1);
          strcpy(server->hostname,name);
      }
  }
  
  if (get_ipaddr(server->hostname,&server->ip) == NULL) {
    return PAM_AUTHINFO_UNAVAIL;
  }
  
  _pam_log(LOG_DEBUG, "IP address configured");

  /*
   *  If the server port hasn't already been defined, go get it.
   */
  if (!server->port) {
    if (p && isdigit(*p)) {	/* the port looks like it's a number */
      unsigned int i = atoi(p) & 0xffff;
      
      if (!server->accounting) {
	server->port = htons((u_short) i);
      } else {
	server->port = htons((u_short) (i + 1));
      }
    } else {			/* the port looks like it's a name */
      struct servent *svp;
      
      if (p) {			/* maybe it's not "radius" */
	svp = getservbyname (p, "udp");
	*(--p) = ':';		/* be sure to put the delimiter back */
      } else {
	if (!server->accounting) {
	  svp = getservbyname ("radius", "udp");
	} else {
	  svp = getservbyname ("radacct", "udp");
	}
      }
      
      if (svp == (struct servent *) 0) {
	return PAM_AUTHINFO_UNAVAIL;
      }
      
      server->port = svp->s_port;
    }
  }

  return PAM_SUCCESS;
}

/*
 * Do XOR of two buffers.
 */
static unsigned char *
xor(unsigned char *p, unsigned char *q, int length)
{
  int i;
  unsigned char *retval = p;
  
  for (i = 0; i < length; i++) {
    *(p++) ^= *(q++);
  }
  return retval;
}

/**************************************************************************
 * MID-LEVEL RADIUS CODE
 **************************************************************************/

/*
 * get a pseudo-random vector.
 */
static void
get_random_vector(unsigned char *vector)
{
#ifdef linux
  int fd = open("/dev/urandom",O_RDONLY); /* Linux: get *real* random numbers */
  int total = 0;
  if (fd >= 0) {
    while (total < AUTH_VECTOR_LEN) {
      int bytes = read(fd, vector + total, AUTH_VECTOR_LEN - total);
      if (bytes <= 0)
	break;			/* oops! Error */
      total += bytes;
    }
    close(fd);
  }

  if (total != AUTH_VECTOR_LEN)
#endif
    {				/* do this *always* on other platforms */
      MD5_CTX my_md5;
      struct timeval tv;
      struct timezone tz;
      static unsigned int session = 0; /* make the number harder to guess */
      
      /* Use the time of day with the best resolution the system can
	 give us -- often close to microsecond accuracy. */
      gettimeofday(&tv,&tz);
      
      if (session == 0) {
	session = getppid();	/* (possibly) hard to guess information */
      }
      
      tv.tv_sec ^= getpid() * session++;
      
      /* Hash things to get maybe cryptographically strong pseudo-random numbers */
      MD5Init(&my_md5);
      MD5Update(&my_md5, (unsigned char *) &tv, sizeof(tv));
      MD5Update(&my_md5, (unsigned char *) &tz, sizeof(tz));
      MD5Final(vector, &my_md5);	      /* set the final vector */
    }
}

/*
 * RFC 2139 says to do generate the accounting request vector this way.
 * However, the Livingston 1.16 server doesn't check it.  The Cistron
 * server (http://home.cistron.nl/~miquels/radius/) does, and this code
 * seems to work with it.  It also works with Funk's Steel-Belted RADIUS.
 */
static void
get_accounting_vector(AUTH_HDR *request, radius_server_t *server)
{
  MD5_CTX my_md5;
  int secretlen = strlen(server->secret);
  int len = ntohs(request->length);
  
  memset(request->vector, 0, AUTH_VECTOR_LEN);
  MD5Init(&my_md5);
  memcpy(((char *)request) + len, server->secret, secretlen);

  MD5Update(&my_md5, (char *)request, len + secretlen);
  MD5Final(request->vector, &my_md5);      /* set the final vector */
}

/*
 * Verify the response from the server
 */
static int
verify_packet(char *secret, AUTH_HDR *response, AUTH_HDR *request)
{
  MD5_CTX my_md5;
  unsigned char	calculated[AUTH_VECTOR_LEN];
  unsigned char	reply[AUTH_VECTOR_LEN];

  /*
   * We could dispense with the memcpy, and do MD5's of the packet
   * + vector piece by piece.  This is easier understand, and maybe faster.
   */
  memcpy(reply, response->vector, AUTH_VECTOR_LEN); /* save the reply */
  memcpy(response->vector, request->vector, AUTH_VECTOR_LEN); /* sent vector */
  
  /* MD5(response packet header + vector + response packet data + secret) */
  MD5Init(&my_md5);
  MD5Update(&my_md5, (unsigned char *) response, ntohs(response->length));

  /* 
   * This next bit is necessary because of a bug in the original Livingston
   * RADIUS server.  The authentication vector is *supposed* to be MD5'd
   * with the old password (as the secret) for password changes.
   * However, the old password isn't used.  The "authentication" vector
   * for the server reply packet is simply the MD5 of the reply packet.
   * Odd, the code is 99% there, but the old password is never copied
   * to the secret!
   */
  if (*secret) {
    MD5Update(&my_md5, secret, strlen(secret));
  }

  MD5Final(calculated, &my_md5);      /* set the final vector */

  /* Did he use the same random vector + shared secret? */
  if (memcmp(calculated, reply, AUTH_VECTOR_LEN) != 0) {
    return FALSE;
  }
  return TRUE;
}

/*
 * Find an attribute in a RADIUS packet.  Note that the packet length
 * is *always* kept in network byte order.
 */
static attribute_t *
find_attribute(AUTH_HDR *response, unsigned char type, attribute_t *attr)
{
  int len = ntohs(response->length) - AUTH_HDR_LEN;

  if (attr == NULL) {
    attr = (attribute_t *) &response->data;
  } else {
    attr = (attribute_t *) ((char *) attr + attr->length);
    len -= (char *)attr - (char *)&response->data;
  }

  while (attr->attribute != type) {
    if ((len -= attr->length) <= 0) {
      return NULL;		/* not found */
    }
    attr = (attribute_t *) ((char *) attr + attr->length);
  }

  return attr;
}

/*
 * Find group authorization = FRAMED-FILTER-ID in a RADIUS packet.
 * Possible syntax for this attribute:
 * 	'group_name=<group1>[,<group2>];'
 * 	'<group1>[,<group2>][;]'
 * The value can be split in more than one FRAMED-FILTER-ID attribute.
 * This routine will add ',' as the separator between attributes when 
 * it is not present.
 */
static char * find_group_name(AUTH_HDR *response)
{
	attribute_t * attr = NULL;
	char *group, *aux, *aux1;
	int len,lfree;

	if ((attr = find_attribute(response, PW_FRAMED_FILTER_ID, NULL)) == NULL) {
		return NULL; // none attribute FRAMED-FILTER-ID;
	}

	group = calloc(1,2048);
	lfree = 2048;

	while (attr && (lfree > 0)) {
		if (attr->length-2) {
			// copy data to group
			aux = attr->data;
			if ((aux1=strstr(aux,"group_name")) &&
			    (((int)aux1 - (int) aux) < (attr->length-2))) {
				aux1 += 10;
				while (isblank(*aux1) && *aux1) aux1++;
				if (*aux1 == '=') { 
					aux = aux1+1;
				} else {
					aux = NULL;
				}
			}
			while (*aux && isblank(*aux)) aux++;
			if (aux) {
				len = attr->length - 2 -((int)aux - (int)attr->data);
				if ((len > 0) && (lfree > len)) {
					aux1 = attr->data + attr->length-3;
					if (*aux1 == ';') {
						len--;
					}
					strncat(group,aux,len);
					lfree -= len;
					if (*aux1 != ',') {
						strcat(group,",");
						lfree--;
					}
				}
			}
		}
		// get next attribute
		attr = find_attribute(response, PW_FRAMED_FILTER_ID, attr);
	}
	return(group);
}

/*
 * Add an attribute to a RADIUS packet.
 */
static void
add_attribute(AUTH_HDR *request, unsigned char type, CONST unsigned char *data, int length)
{
  attribute_t *p;

  p = (attribute_t *) ((unsigned char *)request + ntohs(request->length));
  p->attribute = type;
  p->length = length + 2;		/* the total size of the attribute */
  request->length = htons(ntohs(request->length) + p->length);
  memcpy(p->data, data, length);
}

/*
 * Add an integer attribute to a RADIUS packet.
 */
static void
add_int_attribute(AUTH_HDR *request, unsigned char type, int data)
{
  int value = htonl(data);
  
  add_attribute(request, type, (unsigned char *) &value, sizeof(int));
}

/* 
 * Add a RADIUS password attribute to the packet.  Some magic is done here.
 *
 * If it's an PW_OLD_PASSWORD attribute, it's encrypted using the encrypted
 * PW_PASSWORD attribute as the initialization vector.
 *
 * If the password attribute already exists, it's over-written.  This allows
 * us to simply call add_password to update the password for different
 * servers.
 */
static void
add_password(AUTH_HDR *request, unsigned char type, CONST char *password, char *secret)
{
  MD5_CTX md5_secret, my_md5;
  unsigned char misc[AUTH_VECTOR_LEN];
  int i;
  int length = strlen(password);
  char hashed[256 + AUTH_PASS_LEN]; /* can't be longer than this */
  char *vector;
  attribute_t *attr;

  if (length > MAXPASS) {	/* shorten the password for now */
    length = MAXPASS;
  }

  if (length == 0) {
    length = AUTH_PASS_LEN;	/* 0 maps to 16 */
  } if ((length & (AUTH_PASS_LEN - 1)) != 0) {
    length += (AUTH_PASS_LEN - 1); /* round it up */
    length &= ~(AUTH_PASS_LEN - 1); /* chop it off */
  }                             /* 16*N maps to itself */

  memset(hashed, '\0', length);
  memcpy(hashed, password, length);

  attr = find_attribute(request, PW_PASSWORD, NULL);

  if (type == PW_PASSWORD) {
    vector = request->vector;
  } else {
    vector = attr->data;	/* attr CANNOT be NULL here. */
  }

  /* ************************************************************ */
  /* encrypt the password */
  /* password : e[0] = p[0] ^ MD5(secret + vector) */
  MD5Init(&md5_secret);
  MD5Update(&md5_secret, secret, strlen(secret));
  my_md5 = md5_secret;		/* so we won't re-do the hash later */
  MD5Update(&my_md5, vector, AUTH_VECTOR_LEN);
  MD5Final(misc, &my_md5);      /* set the final vector */
  xor(hashed, misc, AUTH_PASS_LEN);
  
  /* For each step through, e[i] = p[i] ^ MD5(secret + e[i-1]) */
  for (i = 1; i < (length >> 4); i++) {
    my_md5 = md5_secret;	/* grab old value of the hash */
    MD5Update(&my_md5, &hashed[(i-1) * AUTH_PASS_LEN], AUTH_PASS_LEN);
    MD5Final(misc, &my_md5);      /* set the final vector */
    xor(&hashed[i * AUTH_PASS_LEN], misc, AUTH_PASS_LEN);
  }

  if (type == PW_OLD_PASSWORD) {
    attr = find_attribute(request, PW_OLD_PASSWORD, NULL);
  }
  
  if (!attr) {
    add_attribute(request, type, hashed, length);
  } else {
    memcpy(attr->data, hashed, length); /* overwrite the packet */
  }
}

static void
cleanup(radius_server_t *server)
{
  radius_server_t *next;
  
  while (server) {
    next = server->next;
    _pam_drop(server->hostname);
    _pam_forget(server->secret);
    _pam_drop(server);
    server = next;
  }
}

/*
 * allocate and open a local port for communication with the RADIUS
 * server
 */
//[RK]Jun/02/05 - Group Authorization Support - removed get_str
static int
initialize(radius_conf_t *conf, int accounting, int port)
{
  struct sockaddr_storage salocal;
  
  int size_salocal;
  in_port_t *p_port;
  
  u_short local_port;
  char hostname[BUFFER_SIZE];
  char secret[BUFFER_SIZE];
  char srv_type[BUFFER_SIZE];
  char buffer[BUFFER_SIZE];
  char *p;
  FILE *fserver;
  radius_server_t *server = NULL;
  
  struct sockaddr_in * s_in = NULL;
  struct sockaddr_in6 * s_in6 = NULL;
  
  int timeout, tries, svctype;
  int line = 0;
  radius_server_t *tmp;

// [JY] 10/21/04: use port field of pam_appl_data for serial port number
//	int nas_port = 0;	//[RK]Nov/13/02 Bug #31 - add serial port number

    /* the first time around, read the configuration file */
    if ((fserver = fopen (conf_file, "r")) == (FILE*)NULL) {
      _pam_log(LOG_ERR, "Could not open configuration file %s: %s\n",
	      conf_file, strerror(errno));
      if (valid_live_server) {
	_pam_log(LOG_ERR, "Using live_server %s\n", live_server.hostname);
	conf->server = malloc(sizeof(radius_server_t));
	if (conf->server) {
	  memset(conf->server, 0, sizeof(radius_server_t));
	  copy_server_info (conf->server, &live_server);
	  goto using_live_server;
	}
	_pam_log(LOG_ERR, "Out of memory...\n");
      }
      return PAM_ABORT;
    }
  while (1) {
      if (feof(fserver) ||
	      fgets (buffer, sizeof(buffer), fserver) == (char*) NULL ||
	      ferror(fserver)) {
          break;
      }
      p = buffer;

    line++;

    /*
     *  Skip blank lines and whitespace
     */
    while (*p &&
	   ((*p == ' ') || (*p == '\t') ||
	    (*p == '\r') || (*p == '\n'))) p++;
    
    /*
     *  Nothing, or just a comment.  Ignore the line.
     */
    if ((!*p) || (*p == '#')) {
      continue;
    }
    
    timeout = 3;
	tries = 0;
	svctype = 0;
	//[RK]Nov/13/02 Bug#31 - add serial port number
    if (sscanf(p, "%s %s %s %d %d %d", srv_type, hostname, secret, &timeout,
		&tries, &svctype) < 5) {
      _pam_log(LOG_ERR, "ERROR reading %s, line %d: Could not read hostname or secret\n",
	       conf_file, line);
      continue; /* invalid line */
    } else {			/* read it in and save the data */
		if ((accounting) && (strncmp(srv_type, "auth", 4) == 0)) {
			continue;
		}

		if ((!accounting) && (strncmp(srv_type, "acct", 4) == 0)) {
			continue;
		}

      
      tmp = malloc(sizeof(radius_server_t));
      if (server) {
	server->next = tmp;
	server = server->next;
      } else {
	conf->server = tmp;
	server= tmp;		/* first time */
      }
      
      /* sometime later do memory checks here */
      server->hostname = strdup(hostname);
      server->secret = strdup(secret);
      server->accounting = accounting;
      server->port = 0;
      server->tries = tries;
// [JY] 10/21/04: use port field of pam_appl_data for serial port number
	server->nas_port = port;	//[RK]Nov/13/02 Bug#31 - add serial port number
	if (!accounting) {
		server->svctype = (svctype==1)? 1 : 0;
	} else {
		server->svctype = 0;
	}

      if ((timeout < 1) || (timeout > 60)) {
	server->timeout = 3;
      } else {
	server->timeout = timeout;
      }
      server->next = NULL;
    }
  }
    fclose(fserver);
  
  if (!server) {		/* no server found, die a horrible death */
    _pam_log(LOG_ERR, "No RADIUS server found in configuration file %s\n",
	     conf_file);
    return PAM_AUTHINFO_UNAVAIL;
  }
  
using_live_server:
    
  /* open a socket.  Dies if it fails */
  conf->sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
  if (conf->sockfd < 0) {
    _pam_log(LOG_NOTICE, "Failed to open RADIUS IPv6 socket: %s - will try IPv4 now\n", strerror(errno));
    conf->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (conf->sockfd < 0) {
      _pam_log(LOG_ERR, "Failed to open RADIUS socket: %s\n", strerror(errno));
      return PAM_AUTHINFO_UNAVAIL;
    } else {
      /* set up the local end of the socket communications */
      s_in = (struct sockaddr_in *) &salocal;
      memset ((char *) s_in, '\0', sizeof(*s_in));
      s_in->sin_family = AF_INET;
      s_in->sin_addr.s_addr = INADDR_ANY;
      p_port = &s_in->sin_port;
      size_salocal = sizeof(*s_in);
    }
  } else {
    _pam_log(LOG_DEBUG, "will open an IPv6 socket");
    s_in6 = (struct sockaddr_in6 *) &salocal;
    memset ((char *) s_in6, '\0', sizeof(*s_in6));
    s_in6->sin6_family = AF_INET6;
    s_in6->sin6_addr = in6addr_any;
    p_port = &s_in6->sin6_port;
    size_salocal = sizeof(*s_in6);
  }
  
  /*
   *  Use our process ID as a local port for RADIUS.
   */
  local_port = (getpid() & 0x7fff) + 1024;
  do {
    local_port++;
    *p_port = htons(local_port);
  } while ((bind(conf->sockfd, (struct sockaddr *)&salocal, size_salocal) < 0) && 
	   (local_port < 64000));
  
  if (local_port >= 64000) {
    close(conf->sockfd);
    _pam_log(LOG_ERR, "No open port we could bind to.");
    return PAM_AUTHINFO_UNAVAIL;
  }

  return PAM_SUCCESS;
}

/*
 * Helper function for building a radius packet.
 * It initializes *some* of the header, and adds common attributes.
 */
static void
build_radius_packet(AUTH_HDR *request, CONST char *user, CONST char *password, radius_conf_t *conf)
{
  char hostname[256];
  struct sockaddr_in *addr;
  UINT4 ipaddr = 0;
  char originalName[strlen(conf->server->hostname)+1];
  
  hostname[0] = '\0';
  gethostname(hostname, sizeof(hostname) - 1);

  request->length = htons(AUTH_HDR_LEN);

  if (password) {		/* make a random authentication req vector */
    get_random_vector(request->vector);
  }
  
  add_attribute(request, PW_USER_NAME, (char *) user, strlen(user));

  /*
   *  Add a password, if given.
   */
  if (password) {
    add_password(request, PW_PASSWORD, password, conf->server->secret);

    /*
     *  Add a NULL password to non-accounting requests.
     */
  } else if (request->code != PW_ACCOUNTING_REQUEST) {
    add_password(request, PW_PASSWORD, "", conf->server->secret);
  }

  strcpy(originalName,conf->server->hostname);
  
  // initialize server->ip before it is used in the next lines
  if (host2server(conf->server) != PAM_SUCCESS) {
    _pam_log(LOG_ERR, "Failed looking up IP address for RADIUS server %s",
      conf->server->hostname);
  }
  
  _pam_log(LOG_DEBUG, "IP address family %d %s  hostname = %s",conf->server->ip.ss_family,
          conf->server->hostname, hostname);

  // restore original name for when host2server is called again
  strcpy(conf->server->hostname,originalName);
  
  if (conf->server->ip.ss_family == AF_INET) {
  
      addr = (struct sockaddr_in *)&conf->server->ip;
      
      /* the packet is from localhost if on localhost, to make configs easier */
      if ((addr->sin_addr.s_addr == ntohl(0x7f000001)) || (!hostname[0])) {
        ipaddr = 0x7f000001;
      } else {
    // try to get IP address of the eth0
    //    struct hostent *hp;
    //    
    //    if ((hp = gethostbyname(hostname)) == (struct hostent *) NULL) {
    //      ipaddr = 0x00000000;	/* no client IP address */
    //    } else {
    //      ipaddr = ntohl(*(UINT4 *) hp->h_addr); /* use the first one available */
    //    }
            int fd_aux;
            struct ifreq ifr;
            struct sockaddr_in * sin;
            fd_aux = socket(AF_INET,SOCK_STREAM,0);
            strcpy(ifr.ifr_name,"eth0");
            ifr.ifr_addr.sa_family = AF_INET;
            //[RK]Mar/16/06 - fixed problem with bonding Bug#6730
            if (ioctl(fd_aux,SIOCGIFADDR,&ifr) == -1) {
                    strcpy(ifr.ifr_name,"bond0");
                    ifr.ifr_addr.sa_family = AF_INET;
                    if (ioctl(fd_aux,SIOCGIFADDR,&ifr) == -1) {
                            sin = NULL;
                    } else {
                            sin = (struct sockaddr_in *) &ifr.ifr_addr;
                    }
            } else {
                    sin = (struct sockaddr_in *) &ifr.ifr_addr;
            }
            if (sin) {
                    ipaddr = (unsigned long)sin->sin_addr.s_addr; 
            } else {
                    ipaddr = 0;
            }
            close (fd_aux);
      }
  }
  
  _pam_log(LOG_DEBUG, "IPv4 address %X",ipaddr);

  /* If we can't find an IP address, then don't add one */
  if (ipaddr) {
    add_int_attribute(request, PW_NAS_IP_ADDRESS, ipaddr);
  }

  /* There's always a NAS identifier */
	//[RK]Nov/13/02 Bug#31 - NAS identifier is the hostname 
//  if (conf->client_id && *conf->client_id) {
//    add_attribute(request, PW_NAS_IDENTIFIER, conf->client_id,
//		  strlen(conf->client_id));
//  }
	add_attribute(request, PW_NAS_IDENTIFIER, hostname, strlen(hostname));

  /*
   *  Add in the port (pid) and port type (virtual).
   *
   *  We might want to give the TTY name here, too.
   */
	//[RK]Nov/13/02 Bug#31 - NAS_PORT_ID is the serial port number and the type is async.
	if (conf->server->nas_port) {
		add_int_attribute(request, PW_NAS_PORT_ID, conf->server->nas_port);
		add_int_attribute(request, PW_NAS_PORT_TYPE, PW_NAS_PORT_TYPE_ASYNC);
	} else {
		add_int_attribute(request, PW_NAS_PORT_ID, getpid());
		add_int_attribute(request, PW_NAS_PORT_TYPE, PW_NAS_PORT_TYPE_VIRTUAL);
	}
}

/*
 * Talk RADIUS to a server.
 * Send a packet and get the response
 */
static int
talk_radius(radius_conf_t *conf, AUTH_HDR *request, AUTH_HDR *response,
	    char *password, char *old_password, int tries)
{
  int salen, total_length;
  fd_set set;
  struct timeval tv;
  time_t now, end;
  int rcode;
  struct sockaddr_storage saremote;
  int size_saremote;
  struct sockaddr_in *s_in = (struct sockaddr_in *) &saremote;
  struct sockaddr_in6 *s_in6 = (struct sockaddr_in6 *) &saremote;
  radius_server_t *server = conf->server;
  int ok;
  int server_tries;

  /* ************************************************************ */
  /* Now that we're done building the request, we can send it */

  /*
    Hmm... on password change requests, all of the found server information
    could be saved with a pam_set_data(), which means even the radius_conf_t
    information will have to be malloc'd at some point

    On the other hand, we could just try all of the servers again in
    sequence, on the off chance that one may have ended up fixing itself.
    
    */
  
  /* loop over all available servers */
  while (server != NULL) {

    /* only look up IP information as necessary */
    if (host2server(server) != PAM_SUCCESS) {
      _pam_log(LOG_ERR, "Failed looking up IP address for RADIUS server %s",
	       server->hostname);
      ok = FALSE;
      goto next;		/* skip to the next server */
    }

    /* set up per-server IP && port configuration */
    saremote = server->ip;
    if (saremote.ss_family == AF_INET) {
        _pam_log(LOG_DEBUG, "target is IPv4");
        s_in->sin_port = server->port;
        size_saremote = sizeof(*s_in);
    } else {
        _pam_log(LOG_DEBUG, "target is IPv6");
        s_in6->sin6_port = server->port;
        size_saremote = sizeof(*s_in6);
    }
    
    total_length = ntohs(request->length);
    
    if (!password) { 		/* make an RFC 2139 p6 request authenticator */
      get_accounting_vector(request, server);
    }

    server_tries = tries;
	if (tries > 1 && server->tries) server_tries = server->tries;

send:
    /* send the packet */
    if (sendto(conf->sockfd, (char *) request, total_length, 0,
	       (struct sockaddr *)&saremote, size_saremote) < 0) {
      _pam_log(LOG_ERR, "Error sending RADIUS packet to server %s: %s",
	       server->hostname, strerror(errno));
      ok = FALSE;
      goto next;		/* skip to the next server */
    }

    /* ************************************************************ */
    /* Wait for the response, and verify it. */
    salen = sizeof(struct sockaddr_storage);
    tv.tv_sec = server->timeout; /* wait for the specified time */
    tv.tv_usec = 0;
    FD_ZERO(&set);		/* clear out the set */
    FD_SET(conf->sockfd, &set);	/* wait only for the RADIUS UDP socket */
    
    time(&now);
    end = now + tv.tv_sec;
    
    /* loop, waiting for the select to return data */
    ok = TRUE;
    while (ok) {

      rcode = select(conf->sockfd + 1, &set, NULL, NULL, &tv);

      /* select timed out */
      if (rcode == 0) {
	_pam_log(LOG_ERR, "RADIUS server %s failed to respond",
		 server->hostname);
	if (--server_tries)
	  goto send;
	ok = FALSE;
	break;			/* exit from the select loop */
      } else if (rcode < 0) {

	/* select had an error */
	if (errno == EINTR) {	/* we were interrupted */
	  time(&now);
	  
	  if (now > end) {
	    _pam_log(LOG_ERR, "RADIUS server %s failed to respond",
		     server->hostname);
	    if (--server_tries)
	      goto send;
	    ok = FALSE;
	    break;			/* exit from the select loop */
	  }
	  
	  tv.tv_sec = end - now;
	  if (tv.tv_sec == 0) {	/* keep waiting */
	    tv.tv_sec = 1;
	  }

	} else {		/* not an interrupt, it was a real error */
	  _pam_log(LOG_ERR, "Error waiting for response from RADIUS server %s: %s",
		   server->hostname, strerror(errno));
	  ok = FALSE;
	  break;
	}

	/* the select returned OK */
      } else if (FD_ISSET(conf->sockfd, &set)) {

	/* try to receive some data */
	if ((total_length = recvfrom(conf->sockfd, (char *) response,
				     BUFFER_SIZE,
				     0, (struct sockaddr *)&saremote, &salen)) < 0) {
	  _pam_log(LOG_ERR, "error reading RADIUS packet from server %s: %s",
		   server->hostname, strerror(errno));
	  ok = FALSE;
	  break;

	  /* there's data, see if it's valid */
	} else {
	  char *p = server->secret;
	  
	  if ((ntohs(response->length) != total_length) ||
	      (ntohs(response->length) > BUFFER_SIZE)) {
	    _pam_log(LOG_ERR, "RADIUS packet from server %s is corrupted",
		     server->hostname);
	    ok = FALSE;
	    break;
	  }

	  /* Check if we have the data OK.  We should also check request->id */

	  if (password) {
	    if (old_password) {
#ifdef LIVINGSTON_PASSWORD_VERIFY_BUG_FIXED
	      p = old_password;	/* what it should be */
#else
	      p = "";		/* what it really is */
#endif
	    }
	    /* 
	     * RFC 2139 p.6 says not do do this, but the Livingston 1.16
	     * server disagrees.  If the user says he wants the bug, give in.
	     */
	  } else {		/* authentication request */
	    if (conf->accounting_bug) {
	      p = "";
	    }
	  }
	    
	  if (!verify_packet(p, response, request)) {
	    _pam_log(LOG_ERR, "packet from RADIUS server %s fails verification: The shared secret is probably incorrect.",
		     server->hostname);
	    ok = FALSE;
	    break;
	  }

	  /*
	   * Check that the response ID matches the request ID.
	   */
	  if (response->id != request->id) {
	    _pam_log(LOG_WARNING, "Response packet ID %d does not match the request packet ID %d: verification of packet fails", response->id, request->id);
	      ok = FALSE;
	    break;
	  }
	}
	
	/*
	 * Whew!  The select is done.  It hasn't timed out, or errored out.
	 * It's our descriptor.  We've got some data.  It's the right size.
	 * The packet is valid.
	 * NOW, we can skip out of the select loop, and process the packet
	 */
	break;
      }
      /* otherwise, we've got data on another descriptor, keep select'ing */
    }

    /* go to the next server if this one didn't respond */
  next:
    if (!ok) {
      radius_server_t *old;	/* forget about this server */
      
      old = server;
      server = server->next;
      conf->server = server;

      _pam_forget(old->secret);
      free(old->hostname);
      free(old);

      if (server) {		/* if there's more servers to check */
	/* get a new authentication vector, and update the passwords */
	get_random_vector(request->vector);
	request->id = request->vector[0];
	
	/* update passwords, as appropriate */
	if (password) {
	  get_random_vector(request->vector);
	  if (old_password) {	/* password change request */
	    add_password(request, PW_PASSWORD, password, old_password);
	    add_password(request, PW_OLD_PASSWORD, old_password, old_password);
	  } else {		/* authentication request */
	    add_password(request, PW_PASSWORD, password, server->secret);
	  }
	}
      }
      continue;

    } else {
      /* we've found one that does respond, forget about the other servers */
      cleanup(server->next);
      server->next = NULL;
//      live_server = server;	/* we've got a live one! */
      copy_server_info (&live_server, server);
      valid_live_server = 1;
      break;
    }
  }

  if (!server) {
    _pam_log(LOG_ERR, "All RADIUS servers failed to respond.");
    return PAM_AUTHINFO_UNAVAIL;
  }
  
  return PAM_SUCCESS;
}

/**************************************************************************
 * MIDLEVEL PAM CODE
 **************************************************************************/

/* this is our front-end for module-application conversations */

#undef PAM_FAIL_CHECK
#define PAM_FAIL_CHECK if (retval != PAM_SUCCESS) { return retval; }

static int rad_converse(pam_handle_t *pamh, int msg_style, char *message, char **password)
{
  CONST struct pam_conv *conv;
  struct pam_message resp_msg;
  CONST struct pam_message *msg[1];
  struct pam_response *resp = NULL;
  int retval;
  
  resp_msg.msg_style = msg_style;
  resp_msg.msg = message;
  msg[0] = &resp_msg;
  
  /* grab the password */
  retval = pam_get_item(pamh, PAM_CONV, (CONST void **)(void *) &conv); //[GY] 2005/Sep/19  clean warning
  PAM_FAIL_CHECK;
  
  retval = conv->conv(1, msg, &resp,conv->appdata_ptr);
  PAM_FAIL_CHECK;
  
  if (password) {		/* assume msg.type needs a response */
    /* I'm not sure if this next bit is necessary on Linux */
#ifdef sun
    /* NULL response, fail authentication */
    if ((resp == NULL) || (resp->resp == NULL)) {
      return PAM_SYSTEM_ERR;
    }
#endif
    
    *password = resp->resp;
    free(resp);
  }
  
  return PAM_SUCCESS;
}

/**************************************************************************
 * GENERAL CODE
 **************************************************************************/

#undef PAM_FAIL_CHECK
#define PAM_FAIL_CHECK if (retval != PAM_SUCCESS) {	\
	int *pret = malloc( sizeof(int) );		\
	*pret = retval;					\
	pam_set_data( pamh, "rad_setcred_return"	\
	              , (void *) pret, _int_free );	\
	return retval; }

#define DPRINT if (ctrl & PAM_DEBUG_ARG) _pam_log

PAM_EXTERN int 
pam_sm_authenticate(pam_handle_t *pamh,int flags,int argc,CONST char **argv)
{
  CONST char *user;
  char *password = NULL;
  CONST char *rhost;
  char *resp2challenge = NULL;
  int ctrl, val;
  int retval = PAM_AUTH_ERR;
  struct pam_appl_data *pappl;
  attribute_t *a_cb;

  char recv_buffer[4096];
  char send_buffer[4096];
  AUTH_HDR *request = (AUTH_HDR *) send_buffer;
  AUTH_HDR *response = (AUTH_HDR *) recv_buffer;
  radius_conf_t config;
  int tries = 1;

  ctrl = _pam_parse(argc, argv, &config);

#ifdef RADIUS2FACTORenable
  if (ctrl & PAM_RADIUS_TWO_FACTOR) {
	  tries = 3;
  }
#endif

  /* grab the user name */
  retval = pam_get_user(pamh, &user, NULL);
  PAM_FAIL_CHECK;

  pappl = get_cyc_appl(pamh);

  /* check that they've entered something, and not too long, either */
  if ((user == NULL) ||
      (strlen(user) > MAXPWNAM)) {
    int *pret = malloc( sizeof(int) );
    *pret = PAM_USER_UNKNOWN;
    pam_set_data( pamh, "rad_setcred_return", (void *) pret, _int_free );

    DPRINT(LOG_DEBUG, "User name was NULL, or too long");
    return PAM_USER_UNKNOWN;
  }

  DPRINT(LOG_DEBUG, "Got user name %s", user);

  /*
   * Get the IP address of the authentication server
   * Then, open a socket, and bind it to a port
   */
  retval = initialize(&config, FALSE, pappl ? pappl->port : 0);
  PAM_FAIL_CHECK;

  /* 
   * If there's no client id specified, use the service type, to help
   * keep track of which service is doing the authentication.
   */
  if (!config.client_id) {
    retval = pam_get_item(pamh, PAM_SERVICE, (CONST void **) &config.client_id);
    PAM_FAIL_CHECK;
  }

  /* now we've got a socket open, so we've got to clean it up on error */
#undef PAM_FAIL_CHECK
#define PAM_FAIL_CHECK if (retval != PAM_SUCCESS) {goto error; }

  /* build and initialize the RADIUS packet */
  request->code = PW_AUTHENTICATION_REQUEST;
  get_random_vector(request->vector);
  request->id = request->vector[0]; /* this should be evenly distributed */
  
  /* grab the password (if any) from the previous authentication layer */
  retval = pam_get_item(pamh, PAM_AUTHTOK, (CONST void **)(void *) &password); //[GY] 2005/Sep/19  clean warning
  PAM_FAIL_CHECK;

  if(password) {
    password = strdup(password);
    //DPRINT(LOG_DEBUG, "Got password %s", password);
  }

try_again:
//  _pam_log(LOG_DEBUG, "$$$ RADIUS:tries %d $$$\n", tries);
#ifdef RADIUS2FACTORenable
  if (ctrl & PAM_RADIUS_TWO_FACTOR) {
	  _pam_forget(password);
	  password = NULL;
  }
#endif

  /* no previous password: maybe get one from the user */
  if (!password) {
    if (ctrl & PAM_USE_FIRST_PASS) {
      retval = PAM_AUTH_ERR;	/* use one pass only, stopping if it fails */
      goto error;
    }
    
    /* check to see if we send a NULL password the first time around */
    if (!(ctrl & PAM_SKIP_PASSWD)) {
#ifdef RADIUS2FACTORenable
		if (ctrl & PAM_RADIUS_TWO_FACTOR) {
//          Uncomment this section and remove the last rad_converse(..."2nd Password: "...)
//          in case you need to get Pin and OTP separately.

//			char pin_otp[50];
//			char *pin_otp_ptr;

//			retval = rad_converse(pamh, PAM_PROMPT_ECHO_OFF, "Pin: ", &pin_otp_ptr);
//			snprintf(pin_otp, 50, "%s", pin_otp_ptr);
//			_pam_forget(pin_otp_ptr);
//			retval = rad_converse(pamh, PAM_PROMPT_ECHO_OFF, "OTP: ", &pin_otp_ptr);
//			strncat(pin_otp, pin_otp_ptr, 50-strlen(pin_otp));
//			_pam_forget(pin_otp_ptr);

//			password = strdup(pin_otp);
			retval = rad_converse(pamh, PAM_PROMPT_ECHO_OFF, "2nd Password: ", &password);
		} else
#endif
			retval = rad_converse(pamh, PAM_PROMPT_ECHO_OFF, "Password: ", &password);
      PAM_FAIL_CHECK;

    }
  } /* end of password == NULL */

  build_radius_packet(request, user, password, &config);
  /* not all servers understand this service type, but some do */
  add_int_attribute(request, PW_USER_SERVICE_TYPE, PW_AUTHENTICATE_ONLY);

  /*
   *  Tell the server which host the user is coming from.
   *
   *  Note that this is NOT the IP address of the machine running PAM!
   *  It's the IP address of the client.
   */
  retval = pam_get_item(pamh, PAM_RHOST, (CONST void **)(void *) &rhost); //[GY] 2005/Sep/19  clean warning
  PAM_FAIL_CHECK;
  if (rhost) {
    add_attribute(request, PW_CALLING_STATION_ID, rhost, strlen(rhost));
  }

  if (pappl && pappl->caller && pappl->state[0]) {
    /* copy the state over from the servers response */
    add_attribute(request, PW_STATE, pappl->state, strlen(pappl->state));
    pappl->state[0] = 0;
  }

  DPRINT(LOG_DEBUG, "Sending RADIUS request code %d", request->code);

  retval = talk_radius(&config, request, response, password, NULL, config.retries);
  PAM_FAIL_CHECK;

  DPRINT(LOG_DEBUG, "Got RADIUS response code %d", response->code);

  /*
   *  If we get an authentication failure, and we sent a NULL password,
   *  ask the user for one and continue.
   *
   *  If we get an access challenge, then do a response, for as many
   *  challenges as we receive.
   */
#ifdef RADIUSSECIDenable
  while (response->code == PW_ACCESS_CHALLENGE) {
    attribute_t *a_state, *a_reply;
    char challenge[BUFFER_SIZE];
    
    /* Now we do a bit more work: challenge the user, and get a response */
    if (((a_state = find_attribute(response, PW_STATE, NULL)) == NULL) ||
	((a_reply = find_attribute(response, PW_REPLY_MESSAGE, NULL)) == NULL)) {
      _pam_log(LOG_ERR, "RADIUS access-challenge received with State or Reply-Message missing");
      retval = PAM_AUTHINFO_UNAVAIL;
      goto error;
    }
    
    if (pappl && pappl->caller) {
      memcpy(pappl->reply, a_reply->data, a_reply->length - 2);
      pappl->reply[a_reply->length - 2] = 0;
      memcpy(pappl->state, a_state->data, a_state->length - 2);
      pappl->state[a_state->length - 2] = 0;
      retval = PAM_AUTH_ERR;	/* authentication failure */
      pappl->result = 4;
      goto error;
    }

    memcpy(challenge, a_reply->data, a_reply->length - 2);
    challenge[a_reply->length - 2] = 0;
    
    /* It's full challenge-response, we might as well have echo on */
    retval = rad_converse(pamh, PAM_PROMPT_ECHO_OFF, challenge, &resp2challenge);

    /* now that we've got a response, build a new radius packet */
    build_radius_packet(request, user, resp2challenge, &config);
    /* request->code is already PW_AUTHENTICATION_REQUEST */
    request->id++;		/* one up from the request */

    /* copy the state over from the servers response */
    add_attribute(request, PW_STATE, a_state->data, a_state->length - 2);

    retval = talk_radius(&config, request, response, resp2challenge, NULL, 1);
    PAM_FAIL_CHECK;

    DPRINT(LOG_DEBUG, "Got response to challenge code %d", response->code);
  }
#endif

  /* Whew! Done the pasword checks, look for an authentication acknowledge */
  if (response->code == PW_AUTHENTICATION_ACK) {
    retval = PAM_SUCCESS;
#ifdef CBCPenable
    	if((a_cb = find_attribute(response, PW_USER_SERVICE_TYPE, NULL)) != NULL) {
		memcpy (&val, a_cb->data, sizeof(val));
		val = ntohl(val);
		if (val == PW_DIALBACK_FRAMED_USER && pappl) {
			if ((a_cb = find_attribute(response, PW_CALLBACK_NUMBER, NULL)) != NULL) {
				val = a_cb->length-2 > sizeof(pappl->callback_number)-1 ?
					sizeof(pappl->callback_number)-1 : a_cb->length-2;
     		 		memcpy(pappl->callback_number, a_cb->data, val);
      				pappl->callback_number[val] = 0;
			} else {
				pappl->callback_number[0] = 255;
			}
		}
	}
#endif
#ifdef GRPAUTHenable
	//[RK]May/19/05 - Group Authorization Support
	a_cb = NULL;
	if (pappl) {
		pappl->group_name = find_group_name(response);
	}

/* Nei - 12/03/07 - Begin                                                    */
/* Check for Service Type 6 = admin or 1 = regular user. Customer: Bloomberg */
	if (config.server->svctype==1) {
		a_cb = NULL;
		if (pappl && ((a_cb = find_attribute(response, PW_USER_SERVICE_TYPE, NULL)) != NULL)) {
			memcpy (&val, a_cb->data, sizeof(val));
			val = ntohl(val);
			if (val == PW_SHELL_USER) { // admin type
				if (pappl->group_name != NULL) { // previous allocation did happen
					free((void *)(pappl->group_name)); // free previous allocation
				}
				pappl->group_name = strdup("admin"); 
				if (pappl->group_name == NULL) { // this allocation failed
					_pam_log(LOG_WARNING, "group name memory allocation failed");
				}
			}
		}
	}
/* Nei - 12/03/07 - End                                                      */
#endif
	a_cb = NULL;
	while ((a_cb=find_attribute(response, PW_NAS_PORT_ID, a_cb)) != NULL) {
		memcpy(&val, a_cb->data, sizeof(val));
		val = ntohl(val);
		//DPRINT(LOG_DEBUG, "Got RADIUS NAS_Port_Id %d", val);
		if (val == config.server->nas_port) {
    		retval = PAM_SUCCESS;
			break;
		}
    	retval = PAM_AUTH_ERR;	/* authentication failure */
	}
  } else {
	  if (--tries)
		  goto try_again;
	  else
		  retval = PAM_AUTH_ERR;	/* authentication failure */
  }

error:
  /* If there was a password pass it to the next layer */
  if (password && *password) {
    pam_set_item(pamh, PAM_AUTHTOK, password);
  }

  if (ctrl & PAM_DEBUG_ARG) {
    _pam_log(LOG_DEBUG, "authentication %s"
	     , retval==PAM_SUCCESS ? "succeeded":"failed" );
  }
  
  close(config.sockfd);
  cleanup(config.server);
  _pam_forget(password);
  _pam_forget(resp2challenge);
  {
    int *pret = malloc( sizeof(int) );
    *pret = retval;
    pam_set_data( pamh, "rad_setcred_return", (void *) pret, _int_free );
  }
  return retval;
}

/*
 * Return a value matching the return value of pam_sm_authenticate, for
 * greatest compatibility. 
 * (Always returning PAM_SUCCESS breaks other authentication modules;
 * always returning PAM_IGNORE breaks PAM when we're the only module.)
 */
PAM_EXTERN int
pam_sm_setcred(pam_handle_t *pamh,int flags,int argc,CONST char **argv)
{
  int retval, *pret;

  retval = PAM_SUCCESS;
  pret = &retval;
  pam_get_data( pamh, "rad_setcred_return", (CONST void **)(void *) &pret ); //[GY] 2005/Sep/19  clean warning
  return *pret;
}

#undef PAM_FAIL_CHECK
#define PAM_FAIL_CHECK if (retval != PAM_SUCCESS) { return PAM_SESSION_ERR; }

static int
pam_private_session(pam_handle_t *pamh, int flags,
		    int argc, CONST char **argv,
		    int status)
{
  CONST char *user;
  int ctrl;
  int retval = PAM_AUTH_ERR;

  char recv_buffer[4096];
  char send_buffer[4096];
  AUTH_HDR *request = (AUTH_HDR *) send_buffer;
  AUTH_HDR *response = (AUTH_HDR *) recv_buffer;
  radius_conf_t config;
  struct pam_appl_data *pappl;

  ctrl = _pam_parse(argc, argv, &config);

  pappl = get_cyc_appl(pamh);

  /* grab the user name */
  retval = pam_get_user(pamh, &user, NULL);
  PAM_FAIL_CHECK;

  /* check that they've entered something, and not too long, either */
  if ((user == NULL) ||
      (strlen(user) > MAXPWNAM)) {
    return PAM_USER_UNKNOWN;
  }

  /*
   * Get the IP address of the authentication server
   * Then, open a socket, and bind it to a port
   */
  retval = initialize(&config, TRUE, pappl ? pappl->port : 0);
  PAM_FAIL_CHECK;

  /* 
   * If there's no client id specified, use the service type, to help
   * keep track of which service is doing the authentication.
   */
  if (!config.client_id) {
    retval = pam_get_item(pamh, PAM_SERVICE, (CONST void **) &config.client_id);
    PAM_FAIL_CHECK;
  }

  /* now we've got a socket open, so we've got to clean it up on error */
#undef PAM_FAIL_CHECK
#define PAM_FAIL_CHECK if (retval != PAM_SUCCESS) {goto error; }

  /* build and initialize the RADIUS packet */
  request->code = PW_ACCOUNTING_REQUEST;
  get_random_vector(request->vector);
  request->id = request->vector[0]; /* this should be evenly distributed */

  build_radius_packet(request, user, NULL, &config);

  add_int_attribute(request, PW_ACCT_STATUS_TYPE, status);

  sprintf(recv_buffer, "%08d", (int) getpid());
  add_attribute(request, PW_ACCT_SESSION_ID, recv_buffer, strlen(recv_buffer));

  add_int_attribute(request, PW_ACCT_AUTHENTIC, PW_AUTH_RADIUS);

  if (status == PW_STATUS_START) {
    session_time = time(NULL);
  } else {
    add_int_attribute(request, PW_ACCT_SESSION_TIME, time(NULL) - session_time);
  }

  retval = talk_radius(&config, request, response, NULL, NULL, 1);
  PAM_FAIL_CHECK;

  /* oops! They don't have the right password.  Complain and die. */
  if (response->code != PW_ACCOUNTING_RESPONSE) {
    retval = PAM_PERM_DENIED;
    goto error;
  }

  retval = PAM_SUCCESS;

error:

  close(config.sockfd);
  cleanup(config.server);

  return retval;
}

PAM_EXTERN int
pam_sm_open_session(pam_handle_t *pamh, int flags,
		    int argc, CONST char **argv)
{
  return pam_private_session(pamh, flags, argc, argv, PW_STATUS_START);
}

PAM_EXTERN int
pam_sm_close_session(pam_handle_t *pamh, int flags,
		     int argc, CONST char **argv)
{
  return pam_private_session(pamh, flags, argc, argv, PW_STATUS_STOP);
}

#undef PAM_FAIL_CHECK
#define PAM_FAIL_CHECK if (retval != PAM_SUCCESS) {return retval; }
#define MAX_PASSWD_TRIES 3

PAM_EXTERN int
pam_sm_chauthtok(pam_handle_t *pamh, int flags, int argc, CONST char **argv)
{
  CONST char *user;
  char *password = NULL;
  char *new_password = NULL;
  char *check_password = NULL;
  int ctrl;
  int retval = PAM_AUTHTOK_ERR;
  int attempts;
  struct pam_appl_data *pappl;

  char recv_buffer[4096];
  char send_buffer[4096];
  AUTH_HDR *request = (AUTH_HDR *) send_buffer;
  AUTH_HDR *response = (AUTH_HDR *) recv_buffer;
  radius_conf_t config;

  ctrl = _pam_parse(argc, argv, &config);

  pappl = get_cyc_appl(pamh);

  /* grab the user name */
  retval = pam_get_user(pamh, &user, NULL);
  PAM_FAIL_CHECK;

  /* check that they've entered something, and not too long, either */
  if ((user == NULL) ||
      (strlen(user) > MAXPWNAM)) {
    return PAM_USER_UNKNOWN;
  }

  /*
   * Get the IP address of the authentication server
   * Then, open a socket, and bind it to a port
   */
  retval = initialize(&config, FALSE, pappl ? pappl->port : 0);
  PAM_FAIL_CHECK;

  /* 
   * If there's no client id specified, use the service type, to help
   * keep track of which service is doing the authentication.
   */
  if (!config.client_id) {
    retval = pam_get_item(pamh, PAM_SERVICE, (CONST void **) &config.client_id);
    PAM_FAIL_CHECK;
  }

  /* now we've got a socket open, so we've got to clean it up on error */
#undef PAM_FAIL_CHECK
#define PAM_FAIL_CHECK if (retval != PAM_SUCCESS) {goto error; }

  /* grab the old password (if any) from the previous password layer */
  retval = pam_get_item(pamh, PAM_OLDAUTHTOK, (CONST void **)(void *) &password); //[GY] 2005/Sep/19  clean warning
  PAM_FAIL_CHECK;
  if(password) password = strdup(password);

  /* grab the new password (if any) from the previous password layer */
  retval = pam_get_item(pamh, PAM_AUTHTOK, (CONST void **)(void *) &new_password); //[GY] 2005/Sep/19  clean warning
  PAM_FAIL_CHECK;
  if(new_password) new_password = strdup(new_password);

  /* preliminary password change checks. */
  if (flags & PAM_PRELIM_CHECK) {
    if (!password) {		/* no previous password: ask for one */
      retval = rad_converse(pamh, PAM_PROMPT_ECHO_OFF, "Password: ", &password);
      PAM_FAIL_CHECK;
    }
    
    /*
     * We now check the password to see if it's the right one.
     * If it isn't, we let the user try again.
     * Note that RADIUS doesn't have any concept of 'root'.  The only way
     * that root can change someone's password is to log into the RADIUS
     * server, and and change it there.
     */
    
    /* build and initialize the access request RADIUS packet */
    request->code = PW_AUTHENTICATION_REQUEST;
    get_random_vector(request->vector);
    request->id = request->vector[0]; /* this should be evenly distributed */
    
    build_radius_packet(request, user, password, &config);
    add_int_attribute(request, PW_USER_SERVICE_TYPE, PW_AUTHENTICATE_ONLY);
    
    retval = talk_radius(&config, request, response, password, NULL, 1);
    PAM_FAIL_CHECK;

    /* oops! They don't have the right password.  Complain and die. */
    if (response->code != PW_AUTHENTICATION_ACK) {
      _pam_forget(password);
      retval = PAM_PERM_DENIED;
      goto error;
    }

    /*
     * We're now sure it's the right user.
     * Ask for their new password, if appropriate
     */
    
    if (!new_password) {	/* not found yet: ask for it */
      int new_attempts;
      attempts = 0;

      /* loop, trying to get matching new passwords */
      while (attempts++ < 3) {

	/* loop, trying to get a new password */
	new_attempts = 0;
	while (new_attempts++ < 3) {
	  retval = rad_converse(pamh, PAM_PROMPT_ECHO_OFF,
				"New password: ", &new_password);
	  PAM_FAIL_CHECK;
	  
	  /* the old password may be short.  Check it, first. */
	  if (strcmp(password, new_password) == 0) { /* are they the same? */
	    rad_converse(pamh, PAM_ERROR_MSG,
			 "You must choose a new password.", NULL);
	    _pam_forget(new_password);
	    continue;
	  } else if (strlen(new_password) < 6) {
	    rad_converse(pamh, PAM_ERROR_MSG, "it's WAY too short", NULL);
	    _pam_forget(new_password);
	    continue;
	  }

	  /* insert crypt password checking here */
	  
	  break;		/* the new password is OK */
	}
	
	if (new_attempts >= 3) { /* too many new password attempts: die */
	  retval = PAM_AUTHTOK_ERR;
	  goto error;
	}
	
	/* make sure of the password by asking for verification */
	retval =  rad_converse(pamh, PAM_PROMPT_ECHO_OFF,
			       "New password (again): ", &check_password);
	PAM_FAIL_CHECK;
	
	retval = strcmp(new_password, check_password);
	_pam_forget(check_password);
	
	/* if they don't match, don't pass them to the next module */
	if (retval != 0) {
	  _pam_forget(new_password);
	  rad_converse(pamh, PAM_ERROR_MSG,
		       "You must enter the same password twice.", NULL);
	  retval = PAM_AUTHTOK_ERR;
	  goto error;		/* ??? maybe this should be a 'continue' ??? */
	}

	break;			/* everything's fine */
      }	/* loop, trying to get matching new passwords */

      if (attempts >= 3) { /* too many new password attempts: die */
	retval = PAM_AUTHTOK_ERR;
	goto error;
      }
    } /* now we have a new password which passes all of our tests */

    /*
     * Solaris 2.6 calls pam_sm_chauthtok only ONCE, with PAM_PRELIM_CHECK
     * set.
     */
#ifndef sun
    /* If told to update the authentication token, do so. */
  } else if (flags & PAM_UPDATE_AUTHTOK) {
#endif

    if (!password || !new_password) { /* ensure we've got passwords */
      retval = PAM_AUTHTOK_ERR;
      goto error;
    }
    
    /* build and initialize the password change request RADIUS packet */
    request->code = PW_PASSWORD_REQUEST;
    get_random_vector(request->vector);
    request->id = request->vector[0]; /* this should be evenly distributed */

    /* the secret here can not be know to the user, so it's the new password */
    _pam_forget(config.server->secret);
    config.server->secret = strdup(password); /* it's free'd later */

    build_radius_packet(request, user, new_password, &config);
    add_password(request, PW_OLD_PASSWORD, password, password);
    
    retval = talk_radius(&config, request, response, new_password, password, 1);
    PAM_FAIL_CHECK;

    /* Whew! Done password changing, check for password acknowledge */
    if (response->code != PW_PASSWORD_ACK) {
      retval = PAM_AUTHTOK_ERR;
      goto error;
    }
  }
  
  /*
   * Send the passwords to the next stage if preliminary checks fail,
   * or if the password change request fails.
   */
  if ((flags & PAM_PRELIM_CHECK) || (retval != PAM_SUCCESS)) {
  error:
    
    /* If there was a password pass it to the next layer */
    if (password && *password) {
      pam_set_item(pamh, PAM_OLDAUTHTOK, password);
    }
    
    if (new_password && *new_password) {
      pam_set_item(pamh, PAM_AUTHTOK, new_password);
    }
  }
  
  if (ctrl & PAM_DEBUG_ARG) {
    _pam_log(LOG_DEBUG, "password change %s"
	     , retval==PAM_SUCCESS ? "succeeded":"failed" );
  }
  
  close(config.sockfd);
  cleanup(config.server);

  _pam_forget(password);
  _pam_forget(new_password);
  return retval;
}

PAM_EXTERN int
pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
  return(PAM_SUCCESS);
}
 
#ifdef PAM_STATIC

/* static module data */

struct pam_module _pam_radius_modstruct = {
  "pam_radius_auth",
  pam_sm_authenticate,
  pam_sm_setcred,
  NULL,
  pam_sm_open_session,
  pam_sm_close_session,
  pam_sm_chauthtok,
};
#endif

/*
 *  This module is a merger of an old version of pam_radius.c, and
 * code which went into mod_auth_radius.c, with further modifications
 * by Alan DeKok of CRYPTOCard Inc..
 *
 * The original pam_radius.c code is copyright (c) Cristian Gafton, 1996,
 *                                             <gafton@redhat.com>
 *
 * The additional code is copyright (c) CRYPTOCard Inc, 1998.
 *
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
 *
 */
