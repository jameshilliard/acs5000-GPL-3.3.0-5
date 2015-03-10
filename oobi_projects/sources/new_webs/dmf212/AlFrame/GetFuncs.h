/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: GetFuncs.h,v 1.1.1.1 2005/09/01 02:36:38 edson Exp $ */


#ifndef h_GetFuncs
#define h_GetFuncs

#include "Dmf.h"

/*
 * all the dmfGetFuncs must use the following prototype (as defined in Param.h):
 * 
 *  typedef int (*GetFunc)(const char_t* key, const char_t* index, 
 *   char_t** value, void* rawData, Param* param, int request);
 */

extern int dmfGetInt32(const char_t* key, const char_t* index, char_t** value, 
  void* rawData, Param* param, int request);

extern int dmfGetInt16(const char_t* key, const char_t* index, char_t** value, 
  void* rawData, Param* param, int request);

extern int dmfGetInt8(const char_t* key, const char_t* index, char_t** value, 
  void* rawData, Param* param, int request);

extern int dmfGetUInt32(const char_t* key, const char_t* index, char_t** value, 
  void* rawData, Param* param, int request);

extern int dmfGetUInt16(const char_t* key, const char_t* index, char_t** value, 
  void* rawData, Param* param, int request);

extern int dmfGetUInt8(const char_t* key, const char_t* index, char_t** value, 
  void* rawData, Param* param, int request);

extern int dmfGetFloat(const char_t* key, const char_t* index, char_t** value, 
  void* rawData, Param* param, int request);

extern int dmfGetDouble(const char_t* key, const char_t* index, char_t** value, 
  void* rawData, Param* param, int request);


extern int dmfGetString(const char_t* key, const char_t* index, char_t** value, 
  void* rawData, Param* param, int request);

//[RK]Jan/28/04 - Dinamic memory
extern int dmfGetStringPtr(const char_t* key, const char_t* index, char_t** value, 
  void* rawData, Param* param, int request);
extern int dmfGetStringPtrClear(const char_t* key, const char_t* index, char_t** value, 
  void* rawData, Param* param, int request);

#endif
