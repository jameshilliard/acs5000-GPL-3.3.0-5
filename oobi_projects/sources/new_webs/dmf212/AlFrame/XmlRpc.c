/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: XmlRpc.c,v 1.2 2006/01/10 23:51:50 erik Exp $ */

#include "XmlRpc.h"
#ifdef qXmlRpc

#include "wsIntrn.h"
/* include the expat headers. */
#include "xmlparse.h"

#include "Error.h"
#include "GetSet.h"
#include "RpcUtil.h"

#include "XmlRpcFuncs.h"

#define qXmlRpcTypes
#include "ParamCodes.h"


#define kMethodCallTag        T("methodCall")
#define kMethodNameTag        T("methodName")
#define kMethodResponseTag    T("methodResponse")
#define kParamsTag            T("params")
#define kParamTag             T("param")
#define kStructTag            T("struct")
#define kValueTag             T("value")
#define kMemberTag            T("member")
#define kMemberNameTag        T("name")
#define kArrayTag             T("array")
#define kDataTag              T("data")
#define kIntegerTag           T("int")
#define kI4Tag                T("i4")
#define kBooleanTag           T("boolean")
#define kStringTag            T("string")
#define kDoubleTag            T("double")
#define kDateTimeTag          T("dateTime.iso8601") /* not currently supported */
#define kBase64Tag            T("base64") /* not currently supported */

#define kResponseProlog       T("<?xml version=\"1.0\"?>\n<methodResponse>\n")
#define kResponseEpilog       T("</methodResponse>\n")
#define kErrorFormat          kResponseProlog  \
   T("<fault>\n") \
      T("<value>\n") \
         T("<struct>\n") \
            T("<member>\n") \
               T("<name>faultCode</name>\n") \
               T("<value><string>%s</string></value>\n") \
            T("</member>\n") \
            T("<member>\n") \
               T("<name>faultString</name>\n") \
               T("<value><string>%s</string></value>\n") \
            T("</member>\n") \
         T("</struct>\n") \
      T("</value>\n") \
   T("</fault>\n") \
   kResponseEpilog


static void AddValue(RpcParseContext* context, enum RpcDataType type);

/*
 * for each tag that we need to handle, we create a pair of handler functions
 * -- one to handle start of tag, one to handle end of tag. We install them in
 * a GoAhead symbol table and use the hash table to look up the correct
 * function very quickly.
 */
typedef void (*ExpatHandler)(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr);

static void OnStartMethodCall(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr);
static void OnEndMethodCall(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr);

static void OnStartMethodName(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr);
static void OnEndMethodName(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr);

static void OnStartParams(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr);
static void OnEndParams(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr);

static void OnStartParam(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr);
static void OnEndParam(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr);

static void OnStartStruct(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr);
static void OnEndStruct(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr);

static void OnStartValue(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr);
static void OnEndValue(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr);

static void OnStartMember(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr);
static void OnEndMember(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr);

static void OnStartMemberName(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr);
static void OnEndMemberName(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr);


static void OnStartArray(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr);
static void OnEndArray(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr);

static void OnStartData(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr);
static void OnEndData(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr);

/*
 * The 'ValueType functions handle all the i4/integer/double/etc tags.
 */
static void OnStartValueType(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr);
static void OnEndValueType(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr);

/*
 * function to return a string that contains a standard xml-rpc error message,
 * parameterized so we can send back custom fault codes and fault strings.
 */
static char_t* dmfXmlRpcErrorMessage(int code, const char_t* faultString);

/*
 * helper function to write an xml-rpc payload back to the user. Handles all
 * of the HTTP headers, etc.
 */
static void dmfXmlRpcResponse(webs_t wp, char_t* response, SInt32 responseLength);





/*
 * Expat begin element handler. See the expat documentation.
 */
static void dmfXmlRpcStartHandler(void* user, const XML_Char* name, const XML_Char** attr);

/*
 * Expat end element handler, See the expat documentation.
 */
static void dmfXmlRpcEndHandler(void* user, const XML_Char* name);

/*
 * Expat character data handler. The Expat docs note that the character data
 * that's arriving is NOT zero-terminated, and that a single run of characters
 * in the source may result in multiple calls to this function.
 */
static void dmfXmlRpcCharacterHandler(void* user, const XML_Char* name, int len);



/*
 * Walks the tree rooted at Node recursively and build a string containing the
 * XML representation of this tree.
 *
 * NOTE that we really don't want to do this recursively to avoid stack
 * problems -- unroll the recursion into an iterative solution.
 */
static void NodeString(StringBuf* str, RpcNode* node);

/*
 * Dump the entire contents of this tree into the specified StringBuf.
 */
static void TreeString(StringBuf* str, RpcTree* tree);


/*
 * helper functions to render RpcNode objects into their appropriate string
 * representation. Used internally by NodeString.
 */
static void ValueString(StringBuf* str, RpcNode* valueNode);
static void MemberString(StringBuf* str, RpcNode* memberNode);
static void ArrayString(StringBuf* str, RpcNode* arrayNode);
static void StructString(StringBuf* str, RpcNode* structure);
   
/*
 * development test function
 */
static int xmlRpcTestFunc(webs_t wp, RpcTree* params, RpcTree* result);



/*
 * We map from method names to XmlRpcFunction pointers using this symbol
 * table. Created in our init() function, destroyed in our close function. 
 * We populate it with framework-defined functions in the init function, and
 * if user-supplied code would like to add additional functions, it may add
 * them in the dmfUserPostInit() function.
 */
static sym_fd_t sMethods = -1;

/*
 * we also maintain a small symbol table that maps data type tags ('int',
 * 'boolean', etc. to the equivalent integer constant.
 */
static sym_fd_t sTagTypes = -1;


/*
 * we maintain two maps of expat handler functions. Each maps the xml tags
 * that we will be handling to a function that handles start of tag, and
 * another that handles end of tag. The main expat handler for each of these
 * conditions looks up the correct function by tag and calls it. Much nicer
 * than a ladder of if/else/else statements.
 */

static sym_fd_t   sStartTagFunctions = -1;
static sym_fd_t   sEndTagFunctions = -1;

static XML_Parser sParser = NULL;
static RpcParseContext* sContext;

   

static StringBuf* sFuncOutput = NULL;
static RpcTree* sFuncResultTree = NULL;

static int sInitialized = 0;



extern int dmfInitXmlRpc(void)
{
   int retval = kDmfError;
   if (!sInitialized)
   {
      sMethods = symOpen(16);
      /*
       * Add the framework-provided XML RPC methods...
       */
      dmfXmlRpcAddFunction(T("test"), xmlRpcTestFunc);
      dmfXmlRpcAddFunction(T("get"), dmfXmlRpcGet);
      dmfXmlRpcAddFunction(T("set"), dmfXmlRpcSet);
#ifdef qLocalize
      dmfXmlRpcAddFunction(T("getLabel"), dmfXmlRpcGetLabel);
#endif
      dmfXmlRpcAddFunction(T("getErrors"), dmfXmlRpcGetErrors);
      dmfXmlRpcAddFunction(T("echo"), dmfRpcEchoFunc);

      sContext = dmfCreateRpcContext();


      sStartTagFunctions = symOpen(17);
      /*
       * populate the symbol table of start tag functions.
       */
      symEnter(sStartTagFunctions, kMethodCallTag, 
       valueInteger((long) OnStartMethodCall), 0);
      symEnter(sStartTagFunctions, kMethodNameTag, 
       valueInteger((long) OnStartMethodName), 0);
      symEnter(sStartTagFunctions, kParamsTag, 
       valueInteger((long) OnStartParams), 0);
      symEnter(sStartTagFunctions, kParamTag, 
       valueInteger((long) OnStartParam), 0);
      symEnter(sStartTagFunctions, kStructTag,
       valueInteger((long) OnStartStruct), 0);
      symEnter(sStartTagFunctions, kValueTag, 
       valueInteger((long) OnStartValue), 0);
      symEnter(sStartTagFunctions, kMemberTag, 
       valueInteger((long) OnStartMember), 0);
      symEnter(sStartTagFunctions, kMemberNameTag, 
       valueInteger((long) OnStartMemberName), 0);
      symEnter(sStartTagFunctions, kArrayTag, 
       valueInteger((long) OnStartArray), 0);
      symEnter(sStartTagFunctions, kDataTag, 
       valueInteger((long) OnStartData), 0);
      symEnter(sStartTagFunctions, kI4Tag, 
       valueInteger((long) OnStartValueType), 0);
      symEnter(sStartTagFunctions, kIntegerTag, 
       valueInteger((long) OnStartValueType), 0);
      symEnter(sStartTagFunctions, kBooleanTag, 
       valueInteger((long) OnStartValueType), 0);
      symEnter(sStartTagFunctions, kStringTag, 
       valueInteger((long) OnStartValueType), 0);
      symEnter(sStartTagFunctions, kDoubleTag, 
       valueInteger((long) OnStartValueType), 0);
      symEnter(sStartTagFunctions, kDateTimeTag, 
       valueInteger((long) OnStartValueType), 0);
      symEnter(sStartTagFunctions, kBase64Tag, 
       valueInteger((long) OnStartValueType), 0);

      
      sEndTagFunctions = symOpen(17);
      symEnter(sEndTagFunctions, kMethodCallTag, 
       valueInteger((long) OnEndMethodCall), 0);
      symEnter(sEndTagFunctions, kMethodNameTag, 
       valueInteger((long) OnEndMethodName), 0);
      symEnter(sEndTagFunctions, kParamsTag, 
       valueInteger((long) OnEndParams), 0);
      symEnter(sEndTagFunctions, kParamTag, 
       valueInteger((long) OnEndParam), 0);
      symEnter(sEndTagFunctions, kStructTag, 
       valueInteger((long) OnEndStruct), 0);
      symEnter(sEndTagFunctions, kValueTag, 
       valueInteger((long) OnEndValue), 0);
      symEnter(sEndTagFunctions, kMemberTag, 
       valueInteger((long) OnEndMember), 0);
      symEnter(sEndTagFunctions, kMemberNameTag,
       valueInteger((long) OnEndMemberName), 0);
      symEnter(sEndTagFunctions, kArrayTag, 
       valueInteger((long) OnEndArray), 0);
      symEnter(sEndTagFunctions, kDataTag, 
       valueInteger((long) OnEndData), 0);
      symEnter(sEndTagFunctions, kI4Tag, 
       valueInteger((long) OnEndValueType), 0);
      symEnter(sEndTagFunctions, kIntegerTag, 
       valueInteger((long) OnEndValueType), 0);
      symEnter(sEndTagFunctions, kBooleanTag, 
       valueInteger((long) OnEndValueType), 0);
      symEnter(sEndTagFunctions, kStringTag, 
       valueInteger((long) OnEndValueType), 0);
      symEnter(sEndTagFunctions, kDoubleTag, 
       valueInteger((long) OnEndValueType), 0);
      symEnter(sEndTagFunctions, kDateTimeTag, 
       valueInteger((long) OnEndValueType), 0);
      symEnter(sEndTagFunctions, kBase64Tag, 
       valueInteger((long) OnEndValueType), 0);

      sTagTypes = symOpen(7);
      symEnter(sTagTypes, kIntegerTag, valueInteger(kInteger), 0);
      symEnter(sTagTypes, kI4Tag, valueInteger(kInteger), 0);
      symEnter(sTagTypes, kBooleanTag, valueInteger(kBoolean), 0);
      symEnter(sTagTypes, kStringTag, valueInteger(kString), 0);
      symEnter(sTagTypes, kDoubleTag, valueInteger(kDouble), 0);
      symEnter(sTagTypes, kDateTimeTag, valueInteger(kDateTime), 0);
      symEnter(sTagTypes, kBase64Tag, valueInteger(kBase64), 0);
      
      retval = kDmfSuccess;

   }

   return retval;

}


extern void dmfCloseXmlRpc(void)
{

   dmfDestroyRpcContext(sContext);

   symClose(sStartTagFunctions);
   symClose(sEndTagFunctions);
   symClose(sMethods);
   symClose(sTagTypes);

   DestroyStringBuf(sFuncOutput);
   DestroyTree(sFuncResultTree);


}


extern int dmfXmlRpcAddFunction(char_t* methodName, RpcFunc func)
{
   int retval = kDmfError;
   
   if (-1 != sMethods)
   {
      if (NULL != symEnter(sMethods, methodName, valueInteger((long) func), 0))
      {
         retval = kDmfSuccess;
      }
   }
   return retval;

}




extern void dmfXmlRpcHandler(webs_t wp, char_t *path, char_t *query)
{
   char_t* response = NULL;
   int funcRetval = kDmfError;
   int errorCode = kDmfSuccess;
   SInt32 responseLength = 0;
   char_t* errorString = NULL;
   int queryLen = gstrlen(query);


   if (NULL == sFuncOutput)
   {
      sFuncOutput = CreateStringBuf(512);
   }

   if (NULL == sFuncResultTree)
   {
      sFuncResultTree = CreateTree(16);
   }

   sParser = XML_ParserCreate(NULL);
   if ((NULL != sContext) && (NULL != sParser))
   {
      XML_SetUserData(sParser, sContext);
      XML_SetElementHandler(sParser, dmfXmlRpcStartHandler, 
       dmfXmlRpcEndHandler);
      XML_SetCharacterDataHandler(sParser, dmfXmlRpcCharacterHandler);
   }
	a_assert(websValid(wp));

   /*OutputDebugString(query);
   OutputDebugString("\n");*/

   if ((NULL != sFuncOutput) && (XML_Parse(sParser, query, queryLen, 1)) )
   {
      /*
       * first, count this as a request.
       */
      dmfIncrementRequestNumber();
      /* 
       * If we get here, we've successfully parsed the message, and have
       * extracted the function name and the list of parameters (if any).
       * Call the function.
       */
      

      funcRetval = dmfRpcCallFunction(wp, sMethods, sContext->methodName, 
       sContext->paramList, sFuncResultTree);

      dmfSetErrorPage(wp, NULL);

      if (0 == dmfGetErrorCount(wp))
      {
         AddString(sFuncOutput, kResponseProlog);
         TreeString(sFuncOutput, sFuncResultTree);
         AddString(sFuncOutput, kResponseEpilog);

         response = GetString(sFuncOutput);        
         responseLength = GetStringLength(sFuncOutput);
      }
      else
      {        
         /* we encountered some error in the framework, so we need to report that. */
         if (1 == dmfGetErrorCount(wp))
         {
            /* we'll just return the single error as a fault struct. */
            dmfGetErrorString(wp, T("0"), 0, &errorCode, &errorString);                       
         }
         else
         {
            /* there were multiple errors -- they need to call the getErrors() 
             * function to get them (as an array)
             */
            errorCode = kDmfXmlRpcMultiError;
            errorString = dmfFormatErrorString(errorCode, NULL, NULL, NULL);            
         }
         response = dmfXmlRpcErrorMessage(errorCode, errorString);
         responseLength = gstrlen(response);
         bfreeSafe(B_L, errorString);
      }
   }
   else
   {
      /*
       * there was a parsing error -- return the error string as given by
       * expat.
       */
      response = dmfXmlRpcErrorMessage(kDmfXmlRpcParse, 
       XML_ErrorString(XML_GetErrorCode(sParser)) );
      responseLength = gstrlen(response);
   }

   dmfXmlRpcResponse(wp, response, responseLength);

   /* if we reported an error, we need to free the memory that is pointed to
    * by 'response' -- if we were successful, that memory is managed by the
    * sFuncOutput stringbuf object, and we do NOT free the memory.
    */
   if (kDmfSuccess != funcRetval)
   {
      bfreeSafe(B_L, response);
   }
#define qPerRequest 1
#ifndef qPerRequest
   ResetStringBuf(sFuncOutput);
   ResetTree(sFuncResultTree);
#else
   DestroyStringBuf(sFuncOutput);
   sFuncOutput = NULL;
   DestroyTree(sFuncResultTree);
   sFuncResultTree = NULL;
#endif


   /*
    * reset the context variables...free any memory that may still be
    * allocated, and make sure that we are cleaned up for our next call.
    */
   dmfResetRpcContext(sContext); 

   XML_ParserFree(sParser);
   sParser = NULL;
}



static char_t* dmfXmlRpcErrorMessage(int code, const char_t* faultString)
{
   char_t* response = NULL;
   char_t* format = kErrorFormat;

   fmtAlloc(&response, BUF_MAX, format, dmfGetErrorName(code), faultString);
   return response;

}


static void dmfXmlRpcResponse(webs_t wp, char_t* response, SInt32 responseLength)
{
   char_t* dateString = websGetDateString(NULL);
#ifdef qXXXX
	websWrite(wp, T("HTTP/1.0 200 OK\n"));
   websWrite(wp, T("Connection: close\n"));
   websWrite(wp, T("Content-Length: %d\n"), responseLength);
	websWrite(wp, T("Content-Type: text/xml\n"));
   websWrite(wp, T("Date: %s\n"), dateString);

   /*
    *	By license terms the following line of code must not be modified
    */
	websWrite(wp, T("Server: %s\r\n"), WEBS_NAME);
	websWrite(wp, T("\n"));
#else
   websWrite(wp, T("HTTP/1.0 200 OK\nConnection: close\nContent-Length: %d\n")\
    T("Content-Type: text/xml\nDate: %s\nServer: %s\r\n\n"), 
    responseLength, dateString, WEBS_NAME);
#endif
   bfreeSafe(B_L, dateString);
   websWriteBlock(wp, response, responseLength);

   websDone(wp, 200);

}


static void dmfXmlRpcHandlerImpl(sym_fd_t table, void* user, const XML_Char* name, 
 const XML_Char** attr)
{

   ExpatHandler func = NULL;
   RpcParseContext* context = (RpcParseContext*) user;
   sym_t* symbol = NULL; 

   a_assert(-1 != table);
   a_assert(user);
   a_assert(name);

   symbol = symLookup(table, (char_t*) name);
   a_assert(symbol);

   if (NULL != symbol)
   {
      func = (ExpatHandler) symbol->content.value.integer;
      a_assert(NULL != func);
      if (NULL != func)
      {
         (*func)(context, name, attr);
      }
   }
}



static void AddValue(RpcParseContext* context, enum RpcDataType type)
{
   RpcNode* valueNode = NULL;
   char_t* value = GetString(context->currentData);
   if (gstrchr(value, '%'))
   {
      /* if they have replaced any characters as hex entities, decode them */
      int len = gstrlen(value);
      websDecodeUrl(value, value, len);
   }

   valueNode = CreateValueNode(NULL, value, type);
   AddTreeNode(context->paramList, valueNode);
   ResetStringBuf(context->currentData);
}

static void dmfXmlRpcStartHandler(void* user, const XML_Char* name, 
 const XML_Char** attr)
{
   dmfXmlRpcHandlerImpl(sStartTagFunctions, user, name, attr);
}

static void dmfXmlRpcEndHandler(void* user, const XML_Char* name)
{
   dmfXmlRpcHandlerImpl(sEndTagFunctions, user, name, NULL);
}

static void dmfXmlRpcCharacterHandler(void* user, const XML_Char* name, int len)
{
   RpcParseContext* context = (RpcParseContext*) user;
   /*
    * add the new text to the end of the string buffer...We need to add the
    * NULL by hand, because expat doesn't.
    */
   if ((len > 1) || (0x0a != *name))
   {
      /* expat seems to send us the '\n' that comes after an opening tag. We don't
       * want that leading newline.
       */  
      AddBufToString(context->currentData, name, len);
   }
}





static void OnStartMethodCall(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr)
{
   ResetStringBuf(context->currentData);
}

static void OnEndMethodCall(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr)
{
   /* intentionally left (almost) empty. */
}

static void OnStartMethodName(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr)
{
   /* intentionally left (almost) empty. */
}

static void OnEndMethodName(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr)
{
   a_assert(NULL == context->methodName);
   context->methodName = bstrdup(B_L, GetString(context->currentData));
   if (gstrchr(context->methodName, '%'))
   {
      /* if they have replaced any characters as hex entities, decode them */
      int len = gstrlen(context->methodName);
      websDecodeUrl(context->methodName, context->methodName, len);
   }
   ResetStringBuf(context->currentData);
}

static void OnStartParams(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr)
{

   /* intentionally left (almost) empty. */

}

static void OnEndParams(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr)
{
   /* intentionally left (almost) empty. */

}

static void OnStartParam(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr)
{
   /* intentionally left (almost) empty. */

}

static void OnEndParam(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr)
{
   /* intentionally left (almost) empty. */

}

static void OnStartStruct(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr)
{
   RpcNode* structNode = CreateStructNode(NULL);
   if (NULL != structNode)
   {
      AddTreeNode(context->paramList, structNode);
   }

}

static void OnEndStruct(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr)
{
   CloseTreeNode(context->paramList);
}

static void OnStartValue(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr)
{
   /* intentionally left (almost) empty. */

}

static void OnEndValue(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr)
{
   if (NULL != context)
   {
      if (GetStringLength(context->currentData) > 0)
      {
         AddValue(context, kString);
      }
   }

}

static void OnStartMember(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr)
{
   /* YOU might think that we would want to create a StructMemberNode here,
    * but we don't. We need to know the name of the member before we can
    * create a member node, so we don't create the member node until the close
    * handler for the 'name' tag.
    */

}

static void OnEndMember(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr)
{
   /* NOTE that the node that we are closing was actually created as part of
    * handling the member name, since we need the member name to create a
    * struct member node.
    */
   CloseTreeNode(context->paramList);
}

static void OnStartMemberName(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr)
{
   /* intentionally left (almost) empty. */
}

static void OnEndMemberName(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr)
{
   RpcNode* memberNode  = NULL;
   char_t* memberName = GetString(context->currentData);
   if (gstrchr(memberName, '%'))
   {
      /* if they have replaced any characters as hex entities, decode them */
      int len = gstrlen(memberName);
      websDecodeUrl(memberName, memberName, len);
   }


   memberNode = CreateStructMemberNode(memberName, NULL);
   a_assert(NULL != memberNode);
   if (NULL != memberNode)
   {
      AddTreeNode(context->paramList, memberNode);
   }

   ResetStringBuf(context->currentData);

}

static void OnStartArray(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr)
{
   RpcNode* arrayNode = CreateArrayNode(NULL);
   if (NULL != arrayNode)
   {
      AddTreeNode(context->paramList, arrayNode);
   }

}

static void OnEndArray(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr)
{
   CloseTreeNode(context->paramList);
}

static void OnStartData(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr)
{
   /* intentionally left (almost) empty. */

}

static void OnEndData(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr)
{

   /* intentionally left (almost) empty. */
}

/*
 * The 'ValueType functions handle all the i4/integer/double/etc tags.
 */
static void OnStartValueType(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr)
{

   /* intentionally left (almost) empty. */
}




static void OnEndValueType(RpcParseContext* context, const XML_Char* name, 
 const XML_Char** attr)
{
   sym_t* symbol = NULL;
   /*char_t* value = NULL; warning: unused variable `value'*/
   enum RpcDataType type  = kNoType;


   symbol = symLookup(sTagTypes, (char_t*) name);
   if (NULL != symbol)
   {
      type = symbol->content.value.integer;
      AddValue(context, type);
   }
}



static void TreeString(StringBuf* str, RpcTree* tree)
{
   RpcArray* array = NULL;

   a_assert(tree);
   a_assert(str);

   if (tree && str)
   {
      AddString(str, T("<params>\n"));
      AddString(str, T("<param>\n"));
      array = GetArrayNodePtr(tree->root);
      if (NULL != array)
      {
         switch (array->size)
         {
            case 0:
            /* do nothing if we are empty! */
            break;

            case 1:
            NodeString(str, GetNextArrayNode(tree->root, NULL));
            break;

            default:
            NodeString(str, tree->root);
            break;
         }
      }
      AddString(str, T("</param>\n"));
      AddString(str, T("</params>\n"));
   }
}





static void NodeString(StringBuf* str, RpcNode* node)
{
   enum RpcNodeType type;

   a_assert(node);
   a_assert(str);

   if (node && str)
   {
      type = node->nodeType;

      AddString(str, T("<value>"));

      switch (type)
      {
         case kValueNode:
         ValueString(str, node);
         break;

         case kStructNode:
         StructString(str, node);
         break;

         case kArrayNode:       
         ArrayString(str, node);
         break;

         default:
         a_assert(0);
         break;
      }

      AddString(str, T("</value>\n"));

   }
}



static void  ValueString(StringBuf* str, RpcNode* valueNode)
{
   RpcValue* val = GetValueNodePtr(valueNode);
   a_assert(val);
   a_assert(str);
   if (val && str)
   {
      AddString(str, T("<"));
      AddString(str, kXmlRpcTypes[val->type]);
      AddString(str, T(">"));
      AddString(str, val->value);
      AddString(str, T("</"));
      AddString(str, kXmlRpcTypes[val->type]);
      AddString(str, T(">"));
   }

}


static void MemberString(StringBuf* str, RpcNode* member)
{
   /*
   RpcStructMember* member = GetStructMemberNodePtr(memberNode);
   */
   a_assert(member);
   a_assert(str);
   if (member && str)
   {
      AddString(str, T("<member>\n<name>"));
      AddString(str, member->name);
      AddString(str, T("</name>\n"));      
      NodeString(str, member);
      AddString(str, T("</member>\n"));
   }
}


static void StructString(StringBuf* str, RpcNode* structure)
{
   RpcNode* member = NULL;
   if (structure && string)
   {
      AddString(str, T("\n<struct>\n"));
      member = GetNextMember(structure, member);
      while (member != NULL)
      {
         MemberString(str, member);
         /*NodeString(str, member);*/
         member = GetNextMember(structure, member);
      }
      AddString(str, T("</struct>\n"));
   }
}

static void ArrayString(StringBuf* str, RpcNode* arrayNode)
{
   RpcArray* array = GetArrayNodePtr(arrayNode);
   RpcNode* current = NULL;
   if (array && str)
   {
      AddString(str, T("\n<array>\n<data>\n"));
      current = array->head;
      while (NULL != current)
      {
         NodeString(str, current);
         current = current->next;
      }
      AddString(str, T("</data>\n</array>\n"));
   }
}




static int xmlRpcTestFunc(webs_t wp, RpcTree* params, RpcTree* result)
{
   StringBuf* stringBuf = CreateStringBuf(256);
   if (NULL != stringBuf)
   {
      AddTreeNode(result, CreateIntegerValueNode(NULL, 1));

      AddTreeNode(result, CreateDoubleValueNode(NULL, 3.14159));

      AddTreeNode(result, CreateStringValueNode(NULL, T("Have a banana, Pirate!")));

      TreeString(stringBuf, params);
      /*OutputDebugString(GetString(stringBuf));*/
      DestroyStringBuf(stringBuf);
   }

   return kDmfSuccess;
}


#endif /* #define qXmlRpc */
