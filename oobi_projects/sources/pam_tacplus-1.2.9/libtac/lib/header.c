/*
 * Copyright 1997-2000 by Pawel Krawczyk <kravietz@ceti.pl>
 *
 * See http://www.ceti.com.pl/~kravietz/progs/tacacs.html
 * for details.
 *
 * header.c  Create pre-filled header for TACACS+ request.
 */

#include <sys/socket.h>
#include "tacplus.h"
#include "libtac.h"
#include "xalloc.h"
#include "magic.h"
#include <netinet/in.h>   // [GY] 2005/aug/25  clean warnings

/* Miscellaneous variables that are global, because we need
 * store their values between different functions and connections.
 */
/* Session identifier. */
int session_id;
int seq_no;

/* Encryption flag. */
int tac_encryption;

/* Pointer to TACACS+ shared secret string. */
char *tac_secret = 0;

/* Returns pre-filled TACACS+ packet header of given type.
 * 1. you MUST fill th->datalength and th->version
 * 2. you MAY fill th->encryption
 * 3. you are responsible for freeing allocated header 
 * By default packet encryption is enabled. The version
 * field depends on the TACACS+ request type and thus it
 * cannot be predefined.
 */
HDR *_tac_req_header(u_char type, int start) {
 	HDR *th;

 	th=(HDR *) xcalloc(1, TAC_PLUS_HDR_SIZE);

 	/* preset some packet options in header */
 	th->type=type;
 	th->encryption=tac_encryption ? TAC_PLUS_ENCRYPTED : TAC_PLUS_CLEAR;
 
	if (start) {
	  seq_no = 1; /* always 1 for request */
 	  /* make session_id from pseudo-random number */
 	  session_id = magic();
	} else {
	  seq_no += 2;
	}

 	th->seq_no=seq_no;
 	th->session_id = htonl(session_id);

 	return(th);
}
