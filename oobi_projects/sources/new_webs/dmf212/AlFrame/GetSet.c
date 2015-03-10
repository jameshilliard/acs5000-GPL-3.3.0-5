/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: GetSet.c,v 1.2 2006/01/10 23:51:50 erik Exp $ */


#include "GetSet.h"
#include "Locals.h"
 

static int dmfIsIndexValid(Param* param, int index)
{
   int retval = kDmfNotIndexed;
   a_assert(param);
   if (param->startIndex && param->endIndex)
   {
      retval = kDmfIndexRange;
      if (index >= *(param->startIndex) && index < *(param->endIndex))
      {
         retval = kDmfSuccess;
      }
   } 
   return retval;
}


static int dmfCalculateParamAddress(Param* param, char_t* index, int* intIndex, 
 void** address)
{
   int retval = kDmfError;
   int i;

   *address = NULL;
   if (param->dataStart)
   {
      *address = param->dataStart;
      retval = kDmfSuccess;

      if (NULL != index)
      {
         i = gatoi(index);
         if (NULL != intIndex)
         {
            *intIndex = i;
         }
         retval = dmfIsIndexValid(param, *intIndex);
         if (kDmfSuccess == retval)
         {
            (char*) *address += *intIndex * param->offset;
         }
      }
   }
   return retval;
}



static int dmfQueryParam(Param* param, int request)
{
   int retval = kDmfSuccess;
   Param* p = param;

   /*
    * Look for a query function -- if we cannot find one, we continue looking
    * upwards in the parameter tree until we find one.
    */
   while (NULL != p)
   {
      if (NULL != p->queryFunc)
      {
         /* we only actually call the commit function if we need to, i.e., if
          * the request # isn't zero (aka force query) or it's a different
          * request than this parameter has already serviced.
          */
         if (request == 0 || request != p->lastQuery)
         {
            retval = (*(p->queryFunc))(p, request);
            if (request > 0)
            {
               p->lastQuery = request;
            }
         }
         break;
      }
      p = dmfGetParamPtr(p->parent);
   }

   return retval;

}

static int dmfWriteAccessOk(Param* p, int level)
{
   int retval = kDmfAccessLevel;
   while (NULL != p)
   {
      if (-1 != p->writeAccess)
      {
         if (level >= p->writeAccess)
         {
            retval = kDmfSuccess;
         }
         break;
      }
      p = dmfGetParamPtr(p->parent);
   }
   return retval;
}

static int dmfReadAccessOk(Param* p, int level)
{
   int retval = kDmfAccessLevel;
   while (NULL != p)
   {
      if (-1 != p->readAccess)
      {
         if (level >= p->readAccess)
         {
            retval = kDmfSuccess;
         }
         break;
      }
      p = dmfGetParamPtr(p->parent);
   }
   return retval;
}

extern int dmfGetParam(char_t* key, char_t* index, char_t** val, int accessLevel)
{
   int parmIndex = -1;
   int retval = kDmfParameterName;
   /*
    * key names are NOT case-sensitive! 
    */
   key = strlower(key);

   if (dmfKeyIsLocalVar(key))
   {
      retval = dmfGetLocal(key, index, val);
   }
   else
   {  
      parmIndex = dmfFindParam(key);
      if (parmIndex >= 0)
      {
         retval = dmfGetParamByIndex(parmIndex, key, index, val, accessLevel);
      }
   }
   return retval;
}

extern int dmfGetParamByIndex(int parmIndex, char_t* key, char_t* index, 
 char_t** val, int accessLevel)
{
   char_t* original = NULL;
   Param* param = NULL;
   void* address = NULL;
   int intIndex = 0;
   /*int lowerIndex = 0; ../dmf212/AlFrame/GetSet.c:157: warning: unused variable `lowerIndex'*/
   /*int upperIndex = 0; ../dmf212/AlFrame/GetSet.c:158: warning: unused variable `upperIndex'*/
   int request = dmfGetRequestNumber();

   int retval = kDmfParameterName;

   param = &(parameterList[parmIndex]);
   if (NULL != param)
   {
      retval = dmfReadAccessOk(param, accessLevel);
      if (kDmfSuccess == retval)
      {
         retval = kDmfWriteOnly;
         if (NULL != param->getFunc)
         {
            /* pre-fetch the data if needed... */
            retval = dmfQueryParam(param, request);
            if (kDmfSuccess == retval)
            {
               retval = dmfCalculateParamAddress(param, index, &intIndex, &address);
               if (kDmfSuccess == retval)
               {
                  retval = (*(param->getFunc))(key, index, &original, address, 
                   param, request);

                  if (kDmfSuccess == retval)
                  {
                     *val = bstrdup(B_L, original);
                  }

                  bfreeSafe(B_L, original);
               }
            }
         }
      }
   }
   
   return retval;

}


static int dmfPreCommitParam(Param* param, int request)
{
   int retval = kDmfSuccess;
   Param* p = param;
   /*
    * look for a pre-commit function. Keep looking upwards through the
    * parameter tree until we either hit the root or find a function.
    */
   while (NULL != p)
   {
      /* 14 Aug 02 BgP -- we lose the idea of a separate pre-commit function,
       * and instead look to the lastCommit member variable held inside the
       * parameter object to decide if we're looking at old data. We look for
       * the existence of a commitFunc to see if we should look at the
       * lastCommit variable.
       */
      if (NULL != p->commitFunc)
      {
         if (0 != request && p->lastCommit > request)
         {
            retval = kDmfOldData;
         }
         break;
      }
      

      p = dmfGetParamPtr(p->parent);
   }

   return retval;
}

extern int dmfSetParam(char_t* key, char_t* index, char_t* val, int request, 
 int accessLevel)
{
  int retval = kDmfParameterName;
  int parmIndex = -1;
  /*
   * key names are NOT case-sensitive! 
   */
   char_t* lowerKey = bstrdup(B_L, key);
   strlower(lowerKey);
   

   if (kLocalPrefix == key[0])
   {
      retval = dmfSetLocal(lowerKey, index, val, request);
   }
   else
   {
      parmIndex = dmfFindParam(lowerKey);
      if (parmIndex >= 0)
      {
         retval = dmfSetParamByIndex(parmIndex, lowerKey, index, val, request, 
          accessLevel);
      }
   }

   bfreeSafe(B_L, lowerKey);
   return retval;
}


extern int dmfSetParamByIndex(int parmIndex, char_t* key, char_t* index, 
 char_t* val, int request, int accessLevel)
{

   Param* param = NULL;
   void* address = NULL;
   int intIndex = 0;
   /*int lowerIndex = 0; ../dmf212/AlFrame/GetSet.c:269: warning: unused variable `lowerIndex'*/
   /*int upperIndex = 0; ../dmf212/AlFrame/GetSet.c:270: warning: unused variable `upperIndex'*/
   int retval = kDmfArgCount;

   a_assert(NULL != key);
   a_assert(NULL != val);

   if ((NULL != key) && (NULL != val))
   {
      param = dmfGetParamPtr(parmIndex);
      if (NULL != param)
      {
         retval = dmfWriteAccessOk(param, accessLevel);
         if (kDmfSuccess == retval)
         {
            retval = kDmfReadOnly;
            if (NULL != param->setFunc)
            {
               retval = dmfPreCommitParam(param, request);
               if (kDmfSuccess == retval)
               {
                  retval = dmfCalculateParamAddress(param, index, &intIndex, 
                   &address);
                  if (kDmfSuccess == retval)
                  {
                     retval = (*(param->setFunc))(key, index, val, address, 
                      param, request);
                  }
               }
            }
         }
      }
   }

   return retval;
}




extern int dmfCommitParam(char_t* key, int request, char_t* urlQuery)
{
   Param* param = NULL;

   int retval = kDmfSuccess;
   if (!dmfKeyIsLocalVar(key))
   {      
      retval = kDmfParameterName;
      param = dmfGetParamPtr(dmfFindParam(key));
      if (NULL != param)
      {
         retval = kDmfReadOnly;
         if (param->setFunc)
         {
            /*
             * return success if they didn't provide a commit func.
             */
         
            retval = kDmfSuccess;
            while (NULL != param)
            {
               if (NULL != param->commitFunc)
               {
                  if ((0 == request) || (request > param->lastCommit))
                  {
                     retval = (*(param->commitFunc))(param, request, urlQuery);
                     if (0 != request)
                     {
                        param->lastCommit = request;
                     }
                  }
                  break;
               }
               param = dmfGetParamPtr(param->parent);
            }
         }
      }
   }
   return retval;
}
