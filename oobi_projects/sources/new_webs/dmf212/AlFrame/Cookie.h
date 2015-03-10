/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: Cookie.h,v 1.1.1.1 2005/09/01 02:36:38 edson Exp $ */



#ifndef h_Cookie
#define h_Cookie

#ifdef __cplusplus
extern "C" {
#endif

#include "Dmf.h"

/*
 * Make sure that you look at 
 * http://wp.netscape.com/newsref/std/cookie_spec.html
 * for everything that you might want to know about cookies.
 */


extern int dmfEjGetCookieVal(int eid, webs_t wp, int argc, char_t **argv);
extern void dmfCookieHandler(webs_t wp, char_t *path, char_t *query);



#ifdef __cplusplus
}
#endif


#endif
