/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: Cookie.c,v 1.2 2006/01/10 23:51:49 erik Exp $ */


#include "webs.h"
#include "wsIntrn.h"

#include "Cookie.h"
#include "Parse.h"




extern int dmfEjGetCookieVal(int eid, webs_t wp, int argc, char_t **argv)
{
   char_t* name;
   int num = 0;
   char_t* key;
   char_t* val;
   char_t* cookies;
   char_t* start;
   int hitCount = 0;
   int retval = kDmfSuccess;
   int found = 0;
   /* ../dmf212/AlFrame/Cookie.c:25: warning: unused variable `argCount'
   int argCount = ejArgs(argc, argv, T("%s %d"), &name, &num);*/

   if (gstrlen(name) > 0)
   {
      if (num > 0)
      {
         /*
          * change the default failure mode...
          */
         retval = kDmfIndexRange;
      }
      /*
       * start pulling the cookies apart...
       */
      cookies = bstrdup(B_L, wp->cookie);
      start = cookies;
      while (!found)
      {
         if (dmfParseKeyValueString(&start, &key, &val, '=', ';'))
         {
            while (isspace(*key))
            {
               /*
                * move past any spaces...
                */
               ++key;
            }
            if (0 == gstricmp(key, name))
            {
               /*
                * we've found what they're looking for...
                */
               if (num == hitCount++)
               {
                  found = TRUE;
               }
            }
         }
         else
         {
            /*
             * we've run out of things to look for.
             */
            break;
         }
      }
      if (found)
      {
         ejSetResult(eid, val);
      }
      else
      {
         ejSetResult(eid, T(""));
         retval = kDmfParameterName;
      }

      bfree(B_L, cookies);
   }
   else
   {
      /*
       * no cookie name specified, return the entire set of cookies
       */
      ejSetResult(eid, wp->cookie);
   }

   dmfSetErrorCode(eid, retval);
   return kDmfSuccess;
}



/*
 * The 'CookieStruct' data structure is used to first accumulate a 
 * linked list of cookie values (during parsing) and then
 * to write the desired settings out to the user's browser.
 */

typedef struct CookieStructTAG
{
   char_t* key;
   char_t* value;
   char_t* path;
   char_t* domain;
   char_t* expires;
   int secure;
   struct CookieStructTAG* next;
} CookieStruct;

typedef struct CookieListTAG
{
   CookieStruct* head;
   CookieStruct* tail;
} CookieList;



/*
 * Create and initialize a CookieStruct. This should be the only way that 
 * a CookieStruct gets created.
 */

static CookieStruct* CreateCookieStruct(void)
{
   /*
    * Allocate and NULL out a cookie structure.
    */
   CookieStruct* cookie = (CookieStruct*) balloc(B_L, sizeof(CookieStruct));
   if (NULL != cookie)
   {
      cookie->key = NULL;
      cookie->value = NULL;
      cookie->path = NULL;
      cookie->domain = NULL;
      cookie->expires = NULL;
      cookie->secure = 0;
      cookie->next = NULL;
   }

   return cookie;
}

/*
 * free all of the memory used by this CookieStruct.
 */
static void DestroyCookieStruct(CookieStruct* cookie)
{
   /*
    * frees all the memory associated with this cookie, 
    * including the cookie struct itself
    */
   if (NULL != cookie)
   {
      bfreeSafe(B_L, cookie->key);
      bfreeSafe(B_L, cookie->value);
      bfreeSafe(B_L, cookie->path);
      bfreeSafe(B_L, cookie->domain);
      bfreeSafe(B_L, cookie->expires);
      bfree(B_L, cookie);
   }

}


/*
 * Create an empty cookie list.
 */
static CookieList* CreateCookieList(void)
{
   CookieList* retval = (CookieList*) balloc(B_L, sizeof(CookieList));
   if (NULL != retval)
   {
      retval->head = retval->tail = NULL;
   }
   return retval;

}

/*
 * Given a CookieList, this function will walk the linked list
 * that follows, freeing memory for each cookie on the list.
 */
static void DeleteCookieList(CookieList* list)
{
   /*
    * delete a singly-linked list of CookieStructs, starting from the 
    * specified list head.
    */
   CookieStruct* p = list->head;
   CookieStruct* next = NULL;
   while (NULL != p)
   {
      next = p->next;
      DestroyCookieStruct(p);
      p = next;
   }

   bfree(B_L, list);
}

/*
 * A cookie must have a non-NULL key and value for us to attempt 
 * to set it in the user's browser.
 */
static int CookieValid(CookieStruct* cookie)
{
   /*
    * we will not set a cookie unless it has a valid key/value pair.
    * Add any other useful validity tests here 
    * illegal chars?
    * cookie path?
    */
   return (NULL != cookie->key && NULL != cookie->value);
}


/*
 * write the specified cookie into the header stream. Note that this 
 * function will only have the intended effect if it's called while the 
 * http headers are being written!
 */

static void SetCookie(webs_t wp, CookieStruct* cookie)
{
   if (CookieValid(cookie))
   {
      websWrite(wp, T("Set-Cookie: %s=%s; "), cookie->key, cookie->value);
      websWrite(wp, T("path=%s; "), NULL == cookie->path ? T("/") : cookie->path );
      if (NULL != cookie->domain)
      {
         websWrite(wp, T("domain=%s; "), cookie->domain);
      }
      if (NULL != cookie->expires)
      {
         /*
          * if the expiration is NULL, this cookie will expire at the 
          * end of this session
          */
         websWrite(wp, T("expires=%s; "), cookie->expires);
      }
      if (0 != cookie->secure)
      {
         websWrite(wp, T("secure"));
      }
      websWrite(wp, T("\n"));
   }

}

/*
 * walks the list of CookieStructs pointed to by 'list' and sets each of them
 * in the user's browser.
 */
static void SetCookieList(webs_t wp, CookieList* list)
{
   CookieStruct* p = list->head;

   while(NULL != p)
   {
      SetCookie(wp, p);
      p = p->next;
   }
}

/*
 * attaches the CookieStruct 'newCookie' to the end of the list pointed
 * to by 'list'. The 'newCookie' becomes the new tail of the list.
 * Both the head and tail may be changed by this function.
 */
static void AppendCookie(CookieList* list, CookieStruct* newCookie)
{

   if (NULL != newCookie)
   {
      if (CookieValid(newCookie))
      {
         if (NULL == list->head)
         {
            /*
             * case 1: empty list.
             */
            list->head = list->tail = newCookie;
         }
         else
         {
            /*
             * case 2: adding on to the end.
             */
            list->tail->next = newCookie;
            list->tail = newCookie;
         }
      }
      else
      {
         /*
          * the cookie isn't valid, so get rid of it.
          */
         DestroyCookieStruct(newCookie);
      }
   }
}

extern void dmfCookieCommandHandler(CommandStruct* cmd)
{

   char_t* key = NULL;
   char_t* value = NULL;
   /*char_t* cookiePath;
   char_t* expires;*/
   char_t* redirect = NULL;
   CookieList* list = CreateCookieList();
   CookieStruct* current = NULL;



   /*
    * first, we parse through the query string and create a linked list 
    * of CookieStruct structures. 
    */

   while (1)
   {
      if (dmfParseQueryString(&(cmd->queryString), &key, &value))
      {
         if (0 == gstrcmp(T("cookie"), key))
         {
            if (NULL != current)
            {
               /*
                * the last cookie is now finished -- add it to the list & create a 
                * new current cookie struct to fill up!
                */
               AppendCookie(list, current);
               current = NULL;
            }
            /*
             * create a new cookie struct to fill...
             */
            current = CreateCookieStruct();
            current->key = bstrdup(B_L, value);
         }
         else if (0 == gstrcmp(T("val"), key))
         {
            bfreeSafe(B_L, current->value);
            current->value = bstrdup(B_L, value);
         }
         else if (0 == gstrcmp(T("expires"), key))
         {
            bfreeSafe(B_L, current->expires);
            current->expires = bstrdup(B_L, value);
         }
         else if (0 == gstrcmp(T("path"), key))
         {
            bfreeSafe(B_L, current->path);
            current->path = bstrdup(B_L, value);
         }
         else if (0 == gstrcmp(T("domain"), key))
         {
            bfreeSafe(B_L, current->domain);
            current->domain = bstrdup(B_L, value);
         }
         else if (0 == gstrcmp(T("redirect"), key))
         {
            bfreeSafe(B_L, redirect);
            redirect = bstrdup(B_L, value);
         }
         else if (0 == gstrcmp(T("secure"), key))
         {
            current->secure = 1;
         }
         else
         {
            /*
             * there's an error -- ignore for now...
             */
         }
      }
      else
      {
         /*
          * we're finished!
          */
         AppendCookie(list, current);
         current = NULL;
         break;
      }
   }

   websWrite(cmd->wp, T("HTTP/1.0 200 OK\n"));

   /*
   * By license terms the following line of code must not be modified
   */
   websWrite(cmd->wp, T("Server: GoAhead-Webs\r\n"));
   websWrite(cmd->wp, T("Pragma: no-cache\r\n"));
   websWrite(cmd->wp, T("Cache-control: no-cache\r\n"));
   websWrite(cmd->wp, T("Content-Type: text/html\r\n"));
   /*
    * scan through the list of cookie structs and write them all...
    */
   SetCookieList(cmd->wp, list);

   /*
    * resume writing the headers/page
    */
   websWrite(cmd->wp, T("\r\n"));

   websWrite(cmd->wp, T("<html><head>\n"));
   if (NULL == redirect)
   {
      /*
       * redirect to the home page
       */
      redirect = bstrdup(B_L, websGetVar(cmd->wp, T("SERVER_URL"), T("")));
   }

   websWrite(cmd->wp, T("<meta http-equiv=\"refresh\" content=\"0;URL=%s\">\n"), redirect);
   bfree(B_L, redirect);

   websWrite(cmd->wp, T("</head></html>\n"));

   websDone(cmd->wp, 200);

   /*
    * delete the list of cookie structs
    */
   DeleteCookieList(list);
}




