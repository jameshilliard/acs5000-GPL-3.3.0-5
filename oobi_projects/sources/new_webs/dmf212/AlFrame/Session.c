/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: Session.c,v 1.2 2006/01/10 23:51:50 erik Exp $ */


#include "webs.h"
#include "wsIntrn.h"
#include "Session.h"

#ifdef qSession
/*
 * URL handler to check session permissions.
 */
static int dmfSessionUrlHandler(webs_t wp, char_t* urlPrefix, char_t* webdir, 
 int arg, char_t* url, char_t* path, char_t* query);

/*
 * create a new session object
 */
static Session* dmfCreateSession(void);

/*
 * Add a session object to the table
 */
static void dmfAddSession(char_t* key, Session* session);

/* warning: `dmfRemoveSession' defined but not used
 * Remove a specified session object from the table
static void dmfRemoveSession(char_t* key);
 */

/*
 * destroy a session object
 */
static void dmfDestroySession(Session* session);

/*
 * Extract the correct session key from the webs_t structure.
 * The memory returned must be freed.
 */
static char_t* dmfGetSessionKey(webs_t wp);

/*
 * file scope variables...
 */

static int  sMaxInactivity = 0;
static int  sMaxSession = 0;
static int  sInitialized = 0;
static sym_fd_t   sSessions = -1;

static Session* sDefaultSession = NULL;

int dmfInitSession(UInt32 maxInactivity, UInt32 maxSession)
{
   if (!sInitialized)
   {
      sInitialized = 1;
      websUrlHandlerDefine(T(""), NULL, 0, dmfSessionUrlHandler, 0);
      sMaxInactivity = maxInactivity;
      sMaxSession = maxSession;
      sDefaultSession = dmfCreateSession();

      sSessions = symOpen(WEBS_SYM_INIT);

   }
   return 0;
}


extern void dmfCloseSession(void)
{
   if (sInitialized)
   {
      Session* session;
      sym_t* item;

      /*
       * The symbol table doesn't know how to call our dtor function, 
       * so we need to manually scan through the table and kill each one in
       * turn.
       */
      item = symFirst(sSessions);
      while (NULL != item)
      {
         session = (Session*) item->content.value.integer;
         dmfDestroySession(session);
         item = symNext(sSessions);
      }
      symClose(sSessions);
      sSessions = -1;

      if (NULL != sDefaultSession)
      {
         dmfDestroySession(sDefaultSession);
         sDefaultSession = NULL;
      }

      sInitialized = 0;
   }

}



int dmfSessionUrlHandler(webs_t wp, char_t* urlPrefix, char_t* webdir, 
 int arg, char_t* url, char_t* path, char_t* query)
{
   /* retval == 0 means continue processing the URL, retval == 1 means that
    * this handler has completely taken care of it, and no one else should get
    * a look at it. We will call websError(401) before returning 1.
    */
   int retval = 0;
   int now = time(0);
   Session* session = dmfFindSession(wp);
   /*
    * NOTE that the default session never times out. The default session
    * should only be used when looking at unprotected pages and using username
    * as session key. 
    */
   if (NULL != session && session != sDefaultSession)
   {
      if (sMaxInactivity > 0)
      {
         if ((now - session->lastAccess) > sMaxInactivity)
         {
            retval = 1;
         }

      }
      if (sMaxSession > 0)
      {
         if ((now - session->firstAccess) > sMaxSession)
         {
            retval = 1;
         }
      }
   }
   if (1 == retval)
   {
      websError(wp, 401, T("Your session has expired. Please log in again."));
   }
   else
   {
      session->lastAccess = now;
   }

   return retval;
}



extern Session* dmfFindSession(webs_t wp)
{
   Session* session = sDefaultSession;
   char_t* key = dmfGetSessionKey(wp);
   if (key != NULL)
   {
      sym_t* item = symLookup(sSessions, key);
      if (NULL == item)
      {
         /*
          * we need to create a new session object and add it to the table.
          */
         session = dmfCreateSession();
         if (NULL != session)
         {
            dmfAddSession(key, session);
         }
      }
      else
      {
         /* 
          * return the existing session object for this key.
          */
         session = (Session*) item->content.value.integer;
      }
      bfree(B_L, key);
   }

   return session;

}


Session* dmfCreateSession(void)
{
   Session* session = (Session*) balloc(B_L, sizeof(Session));
   if (session != NULL)
   {
      session->firstAccess = time(0);
      session->lastAccess = time(0);
      session->errorTable = dmfCreateErrors();
   }

   return session;
}

void dmfAddSession(char_t* key, Session* session)
{
   symEnter(sSessions, key, valueInteger((long)session), 0);
}

/* warning: `dmfRemoveSession' defined but not used
void dmfRemoveSession(char_t* key)
{
   sym_t* symbol = symLookup(sSessions, key);
   if (NULL != symbol)
   {
      Session* session = (Session*) symbol->content.value.integer;
      dmfDestroySession(session);
      symDelete(sSessions, key);
   }
} */

void dmfDestroySession(Session* session)
{
   if (NULL != session)
   {
      dmfDestroyErrors(session->errorTable);
      bfree(B_L, session);
   }
}

char_t* dmfGetSessionKey(webs_t wp)
{
   char_t* retval = NULL;
   if (NULL != wp && NULL != wp->ssid && 0 != gstrlen(wp->ssid))
   {
      retval = bstrdup(B_L, wp->ssid);
   }
   return retval;

}


#endif /* qSession */
