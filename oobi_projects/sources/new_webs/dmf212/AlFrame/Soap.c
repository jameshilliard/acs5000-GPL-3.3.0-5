/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: Soap.c,v 1.2 2006/01/10 23:51:50 erik Exp $ */

#include "wsIntrn.h"
/* include the expat headers. */
#include "xmlparse.h"

#include "Soap.h"
#ifdef qSoap

#include "Error.h"
#include "ErrorCodes.h"
#include "GetSet.h"
#include "Localize.h"
#include "Locals.h"
#include "ParamCodes.h"
#include "StringBuf.h"

/* Now, we re-include paramcodes to build the array of strings...*/
#define qSoapTypes
#include "ParamCodes.h"

/*
#define qSoapTest
*/
#ifdef qSoapTest
#include "SoapTest.c"
#endif

#include "XmlRpcFuncs.h"


#define kNamespaceSeparator   T("|")
#define kDmfNamespace   T("http://www.artlogic.com/dmf/")
#define kDmfNamespaceTag   T("d")

#define kSoapEnv  T("http://schemas.xmlsoap.org/soap/envelope/")
#define kSoapEnc  T("http://schemas.xmlsoap.org/soap/encoding/")
#define kXsi      T("http://www.w3.org/2001/XMLSchema-instance")
#define kXsd      T("http://www.w3.org/2001/XMLSchema")
#define kResponseProlog T("<?xml version=\"1.0\"?>\n <SOAP-ENV:Envelope\n") \
   T("xmlns:SOAP-ENV=\"") kSoapEnv T("\"\n") \
   T("SOAP-ENV:encodingStyle=\"") kSoapEnc  T("\"\n") \
   T("xmlns:SOAP-ENC=\"") kSoapEnc T("\"\n") \
   T("xmlns:xsi=\"") kXsi T("\"\n") \
   T("xmlns:xsd=\"") kXsd T("\">\n") \
   T("<SOAP-ENV:Body>\n")

#define kResponseEpilog T("</SOAP-ENV:Body>\n</SOAP-ENV:Envelope>\n")



/* "SOAP-ENV:Envelope"   */
#define kEnvelopeTag kSoapEnv kNamespaceSeparator   T("Envelope")
/* "SOAP-ENV:Header"   */   
#define kHeaderTag kSoapEnv kNamespaceSeparator   T("Header")
/* SOAP-ENV:Body" */   
#define kBodyTag  kSoapEnv kNamespaceSeparator   T("Body")
/* "SOAP-ENC:Array" */   
#define kArrayTag kSoapEnc kNamespaceSeparator   T("Array")
/* "SOAP-ENC:arrayType"   */
#define kArrayType kSoapEnc kNamespaceSeparator   T("arrayType")
/* "xsi:type" */   
#define kTypeAttribute  kXsi kNamespaceSeparator   T("type")

#define kParmKey  T("key")
#define kParmIndex   T("index")
#define kParmValue T("value")


/*
 * We map from method names to RpcFunc pointers using this symbol
 * table. Created in our init() function, destroyed in our close function. 
 * We populate it with framework-defined functions in the init function, and
 * if user-supplied code would like to add additional functions, it may add
 * them in the dmfUserPostInit() function.
 */
static sym_fd_t sMethods = -1;

/*
 * We create a symbol table that lets us map from "xsd:int" style type codes
 * as carried in attributes back to our internal enumerated type codes.
 */
static sym_fd_t sTypes = -1;

static RpcParseContext* sContext = NULL;
static StringBuf* sFuncOutput = NULL;
static RpcTree* sFuncResultTree = NULL;
static XML_Parser sParser = NULL;

/*
 * function to return a string that contains a standard SOAP error message,
 * parameterized so we can send back custom fault codes and fault strings.
 */
static void dmfSoapFault(StringBuf* str, int code, const char_t* faultString);

/*
 * helper function to write an xml-rpc payload back to the user. Handles all
 * of the HTTP headers, etc.
 */
static void dmfSoapResponse(webs_t wp, int code, char_t* response, SInt32 responseLength);


static int dmfSoapGet(webs_t wp, RpcTree* params, RpcTree* output);
static int dmfSoapSet(webs_t wp, RpcTree* params, RpcTree* output);
#ifdef qLocalize
static int dmfSoapGetLabel(webs_t wp, RpcTree* params, RpcTree* output);
#endif

/*
 * utility function to convert the output of the SOAP method (contained in the
 * RpcTree 'source' into text, appended to the 'str' string buffer.
 */
static void FormatSoapResponse(char_t* methodName, StringBuf* str, 
 RpcTree* source);

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
static void ArrayString(StringBuf* str, RpcNode* arrayNode);
static void StructString(StringBuf* str, RpcNode* structure);
/*
 * Expat begin element handler. See the expat documentation.
 */
static void dmfSoapStartHandler(void* user, const XML_Char* name, const XML_Char** attr);

/*
 * Expat end element handler, See the expat documentation.
 */
static void dmfSoapEndHandler(void* user, const XML_Char* name);

/*
 * Expat character data handler. The Expat docs note that the character data
 * that's arriving is NOT zero-terminated, and that a single run of characters
 * in the source may result in multiple calls to this function.
 */
static void dmfSoapCharacterHandler(void* user, const XML_Char* name, int len);





extern int dmfInitSoap(void)
{

   int typeCount = sizeof(kSoapTypes) / sizeof(kSoapTypes[0]);
   int i = 0;
   
   sMethods = symOpen(16);
   /*
    * add our builtin SOAP methods...
    */
   dmfSoapAddFunction(kDmfNamespace, "echo", dmfRpcEchoFunc);
   
   dmfSoapAddFunction(kDmfNamespace, "get", dmfSoapGet);
   dmfSoapAddFunction(kDmfNamespace, "set", dmfSoapSet);
#ifdef qLocalize
   dmfSoapAddFunction(kDmfNamespace, "getLabel", dmfSoapGetLabel);
#endif
   dmfSoapAddFunction(kDmfNamespace, "getErrors", dmfXmlRpcGetErrors);
   
   sContext = dmfCreateRpcContext();

   /*
    * create and populate a symbol table to map 'xsd:int' style type
    * information back into our internal enumerated type codes.
    */
   sTypes = symOpen(typeCount);
   for (i = 0; i < typeCount; ++i)
   {
      if (gstrlen(kSoapTypes[i]) > 0)
      {
         symEnter(sTypes, (char_t*) kSoapTypes[i], valueInteger(i), 0);
      }
   }
   
   return kDmfSuccess;
}

extern void dmfCloseSoap(void)
{
   symClose(sMethods);
   symClose(sTypes);
   dmfDestroyRpcContext(sContext);
}


extern void dmfSoapHandler(webs_t wp, char_t *path, char_t *query)
{
   char_t* response = NULL;
   int funcRetval = kDmfError;
   int errorCode = kDmfSuccess;
   int httpCode = 500;
   SInt32 responseLength = 0;
   char_t* errorString = NULL;
   char_t* msg = query;
   int queryLen = gstrlen(query);
   
#ifdef qSoapTest
   msg = kTest2;
   queryLen = gstrlen(msg);
#endif 

   if (NULL == sFuncOutput)
   {
      sFuncOutput = CreateStringBuf(512);
   }

   if (NULL == sFuncResultTree)
   {
      sFuncResultTree = CreateTree(16);
   }

   sParser = XML_ParserCreateNS(NULL, kNamespaceSeparator[0]);
   if ((NULL != sContext) && (NULL != sParser))
   {
      XML_SetUserData(sParser, sContext);
      XML_SetElementHandler(sParser, dmfSoapStartHandler, 
       dmfSoapEndHandler);
      XML_SetCharacterDataHandler(sParser, dmfSoapCharacterHandler);
   }
	a_assert(websValid(wp));

   if ((NULL != sFuncOutput) && (XML_Parse(sParser, msg, queryLen, 1)) )
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
         FormatSoapResponse(sContext->methodName, sFuncOutput, sFuncResultTree);
         httpCode = 200;
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
         dmfSoapFault(sFuncOutput, errorCode, errorString);         
         bfreeSafe(B_L, errorString);
      }
   }
   else
   {
      /*
       * there was a parsing error -- return the error string as given by
       * expat.
       */
      dmfSoapFault(sFuncOutput, kDmfXmlRpcParse, 
       XML_ErrorString(XML_GetErrorCode(sParser)) );
   }

   response = GetString(sFuncOutput);        
   responseLength = GetStringLength(sFuncOutput);
   dmfSoapResponse(wp, httpCode, response, responseLength);


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





extern int dmfSoapAddFunction(char_t* nameSpace, char_t* methodName, RpcFunc func)
{
   int retval = kDmfError;
   char_t* fullName = NULL;

   
   if (-1 != sMethods)
   {
      if (gstrlen(nameSpace) > 0)
      {
         /*
          * We concatenate the namespace and the methodname to create the
          * fully-qualified method name (as expat will give it to us)
          */
         fmtAlloc(&fullName, BUF_MAX, T("%s%s%s"), nameSpace, 
          kNamespaceSeparator, methodName);
      }
      else
      {
         fullName = bstrdup(B_L, methodName);
      }
      if (NULL != symEnter(sMethods, fullName, valueInteger((long) func), 0))
      {
         retval = kDmfSuccess;
      }
      bfreeSafe(B_L, fullName);
   }
   return retval;

}



static void dmfSoapFault(StringBuf* str, int code, const char_t* faultString)
{
   AddString(str, kResponseProlog);
   AddString(str, T("<SOAP-ENV:Fault>\n"));
   AddString(str, T("<faultcode>Client."));
   if (code < 0 || code > kDmfInternalError)
   {
      code = kDmfInternalError;
   }
   AddString(str, dmfGetErrorName(code));
   AddString(str, T("</faultcode>\n<faultstring>"));
   AddString(str, faultString);
   AddString(str, T("</faultstring>\n</SOAP-ENV:Fault>"));
   AddString(str, kResponseEpilog);

}

static void dmfSoapResponse(webs_t wp, int code, char_t* response, SInt32 responseLength)
{

	websWrite(wp, T("HTTP/1.0 %d "), code);
   if (200 == code)
   {
      websWrite(wp, T("OK"));
   }
   else
   {
      websWrite(wp, T("Server Error"));
   }
   websWrite(wp, T("\nConnection: close\n"));
	websWrite(wp, T("Content-Type: text/xml; charset=\"utf-8\"\n"));
   websWrite(wp, T("Content-Length: %d\n"), responseLength);

   /*
    *	By license terms the following line of code must not be modified
    */
	websWrite(wp, T("Server: %s\r\n"), WEBS_NAME);
	websWrite(wp, T("\n"));
   websWriteBlock(wp, response, responseLength);

   websDone(wp, code);
}


static int dmfGetAttributes(sym_fd_t attributes, const XML_Char** attr)
{
   int i = 0;
   
   if ((attributes != -1) && (NULL != attr))
   {
      /* 
       * the attrbutes are packaged as pairs of strings.
       */
      for (i = 0; (attr[i] != NULL); i += 2)
      {
         symEnter(attributes, (char_t*) attr[i], 
          valueString((char_t*)attr[i + 1], VALUE_ALLOCATE), 0);
      }      
   }

   /*
    * return the number of attributes that we actually found.
    */
   return i / 2;

}

static enum RpcDataType dmfGetTypeCode(const char_t* type)
{
   enum RpcDataType retval = kNoType;
   sym_t* sym = NULL;

   sym = symLookup(sTypes, (char_t*) type);
   if (NULL != sym)
   {
      retval = sym->content.value.integer;
   }
   return retval;
}


static void BodyStartHandler(RpcParseContext* context, char_t* name, 
 sym_fd_t attributes)
{
   RpcTree* paramList = context->paramList;
   RpcNode* newNode = NULL;
   sym_t* sym = NULL;
   char_t* type = NULL;
   
   if (NULL == context->methodName)
   {
      context->methodName = bstrdup(B_L, name);
   }
   if (NULL != context->currentTag)
   {
      /* we've got an open tag that we didn't know how to handle. If we get 
       * here and there's still text in the currentTag, it means that we're 
       * processing XML that looks like:
       * <tag1>
       *    <tag2>something here...</tag2>
       *
       * ...so we create a new struct node named 'tag1' and clear out the
       * currentTag.
       */
      AddTreeNode(paramList, CreateStructNode(context->currentTag));
      bfreeSafe(B_L, context->currentTag);
      context->currentTag = NULL;
   }

   if (-1 != attributes)
   {

      /* see if there's a type attribute...*/
      sym = symLookup(attributes, kTypeAttribute);
      if (sym != NULL)
      {
         /* if present, this will be something like "xsd:int"*/
         type = sym->content.value.string;
         /* convert it back to our internal code... */
         context->currentType = dmfGetTypeCode(type);
      }
   }
   if (kSoapArray == context->currentType)
   {
      /* we're receiving a new array -- first, create an array
       * container to hold the elements that will be arriving...
       */
      newNode = CreateArrayNode((char_t*)name);
      AddTreeNode(paramList, newNode);
      /*
       * then, look for the type of the elements that are
       * coming... (we need to get rid of the array dimensions)
       */
      /* !!! */
   }
   else
   {
      context->currentTag = bstrdup(B_L, name);
   }
}
      




static void dmfSoapStartHandler(void* user, const XML_Char* name, const XML_Char** attr)
{
   RpcParseContext* context = (RpcParseContext*) user;
#if 1
#else
   RpcTree* paramList = context->paramList; //warning: unused variable `paramList'
   RpcNode* newNode = NULL; //warning: unused variable `newNode'
   sym_t* sym = NULL;// warning: unused variable `sym'
   char_t* type = NULL; //warning: unused variable `type'
#endif
   enum SoapState nextState = context->currentState;
   /*enum RpcDataType typeCode = kNoType; warning: unused variable `typeCode' */
   int stateChange = 0;
   sym_fd_t attributes = -1;
   int attributeCount = 0;

   if (0 == gstricmp((char_t*) name, kEnvelopeTag))
   {
      nextState = kEnvelope;
   }
   else if (0 == gstricmp((char_t*) name, kHeaderTag))
   {
      nextState = kHeader;
   }
   else if (0 == gstricmp((char_t*) name, kBodyTag))
   {
      nextState = kBody;
   }

   stateChange = (nextState != context->currentState);

   if (stateChange)
   {
      /* change to the next state. */
      context->currentState = nextState;
   }
   else
   {
      switch (context->currentState)
      {
         case kEnvelope:
         case kHeader:
         /* for the time being, we discard anything outside the body.*/
         break;


         case kBody:
         
         if (NULL == context->methodName)
         {
            /* The first tag inside the body will be the method name -- we cache 
             * this so we can correctly format the response.
             */
            context->methodName = bstrdup(B_L, (char_t*)name);
         }
         if (NULL != *attr)
         {
            attributes = symOpen(16);
            attributeCount = dmfGetAttributes(attributes, attr);
         }
#if 1
         BodyStartHandler(context, (char_t*) name, attributes);
#else
         if (attributeCount > 0)
         {
            /* see if there's a type attribute...*/
            sym = symLookup(attributes, kTypeAttribute);
            if (sym != NULL)
            {
               /* if present, this will be something like "xsd:int"*/
               type = sym->content.value.string;
               /* convert it back to our internal code... */
               context->currentType = dmfGetTypeCode(type);
               if (kSoapArray == context->currentType)
               {
                  /* we're receiving a new array -- first, create an array
                   * container to hold the elements that will be arriving...
                   */
                  newNode = CreateArrayNode((char_t*)name);
                  AddTreeNode(paramList, newNode);
                  /*
                   * then, look for the type of the elements that are
                   * coming... (we need to get rid of the array dimensions)
                   */
                  /* !!! */
               }
               else
               {
                  newNode = CreateStructMemberNode((char_t*)name, NULL);
                  AddTreeNode(paramList, newNode);
               }
            }
            else
            {
               context->currentType = kNoType;
            }
            symClose(attributes);
         }
         else
         {
            /* assume that this is a new structure... */
            newNode = CreateStructNode((char_t*) name);
            AddTreeNode(paramList, newNode);
         }
#endif    
         
         
         break;

         default:
         a_assert(0);
         break;
      }
   }
   context->started = 1;
   ResetStringBuf(context->currentData);
}







static void dmfSoapEndHandler(void* user, const XML_Char* name)
{
   RpcParseContext* context = (RpcParseContext*) user;
   RpcNode* newNode = NULL;
   RpcTree* tree = context->paramList;
   StringBuf* text = context->currentData;

   if (kBody == context->currentState)
   {
      if (GetStringLength(text) > 0)
      {
         if (NULL != context->currentTag)
         {
            newNode = CreateStructMemberNode((char_t*) name, NULL);
            AddTreeNode(tree, newNode);
            bfreeSafe(B_L, context->currentTag);
            context->currentTag = NULL;
            newNode = NULL;
         }
         
         /* if there is anything in the text buffer, we use it to create a value
          * node of the correct type.
          */
         newNode = CreateValueNode((char_t*) name,  GetString(text), context->currentType);
         AddTreeNode(tree, newNode);
         CloseTreeNode(tree); /*!!!*/
         ResetStringBuf(text);
      }

      else
      {
         /* Close the tree node. */
         CloseTreeNode(tree);    
      }
   }
   context->started = 0;
}

static void dmfSoapCharacterHandler(void* user, const XML_Char* name, int len)
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
      if (context->started)
      {
         AddBufToString(context->currentData, name, len);
      }
   }
}


static void TreeString(StringBuf* str, RpcTree* tree)
{
   RpcArray* array = NULL;
   RpcNode* node = NULL;
   /*RpcNode* data = NULL; warning: unused variable `data'*/

   a_assert(tree);
   a_assert(str);

   if (tree && str)
   {
      array = GetArrayNodePtr(tree->root);
      if (NULL != array)
      {
         switch (array->size)
         {
            case 0:
            /* do nothing if we are empty! */
            break;

            case 1:
            /*
             * If there's a single item, we change its name to 'Result' and
             * send it back as the response.
             */
            node = GetNextArrayNode(tree->root, NULL);
            
            if (kValueNode == node->nodeType)
            {
               /* there's a single value coming back -- we change its name
                * to 'Result' and return it as the response
                */
               bfreeSafe(B_L, node->name);
               node->name = bstrdup(B_L, T("Result"));
               NodeString(str, node);
            }
            else
            {
               /* there's a struct or array that needs to be returned inside
                * the outer 'Result' struct.
                */
               AddString(str, T("<Result>\n"));
               NodeString(str, node);
               AddString(str, T("</Result>\n"));
            }
            break;

            default:
            AddString(str, T("<Result>\n"));
            NodeString(str, tree->root);
            AddString(str, T("</Result>\n"));
            break;
         }
      }
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
   }
}


static void ValueString(StringBuf* str, RpcNode* valueNode)
{
   RpcValue* val = GetValueNodePtr(valueNode);
   a_assert(val);
   a_assert(str);
   if (val && str)
   {
      AddString(str, T("<"));
      AddString(str, valueNode->name);
      AddString(str, T(" xsi:type=\""));
      AddString(str, kSoapTypes[val->type]);
      AddString(str, T("\">"));
      AddString(str, val->value);
      AddString(str, T("</"));
      AddString(str, valueNode->name);
      AddString(str, T(">\n"));
   }
}

static void ArrayString(StringBuf* str, RpcNode* arrayNode)
{
   RpcArray* array = GetArrayNodePtr(arrayNode);
   RpcValue* value = NULL;
   RpcNode* item = NULL;
   enum RpcDataType type = kNoType;
   char_t arrayLen[32];
   StringBuf* tempString = CreateStringBuf(512);
   
   a_assert(str);
   a_assert(arrayNode);

   if (str && arrayNode && array)
   {
      /*
       * first, scan the array, seeing if all the elements are of the same
       * type. We also output their data into a temporary stringbuf to avoid
       * scanning the array twice.
       */
      item = GetNextArrayNode(arrayNode, NULL);
      if (item != NULL)
      {
         value = GetValueNodePtr(item);
         if (value != NULL)
         {
            type = value->type;
         }
         RenameNode(item, T("Item"));
         /*
          * add this item to the temp stringbuf
          */
         NodeString(tempString, item);
         
         item = GetNextArrayNode(arrayNode, item);
         while (item != NULL)
         {
            /*
             * we use the 'kNoType' type as a flag that the array either
             * carries other complex types, or that it carries mixed types.
             */
            if (kNoType != type)
            {
               value = GetValueNodePtr(item);
               if (type != value->type)
               {
                  type = kNoType;
               }
            }
            RenameNode(item, T("Item"));
            NodeString(tempString, item);
            item = GetNextArrayNode(arrayNode, item);
         }
      }
      /*
       * now that we're here, we have a temp string holding the contents of
       * the array, and knowledge of the type(s) held in the array. Let's
       * create the opening tag for the array...
       */
      AddString(str, T("<"));
      AddString(str, arrayNode->name);
      AddString(str, T(" SOAP-ENC:arrayType=\""));
      if (type != kNoType)
      {
         AddString(str, kSoapTypes[type]);
      }
      else
      {
         AddString(str, T("xsd:ur-type"));
      }
      AddString(str, T("["));
      fmtStatic(arrayLen, 32, "%d", array->size);
      AddString(str, arrayLen);
      AddString(str, T("]\" xsi:type=\"SOAP-ENC:Array\">\n"));
      AddString(str, GetString(tempString));
      AddString(str, T("</"));
      AddString(str, arrayNode->name);
      AddString(str, T(">\n"));

      DestroyStringBuf(tempString);

   }
   

}

static void StructString(StringBuf* str, RpcNode* structure)
{
   RpcNode* member = NULL;
   
   a_assert(NULL != str);
   a_assert(NULL != structure);

   if (str && structure)
   {
      AddString(str, T("<"));
      AddString(str, structure->name);
      AddString(str, T(">\n"));
      /* 
       * add all of the structure's members...
       */
      member = GetNextMember(structure, NULL);
      while (NULL != member)
      {
         NodeString(str, member);
         member = GetNextMember(structure, member);
      }
      /* and close out the struct. */

      AddString(str, T("</"));
      AddString(str, structure->name);
      AddString(str, T(">\n"));
   }

}



static int dmfSoapGet(webs_t wp, RpcTree* params, RpcTree* output)
{
   char_t* key = NULL;
   char_t* index = NULL;
   char_t* val = NULL;
   int parmIndex = -1;
   RpcNode* node = NULL;
   RpcNode* parmStruct = NULL;
   int retval = kDmfError;

   dmfClearErrors(wp);

   parmStruct = GetNextArrayNode(params->root, NULL);

   node = GetNextMember(parmStruct, NULL);
   while (node != NULL)
   {

      if (0 == gstrcmp(kParmKey, node->name))
      {
         GetValueRaw(node, &key);
      }
      else if (0 == gstrcmp(kParmIndex, node->name))
      {
         GetValueRaw(node, &index);
      }
      node = GetNextMember(parmStruct, node);

   }

   if (NULL != key)
   {
      if (dmfKeyIsLocalVar(key))
      {
         retval = dmfGetLocal(key, index, &val);
         if (kDmfSuccess == retval)
         {
            /* we treat all server variables as if they're a string. */
            node = CreateValueNode(key, val, kString);
            if (node != NULL)
            {
               AddTreeNode(output, node);
            }
            else
            {
               retval = kDmfInternalError;
               dmfAddError(wp, retval, key, NULL, NULL);
            }
         }
         else
         {
            dmfAddError(wp, retval, key, NULL, NULL);
         }
         bfreeSafe(B_L, val);
      }
      else
      {  
         parmIndex = dmfFindParam(key);
         if (parmIndex > -1)
         {
            dmfRpcGetParam(wp, parmIndex, index, output);
            retval = kDmfSuccess;
         }
         else
         {
            retval = kDmfParameterName;
            dmfAddError(wp, retval, key, NULL, NULL);
         }
      }
   }
   

   bfreeSafe(B_L, key);
   bfreeSafe(B_L, index);
   
   return retval;
}


static int dmfSoapSet(webs_t wp, RpcTree* params, RpcTree* output)
{
   char_t* key = NULL;
   char_t* index = NULL;
   char_t* value = NULL;
   /*int parmIndex = -1; warning: unused variable `parmIndex'*/
   RpcNode* parmStruct = NULL;
   RpcNode* node = NULL;
   int retval = kDmfError;

   dmfClearErrors(wp);

   parmStruct = GetNextArrayNode(params->root, NULL);
   node = GetNextMember(parmStruct, NULL);
   while (NULL != node)
   {
      if (0 == gstrcmp(kParmKey, node->name))
      {
         GetValueRaw(node, &key);
      }
      else if (0 == gstrcmp(kParmIndex, node->name))
      {
         GetValueRaw(node, &index);
      }
      else if (0 == gstrcmp(kParmValue, node->name))
      {
         GetValueRaw(node, &value);
      }
      node = GetNextMember(parmStruct, node);
   }
   if (NULL == key)
   {
      retval = kDmfParameterMissing;
   }
   else if (NULL == value)
   {
      retval = kDmfValueMissing;
   }
   else
   {
      /* for the time being, we force-set the new value -- ignore the
       * request number
       */
      /* !!! bypass access level checking*/
      retval = dmfSetParam(key, index, value, 0, 256); 

      if (kDmfSuccess == retval)
      {
         /* don't forget to make this change permanent! */
         char_t urlQuery[255]; // magic number!!! (plus, this is bogus, here)
         retval = dmfCommitParam(key, 0, urlQuery);
         if (kDmfSuccess == retval)
         {
            /* and add our return value to the results. */
            AddTreeNode(output, CreateStringValueNode(NULL, 
             dmfGetErrorName(retval)));
         }
      }
   }
   bfreeSafe(B_L, key);
   bfreeSafe(B_L, index);
   bfreeSafe(B_L, value);

   if (kDmfSuccess != retval)
   {
      dmfAddError(wp, retval, key, index, value);
   }           


   return retval;
}

#ifdef qLocalize
static int dmfSoapGetLabel(webs_t wp, RpcTree* params, RpcTree* output)
{
   char_t* key = NULL;
   char_t* label = NULL;
   /*int parmIndex = -1; warning: unused variable `parmIndex'*/
   RpcNode* node = NULL;
   RpcNode* parmStruct = NULL;
   int retval = kDmfError;

   dmfClearErrors(wp);

   parmStruct = GetNextArrayNode(params->root, NULL);

   node = GetNextMember(parmStruct, NULL);
   while (node != NULL)
   {

      if (0 == gstrcmp(kParmKey, node->name))
      {
         GetValueRaw(node, &key);
      }
      node = GetNextMember(parmStruct, node);
   }

   if (NULL != key)
   {
      retval = dmfGetLabel(key, &label);
      if (kDmfSuccess == retval)
      {
         AddTreeNode(output, CreateStringValueNode(NULL, label));
      }
      else
      {
         dmfAddError(wp, retval, key, NULL, NULL);
      }
      bfreeSafe(B_L, key);
   }

   return retval;
}

#endif

static void FormatSoapResponse(char_t* methodName, StringBuf* str, 
 RpcTree* source)
{
   char_t* responseTag = NULL;
   char_t* temp = NULL;
   char_t* nameSpace = gstrchr(methodName, kNamespaceSeparator[0]);
   if (NULL != nameSpace)
   {
      /*
       * if the methodname is namespace-qualified (like "nameSpace|method"),
       * we swap things around, so the methodName variable points to the
       * actual method name, and nameSpace points to the namespace part
       * (suitably NULL terminated). Below, we reformat to use the correct
       * namespace notation.
       */
      *nameSpace = '\0';
      temp = methodName;
      methodName = ++nameSpace;
      nameSpace = temp;
   }

   fmtAlloc(&responseTag, BUF_MAX, "%sResponse", methodName);

   AddString(str, kResponseProlog);
   AddString(str, T("<"));
   if (NULL != nameSpace)
   {
      AddString(str, kDmfNamespaceTag T(":"));
   }
   AddString(str, responseTag);
   if (NULL != nameSpace)
   {
      AddString(str, T(" xmlns:") kDmfNamespaceTag T("=\""));
      AddString(str, nameSpace);
      AddString(str, "\"");
   }
   AddString(str, T(">\n"));
   TreeString(str, source);
   AddString(str, T("</"));
   if (NULL != nameSpace)
   {
      AddString(str, kDmfNamespaceTag T(":"));
   }
   AddString(str, responseTag);
   AddString(str, T(">\n"));
   AddString(str, kResponseEpilog);

}


#endif   /* this MUST be the last line in this file.. */
