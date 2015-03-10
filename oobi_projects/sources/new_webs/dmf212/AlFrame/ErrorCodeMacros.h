/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: ErrorCodeMacros.h,v 1.3 2006/09/21 14:58:58 mpeccin Exp $ */

#ifdef qErrorStringTable
/* include this file again, but w/ array-of-struct macros rather than enum macros */
#undef h_ErrorCodeMacros
#endif

#ifndef h_ErrorCodeMacros
#define h_ErrorCodeMacros

#undef mErrorTableBegin
#undef mErrorTableEntry
#undef mErrorTableEntryLogin
#undef mErrorTableEnd

#ifndef qErrorStringTable

   #define mErrorTableBegin(name) typedef enum name##Tag {
   #define mErrorTableEntry(id, name, fmt, replace) id
   #define mErrorTableEntryLogin(id, name, fmt, replace) id
   #define mErrorTableEnd(name) } name;

   typedef struct ErrorStringsTAG
   {
      char_t*  errorName;
      char_t*  errorFormat;
      int      replaceType;
	  int	   loginReq;
   } ErrorStrings;

   typedef enum ReplaceTypeTAG
   {
      kReplaceNone = 0,
      kParm1 = 1,
      kParm2 = 2,
      kParm3 = 4
   } ReplaceType;

#else

// #define mErrorTableBegin(name) extern ErrorStrings k##name##Strings[] = {
   #define mErrorTableBegin(name) ErrorStrings k##name##Strings[] = {
//the change above avoids a warning message
   #define mErrorTableEntry(id, name, fmt, replace) { (name), (fmt), (replace), 0 }
   #define mErrorTableEntryLogin(id, name, fmt, replace) { (name), (fmt), (replace), 1 }
   #define mErrorTableEnd(name) };

#endif /* #else */

#endif /* #ifndef h_ErrorCodeMacros */
