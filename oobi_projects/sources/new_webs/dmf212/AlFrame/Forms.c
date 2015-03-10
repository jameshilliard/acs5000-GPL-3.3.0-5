/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: Forms.c,v 1.2 2006/01/10 23:51:49 erik Exp $ */


#include "wsIntrn.h"

#include "Forms.h"

#include "Access.h"
#include "Error.h"
#include "GetSet.h"
#include "Locals.h"
#include "Parse.h"

/*
 * define some reserved key names in forms. We don't attempt to perform a
 * 'set' on anything in this list.
 */


#define kUrlOk       T("urlOk")
#define kUrlError    T("urlError")
#define kUrlCancel   T("urlCancel")
#define kCommand     T("command")
#define kIndex       T("index")
#define kRequest     T("request")
#define kSubmit      T("submit")

static char_t* sReservedWords[] =
{
   kUrlOk,  
   kUrlError,
   kUrlCancel,
   kCommand, 
   kIndex,  
   kRequest,
   kSubmit
};
   

static int dmfIsReservedKey(char_t* key)
{
   int i = 0; 
   int count = sizeof(sReservedWords) / sizeof(sReservedWords[0]);

   for (i = 0; i < count; ++i)
   {
      if (0 == gstricmp(key, sReservedWords[i]))
      {
         return 1;
      }
   }
   return 0;
}

#define MAX_URL_SIZE 512		// magic number!!!

static void dmfAppendUrlQuery(char_t* url, const char_t* query)
{
   char_t* queryStart = strchr(url, '?');
   if (NULL == queryStart)
      strcat(url, "?");
   else
      strcat(url, "&");

   if ((strlen(url) + strlen(query)) >= MAX_URL_SIZE)
      return;

   strcat(url, query);
}

static char_t urlOkFinal[MAX_URL_SIZE+1];
static char_t urlErrorFinal[MAX_URL_SIZE+1];

extern void dmfSetUrlOk(char_t *path, char_t *query)
{
   strncpy(urlOkFinal, path, MAX_URL_SIZE);
   if ((strlen(path) + strlen(query)) < MAX_URL_SIZE)
     strcat(urlOkFinal, query);
}

extern void dmfFormHandler(webs_t wp, char_t *path, char_t *query)
{
   char_t* key = NULL;
   char_t* val = NULL;
   char_t* urlOk = NULL;
   char_t* urlError = NULL;
   char_t* urlCancel = NULL;
   char_t* command = NULL;
   char_t* index = NULL;
   /*char_t* error = NULL; ../dmf212/AlFrame/Forms.c:91: warning: unused variable `error'*/
   char_t* request = NULL;
   char_t  *p, *t;
   int requestInt = 0;
   int formRequest = dmfIncrementRequestNumber();
   int result = kDmfSuccess;
   int accessLevel = dmfUserAccessLevel(wp->userName);
   
   sym_fd_t keyTable = symOpen(64);
   sym_t* symbol = NULL;

   /*int errorCount = 0; ../dmf212/AlFrame/Forms.c:102: warning: unused variable `errorCount'*/
   /*int i = 0; ../dmf212/AlFrame/Forms.c:103: warning: unused variable `i'*/
   CommandStruct cmd;

   /*
    * first, we'll look to see if some reserved keys are present.
    */

   urlOk = websGetVar(wp, kUrlOk, "");
   strncpy(urlOkFinal, urlOk, sizeof(urlOkFinal));
   urlError = websGetVar(wp, kUrlError, "");
   strncpy(urlErrorFinal, urlError, sizeof(urlErrorFinal));
   urlCancel = websGetVar(wp, kUrlCancel, NULL);
   command = websGetVar(wp, kCommand, NULL);
   index = websGetVar(wp, kIndex, NULL);
   request = websGetVar(wp, kRequest, NULL);

   if (*urlOk && (p = strstr(urlOk, "SSID=")) != NULL) {
      p += 5;
      if ((t = strchr(p, '&')) != NULL) *t = 0;
      bfreeSafe(B_L, wp->ssid);
      wp->ssid = bstrdup(B_L, p);
      if (t) *t = '&';
   }

   if (NULL != request)
   {
      requestInt = gatoi(request);
   }

   /*
    * clear our error string before we start processing anything.
    */
   dmfClearErrors(wp);
   
   if (NULL != command)
   {
      /*
       * The ASP page specified a 'command' to handle this form instead 
       * of letting this generic code handle it. We invoke the specified
       * command procedure (looking it up in the CmdFunctions list)
       * and pass that function the entire query string that we got.
       *
       * We ignore the return value from dmfCommand -- if the command function
       * encounters an error, it should call 'dmfSetError'
       */
      cmd.command = command;
      cmd.queryString = query;
      cmd.wp = wp;
      dmfCommand(&cmd);
   }
   else
   {
      /*
       * parse down the query string, attempting to set each parameter. 
       * key1=val1&key2=val2&key3=val3
       */
      while (dmfParseQueryString(&query, &key, &val))
      {
         key = strlower(key);
         /*
          * If the key has an underscore in it, it may be encoding an index
          * using the format keyName_indexNum, or it may be attempting to set
          * a 'local' variable if it's in the format _keyName. Look for the
          * underscore, then test to see if it's at the beginning of the
          * string to decide what to do with this key...
          */
         index = gstrchr(key, '_');
         if (NULL != index && index != key)
         {
            *index = '\0';
            ++index;
         }
         else
         {
            index = NULL;
         }

         if (!dmfIsReservedKey(key))
         {
            result = dmfSetParam(key, index, val, requestInt, accessLevel);
            if (kDmfSuccess == result)
            {
               /*
                * remember that we've tried to set this key.
                *  
                */
               symEnter(keyTable, key, valueInteger(1), 0);
            }
            else
            {
               dmfAddError(wp, result, key, index, val);
               /* if we encountered an old data error, we should just stop, 
                * because we need to abandon the entire form submission.
                */
               if (kDmfOldData == result)
               {
                  break;
               }
            }
         }
      }
   }
   /*
    * ...now walk through the table of keys that we've set and call the
    * corresponding commit function (assuming that there were no errors...).
    */
   if (0 == dmfGetErrorCount(wp))
   {
      char urlQuery[255]; // magic number!!!
      symbol = symFirst(keyTable);
      while (NULL != symbol)
      {
         urlQuery[0] = 0;
         result = dmfCommitParam(symbol->name.value.string, formRequest, urlQuery);
         if (kDmfSuccess != result && kDmfLastError != result)
         {
            dmfAddError(wp, result, symbol->name.value.string, NULL, NULL);     
            if (0 != urlQuery[0])
               dmfAppendUrlQuery(urlErrorFinal, urlQuery);
         }
         else if (0 != urlQuery[0])
            dmfAppendUrlQuery(urlOkFinal, urlQuery);
         symbol = symNext(keyTable);
      }
   }

   /* 
    * If any of the parameters that were set were local variables (aka server
    * variables), we persist them to disk. If no locals were modified, this
    * call is a no-op.
    */
   dmfPersistLocals();

   symClose(keyTable);
   dmfFormCleanup(wp, urlOkFinal, urlErrorFinal);
} 





extern void dmfFormCleanup(webs_t wp, char_t* ok, char_t* error)
{
   int errorCount;
   char_t index[32];
   int i;
   char_t* errorString;

   errorCount = dmfGetErrorCount(wp);
   if (errorCount > 0)
   {
      if (NULL != error)
      {
         /*
          * remember the page that was supposed to handle the errors...
          */
         dmfSetErrorPage(wp, error);

         /*
          * send the user's browser to our error handling URL.
          */

         websRedirect(wp, error);
      }
      else
      {
         /*
          * There's no specific error page provided -- just 
          * print out the current list of errors in a rudimentary format.
          */
         websHeader(wp);
         for (i = 0; i < errorCount; ++i)
         {
            fmtStatic(index, 32, T("%d"), i);
            if(kDmfSuccess == dmfGetErrorString(wp, index, 0, NULL, 
             &errorString))
            {
               websWrite(wp, T("ERROR %d: %s<br>\n"), i+1, errorString);
            }

            bfreeSafe(B_L, errorString);
            errorString = NULL;
         }
         dmfClearErrors(wp);
         websFooter(wp);
         websDone(wp, 200);
      }
   }
   else
   {
      if (NULL != ok)
      {
         websRedirect(wp, ok);
      }
      else
      {
         /* 204 = no content */
         websDone(wp, 204);
      }
   }
}

