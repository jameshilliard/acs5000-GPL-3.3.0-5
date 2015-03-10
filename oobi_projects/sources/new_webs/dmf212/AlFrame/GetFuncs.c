/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: GetFuncs.c,v 1.1.1.1 2005/09/01 02:36:38 edson Exp $ */


#include "GetFuncs.h"



extern int dmfGetInt32(const char_t* key, const char_t* index, char_t** value, 
  void* rawData, Param* param, int request)
{
   int retval = kDmfError;
   if (rawData)
   {
      fmtAlloc(value, BUF_MAX, T("%d"), *((SInt32*)(rawData)));
      retval = kDmfSuccess;
   }
   return retval;
}

extern int dmfGetInt16(const char_t* key, const char_t* index, char_t** value, 
  void* rawData, Param* param, int request)
{
   int retval = kDmfError;
   if (rawData)
   {
      fmtAlloc(value, BUF_MAX, T("%d"), *((SInt16*)(rawData)));
      retval = kDmfSuccess;
   }
   return retval;
}


extern int dmfGetInt8(const char_t* key, const char_t* index, char_t** value, 
  void* rawData, Param* param, int request)
{
   int retval = kDmfError;
   if (rawData)
   {
      fmtAlloc(value, BUF_MAX, T("%d"), *((SInt8*)(rawData)));
      retval = kDmfSuccess;
   }
   return retval;
}

extern int dmfGetUInt32(const char_t* key, const char_t* index, char_t** value, 
  void* rawData, Param* param, int request)
{
   int retval = kDmfError;
   if (rawData)
   {
      fmtAlloc(value, BUF_MAX, T("%u"), *((UInt32*)(rawData)));
      retval = kDmfSuccess;
   }
   return retval;
}

extern int dmfGetUInt16(const char_t* key, const char_t* index, char_t** value, 
  void* rawData, Param* param, int request)
{
   int retval = kDmfError;
   if (rawData)
   {
      fmtAlloc(value, BUF_MAX, T("%u"), *((UInt16*)(rawData)));
      retval = kDmfSuccess;
   }
   return retval;
}

extern int dmfGetUInt8(const char_t* key, const char_t* index, char_t** value, 
  void* rawData, Param* param, int request)
{
   int retval = kDmfError;
   if (rawData)
   {
      fmtAlloc(value, BUF_MAX, T("%u"), *((UInt8*)(rawData)));
      retval = kDmfSuccess;
   }
   return retval;
}


extern int dmfGetFloat(const char_t* key, const char_t* index, char_t** value, 
  void* rawData, Param* param, int request)
{
   int retval = kDmfError;
   if (rawData)
   {
      /* DANGER! 32 bytes should be enough. Right? */
      *value = balloc(B_L, 32 * sizeof(char_t));
      if (NULL != *value)
      {
         gsprintf(*value,  T("%.4f"), *((float*)(rawData)));      
         retval = kDmfSuccess;
      }

   }
   return retval;
}

extern int dmfGetDouble(const char_t* key, const char_t* index, char_t** value, 
  void* rawData, Param* param, int request)
{
   int retval = kDmfError;
   if (rawData)
   {
      /* DANGER! 32 bytes should be enough. Right? */
      *value = balloc(B_L, 32);
      if (NULL != *value)
      {
         gsprintf(*value,  T("%.4f"), *((double*)(rawData)));      
         retval = kDmfSuccess;
      }
   }
   return retval;
}



extern int dmfGetString(const char_t* key, const char_t* index, char_t** value, 
  void* rawData, Param* param, int request)
{
   int retval = kDmfError;
   if (rawData)
   {
      *value = bstrdup(B_L, (char_t*) rawData);
      retval = kDmfSuccess;
   }
   return retval;
}

//[RK]Jan/28/04 - Dinamic Memory
extern int dmfGetStringPtr(const char_t* key, const char_t* index, char_t** value, 
  void* rawData, Param* param, int request)
{
   int retval = kDmfError;
   if (rawData)
   {
	if (*(char_t *)rawData) {
		*value = bstrdup(B_L, (char_t *)(*(char_t**)rawData));
	} else {
		char empty[2] = {0,0};
		*value = bstrdup(B_L,empty);
	}
		
      retval = kDmfSuccess;
   }
   return retval;
}

extern int dmfGetStringPtrClear(const char_t* key, const char_t* index, char_t** value, 
  void* rawData, Param* param, int request)
{
   int retval = kDmfError;
   if (rawData)
   {
	if (*(char_t *)rawData) {
		*value = bstrdup(B_L, (char_t *)(*(char_t**)rawData));
		bfreeSafe(B_L, *(char_t **)rawData);
		*(char_t **)rawData = NULL;
	} else {
		char empty[2] = {0,0};
		*value = bstrdup(B_L,empty);
	}
		
      retval = kDmfSuccess;
   }
   return retval;
}

