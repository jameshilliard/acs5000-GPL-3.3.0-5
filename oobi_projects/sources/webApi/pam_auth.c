/*****************************************************************
 * File: pam_auth.c
 *
 * Copyright (C) 2003 Cyclades Corporation
 *
 * Maintainer: www.cyclades.com
 *
 * Description: Contains functions for authentication through PAM.
 *
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <grp.h>
#include <pwd.h>
#include <dirent.h>
#include <linux/version.h>
#include <webApi.h>
#include "WebApi2.h"

#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <security/pam_appdata.h>
#include "userinfo.h"
#include "pam_auth.h"

static pam_handle_t *pamh=NULL;
static char currentPwd[kPasswordLength] = {0};

#define PAM_FAIL_CHECK(str, retcode, flag) \
  if (retcode != PAM_SUCCESS) { \
    syslog(LOG_ERR,"%s : %s", str, pam_strerror(pamh, retcode)); \
    if (flag) { \
      return(1); \
    } \
  }

//========================================================================
// WebLoginConv - This is the conversation function called from a PAM
// authentication module to print error messages or garner information
// from the user.
//========================================================================
static int WebLoginConv(int num_msg, const struct pam_message **msgm,
        struct pam_response **response, void *appdata_ptr)
{
  struct pam_response *reply;
  int count;
	char *passwd = currentPwd;

  /* PAM will free this later */
  reply = malloc(num_msg * sizeof(*reply));
  if (reply == NULL)
    return PAM_CONV_ERR;

  for (count = 0; count < num_msg; count++) {
    /*
     * Username and Password alread in struct Auth...
     */
    switch(msgm[count]->msg_style) 
		{
			case PAM_PROMPT_ECHO_ON:
				free(reply);
				return PAM_CONV_ERR;
			case PAM_PROMPT_ECHO_OFF:
				if (!*passwd) {
					free(reply);
					return PAM_CONV_ERR;
				}
				reply[count].resp = x_strdup(passwd);
				reply[count].resp_retcode = PAM_SUCCESS;
				break;
			case PAM_ERROR_MSG:
			case PAM_TEXT_INFO:
				reply[count].resp = x_strdup("");
				reply[count].resp_retcode = PAM_SUCCESS;
				break;
			default:
				free(reply);
				return PAM_CONV_ERR;
    }
  }

  *response = reply;

  return PAM_SUCCESS;
}

// Setting the conversation function into the pam_conv structure
static struct pam_appl_data cyc_appl_data = {
	id : 0x1234fedc,
	sock : 1,
	ppp : 0,
	caller : 0,
	port : 0,
	flag : 1
};

static struct pam_conv conv = 
{
   WebLoginConv,
   &cyc_appl_data
};

//========================================================================
// DoPamAuthentication - This function calls a series of PAM APIs that
// performs the authentication of the user. 
//========================================================================
static int DoPamAuthentication(char *service, int type)
{
	int retcode;

	dolog("DoPamAuthentication()");

	/* set PAM_SERVICE to the correct value */
	retcode = pam_set_item(pamh, PAM_SERVICE, service);
	PAM_FAIL_CHECK("pam_set_item PAM_SERVICE", retcode, 0);

	/*while (!authenticated && retry++ < MAX_RETRIES) 
	{
		retcode = pam_authenticate(pamh, 0);
    authenticated = (retcode == PAM_SUCCESS);
  }*/

	if ((retcode = pam_authenticate(pamh, 0)) != PAM_SUCCESS)
	{
		dolog("Pam authenticate failed");
		PAM_FAIL_CHECK("pam_authenticate", retcode, 1);
	}

	/* Only web  performs authorization and session */
/*	if (type > PAM_WEB) {
		return(0);
	}		*/ //Christine:comments this line to support getting groupname from Tacacas auth server 

	/* the user is valid, but should they have access at this time? */
	if ((retcode = pam_acct_mgmt(pamh, 0)) != PAM_SUCCESS) /* `0' could be as above */
	{
		PAM_FAIL_CHECK("pam_acct_mgmt", retcode, 1);
	}

	if (retcode == PAM_NEW_AUTHTOK_REQD) 
	{
		if ((retcode = pam_chauthtok(pamh,PAM_CHANGE_EXPIRED_AUTHTOK)) != PAM_SUCCESS)
		{
			PAM_FAIL_CHECK("pam_chauthtok", retcode, 1);
		}
	}

	if ((retcode = pam_setcred(pamh, PAM_ESTABLISH_CRED)) != PAM_SUCCESS)
	{
		PAM_FAIL_CHECK("pam_setcred", retcode, 1);
	}

	/* open a session for the user --- `0' could be PAM_SILENT */
	if ((retcode = pam_open_session(pamh,0)) != PAM_SUCCESS)
	{
		PAM_FAIL_CHECK("pam_open_session", retcode, 1);
	}

	return(0);
}

//========================================================================
// PamClose - Contains functions that allows PAM to cleanup before 
// process exits. 
//========================================================================
static int PamClose(int type)
{
  int retcode;

	dolog("PamClose()");

  if (type == PAM_WEB) {

    if ((retcode = pam_close_session(pamh,0)) != PAM_SUCCESS)
	{	
		PAM_FAIL_CHECK("pam_close_session", retcode, 0);
	}

    if ((retcode = pam_setcred(pamh, PAM_DELETE_CRED)) != PAM_SUCCESS)
	{
		PAM_FAIL_CHECK("pam_setcred", retcode, 0);
	}

  }
  pam_end(pamh, PAM_SUCCESS);

  return(0);
}

//========================================================================
// PamAuthenticate - This function calls some PAM APIs that initiates
// an authentication  transaction.
//========================================================================
int PamAuthenticate(const char *user, const char *passw, int type)
{
	char buf[256];
	int retcode;
	char stype[7];

	dolog("PamAuthenticate()");
	
	switch(type) {
		case PAM_WEB:
			strncpy(stype, "web", 4);
			break;
#if defined(KVM) || defined(ONS)
		case PAM_OSD:
			strncpy(stype, "osd", 4);
			break;
		case PAM_KVM:
			strncpy(stype, "kvm", 4);
			break;
#endif
		default:
			return 0;

	}

#if defined(KVM) || defined(ONS)
//assure admin user will always fallback to local authentication
// if service unavailable (...local)

	if (!strncmp(user, ADMIN_USER, strlen(ADMIN_USER) + 1)) {
		if (type != PAM_KVM) {
			strncpy(stype, "admin", 6);
		} else {
			strncpy(stype, "kvmadm", 7);
		}
	}
#endif

	strncpy(currentPwd, passw, kPasswordLength);

	/* initialize the Linux-PAM library */
	if ((retcode = pam_start(stype, user, &conv, &pamh)) != PAM_SUCCESS)
	{
		PAM_FAIL_CHECK("pam_start", retcode, 1);
	}

	/* set PAM_RHOST  */
	gethostname(buf, sizeof(buf));
	retcode = pam_set_item(pamh, PAM_RHOST, buf);
	PAM_FAIL_CHECK("pam_set_item PAM_RHOST", retcode, 0);

	/* set tty for web login */
	retcode = pam_set_item(pamh, PAM_TTY, stype);
	PAM_FAIL_CHECK("pam_set_item PAM_TTY", retcode, 0);
	

	retcode = DoPamAuthentication(stype, type);

	PamClose(type);

	return(retcode);
}

#ifdef GRPAUTHenable
void PAM_get_group_name(char ** group_name)
{
	if (cyc_appl_data.group_name) {
		*group_name = cyc_appl_data.group_name;
		cyc_appl_data.group_name = NULL;
	} else {
		*group_name = NULL;
	}
}
#endif

