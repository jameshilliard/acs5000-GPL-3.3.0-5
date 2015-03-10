/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */

#include "Access.h"
#include "Dmf.h"
#include "Cookie.h"
#include "EjFuncs.h"
#include "Error.h"
#include "Forms.h"
/*#include "UserFunc.h"*/
#ifdef qLocalize
#include "Localize.h"
#endif
#include "Locals.h"
#include "Param.h"
#include "Parse.h"
#ifdef qSession
#include "Session.h"
#endif
#ifdef qSoap
#include "Soap.h"
#endif
#ifdef qXmlRpc
#include "XmlRpc.h"     
#endif

#ifdef AVWORKS
#include "webApi.h"
#include "ResultCodes.h" 
#endif

/*
 * File Scope variables/constants/macros:
 */



static int sInitialized = 0;

static int sRequestCount = 0;
/*../dmf212/AlFrame/Dmf.c:38: warning: `sCommitRequestCount' defined but not used
static int sCommitRequestCount = 0;*/



static sym_fd_t   sParams;

   /* mapping keys to CmdFuncs */
static sym_fd_t   cmdFunctions;




   /*    
    *    We support simple markup-style replacement, by surrounding a 
    *    variable with the following markup delimiters. You may change the
    *    delimiters here, should the default [[variableName]] style not 
    *    work for some reason.
    */    
#define kMarkupStart    T("[[")
#define kMarkupEnd      T("]]")



#define kDmfFormPath       T("Dmf")
#ifdef qXmlRpc
#define kDmfXmlRpcPath     T("RPC2")
#endif

#ifdef qSoap
#define kDmfSoapPath          T("SOAP")
#endif


#define kDmfCookiePath     T("Cookies")

#define kMaxSymbolLength   512
#define kLocalsFile  T("LocalVar.txt")

/*
 * we create our own URL handler to track the number of ASP requests. Any time
 * a user requests an ASP page, we increment a counter of requests and can use
 * this in our query functions to decide if we need to ask the device for
 * updated data. This is useful in instances where the device provides an API
 * that returns multiple parameter values in a structure -- by checking the
 * request number for each individual parameter gotten as part of a single ASP
 * request, we can intelligently decide to only refresh our cached copy once.
 */
static int dmfUrlHandler(webs_t wp, char_t* urlPrefix, char_t* webdir, int arg, 
		char_t* url, char_t* path, char_t* query);

#ifdef AVWORKS
static int avworksUrlHandler(webs_t wp, char_t* urlPrefix, char_t* webdir, int arg, 
		char_t* url, char_t* path, char_t* query);
#endif
/*
 * We can support up to 16 levels of hierarchical values in each parameter
 * type. As we traverse down (and back up) the hierarchy contained in the list
 * of parameters (indicated by pseudo-parameters with data types of
 * kSectionStart and kSectionEnd), we take any parameters that may be present
 * in the pseudo parameter and remember them. Children of that
 * pseudo-parameter are considered to use the parameter set by their parent.
 */

#define kHierarchyMax   16
extern void dmfInitParams(Param* parameters, int parameterCount)
{
   int i;
   char_t* keyBuf = NULL;

   /*
    * start out with room for 128 characters of section headings...
    */
   int keyBufLen = 128;
   int keyBufNeeded = 0;
   char_t* newKey = NULL;
   Param* parm = NULL;
   int parent = -1;
   
   sParams = symOpen(parameterCount);
   keyBuf = balloc(B_L, keyBufLen);
   gstrcpy(keyBuf, T(""));

   for (i = 0; i < parameterCount; ++i)
   {
      parm = &(parameters[i]);
      if (kEndSection == parm->dataType)
      {
         /*
          * delete the last section heading
          */
         newKey = gstrrchr(keyBuf, '.');
         if (NULL != newKey)
         {
            /* we found a dot, so put a NULL there to terminate the string */
            *newKey = '\0';
         }
         if (-1 != parent)
         {          
            parent = parameterList[parent].parent;
         }
      }
      else
      {
         if (kBeginSection == parm->dataType)
         {
            parm->parent = parent;
            parent = i;
            /*
             * make sure that we have room for the new section key that we're
             * about to append...
             */
            a_assert(0 != gstrlen(parm->key));

            keyBufNeeded = gstrlen(keyBuf) + gstrlen(parm->key);
            if (keyBufNeeded > keyBufLen)
            {
               brealloc(B_L, keyBuf, keyBufNeeded);
               keyBufLen = keyBufNeeded;
            }
            /*
             * ...and add the last section heading onto the key.
             */
            if (gstrlen(keyBuf) > 0)
            {
               gstrcat(keyBuf, T("."));
            }
            gstrcat(keyBuf, parm->key);
            newKey = bstrdup(B_L, keyBuf);
         }
         else /* this is a real parameter */
         {    
            parm->parent = parent;
            /*
             * build the long key name, as we look it up
             */
            a_assert(gstrlen(parm->key) > 0);

            fmtAlloc(&newKey, BUF_MAX, T("%s.%s"), keyBuf, parm->key);
         }
         parm->lastCommit = 0;
         parm->lastQuery = 0;

         symEnter(sParams, strlower(newKey), valueInteger(i), 0);
         bfreeSafe(B_L, newKey);
      }

   }

   bfreeSafe(B_L, keyBuf);

}
   


extern int dmfInit(void)
{

   if (!sInitialized)
   {

      /* open and reload the symbol tables for our use... */
      dmfInitLocals(64);
      dmfReloadLocals(kLocalsFile);
      
      /* 
       * call the user-supplied pre initialization function.
       */
      dmfUserPreInit();



      /*!!! dmfInitParams();*/

#ifdef qXmlRpc
      dmfInitXmlRpc();
#endif

#ifdef qSoap
      dmfInitSoap();
#endif

#ifdef qSmtp
      dmfInitSmtp();
#endif


      /*
       * set the realm used for authentication
       */
      websSetRealm(T("DMF"));
      
      cmdFunctions = symOpen(13);
      symEnter(cmdFunctions, T("ADDUSER"), 
       valueInteger((long) dmfUserCommandHandler), 0);
      symEnter(cmdFunctions, T("MODIFYUSER"), 
       valueInteger((long) dmfUserCommandHandler), 0);
      symEnter(cmdFunctions, T("DELETEUSER"), 
       valueInteger((long) dmfUserCommandHandler), 0);
      symEnter(cmdFunctions, T("ADDGROUP"), 
       valueInteger((long) dmfGroupCommandHandler), 0);
      symEnter(cmdFunctions, T("MODIFYGROUP"), 
       valueInteger((long) dmfGroupCommandHandler), 0);
      symEnter(cmdFunctions, T("DELETEGROUP"), 
       valueInteger((long) dmfGroupCommandHandler), 0);


      /*
       * hook up any ASP functions...
       */

      /*
        ../dmf212/AlFrame/Dmf.c:243: warning: passing arg 2 of `websAspDefine' from incompatible pointer type
        ../dmf212/AlFrame/Dmf.c:244: warning: passing arg 2 of `websAspDefine' from incompatible pointer type
        ../dmf212/AlFrame/Dmf.c:245: warning: passing arg 2 of `websAspDefine' from incompatible pointer type
        ../dmf212/AlFrame/Dmf.c:246: warning: passing arg 2 of `websAspDefine' from incompatible pointer type
        ../dmf212/AlFrame/Dmf.c:247: warning: passing arg 2 of `websAspDefine' from incompatible pointer type
        ../dmf212/AlFrame/Dmf.c:248: warning: passing arg 2 of `websAspDefine' from incompatible pointer type
        ../dmf212/AlFrame/Dmf.c:249: warning: passing arg 2 of `websAspDefine' from incompatible pointer type
        ../dmf212/AlFrame/Dmf.c:250: warning: passing arg 2 of `websAspDefine' from incompatible pointer type
        ../dmf212/AlFrame/Dmf.c:251: warning: passing arg 2 of `websAspDefine' from incompatible pointer type
        ../dmf212/AlFrame/Dmf.c:252: warning: passing arg 2 of `websAspDefine' from incompatible pointer type
      websAspDefine(T("debugInfo"), (AspFunc) dmfEjDebugInfo);
      websAspDefine(T("formatTime"), (AspFunc) dmfEjFormatTime);
      websAspDefine(T("get"), (AspFunc) dmfEjGet);
      websAspDefine(T("getCookieVal"), (AspFunc) dmfEjGetCookieVal);
      websAspDefine(T("getLastError"), (AspFunc) dmfEjGetLastErrorCode);
      websAspDefine(T("getErrorCount"), (AspFunc) dmfEjGetErrorCount);
      websAspDefine(T("getErrorMessage"), (AspFunc) dmfEjGetErrorMessageByIndex);
      websAspDefine(T("getErrorName"), (AspFunc) dmfEjGetErrorNameByIndex);
      websAspDefine(T("getNextGroup"), (AspFunc) dmfEjGetGroup);
      websAspDefine(T("getGroupDetail"), (AspFunc) dmfEjGetGroupDetail);
      */

      websAspDefine(T("debugInfo"), dmfEjDebugInfo);
      websAspDefine(T("formatTime"), dmfEjFormatTime);
      websAspDefine(T("get"), dmfEjGet);
      websAspDefine(T("getCookieVal"), dmfEjGetCookieVal);
      websAspDefine(T("getLastError"), dmfEjGetLastErrorCode);
      websAspDefine(T("resetError"), dmfEjResetError);
      websAspDefine(T("getErrorCount"), dmfEjGetErrorCount);
      websAspDefine(T("getErrorMessage"), dmfEjGetErrorMessageByIndex);
      websAspDefine(T("getErrorName"), dmfEjGetErrorNameByIndex);
      websAspDefine(T("getNextGroup"), dmfEjGetGroup);
      websAspDefine(T("getGroupDetail"), dmfEjGetGroupDetail);


#ifdef qLocalize
      websAspDefine(T("getLabel"), dmfEjGetLabel);
      websAspDefine(T("getLabelVal"), dmfEjGetLabelVal);
      websAspDefine(T("getLanguage"), dmfEjGetLanguageCode);
#endif
      /*
        ../dmf212/AlFrame/Dmf.c:254: warning: passing arg 2 of `websAspDefine' from incompatible pointer type
        ../dmf212/AlFrame/Dmf.c:255: warning: passing arg 2 of `websAspDefine' from incompatible pointer type
        ../dmf212/AlFrame/Dmf.c:256: warning: passing arg 2 of `websAspDefine' from incompatible pointer type
        ../dmf212/AlFrame/Dmf.c:258: warning: passing arg 2 of `websAspDefine' from incompatible pointer type
        ../dmf212/AlFrame/Dmf.c:259: warning: passing arg 2 of `websAspDefine' from incompatible pointer type
        ../dmf212/AlFrame/Dmf.c:260: warning: passing arg 2 of `websAspDefine' from incompatible pointer type
        ../dmf212/AlFrame/Dmf.c:261: warning: passing arg 2 of `websAspDefine' from incompatible pointer type
        ../dmf212/AlFrame/Dmf.c:262: warning: passing arg 2 of `websAspDefine' from incompatible pointer type
        ../dmf212/AlFrame/Dmf.c:263: warning: passing arg 2 of `websAspDefine' from incompatible pointer type
        ../dmf212/AlFrame/Dmf.c:264: warning: passing arg 2 of `websAspDefine' from incompatible pointer type
        ../dmf212/AlFrame/Dmf.c:265: warning: passing arg 2 of `websAspDefine' from incompatible pointer type
        ../dmf212/AlFrame/Dmf.c:266: warning: passing arg 2 of `websAspDefine' from incompatible pointer type
        ../dmf212/AlFrame/Dmf.c:267: warning: passing arg 2 of `websAspDefine' from incompatible pointer type
        ../dmf212/AlFrame/Dmf.c:268: warning: passing arg 2 of `websAspDefine' from incompatible pointer type
        ../dmf212/AlFrame/Dmf.c:269: warning: passing arg 2 of `websAspDefine' from incompatible pointer type
      */
      websAspDefine(T("getNextUser"), dmfEjGetNextUser);
      websAspDefine(T("getQueryVal"), dmfEjGetQueryVal);
      websAspDefine(T("getRequestNumber"), dmfEjGetRequestNumber);
      websAspDefine(T("getTime"), dmfEjGetTime);
      websAspDefine(T("getUserDetail"), dmfEjGetUserDetail);
      websAspDefine(T("getVal"), dmfEjGetVal);
      websAspDefine(T("max"), dmfEjMaxVal);
      websAspDefine(T("min"), dmfEjMinVal);
      websAspDefine(T("set"), dmfEjSet);
      websAspDefine(T("userName"), dmfEjGetUserName);
      websAspDefine(T("userCanAccess"), dmfEjUserCanAccess);
      websAspDefine(T("split"), dmfEjSplit);
    
#ifdef AVWORKS 
      websUrlHandlerDefine(T("/AvWorks"), NULL, 0, avworksUrlHandler, WEBS_HANDLER_FIRST);
#endif
 
      websFormDefine(kDmfFormPath, dmfFormHandler);

#ifdef qXmlRpc
      websFormDefine(kDmfXmlRpcPath, dmfXmlRpcHandler);
#endif

#ifdef qSoap
      websFormDefine(kDmfSoapPath, dmfSoapHandler);
#endif

      /*websFormDefine(kDmfCookiePath, dmfCookieHandler);*/
      
      /* register our private URL handler. We don't care where in the chain of
       * URL handlers we get called, but we don't want to be first and don't
       * want to be last.
       */
      websUrlHandlerDefine(T(""), NULL, 0, dmfUrlHandler, WEBS_HANDLER_FIRST);

#ifdef qLocalize
      dmfLoadLanguage();
#endif
      /*
       * If project-specific code didn't initialize the session engine with timeouts, 
       * initialize it here to never time out.
       */
#ifdef qSession
      if (NULL == dmfFindSession(NULL))
      {
         dmfInitSession(0, 0);
      }
#endif

      /*
       * call the user-supplied post initialization function.
       */
      dmfUserPostInit();


      /*
       * if any of the initialization code resulted in errors in the default 
       * session, clear them away before we start serving pages...
       */
      dmfClearErrors(NULL);
   }

   sInitialized = 1;

   return kDmfSuccess;
}



extern int dmfClose(void)
{
   if (sInitialized)
   {

      /*
       * Call the user-supplied pre-close function
       */
      dmfUserPreClose();

      symClose(sParams);
      symClose(cmdFunctions);

      dmfCloseLocals();

#ifdef qSmtp
      dmfCloseSmtp();
#endif

#ifdef qSoap
      dmfCloseSoap();
#endif

#ifdef qXmlRpc
      dmfCloseXmlRpc();
#endif
      /*
       * close the session management subsystem.
       */
#ifdef qSession
      dmfCloseSession();
#endif
#ifdef qLocalize
      dmfCloseLanguage();
#endif

      sInitialized = 0;

      /*
       * Call the user-supplied post-close function
       */
      dmfUserPostClose();
   }

   return kDmfSuccess;
}


extern int dmfFindParam(char_t* key)
{
   int retval = -1;
   sym_t* symbol = symLookup(sParams, strlower(key));
   if (NULL != symbol)
   {
      retval = symbol->content.value.integer;
   }
   return retval;
}


extern Param* dmfGetParamPtr(int parmIndex)
{
   Param* retval = NULL;
   if (parmIndex > -1 && parmIndex < parameterCount)
   {
      retval = &(parameterList[parmIndex]);
   }
   return retval;
}

#ifdef AVWORKS
char* getQueryValues(char* key) {
	char value[32];
	char *equalSign;

	equalSign = strchr(key, '=');
	strcpy(value, equalSign+1);
	return value;
}
#endif

#ifdef AVWORKS
void queryTokenizer(char* query, char* pass, char* user, char* sids) {
	char delimiter[] = "&";
	char *result = NULL;
	char keys[3][32];
	int j = 0;
	
	result = strtok(query, delimiter);

	for (j = 0; j < 3; j++) {
		strcpy(keys[j],result);
		result = strtok( NULL, delimiter );
	}
	
	strcpy(pass, getQueryValues(keys[0]));
	strcpy(user, getQueryValues(keys[1]));
	strcpy(sids, getQueryValues(keys[2]));
}
#endif

#ifdef AVWORKS
extern int avworksUrlHandler(webs_t wp, char_t *urlPrefix, char_t *webdir, int arg, 
                char_t *url, char_t *path, char_t *query)
{
   char_t* val = NULL;
   char *msgbuf;
   //syslog(1, "AVWORKS call:   wp-->%d   port--:%d query-->%s  tamanho->%d", wp, wp->port, wp->query, strlen(wp->query));

   unsigned int sid = 0;
   WebUserInfo wUser;
   Result result;
   char username[31];
   char password[31];
   char sidp[31];
   char userType;
   char successResponse[255];
   char sidTemp[19];

   memset(password, 0x00, 31);
   memset(username, 0x00, 31);
   memset(sidp,     0x00, 31);

   queryTokenizer(wp->query, password, username, sidp);
                 
   result = CycLogIn(username, password, &sid, &wUser, 0, 0);
  
   if (wUser.uType == kAdminUser) {
   	userType = 'e'; // admin
        CycBecomeAdminBoss(sid);
   }
   else {
	userType = 'n'; //regular
   }

   if (kCycSuccess == result || kSessUserOk == result || kAnotherAdminIsOn == result) { 
       strcpy(successResponse, "<html><head></head><body>system.req.sid=");
       sprintf(sidTemp, "%d&x=%c", sid, userType);
       strcat(successResponse, sidTemp);
       strcat(successResponse, "</body></html>");
       fmtAlloc(&msgbuf, 4096 + 80, T(successResponse), url);
       websResponse(wp, 200, msgbuf, url);  
   } 
   else {
    	fmtAlloc(&msgbuf, 4096 + 80,
		T("<html><head></head><body>Server did not authorize you. Give me your identity</body></html>"), url);
       	websResponse(wp, 401, msgbuf, url);  
   }

   bfreeSafe(B_L, msgbuf);

   return 1;
}
#endif

extern int dmfUrlHandler(webs_t wp, char_t *urlPrefix, char_t *webdir, int arg, 
		char_t *url, char_t *path, char_t *query)
{
   /*int isRequest = 0; ../dmf212/AlFrame/Dmf.c:400: warning: unused variable `isRequest'*/
   /*
    * we only increment the request count if this is an ASP request -- that's
    * the only case where ejScript code has a chance to be executed.
    */
   if (WEBS_ASP & wp->flags)
   {
      dmfIncrementRequestNumber();
   }



   /*
    * we always return zero so that the request gets handled down the line --
    * we just want to peek in and see what's going on.
    */
   return 0;
}



extern int dmfGetRequestNumber(void)
{
   return sRequestCount;
}


extern int dmfIncrementRequestNumber(void)
{
   return ++sRequestCount;
}



extern int dmfCommand(CommandStruct* cmd)
{
   sym_t* symbol = NULL;
   CmdFunc cf = NULL;
   int retval = kDmfError;

   a_assert(sInitialized);

   symbol = symLookup(cmdFunctions, cmd->command);
   if (NULL != symbol)
   {
      cf = (CmdFunc) symbol->content.value.integer;
      if (NULL != cf)
      {
         retval = (*cf)(cmd);
      }
   }

   return retval;
}

#ifdef qDebug
extern void dmfDebugTraceSymbolTable(sym_fd_t table)
{
/*
   sym_t* elem = NULL;
   char_t buf[1024];

   elem = symFirst(table);
   while (NULL != elem)
   {
      sprintf(buf, T("%s -> %s\n"), elem->name.value.string, elem->content.value.string);
      OutputDebugString(buf);
      elem = symNext(table);
   }
*/
}

#endif

extern int dmfPersistSymbolTable(sym_fd_t table, char_t* fileName)
{
   int retval = kDmfError;

#ifdef _WINDOWS
#define kOpenWriteAttribute   T("wt")
#else
#define kOpenWriteAttribute   T("w")
#endif

   FILE* f = gfopen(fileName, kOpenWriteAttribute);
   sym_t* elem = NULL;

   if (NULL != f)
   {
      /* 
       * retrieve the 'first' element in the table
       */
      elem = symFirst(table);
      while (NULL != elem)
      {
         switch (elem->content.type)
         {
            case string:
            gfprintf(f, T("%s=%s\n"), elem->name.value.string, 
             elem->content.value.string);
            break;

            case integer:
            /* ../dmf212/AlFrame/Dmf.c:547: warning: int format, long int arg (arg 4)
		gfprintf(f, T("%s=%d\n"), elem->name.value.string, elem->content.value.integer);*/
            	gfprintf(f, T("%s=%d\n"), elem->name.value.string, (int)elem->content.value.integer);
            break;

            default:
            a_assert(0);
            break;
         }


         /*
          * retrieve the next item
          */
         elem = symNext(table);
      }

      fclose(f);

      retval = kDmfSuccess;
   }

   return retval;

}


extern int dmfReloadSymbolTable(sym_fd_t table, char_t* fileName, int type)
{
   char_t* lineBuf = balloc(B_L, kMaxSymbolLength);
   char_t* line;
   char_t* key;
   char_t* val;
   char_t* newLine = NULL;
   int retval = kDmfError;

#ifdef _WINDOWS
#define kOpenReadAttribute   T("rt")
#else
#define kOpenReadAttribute   T("r")
#endif
   FILE* f = gfopen(fileName, kOpenReadAttribute);

   if ((NULL != lineBuf) && (NULL != f))
   {
      while (1)
      {
         if (NULL == gfgets(lineBuf, kMaxSymbolLength, f))
         {
            /*
             * we've run out of file to process!
             */
            break;
         }
         line = lineBuf;
         /*
          * else, lineBuf contains the next line of the file.
          * First, replace the newline that fgets left on the end.
          */
         newLine = gstrchr(line, '\n');
         if (NULL != newLine)
         {
            *newLine = '\0';
         }
         key = NULL;
         val = NULL;
         if (dmfParseKeyValueString(&line, &key, &val, '=', '\0'))
         {
            /*
             * store the new key and value into our table
             */
            switch (type)
            {
               case string:
               symEnter(table, key, valueString(val, VALUE_ALLOCATE), 0);
               break;

               case integer:
               symEnter(table, key, valueInteger(gatoi(val)), 0);
               break;

               default:
               a_assert(0);
               break;
            }
         }
      }

      fclose(f);
      retval = kDmfSuccess;
   }

   bfreeSafe(B_L, lineBuf);
   return retval;

}
