/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: XmlRpcFuncs.c,v 1.2 2006/01/10 23:51:50 erik Exp $ */


#include "XmlRpcFuncs.h"

#if defined(qXmlRpc) || defined(qSoap)
#include "Error.h"
#include "GetSet.h"
#include "Localize.h"
#include "Locals.h"
#include "RpcUtil.h"
#include "XmlRpc.h"


extern int dmfXmlRpcGetErrors(webs_t wp, RpcTree* params, RpcTree* result)
{
   int errorCount = dmfGetErrorCount(wp);
   int i = 0;
   int errorCode;
   char_t index[32];
   char_t* errorString = NULL;
   RpcNode* node = NULL;

   if (errorCount > 0)
   {
      node = CreateArrayNode(T("errors"));
      AddTreeNode(result, node);
      for (i = 0; i < errorCount; ++i)
      {
         node = CreateStructNode(NULL);
         AddTreeNode(result, node);
         fmtStatic(index, 32, T("%d"), i);
         dmfGetErrorString(wp, index, 0, &errorCode, &errorString);
         node = CreateStringValueNode(T("errorCode"), dmfGetErrorName(errorCode));
         AddTreeNode(result, node);

         node = CreateStringValueNode(T("errorString"), errorString);
         AddTreeNode(result, node);

         CloseTreeNode(result);
      }
      CloseTreeNode(result);

      dmfClearErrors(wp);
   }
   
   return kDmfSuccess;
   
}

#endif

#ifdef qXmlRpc



extern int dmfXmlRpcGet(webs_t wp, RpcTree* params, RpcTree* result)
{
   /* 
    * We expect this function to be called like:
    * get('parmName'[, index])
    * It returns the value of the specified parameter, or an error if the
    * parameter doesn't exist, etc.
    * Some unusual aspects of this function include:
    * 
    *  - If the requested parameter is indexed, but no index is provided, this
    *  function will return the value of each indexed value between its
    *  minIndex and its maxIndex (as an XmlRpc array)
    *
    *  - If the requested parameter is a section name, we return all the
    *  values in that section as a struct. If there are indexed values in 
    *  this section, it packages them into an array. If there are other
    *  sections under this one, it nests them as structs inside the top level
    *  struct. This means that calling get() on the topmost section of our
    *  parameter list will return the entire current state of the device in a
    *  single call! 
    */
   char_t* key = NULL;
   char_t* index = NULL;
   char_t* val = NULL;
   int parmIndex = -1;
   RpcNode* node = NULL;
   int retval = kDmfError;

   dmfClearErrors(wp);

   node = GetNextArrayNode(params->root, NULL);
   if (NULL != node)
   {
      GetValueString(node, &key);
      node = GetNextArrayNode(params->root, node);
      if (NULL != node)
      {
         GetValueRaw(node, &index);
      }
      if (dmfKeyIsLocalVar(key))
      {
         retval = dmfGetLocal(key, index, &val);
         if (kDmfSuccess == retval)
         {
            /* we treat all server variables as if they're a string. */
            node = CreateValueNode(key, val, kString);
            if (node != NULL)
            {
               AddTreeNode(result, node);
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
            dmfRpcGetParam(wp, parmIndex, index, result);
            retval = kDmfSuccess;
         }
         else
         {
            retval = kDmfParameterName;
            dmfAddError(wp, retval, key, NULL, NULL);
         }
      }
   

      bfreeSafe(B_L, key);
      bfreeSafe(B_L, index);
   }

   return retval;
}


extern int dmfXmlRpcSet(webs_t wp, RpcTree* params, RpcTree* result)
{
   char_t* key = NULL;
   char_t* index = NULL;
   char_t* value = NULL;
   /*int parmIndex = -1; warning: unused variable `parmIndex'*/
   RpcNode* node = NULL;
   int retval = kDmfError;

   dmfClearErrors(wp);

   node = GetNextArrayNode(params->root, NULL);
   if (NULL != node)
   {
      GetValueString(node, &key);
      node = GetNextArrayNode(params->root, node);
      if (NULL != node)
      {
         /* assume that they are sending:
          * set(parmName, newValue);
          */
         GetValueRaw(node, &value);
         node = GetNextArrayNode(params->root, node);
         if (NULL != node)
         {
            /* if there was a third parameter, then they were really sending:
             * set(parmName, index, newValue);
             */
            index = value;
            GetValueRaw(node, &value);
         }
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
               AddTreeNode(result, CreateStringValueNode(NULL, 
                dmfGetErrorName(retval)));
            }
         }
         /* a separate if() statement, since the value of retval may have 
          * changed inside the above if().
          */
         if (kDmfSuccess != retval)
         {
            dmfAddError(wp, retval, key, index, value);
         }           
      }
      bfreeSafe(B_L, key);
      bfreeSafe(B_L, index);
      bfreeSafe(B_L, value);
   }
   return retval;     

}

#ifdef qLocalize
extern int dmfXmlRpcGetLabel(webs_t wp, RpcTree* params, RpcTree* result)
{
   int retval = kDmfError;


   RpcNode* node = NULL;
   char_t* key = NULL;
   char_t* label = NULL;

   dmfClearErrors(wp);
   node = GetNextArrayNode(params->root, NULL);
   if (NULL != node)
   {
      GetValueString(node, &key);
      retval = dmfGetLabel(key, &label);
      if (kDmfSuccess == retval)
      {
         AddTreeNode(result, CreateStringValueNode(NULL, label));
      }
      bfreeSafe(B_L, key);      
   }
   return retval;
}
#endif




#endif /* #ifdef qXmlRpc */
