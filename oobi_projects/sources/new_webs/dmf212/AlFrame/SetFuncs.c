/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: SetFuncs.c,v 1.2 2006/01/10 23:51:50 erik Exp $ */


#include <limits.h>

#include "SetFuncs.h"



#define mRangeCheck(p, val) (p->upperLimit > p->lowerLimit) ? \
 ((val < p->lowerLimit) ? kDmfDataLowerRange : \
 ((val >= p->upperLimit) ? kDmfDataUpperRange : kDmfSuccess)) : \
 kDmfSuccess


extern int dmfSetInt32(const char_t* key, const char_t* index, char_t* value, 
  void* rawData, Param* param, int request)
{
   int retval = kDmfError;
   int intVal = gatoi(value);
   if (NULL != rawData)
   {
      retval = mRangeCheck(param, intVal);
      if (kDmfSuccess == retval)
      {
         *((SInt32*) rawData) = (SInt32) intVal;
      }
   }
   return retval;
}

extern int dmfSetInt16(const char_t* key, const char_t* index, char_t* value, 
  void* rawData, Param* param, int request)
{
   int retval = kDmfError;
   int intVal = gatoi(value);
   int rangeCheck = (param->upperLimit > param->lowerLimit);
   if (NULL != rawData)
   {
      retval = kDmfSuccess;
      /* whether there's a separate limit for this parameter or not, we need
       * to see if it's in a valid range for the data type..
       */
      if ((intVal > SHRT_MAX) || (rangeCheck && (intVal >= param->upperLimit)) )
      {
         retval = kDmfDataUpperRange;
      }
      if ((intVal < SHRT_MIN) || (rangeCheck && ((intVal < param->lowerLimit))) ) 
      {
         retval = kDmfDataLowerRange;
      }

      if (kDmfSuccess == retval)
      {
         *((SInt16*) rawData) = (SInt16) intVal;
      }
   }
   return retval;
}


extern int dmfSetInt8(const char_t* key, const char_t* index, char_t* value, 
  void* rawData, Param* param, int request)
{
   int retval = kDmfError;
   int intVal = gatoi(value);
   int rangeCheck = (param->upperLimit > param->lowerLimit);
   if (NULL != rawData)
   {
      retval = kDmfSuccess;
      /* whether there's a separate limit for this parameter or not, we need
       * to see if it's in a valid range for the data type..
       */
      if ((intVal > SCHAR_MAX) || (rangeCheck && (intVal >= param->upperLimit)) )
      {
         retval = kDmfDataUpperRange;
      }
      if ((intVal < SCHAR_MIN) || (rangeCheck && ((intVal < param->lowerLimit))) ) 
      {
         retval = kDmfDataLowerRange;
      }

      if (kDmfSuccess == retval)
      {
         *((SInt8*) rawData) = (SInt8) intVal;
      }
   }
   return retval;
}


extern int dmfSetUInt32(const char_t* key, const char_t* index, char_t* value, 
  void* rawData, Param* param, int request)
{
   int retval = kDmfError;
   int intVal = gatoi(value);
   int rangeCheck = (param->upperLimit > param->lowerLimit);   
   if (NULL != rawData)
   {
      retval = kDmfSuccess;

      if (rangeCheck && ((UInt32) intVal < (UInt32) param->lowerLimit))
      {
         retval = kDmfDataLowerRange;
      }
      if (rangeCheck && ((UInt32) intVal >= (UInt32) param->upperLimit))
      {
         retval = kDmfDataUpperRange;
      }

      if (kDmfSuccess == retval)
      {
         *((UInt32*) rawData) = (UInt32) intVal;
      }
   }
   return retval;
}


extern int dmfSetUInt16(const char_t* key, const char_t* index, char_t* value, 
  void* rawData, Param* param, int request)
{
   int retval = kDmfError;
   int intVal = gatoi(value);
   int rangeCheck = (param->upperLimit > param->lowerLimit);
   if (NULL != rawData)
   {
      retval = kDmfSuccess;

      if (rangeCheck && (intVal < param->lowerLimit))
      {
         retval = kDmfDataLowerRange;
      }
      if ( ((UInt32) intVal > USHRT_MAX) || 
       (rangeCheck && (intVal >= param->upperLimit)) )
      {
         retval = kDmfDataUpperRange;
      }

      if (kDmfSuccess == retval)
      {
         *((UInt16*) rawData) = (UInt16) intVal;
      }
   }
   return retval;
}


extern int dmfSetUInt8(const char_t* key, const char_t* index, char_t* value, 
  void* rawData, Param* param, int request)
{
   int retval = kDmfError;
   int intVal = gatoi(value);
   int rangeCheck = (param->upperLimit > param->lowerLimit);
   if (NULL != rawData)
   {
      retval = kDmfSuccess;

      if (rangeCheck && (intVal < param->lowerLimit))
      {
         retval = kDmfDataLowerRange;
      }
      if (((UInt32) intVal > UCHAR_MAX) || 
       (rangeCheck && (intVal >= param->upperLimit)) )
      {
         retval = kDmfDataUpperRange;
      }

      if (kDmfSuccess == retval)
      {
         *((UInt8*) rawData) = (UInt8) intVal;
      }
   }
   return retval;
}


extern int dmfSetFloat(const char_t* key, const char_t* index, char_t* value, 
  void* rawData, Param* param, int request)
{
   int retval = kDmfError;
   double floatVal = atof(value);
   /*int rangeCheck = (param->upperLimit > param->lowerLimit); warning: unused variable `rangeCheck'*/
   if (NULL != rawData)
   {
      retval = kDmfSuccess;
      if (param->upperLimit > param->lowerLimit)
      {
         if (floatVal < param->lowerLimit)
         {
            retval = kDmfDataLowerRange;
         }
         if (floatVal >= param->upperLimit)
         {
            retval = kDmfDataUpperRange;
         }
      }
      if (kDmfSuccess == retval)
      {
         *((float*) rawData) = (float) floatVal;
      }
      
   }
   return retval;
}


extern int dmfSetDouble(const char_t* key, const char_t* index, char_t* value, 
  void* rawData, Param* param, int request)
{
   int retval = kDmfError;
   double floatVal = atof(value);
   /*int rangeCheck = (param->upperLimit > param->lowerLimit); warning: unused variable `rangeCheck'*/
   if (NULL != rawData)
   {
      retval = kDmfSuccess;
      if (param->upperLimit > param->lowerLimit)
      {
         if (floatVal < param->lowerLimit)
         {
            retval = kDmfDataLowerRange;
         }
         if (floatVal >= param->upperLimit)
         {
            retval = kDmfDataUpperRange;
         }
      }
      if (kDmfSuccess == retval)
      {
         *((double*) rawData) = floatVal;
      }
   }
   return retval;
}



extern int dmfSetStringDynamic(const char_t* key, const char_t* index, char_t* value, 
  void* rawData, Param* param, int request)
{
   int retval = kDmfError;
   int len =  gstrlen(value);
   int rangeCheck = (param->upperLimit > param->lowerLimit);

   if (NULL != rawData)
   {
      retval = kDmfSuccess;
      if (rangeCheck)
      {
         if (len < param->lowerLimit)
         {
            retval = kDmfDataLowerRange;
         }
         if (len >= param->upperLimit)
         {
            retval = kDmfDataUpperRange;
         }
         /*
          * NOTE an important difference here -- this dmfSetFunc absolutely requires that 
          * the upperLimit member be set, because that's what we use to know when we 
          * must stop copying the string in.
          */
         if (kDmfSuccess == retval)
         {
            int len = strlen(value)+1;
            char **pptr = (char_t **)rawData;

            if(*pptr) free(*pptr);
            len = param->upperLimit < len ? param->upperLimit+1 : len;
            *pptr = malloc(len);
            gstrncpy(*pptr, value, len);
            (*pptr)[len] = 0;
         }
      }
   }
   return retval;
}

extern int dmfSetStringFixed(const char_t* key, const char_t* index, char_t* value, 
  void* rawData, Param* param, int request)
{
   int retval = kDmfError;
   int len =  gstrlen(value);
   int rangeCheck = (param->upperLimit > param->lowerLimit);

   if (NULL != rawData)
   {
      retval = kDmfSuccess;
      if (rangeCheck)
      {
         if (len < param->lowerLimit)
         {
            retval = kDmfDataLowerRange;
         }
         if (len >= param->upperLimit)
         {
            retval = kDmfDataUpperRange;
         }
         /*
          * NOTE an important difference here -- this dmfSetFunc absolutely requires that 
          * the upperLimit member be set, because that's what we use to know when we 
          * must stop copying the string in.
          */
         if (kDmfSuccess == retval)
         {
            gstrncpy((char_t*) rawData, value, param->upperLimit);
         }
      }
   }
   return retval;

}

