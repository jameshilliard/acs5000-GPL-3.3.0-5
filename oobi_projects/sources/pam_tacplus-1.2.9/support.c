/* support.c	support functions for pam_tacplus.c
 * 
 * Copyright 1998,1999,2000 by Pawel Krawczyk <kravietz@ceti.pl>
 * See `CHANGES' file for revision history.
 */
#include <stdlib.h>
#include <syslog.h>
#include <stdarg.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#define PAM_SM_AUTH
#define PAM_SM_ACCOUNT
#define PAM_SM_SESSION
/* #define PAM_SM_PASSWORD */

#include <security/pam_modules.h>

#include "pam_tacplus.h"
#include "tacplus.h"
#include "libtac.h"

char tac_vers[MAX_TAC_VERS];
int max_tac_vers=0;
struct sockaddr_storage tac_srv[TAC_MAX_SERVERS];
int tac_srv_no = 0;
char *tac_service = NULL;
char *tac_protocol = NULL;
int tac_timeout=30,tac_retries=3;

/* libtac */
extern char *tac_secret;
extern int tac_encryption;

/**
 * Returns NULL if there was no error or an error message.
 */
const char *xinet_aton(const char *buffer, struct sockaddr_storage *server) {
	
	struct addrinfo *res = NULL;
	struct addrinfo hints;
	int n;
	memset(&hints,0,sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_flags = AI_NUMERICHOST;
	n = getaddrinfo(buffer,NULL,&hints,&res);
	
	if (n == 0) {
		memcpy(server,res->ai_addr,res->ai_addrlen);
		freeaddrinfo(res);
		return NULL;
	} else {
		const char *msg = gai_strerror(n);
		return msg;
	}
}

#ifndef xcalloc
void *_xcalloc (size_t size)
{
  register void *val = calloc (1, size);
  if (val == 0)
    {
      syslog (LOG_ERR, "xcalloc: calloc(1,%u) failed", size);
      exit (1);
    }
  return val;
}
#else
#define _xcalloc xcalloc
#endif

char *_pam_get_terminal(pam_handle_t *pamh) {
		int retval;
		char *tty;

  		retval = pam_get_item (pamh, PAM_TTY, (const void **)(void *) &tty); //[GY] 2005/Sep/19  clean warning
  		if (retval != PAM_SUCCESS || tty == NULL || *tty == '\0') {
		  tty = ttyname(STDIN_FILENO);
		  if(tty == NULL || *tty == '\0') {
				  tty = "unknown";
		  }
  		}
		return(tty);
}

void _pam_log (int err, const char *format,...)
{
  va_list args;

  va_start (args, format);
  openlog ("PAM-tacplus", LOG_PID, LOG_AUTH);
  vsyslog (err, format, args);
  va_end (args);
  closelog ();
}

/**
 * The buffer parameter needs to have at least INET6_ADDRSTRLEN bytes.
 */
char *xinet_ntoa(struct sockaddr *server, char *buffer, int b_len) {
	
	struct sockaddr_storage addr;
	int addr_len = sizeof(addr);
	int n;
	
	if (server->sa_family == AF_INET) {
		addr_len = sizeof(struct sockaddr_in);
		memcpy(&addr,server,addr_len);
		server = (struct sockaddr *)&addr;
	} else if (server->sa_family == AF_INET6) {
		addr_len = sizeof(struct sockaddr_in6);
		memcpy(&addr,server,addr_len);
		server = (struct sockaddr *)&addr;
	}
	
	n = getnameinfo(server,addr_len,
			buffer,b_len,NULL,0,NI_NUMERICHOST);
	
	if (n != 0) {
		_pam_log(LOG_ERR, "error in %s getnameinfo - %d", __FUNCTION__, n);
	}
	
	return buffer;
}

/* stolen from pam_stress */
int converse (pam_handle_t * pamh, int nargs
	  ,struct pam_message **message
	  ,struct pam_response **response)
{
  int retval;
  struct pam_conv *conv;

  if ((retval = pam_get_item (pamh, PAM_CONV, (const void **)(void *) &conv)) //[GY] 2005/Sep/19  clean warning
      == PAM_SUCCESS)
    {
      retval = conv->conv (nargs, (const struct pam_message **) message
			   ,response, conv->appdata_ptr);
      if (retval != PAM_SUCCESS)
	{
	  _pam_log (LOG_ERR, "(pam_tacplus) converse returned %d", retval);
	  _pam_log (LOG_ERR, "that is: %s", pam_strerror (pamh, retval));
	}
    }
  else
    {
      _pam_log (LOG_ERR, "(pam_tacplus) converse failed to get pam_conv");
    }

  return retval;
}

/* stolen from pam_stress */
int tacacs_get_password (pam_handle_t * pamh, int flags
		     ,int ctrl, char **password)
{
  char *pass=NULL;   //[GY] 2005/Aug/31  clean warnings

  struct pam_message msg[1], *pmsg[1];
  struct pam_response *resp;
  int retval;

  if (ctrl & PAM_TAC_DEBUG)
    syslog (LOG_DEBUG, "%s: called", __FUNCTION__);

  /* set up conversation call */

  pmsg[0] = &msg[0];
  msg[0].msg_style = PAM_PROMPT_ECHO_OFF;
  msg[0].msg = "Password: ";
  resp = NULL;

  if ((retval = converse (pamh, 1, pmsg, &resp)) != PAM_SUCCESS)
    {
      return retval;
    }

  if (resp)
    {
      if ((resp[0].resp == NULL) && (ctrl & PAM_TAC_DEBUG))
	{
	  _pam_log (LOG_DEBUG,
		    "pam_sm_authenticate: NULL authtok given");
	}
      pass = resp[0].resp;	/* remember this! */

      resp[0].resp = NULL;
    }
  else if (ctrl & PAM_TAC_DEBUG)
    {
      _pam_log (LOG_DEBUG, "pam_sm_authenticate: no error reported");
      _pam_log (LOG_DEBUG, "getting password, but NULL returned!?");
      return PAM_CONV_ERR;
    }
  /*        free(resp); *//* this causes SIGILL - need to investigate :) */
  free(resp);
  resp = NULL;

  *password = pass;		/* this *MUST* be free()'d by this module */

  if(ctrl & PAM_TAC_DEBUG)
	syslog(LOG_DEBUG, "%s: obtained password [%s]", __FUNCTION__,
			pass);

  return PAM_SUCCESS;
}

const char * _resolve_name (char *serv, struct sockaddr_storage *addr) {

	struct addrinfo *res;
	
	int n = getaddrinfo(serv, NULL, NULL, &res);

	if (n != 0) {
		const char *msg = gai_strerror(n);
		if (PAM_TAC_DEBUG) {
			_pam_log (LOG_ERR, "pam_sm_authenticate: no error reported %s",msg);
		}
		return msg;
	} else {
		memcpy(addr,res->ai_addr,res->ai_addrlen);
		freeaddrinfo(res);
		return NULL;
	}
}

int _pam_parse_argv(int ctrl, char *argv)
{
    if (!strcmp (argv, "debug")) /* all */
	  ctrl |= PAM_TAC_DEBUG;
    else if (!strcmp(argv, "use_first_pass"))
      ctrl |= PAM_USE_FIRST_PASS;
    else if (!strncmp (argv, "tacplus_vers=", 13))
	{
	  char *p = argv+13;
	  if (!strcmp(p, "V0_V1")) {
		max_tac_vers = 2;
		tac_vers[0] = 0;
		tac_vers[1] = 1;
	  } else if (!strcmp(p, "V1_V0")) {
		max_tac_vers = 2;
		tac_vers[0] = 1;
		tac_vers[1] = 0;
	  } else if (!strcmp(p, "V1")) {
		max_tac_vers = 1;
		tac_vers[0] = 1;
	  } else if (!strcmp(p, "V0")) {
		max_tac_vers = 1;
		tac_vers[0] = 0;
	  }
	}
    else if (!strcmp (argv, "encrypt"))
	{
	  ctrl |= PAM_TAC_ENCRYPT;
	  tac_encryption = 1;
	}
	  else if (!strncmp (argv, "timeout=",8)) /* timeout for authentication */
	  {
		tac_timeout=atoi(argv+8);
	  }
	  else if (!strncmp (argv, "retries=",8)) /* timeout for authentication */
	  {
		tac_retries=atoi(argv+8);
	  }
      else if (!strcmp (argv, "first_hit")) /* authentication */
	ctrl |= PAM_TAC_FIRSTHIT;
      else if (!strncmp (argv, "service=", 8)) /* author & acct */
	{
	  tac_service = (char *) _xcalloc (strlen (argv + 8) + 1);
	  strcpy (tac_service, argv + 8);
	}
      else if (!strncmp (argv, "protocol=", 9)) /* author & acct */
	{
	  tac_protocol = (char *) _xcalloc (strlen (argv + 9) + 1);
	  strcpy (tac_protocol, argv + 9);
	}
      else if (!strcmp (argv, "acct_all")) {
	      ctrl |= PAM_TAC_ACCT;
      } 
      else if (!strncmp (argv, "server=", 7)) /* authen & acct */
	{
      struct sockaddr_storage addr;
	  
	  if (tac_srv_no < TAC_MAX_SERVERS) {		  
	  	  const char *error_msg = _resolve_name (argv + 7, &addr);
	  	  if (error_msg == NULL) {
	      	  	tac_srv[tac_srv_no] = addr;
	      		tac_srv_no++;
	    	  } else {
	    		_pam_log (LOG_ERR,
		      		"skip invalid server: %s (%s)",
		      		argv + 7, error_msg);
	    	  }
	  } else {
		  _pam_log(LOG_ERR, "maximum number of servers (%d) exceeded, skipping",
				  TAC_MAX_SERVERS);
	  }
	}
      else if (!strncmp (argv, "secret=", 7))
	{
	  tac_secret = (char *) _xcalloc (strlen (argv + 7) + 1);
	  strcpy (tac_secret, argv + 7);
	}
      else
	_pam_log (LOG_WARNING, "unrecognized option: %s", argv);

	return (ctrl);
}

//[RK]Jun/02/05 - Group Authorization Support - removed the get_str
int _pam_parse ( int acct, int port, int argc, const char **argv)
{
//int ix = 0;                 [GY] 2005/aug/25  clean warnings
  int ctrl = 0;
//char *p, *ptr, *line;       [GY] 2005/aug/25  clean warnings
  char *ptr, *line;
  char buf[BUFFER_SIZE], tmpstr[100];
  FILE *fp = NULL;
//unsigned long addr = 0;     [GY] 2005/aug/25  clean warnings
  int linenum = 0, do_encrypt = 0;

  /* otherwise the list will grow with each call */
  tac_srv_no = 0;
  max_tac_vers=0;

	// read from default configuration file
	if ((fp = fopen(DEF_CONF_FILE, "r")) != NULL) {
		while (!feof(fp) &&
			(fgets (buf, sizeof(buf), fp) != NULL) &&
			!ferror(fp)) {
			line = ptr = buf;
			if ((*ptr == '#') || !ptr ||
				(*ptr == '\n')) {
      			continue;
			}
			while (*ptr && (*ptr != '\n'))
				ptr++;
			if (*ptr == '\n') *ptr = 0;

			linenum++;

			if (!strncmp(line, "authhost1=", 10) ||
				!strncmp(line, "authhost2=", 10)) {
					if (acct == 0) {
						memset(tmpstr, 0, sizeof(tmpstr));
						sprintf(tmpstr, "server=%s", line+10);
						ctrl = _pam_parse_argv(ctrl, tmpstr);
					}
			} else if (!strncmp(line, "accthost1=", 10) ||
						!strncmp(line, "accthost2=", 10)) {
					if (acct == 1) {
						memset(tmpstr, 0, sizeof(tmpstr));
						sprintf(tmpstr, "server=%s", line+10);
						ctrl = _pam_parse_argv(ctrl, tmpstr);
					}
			} else if (!strncmp(line, "secret=", 7)) {
					ctrl = _pam_parse_argv(ctrl, line);
			} else if (!strncmp(line, "encrypt=", 8)) {
				if (sscanf(line, "encrypt=%d", &do_encrypt) == 1) {
					if (do_encrypt) {
						ctrl = _pam_parse_argv(ctrl, "encrypt");
					}
				} else {
					_pam_log(LOG_ERR, "invalid line%d in reading %s",
					linenum, DEF_CONF_FILE);
				}
			} else if (!strncmp(line, "service=", 8)) {
					ctrl = _pam_parse_argv(ctrl, line);
			} else if (!strncmp(line,  "protocol=", 9)) {
					ctrl = _pam_parse_argv(ctrl, line);
			} else if (!strncmp(line,  "timeout=", 8)) {
					ctrl = _pam_parse_argv(ctrl, line);
			} else if (!strncmp(line,  "retries=", 8)) {
					ctrl = _pam_parse_argv(ctrl, line);
    		} else if (!strncmp (line, "tacplus_vers=", 13)) {
					ctrl = _pam_parse_argv(ctrl, line);
			} else {
				_pam_log(LOG_ERR, "invalid line%d in reading %s",
					linenum, DEF_CONF_FILE);
			}
		}
	}

  if (fp) {
	fclose(fp);
  }

  for ( ; argc-- > 0; ++argv) {
    ctrl = _pam_parse_argv(ctrl, (char *)*argv);  //[GY] 2005/Sep/19  clean warning
  }

  if (max_tac_vers == 0) {
	max_tac_vers = 1;
	tac_vers[0] = 1;
  }

  return ctrl;

}				/* _pam_parse */

/*
 * Copyright (c) Cristian Gafton, 1996, <gafton@redhat.com>
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
