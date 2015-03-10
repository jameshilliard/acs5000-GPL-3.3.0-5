/*
 * $Id: xsh.c,v 1.1.1.1 2005/09/01 02:36:37 edson Exp $
 */

/* Andrew Morgan (morgan@kernel.org) -- an example application
 * that invokes a shell, based on blank.c */

#include <stdio.h>
#include <stdlib.h>

#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <security/pam_appdata.h>


/* ------ some local (static) functions ------- */

static void bail_out(pam_handle_t *pamh,int really, int code, const char *fn)
{
     fprintf(stderr,"==> called %s()\n  got: `%s'\n", fn,
	     pam_strerror(pamh,code));
     if (really && code)
	  exit (1);
}

/* ------ some static data objects ------- */

static struct pam_appl_data my_appl_data = {
    id : 0x1234fedc,
    sock : 1,
    ppp : 0,
    caller : 0,
    port : 0,
    flag : 1
};

static struct pam_conv conv = {
     misc_conv,
     NULL
};

/* ------- the application itself -------- */

int main(int argc, char **argv)
{
     pam_handle_t *pamh=NULL;
     const char *username=NULL;
     const char *service=NULL;
     const char *command="/bin/sh";
     char *aux, buf[256];
     int retcode, opt;
	extern char *optarg;
	extern int optind;

     /* did the user call with a username as an argument ?
      * did they also */

    while ((opt = getopt(argc, argv, "pPc:C:s:S:u:U")) != EOF) {
        switch (opt) {
		case 'c':
		case 'C':
			command = optarg;
			break;
		case 'p':
		case 'P':
			conv.appdata_ptr = &my_appl_data;
			break;
        case 'u':
        case 'U':
			username = optarg;
			break;
        case 's':
        case 'S':
	 		service = optarg;
			break;
		default:
			printf("USAGE : %s [-p] [-c command] [-s service] [-u username]\n", argv[0]);
			return(0);
			break;
		}
     }

     /* initialize the Linux-PAM library */
     retcode = pam_start("xsh", username, &conv, &pamh);
     bail_out(pamh,1,retcode,"pam_start");

     if (service && *service) {
        retcode = pam_set_item(pamh, PAM_SERVICE, service);
        bail_out(pamh,0,retcode,"pam_set_item PAM_SERVICE");
     }
     /* set PAM_TTY to make pam_security working */
     aux = ttyname(0);
     retcode = pam_set_item(pamh, PAM_TTY, aux);
     bail_out(pamh,0,retcode,"pam_set_item PAM_TTY");

     /* set PAM_RHOST  */
     gethostname(buf, sizeof(buf));
     retcode = pam_set_item(pamh, PAM_RHOST, buf);
     bail_out(pamh,0,retcode,"pam_set_item PAM_RHOST");

     /* to avoid using goto we abuse a loop here */
     for (;;) {
	  /* authenticate the user --- `0' here, could have been PAM_SILENT
	   *	| PAM_DISALLOW_NULL_AUTHTOK */

	  retcode = pam_authenticate(pamh, 0);
	  bail_out(pamh,0,retcode,"pam_authenticate");

	  /* has the user proved themself valid? */
	  if (retcode != PAM_SUCCESS) {
	       fprintf(stderr,"%s: invalid request\n",argv[0]);
	       break;
	  }

	  /* the user is valid, but should they have access at this
	     time? */

	  retcode = pam_acct_mgmt(pamh, 0); /* `0' could be as above */
	  bail_out(pamh,0,retcode,"pam_acct_mgmt");

	  if (retcode == PAM_NEW_AUTHTOK_REQD) {
	       fprintf(stderr,"Application must request new password...\n");
	       retcode = pam_chauthtok(pamh,PAM_CHANGE_EXPIRED_AUTHTOK);
	       bail_out(pamh,0,retcode,"pam_chauthtok");
	  }

	  if (retcode != PAM_SUCCESS) {
	       fprintf(stderr,"%s: invalid request\n",argv[0]);
	       break;
	  }

	  /* `0' could be as above */
	  retcode = pam_setcred(pamh, PAM_ESTABLISH_CRED);
	  bail_out(pamh,0,retcode,"pam_setcred");

	  if (retcode != PAM_SUCCESS) {
	       fprintf(stderr,"%s: problem setting user credentials\n"
		       ,argv[0]);
	       break;
	  }

	  /* open a session for the user --- `0' could be PAM_SILENT */
	  retcode = pam_open_session(pamh,0);
	  bail_out(pamh,0,retcode,"pam_open_session");
	  if (retcode != PAM_SUCCESS) {
	       fprintf(stderr,"%s: problem opening a session\n",argv[0]);
	       break;
	  }

	  pam_get_item(pamh, PAM_USER, (const void **) &username);
	  fprintf(stderr,
		  "The user [%s] has been authenticated and `logged in'\n",
		  username);

	  /* this is always a really bad thing for security! */
	  system(command);

	  /* close a session for the user --- `0' could be PAM_SILENT
	   * it is possible that this pam_close_call is in another program..
	   */

	  retcode = pam_close_session(pamh,0);
	  bail_out(pamh,0,retcode,"pam_close_session");
	  if (retcode != PAM_SUCCESS) {
	       fprintf(stderr,"%s: problem closing a session\n",argv[0]);
	       break;
	  }
	  
	  /* `0' could be as above */
	  retcode = pam_setcred(pamh, PAM_DELETE_CRED);
	  bail_out(pamh,0,retcode,"pam_setcred");
	  if (retcode != PAM_SUCCESS) {
	       fprintf(stderr,"%s: problem deleting user credentials\n"
		       ,argv[0]);
	       break;
	  }

	  break;                      /* don't go on for ever! */
     }

     /* close the Linux-PAM library */
     retcode = pam_end(pamh, PAM_SUCCESS);
     pamh = NULL;
     bail_out(pamh,1,retcode,"pam_end");

     exit(0);
}
