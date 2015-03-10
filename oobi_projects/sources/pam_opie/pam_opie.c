/* pam_opie.c - OPIE authentication module for PAM */

/* Copyright (C) 1997-1998, Andrew Berkheimer <andy@tho.org>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version. 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * History:
 * 	0.1 - Jul 31, 1997
 *		Initial Release 
 *      0.2 - Apr 20, 1998
 * 		Long awaited update. No major changes. Updated libopie
 *		from 2.31 to 2.32, moved xstrdup into this file to avoid
 *		dynamic linking problems, got it to work apparantly
 *		flawlessly under Redhat 5.
 *      0.21 - May 26, 1998
 *              Added the libopie Makefile that I left out of 0.2. Oops.
 */

#define PAM_SM_AUTH
#include <security/pam_modules.h>
#include <security/pam_appdata.h>
#include <stdlib.h>
#include "opie.h"

int converse(pam_handle_t*, int, struct pam_message**, struct pam_response**);
int send_text(pam_handle_t*, const char*);
char *xstrdup(const char *);

char *xstrdup(const char *x)
{
     register char *new=NULL;

     if (x != NULL) {
          register int i;

          for (i=0; x[i]; ++i);                       /* length of string */
          if ((new = malloc(++i)) == NULL) {
               i = 0;
          } else {
               while (i-- > 0) {
                    new[i] = x[i];
               }
          }
          x = NULL;
     }

     return new;                 /* return the duplicate or NULL on error */
}

int converse(pam_handle_t *pamh, int nargs, struct pam_message **message,
             struct pam_response **response)
{
	int retval;
	struct pam_conv *conv;

	retval = pam_get_item(pamh, PAM_CONV, (const void **) &conv);
	if (retval == PAM_SUCCESS) {
		retval = conv->conv(nargs,
				    (const struct pam_message **) message,
				    response, conv->appdata_ptr);
     	}
	return retval;
}

/* NOTE - currently ignores flags and arguments */
 
PAM_EXTERN
int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, 
			const char **argv)
{
	struct opie opie;
	const char *username;
	char *principal;
	char challenge[OPIE_CHALLENGE_MAX+1];
        struct pam_message msg[1], *pmsg[1];
        struct pam_response *resp;
	char *tok;
	int knownuser;
	int retval;

	retval = pam_get_user(pamh, &username, "login: ");
	if (retval != PAM_SUCCESS)
		return retval;
	
	principal = xstrdup(username);
	retval = opiechallenge(&opie, principal, challenge);
	if (principal != NULL) free(principal);
	if ((retval < 0) || (retval > 1)) {
		return PAM_AUTHINFO_UNAVAIL;
	}
	knownuser = (retval == 0) ? 1 : 0;

	/* Print out challenge */
	pmsg[0] = &msg[0];
	msg[0].msg_style = PAM_TEXT_INFO;
	msg[0].msg = challenge;
	resp = NULL;
	converse(pamh, 1, pmsg, &resp);
	if (resp != NULL) free(resp);

	/* Get the response */
	pmsg[0] = &msg[0];
	msg[0].msg_style = PAM_PROMPT_ECHO_OFF;
	msg[0].msg = "Response: ";
	resp = NULL;
	retval = converse(pamh, 1, pmsg, &resp);
	if (retval != PAM_SUCCESS)
		return retval;
	tok = xstrdup(resp[0].resp);
	if (resp != NULL) free(resp);
	pam_set_item(pamh, PAM_AUTHTOK, tok);

	//[RK]Jan/24/06 - OTP/LOCAL
	if (knownuser == 0) {
		return PAM_USER_UNKNOWN;
	}

	/* test the response and return accordingly */
	retval = opieverify(&opie, tok);
	if (tok != NULL) free(tok);
	if (retval == 0) {
		return PAM_SUCCESS;
	}
	return PAM_AUTH_ERR;
}

/* 
 * No credentials to set, so just return PAM_SUCCESS for now. 
 */ 

PAM_EXTERN 
int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
	return PAM_SUCCESS;	
}

/* static module data */
#ifdef PAM_STATIC
struct pam_module _pam_opie_modstruct = {
    "pam_opie",
    pam_sm_authenticate,
    pam_sm_setcred,
    NULL,
    NULL,
    NULL,
    NULL,
};
#endif
