//* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */

#include "wsIntrn.h"

#include "Error.h"
#include "GetSet.h"
#include "Parse.h"
#include "Session.h"
/*
 * defining qErrorStringTable here lets us include the error code header file
 * again, but this time we build an array of strings that can be indexed by
 * the integer error codes used in the framework.
 */
#define qErrorStringTable
#include "ErrorCodes.h"
#undef qErrorStringTable



/*
 * Errors -- There are two different kinds of errors that we're interested in
 * tracking. The first, simplest kind is the last error that happened on the
 * ASP request that we are currently processing. For example, we might have
 * code that tries to do something as part of a simple ASP request and
 * branches differently depending on the error code that's set by the request
 * -- something like:
 * <% val = get("someParam", 0);
 *    if ("NotIndexed" == getError() )
 *    {
 *       // do something useful here...
 *    }
 *    else
 *    {
 *       // this parameter is indexed, proceed accordingly
 *    }
 *
 * Alternatively, when we are processing a form submission, we may encounter
 * several errors, which we would like to have hang around until the next
 * request from that user -- if values are out of range, etc., we want to be
 * able to remember that until the error page that was specified in the form
 * gets served. So, we also maintain 1 (or more, I hope) symbol tables that
 * map integer numbers to an error string.
 */

/* DMF errors */
static int sErrorCode = kDmfSuccess;
/* Application errors (see dmfSetAppErrors()) */
typedef struct
{
   ErrorStrings* strings;
   int lastError;
} AppErrors;
static AppErrors sAppErrors = {NULL, 0};
/* Session */
#ifndef qSession
static Errors sErrors = {0};
#endif


/* Functions */

static Errors* dmfGetErrorStruct(webs_t wp)
{
#ifdef qSession
   Session* session = dmfFindSession(wp);
   a_assert(NULL != session);
   return session->errorTable;
#else
   return &sErrors;
#endif

}


extern Errors* dmfCreateErrors(void)
{
   Errors* err = (Errors*) balloc(B_L, sizeof(Errors));
   if (NULL != err)
   {
      size_t i;

      err->errorCapacity = kStartingErrorCapacity;
      err->errorTable = (char_t**) balloc(B_L, sizeof(char_t*) * err->errorCapacity);
      err->errorCount = 0;
      err->errorPage = NULL;

      for (i = 0; i < err->errorCapacity; ++i)
      {
         err->errorTable[i] = NULL;
      }
   }

   return err;

}


extern void dmfDestroyErrors(Errors* err)
{
   size_t i = 0;
   if (NULL != err)
   {
      for (i = 0; i < err->errorCapacity; ++i)
      {
         bfreeSafe(B_L, err->errorTable[i]);
         err->errorTable[i] = NULL;
      }
      bfreeSafe(B_L, err->errorPage);
      bfree(B_L, err);
   }

}

static int dmfResizeErrors(Errors* err)
{
   int retval = kDmfInternalError;
   if (NULL != err)
   {
      /*
       * grow our capacity by 150%
       */
      size_t newCapacity = (err->errorCapacity * 3) / 2;
      char_t** newBuf = (char_t**) brealloc(B_L, err->errorTable, 
       sizeof(char_t*) * newCapacity);
      if (newBuf != NULL)
      {
         err->errorTable = newBuf;
         err->errorCapacity = newCapacity;
         retval = kDmfSuccess;
      }
   }

   return retval;
}

extern int checkWebErrors(webs_t wp)
{
	Errors* err = dmfGetErrorStruct(wp);
	size_t i = 0; 
	char_t *p;
	int code;
	ErrorStrings* errorStr=NULL;

	if (NULL == err) return 0;

	for (i = 0; i < err->errorCount; ++i) {
		p = strstr(err->errorTable[i], "code=");
		if (p) {
			p += 5;
			code = atoi(p);
     		if (code > kDmfSuccess && code < kDmfLastError) {
         		errorStr = kDmfCodesStrings;
      		} else if (code >= kDmfLastError && code < sAppErrors.lastError) {
         		errorStr = sAppErrors.strings;
				code -= kDmfLastError;
			}
			if (errorStr && errorStr[code].loginReq) return(1);
		}
	}
	return(0);
}


extern void dmfClearErrors(webs_t wp)
{
   Errors* err = dmfGetErrorStruct(wp);
   if (NULL != err)
   {
      size_t i = 0; 
      for (i = 0; i < err->errorCount; ++i)
      {
         bfreeSafe(B_L, err->errorTable[i]);
         err->errorTable[i] = NULL;
      }
      err->errorCount = 0;
      bfreeSafe(B_L, err->errorPage);
      err->errorPage = NULL;
   }
}

static int dmfCheckErrorPage(webs_t wp)
{
   /* 
    * return true if the current page (as indicated by wp->path) is the page
    * that the current user is using to handle the error list. Else, return
    * false. If there is no current error page (err->errorPage == NULL),
    * return true.
    */
   int retval = 0;
   Errors* err = dmfGetErrorStruct(wp);
   if (NULL != err)
   {
      if ((NULL == err->errorPage) || (0 == gstrcmp(err->errorPage, wp->path)))
      {
         retval = 1;
      }
   }
   return retval;
}



extern int dmfEjGetLastErrorCode(int eid, webs_t wp, int argc, char_t** argv)
{
   char_t code[32];
   fmtStatic(code, 32, T("%d"), sErrorCode);
   ejSetResult(eid, code);
   return 0;
}

extern int dmfEjGetErrorNameByIndex(int eid, webs_t wp, int argc, char_t** argv)
{
   char_t* index = NULL;
   char_t* errorName = NULL;
   int retval = kDmfArgCount;

   /* Parse argv */
   int num = ejArgs(argc, argv, T("%s"), &index);
   /* Confirm our role */
   if (0 == dmfCheckErrorPage(wp))
      retval = kDmfIndexRange; /* not the correct error-handling page */
   /* Process request */
   else if (num == 1)
   {
      /* Find error in dynamic error table */
      char_t *es;
      retval = dmfGetErrorString(wp, index, 1, NULL, &es);
      if (kDmfSuccess == retval)
      {
         /* Extract error code */
         char_t *k, *code, *ex = es;
         dmfParseKeyValueString(&ex, &k, &code, '=', '&');
         a_assert(0 != gstrlen(code));
         /* Get error name by code */
         errorName = dmfGetErrorName(gatoi(code));

         /* Clean up */
         ejSetResult(eid, errorName);
         bfreeSafe(B_L, es);
      }
   }
   return 0;
}

extern int dmfEjResetError(int eid, webs_t wp, int argc, char_t** argv)
{
	dmfClearErrors(wp);
	return 0;
}

extern int dmfEjGetErrorCount(int eid, webs_t wp, int argc, char_t** argv)
{
   Errors* err = dmfGetErrorStruct(wp);
   char_t count[32];
   int countVal = 0;
   
   if (NULL != err)
   {
      /*
       * Make sure that they are asking about this from the correct page...
       */
      if (dmfCheckErrorPage(wp))
      {
         /* return the actual number of errors instead of pretending that
          * there are no errors...
          */
         countVal = err->errorCount;
      }
      
      fmtStatic(count, 32, T("%d"), countVal);
      ejSetResult(eid, count);
   }
   return 0;

}

extern int dmfEjGetErrorMessageByIndex(int eid, webs_t wp, int argc, char_t** argv)
{
   char_t* index = NULL; 
   char_t* format = NULL;
   char_t* result = NULL;
   int retval = kDmfArgCount;
   int notFormatted = 0;

   int num = ejArgs(argc, argv, T("%s %s"), &index, &format);
   /*
    * If we're not calling this function from the 'correct' error handling
    * page, we return an index error (as if we've run off the end of the list)
    */
   if (0 == dmfCheckErrorPage(wp))
   {
      retval = kDmfIndexRange;
   }
   else if (num >= 1)
   {
      /* the index is okay at least -- let's see if we have a 
       * format parameter
       */
      if (2 == num)
      {
         if (0 == gstrcmp(T("1"), format))
         {
            notFormatted = 1;
         }
      }
      retval = dmfGetErrorString(wp, index, notFormatted, NULL, &result);
      if (kDmfSuccess == retval)
      {
         ejSetResult(eid, result);
         bfreeSafe(B_L, result);
      }
   }
   dmfSetErrorCode(eid, retval);
   return 0;
}


/*
 * When we store error information into the error table, we format it into a
 * keyword/value pair string, delimited like a query string -- 
 * key=value&key2=value2
 * -- this lets us use our existing parsing code to get the useful bits out
 * again when someone asks for them. We also provide an avenue to get the
 * error string in this format so that more ambitious client-side javascript
 * programmers can parse the error info themself and display it as best suits
 * the project.
 */
static char_t* dmfMakeErrorString(ErrorStrings* errorStrings, int i, int code,
 char_t* key, char_t* index, char_t* val)
{
   char_t* errStr = NULL;
   fmtAlloc(&errStr, BUF_MAX,T("code=%d&error=%s&parm1=%s&parm2=%s&parm3=%s"), 
    code, errorStrings[i].errorName, key, index, val);
   return errStr;
}


extern char_t* dmfGetErrorName(int code)
{
   if (code >= kDmfSuccess && code < kDmfLastError)
   {
      return kDmfCodesStrings[code].errorName;
   }
   else if (code >= kDmfLastError && code < sAppErrors.lastError)
   {
      return sAppErrors.strings[code - kDmfLastError].errorName;
   }
   else
   {
      return T("");
   }
}


/*
 * format the error parameters using the format string that's specified in the
 * table of error strings (see ErrorCodes.h)
 * Allocates memory (returned through the errStr parameter) -- caller's
 * reponsibility to free that memory.
 */
extern char_t* dmfFormatErrorString(int code, char_t* parm1, char_t* parm2, 
 char_t* parm3)
{
   char_t* fmt = NULL;
   char_t* errStr = NULL;
   int index = -1;
   ErrorStrings* strings = NULL;
   if (code >= kDmfSuccess && code < kDmfLastError)
   {
      index = code;
      strings = kDmfCodesStrings;
   }
   else if (code >= kDmfLastError && code < sAppErrors.lastError)
   {
      index = code - kDmfLastError;
      strings = sAppErrors.strings;
   }
   if (-1 != index)
   {
      fmt = strings[index].errorFormat;

      switch (strings[index].replaceType)
      {
         /*
          * format strings that require no replacement:
          */
         case kReplaceNone:
         errStr = bstrdup(B_L, fmt);
         break;

         /*
          * format strings that only replace the 'parm1'
          */
         case kParm1:
         fmtAlloc(&errStr, BUF_MAX, fmt, parm1);
         break;

         /*
          * format strings that replace parm1 and parm2.
          */
         case (kParm1 | kParm2):
         fmtAlloc(&errStr, BUF_MAX, fmt, parm1, parm2);
         break;

         /*
          * format strings that replace parm1 and parm3
          */
         case (kParm1 | kParm3):
         fmtAlloc(&errStr, BUF_MAX, fmt, parm1, parm3);
         break;
          

         /*
          * flag an error in any case where we've missed adding a code to this 
          * switch statement
          */
         default:
         fmtAlloc(&errStr, BUF_MAX,
          T("ERROR -- missing case statement for error code %d (%s)in dmfFormatErrorString()"), 
          code, strings[index].errorName);
         break;

      }
   }
   else
   {
      fmtAlloc(&errStr, BUF_MAX, 
       T("ERROR -- invalid error code %d in dmfFormatErrorString()"), code );
   }
   return errStr;
}



/*
 * Using the provided parameters, add a new error string to the end of the
 * list held inside the provided errorList object.
 */
extern void dmfAddError(webs_t wp, int code, char_t* parm1, char_t* parm2, 
 char_t* parm3)
{
   /*
    * When we add support for per-user error tracking, we should be able to
    * get any user info we need out of the webs_t struct that we get passed
    * here. For the moment, we just use the static per-server error struct.
    */
   Errors* errorList = dmfGetErrorStruct(wp);
   //char_t* number = NULL; warning: unused variable `number'
   //char_t* errStr = NULL; warning: unused variable `errStr'

   if (NULL != errorList)
   {
      /* 
       * if we need to grow the error table, do so before trying to 
       * add the new error in.
       */
      if (errorList->errorCount == errorList->errorCapacity)
      {
         if (kDmfSuccess != dmfResizeErrors(errorList))
         {
            return;
         }
      }
      if (code > kDmfSuccess && code < kDmfLastError)
      {
         errorList->errorTable[errorList->errorCount] = 
          dmfMakeErrorString(kDmfCodesStrings, code, code, parm1, parm2, parm3);
         errorList->errorCount++;
      }
      else if (code >= kDmfLastError && code < sAppErrors.lastError)
      {
         errorList->errorTable[errorList->errorCount] = 
          dmfMakeErrorString(sAppErrors.strings, code - kDmfLastError, code,
          parm1, parm2, parm3);
         errorList->errorCount++;
      }
   }
}

extern void dmfSetAppErrors(ErrorStrings* errorStrings, int lastErrorId)
{
   sAppErrors.strings = errorStrings;
   sAppErrors.lastError = lastErrorId + kDmfLastError;
}


/*
 * 
 *
 */
extern char_t* dmfFormatError(char_t* errorString /*in*/, 
 int* alFrameCode /*out, opt*/)
{

   /* build a local copy that we can modify */
   char_t* err = bstrdup(B_L, errorString);
   /* the parse function below modifies the pointer that it's given -- we need
    * to remember the original address 'err' so we free the right memory.
    */
   char_t* pErr = err;
   char_t* tempKey = NULL;
   char_t* code = NULL;
   char_t* codeStr = NULL;
   char_t* key = NULL;
   char_t* index = NULL;
   char_t* val = NULL;
   char_t* retval = NULL;
   int codeInt = kDmfSuccess;

   /*
    * pull apart the error string, which we are given in key/value pairs,
    * where the keys are:
    * code
    * key
    * index
    * value
    * 
    * We use our existing key/value parser to do the lifting.
    * Note that not all of the parameters are available or meaningful in
    * each instance. We default to returning this weird delimited string so
    * that clever client-side javascript programmers can parse the data out
    * & display it as needed. Otherwise, they request the error string and
    * then pass it right back down to this function, which parses it into
    * a format that it human-readable.
    */
   dmfParseKeyValueString(&pErr, &tempKey, &code, '=', '&');
   if (0 != gstrlen(code))
   {
      codeInt = gatoi(code);
   }
   dmfParseKeyValueString(&pErr, &tempKey, &codeStr, '=', '&');
   dmfParseKeyValueString(&pErr, &tempKey, &key, '=', '&');
   dmfParseKeyValueString(&pErr, &tempKey, &index, '=', '&');
   dmfParseKeyValueString(&pErr, &tempKey, &val, '=', '&');
   
   retval = dmfFormatErrorString(codeInt, key, index, val);
   bfreeSafe(B_L, err);

   if (NULL != alFrameCode)
   {
      *alFrameCode = codeInt;
   }
   return retval;
}


extern void dmfSetErrorPage(webs_t wp, char_t* errorPage)
{
   Errors* err = dmfGetErrorStruct(wp);
   char_t *p;

   if (err != NULL)
   {
      bfreeSafe(B_L, err->errorPage);
      if ((p=strchr(errorPage, '?')) != NULL) *p = 0;
      err->errorPage = bstrdup(B_L, errorPage);
      if (p != NULL) *p = '?';
   }


}


   
extern int dmfGetErrorCount(webs_t wp)
{
   /*
    * When we add support for per-user error tracking, we should be able to
    * get any user info we need out of the webs_t struct that we get passed
    * here. For the moment, we just use the static per-server error struct.
    */
   Errors* err = dmfGetErrorStruct(wp);
   int retval = 0;
   
   if (NULL != err)
   {
      retval = err->errorCount;
   }
   return retval;
}


   

extern int dmfGetErrorString(webs_t wp, char_t* index, int unFormatted, 
 int* alFrameCode, char_t** errorString)
{
   int retval = kDmfError;
   //sym_t* symbol = NULL; warning: unused variable `symbol'

   Errors* errorList = dmfGetErrorStruct(wp);
   if (NULL != errorList)
   {
      size_t i = 0;
      retval = kDmfIndexRange;

      i = gatoi(index);
      if (i >= 0 && i < errorList->errorCount)
      {
         a_assert(NULL != errorList->errorTable[i]);
         retval = kDmfSuccess;
         if (unFormatted)
         {
            *errorString = bstrdup(B_L, errorList->errorTable[i]);
         }
         else
         {
            *errorString = dmfFormatError(errorList->errorTable[i], alFrameCode);
         }

      }

   }
   return retval;
}

extern int dmfGetErrorCodeImpl(void)
{
   return sErrorCode;
}

extern void dmfSetErrorCode(int eid, int code)
{
   char_t* name = NULL; 
   if ((code >= 0 && code < kDmfLastError)
    || (code >= kDmfLastError && code < sAppErrors.lastError))
   {
      sErrorCode = code;
   }
   else
   {
      sErrorCode = kDmfInternalError;
   }
  
   /*
    * Set the magic error code variable to contain the name of the last
    * result. This lets web programmers access the result without requiring an
    * explicit call to GetError();
    * Instead, they can just write code like:
    * <% 
    *    val = getVal("my.parameter.name", 0);
    *    if (__errorCode__ != "Success")
    *    {
    *       write("Error = " + __errorCode__);
    *    }
    * %>
    */      
   if (code >= 0 && code < kDmfLastError)
   {
      name = kDmfCodesStrings[sErrorCode].errorName;
   }
   else if (code >= kDmfLastError && code < sAppErrors.lastError)
   {
      a_assert(NULL != sAppErrors.strings);
      name = sAppErrors.strings[sErrorCode-kDmfLastError].errorName;
   }
   ejSetVar(eid, kErrorCodeVariable, name);

   /*
    * ...and also set the magic 'success' variable -- if the last error code
    * was kDmfSuccess or kDmfLastError (first user error), set it to 1, otherwise
    * 0. We can use this variable as if it were a boolean.
    */
   ejSetVar(eid, kSuccessVariable, 
    ((kDmfSuccess == code || kDmfLastError == code) ? T("1") : T("0")));

}


/*
 * tests to see if the specified page exists. If the specified path exists and
 * can be opened, stat()-ed, etc., we return a pointer to the local path
 * equivalent of the specified URL (which must be freed later). Else it
 * returns NULL.
 */
static char_t* dmfCheckRichErrorPage(webs_t wp, char_t* url)
{
   char_t* localPath = NULL;
   int nchars;
   websStatType   sbuf;
   /*
    * the following code is heavily cribbed from the default handler (see
    * default.c
    */
   if (websValidateUrl(wp, url) < 0) 
   {
      return 0;
   }
   localPath = websGetRequestLpath(wp);
   nchars = gstrlen(localPath) - 1;
   if (localPath[nchars] == '/' || localPath[nchars] == '\\') 
   {
      localPath[nchars] = '\0';
   }

   /*
    *	Open the document. Stat for later use.
    */
   if (websPageOpen(wp, localPath, url, SOCKET_RDONLY | SOCKET_BINARY, 
    0666) < 0) 
   {
      bfreeSafe(B_L, localPath);
      localPath = NULL;
   } 
   else if (websPageStat(wp, localPath, url, &sbuf) < 0) 
   {
      bfreeSafe(B_L, localPath);
      localPath = NULL;
   }

   return localPath;
}


/*
 * dmfSafeUrl -- utility function to clean up URLs that will be printed by
 * accessing the _errorUrl_ variable, below. To prevent problems with the 'cross-site
 * scripting exploit', where attackers request an URL containing embedded
 * JavaScript code, we replace all '<' and '>' characters with HTML entities
 * so that the user's browser will not interpret the URL as JavaScript.
 *
 * Allocates a new string that must be freed.
 */

#define kLt '<'
#define kLessThan T("&lt;")
#define kGt '>'
#define kGreaterThan T("&gt;")




static int charCount(const char_t* str, char_t ch)
{
   int count = 0;
   char_t* p = (char_t*) str;

   while (1)
   {
      p = gstrchr(p, ch);
      if (NULL == p)
      {
         break;
      }
      /*
       * increment the count, and begin looking at the next character
       */
      ++count;
      ++p;
   }
   return count;
}



static char_t* dmfSafeUrl(const char_t* url)
{
   int ltCount = charCount(url, kLt);
   int gtCount = charCount(url, kGt);
   int safeLen = gstrlen(url);
   char_t* safeUrl = NULL;
   char_t* src = NULL;
   char_t* dest = NULL;

   if (ltCount == 0 && gtCount == 0)
   {
      safeUrl = bstrdup(B_L, (char_t*) url);
   }
   else
   {
      safeLen += (ltCount * 4);
      safeLen += (gtCount * 4);

      safeUrl = balloc(B_L, safeLen);
      if (safeUrl != NULL)
      {
         src = (char_t*) url;
         dest = safeUrl;
         while (*src)
         {
            if (*src == kLt)
            {
               gstrcpy(dest, kLessThan);
               dest += gstrlen(kLessThan);
            }
            else if (*src == kGt)
            {
               gstrcpy(dest, kGreaterThan);
               dest += gstrlen(kGreaterThan);
            }
            else
            {
               *dest++ = *src;
            }
            ++src;
         }
         /* don't forget to terminate the string...*/
         *dest = '\0';
      }
   }
   return safeUrl;
}


extern int dmfRichError(webs_t wp, int code, char_t* userMsg)
{
   char_t codeStr[32];
   char_t* path = NULL;
   char_t* localPath = NULL;
   char_t* date = NULL;
   char_t* url = NULL;
   if (401 == code)
   {
      /* if the error is that we need to authenticate, always use the 
       * default behavior!
       */
      return 0;
   }


   /*
    * first we look for a page that handles this error specifically (e.g.,
    * 'error404.asp'
    */
   fmtAlloc(&path, BUF_MAX, kErrorPageFormat, code);
   localPath = dmfCheckRichErrorPage(wp, path);
   bfreeSafe(B_L, path);
   if (NULL == localPath)
   {
      /*
       * there's no specific error handling page, attempt to load the generic
       * error handling page...
       */
      localPath = dmfCheckRichErrorPage(wp, kErrorDefaultPage);
      if (NULL == localPath)
      {
         /*
          * we failed on both counts -- revert to the builtin error reporting
          * behavior.
          */
         return 0;
      }
   }

   

   /*
    * set the ephemeral local variables that we will use to communicate with
    * the ASP page that displays the messages.
    */
   fmtStatic(codeStr, 32, T("%d"), code);
   dmfSetParam(kRichErrorCode, NULL, codeStr, 0, -1 );
   dmfSetParam(kRichErrorText, NULL, websErrorMsg(code), 0, -1);
   dmfSetParam(kRichErrorMsg, NULL, userMsg, 0, -1);
   url = dmfSafeUrl(wp->url);
   dmfSetParam(kRichErrorUrl, NULL, url, 0, -1);
   bfreeSafe(B_L, url);

   /* if we can open and stat the file, we assume that it's okay to use. */
   /* first, send all the HTTP headers (etc). */
   websWrite(wp, T("HTTP/1.1 %d %s\r\n"), code, websErrorMsg(code));
   /*		
    *		By license terms the following line of code must not be modified.
    */
   websWrite(wp, T("Server: %s\r\n"), WEBS_NAME);

   /*		
    *		Timestamp/Date is usually the next to go
    */
   if ((date = websGetDateString(NULL)) != NULL) 
   {
	   websWrite(wp, T("Date: %s\r\n"), date);
	   bfree(B_L, date);
   }

   if (wp->flags & WEBS_KEEP_ALIVE) {
	   websWrite(wp, T("Connection: keep-alive\r\n"));
   }

   websWrite(wp, T("Pragma: no-cache\r\nCache-Control: no-cache\r\n"));
   websWrite(wp, T("Content-Type: text/html\r\n"));
	websWrite(wp, T("\r\n"));

   /* then process the error page as an ASP request. */
   websSetRequestLpath(wp, localPath);
   websAspRequest(wp, localPath);

   /* ...and finish up. */
   websDone(wp, code);

   bfreeSafe(B_L, localPath);

   /*
    * reset all the error page variables, just in case...
    */
   dmfSetParam(kRichErrorCode, NULL, T(""), 0, -1);
   dmfSetParam(kRichErrorText, NULL, T(""), 0, -1);
   dmfSetParam(kRichErrorMsg, NULL, T(""), 0, -1);
   dmfSetParam(kRichErrorUrl, NULL, T(""), 0, -1);


   return 1;
}
