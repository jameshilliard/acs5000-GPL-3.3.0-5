/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: Forms.h,v 1.1.1.1 2005/09/01 02:36:38 edson Exp $ */


#ifndef h_Forms
#define h_Forms

#include "Dmf.h"

/*
 * TODO: Document this.
 */
extern void dmfFormHandler(webs_t wp, char_t *path, char_t *query);
extern void dmfSetUrlOk(char_t *path, char_t *query);


/*
 * Utility function to call at the end of a form handler. 
 * If there are no errors (determined by calling dmfRetrieveError()), 
 * redirects the user's browser to the 'ok' page. Otherwise, 
 * redirects to the 'error' page.
 */
extern void dmfFormCleanup(webs_t wp, char_t* ok, char_t* error);


#endif
