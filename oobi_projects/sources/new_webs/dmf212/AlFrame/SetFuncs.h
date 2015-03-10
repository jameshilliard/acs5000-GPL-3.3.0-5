/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: SetFuncs.h,v 1.1.1.1 2005/09/01 02:36:38 edson Exp $ */


#ifndef h_SetFuncs
#define h_SetFuncs

#include "Dmf.h"

/*
 * all the dmfSetFuncs must use the following prototype (as defined in Param.h):
 * 
 *  typedef int (*SetFunc)(const char_t* key, const char_t* index, 
 *   char_t* value, void* rawData, Param* param, int request);
 */

extern int dmfSetInt32(const char_t* key, const char_t* index, char_t* value, 
  void* rawData, Param* param, int request);

extern int dmfSetInt16(const char_t* key, const char_t* index, char_t* value, 
  void* rawData, Param* param, int request);

extern int dmfSetInt8(const char_t* key, const char_t* index, char_t* value, 
  void* rawData, Param* param, int request);

extern int dmfSetUInt32(const char_t* key, const char_t* index, char_t* value, 
  void* rawData, Param* param, int request);

extern int dmfSetUInt16(const char_t* key, const char_t* index, char_t* value, 
  void* rawData, Param* param, int request);

extern int dmfSetUInt8(const char_t* key, const char_t* index, char_t* value, 
  void* rawData, Param* param, int request);

extern int dmfSetFloat(const char_t* key, const char_t* index, char_t* value, 
  void* rawData, Param* param, int request);

extern int dmfSetDouble(const char_t* key, const char_t* index, char_t* value, 
  void* rawData, Param* param, int request);

/*
 * frees the existing memory at rawData, allocates new memory at the same
 * location (if necessary) and copies the string in.
 * Casts rawData to char_t**
 */
extern int dmfSetStringDynamic(const char_t* key, const char_t* index, char_t* value, 
  void* rawData, Param* param, int request);


/*
 * Useful to copy a string into a fixed-size buffer of char_t. Uses the
 * 'upperLimit' member of the param structure to know when to stop copying.
 */
extern int dmfSetStringFixed(const char_t* key, const char_t* index, char_t* value, 
  void* rawData, Param* param, int request);


#endif
