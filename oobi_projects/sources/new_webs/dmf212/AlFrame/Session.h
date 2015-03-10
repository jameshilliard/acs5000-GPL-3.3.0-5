/* (c) 2001 Art & Logic, Inc. All Rights Reserved. */
/* $Id: Session.h,v 1.1.1.1 2005/09/01 02:36:38 edson Exp $ */



#ifndef h_Session
#define h_Session

#include "Dmf.h"
#include "Error.h"

/* Session.h -- Session management subsystem for the Art & Logic Device
 * Management Framework. Important features:
 *  - stores per-user error lists.
 *  - records time of first access
 *  - records time of last access
 *  - adds an URL handler to the WebServer to handle session management
 *
 *  Basic design:
 *  - The URL handler retrieves the session key from the webs_t variable
 *  - If there is a current session for that user, checks to see if the user
 *    has exceeded the maximum time allowed since last access. If so, it logs
 *    the user out and makes them authenticate again. Logging the user out
 *    deletes their session object.
 *  - If there's a setting for maximum total session duration, checks to see
 *    if the current session has gone on too long. If it has, logs the user
 *    out.
 *  - If there is no session object for the current user, creates a new one
 *    (logging them in)
 */


#ifdef qSession

typedef struct SessionTAG
{
   int firstAccess;
   int lastAccess;
   Errors* errorTable;
} Session;


/*
 * Open the session management subsystem. If maxInactivity is greater than
 * zero, any attempt to access the device more than this many seconds since
 * the last attempt will cause the user to be logged out (well...they'll be
 * required to log in again, which is the same thing.)
 * If the maxSession parameter is greater than zero, individual sessions will
 * be capped at that many seconds long.
 */
extern int dmfInitSession(UInt32 maxInactivity, UInt32 maxSession);

/*
 * Close the session management subsystem. Clean up all resources.
 */
extern void dmfCloseSession(void);


/*
 * Locate and return the session object for the current user, creating one if
 * necessary. If there's no usable key for the current user (not a protected
 * page, for example) returns the default session object.
 */

extern Session* dmfFindSession(webs_t wp);






#endif /* qSession */
#endif /* h_Session */
