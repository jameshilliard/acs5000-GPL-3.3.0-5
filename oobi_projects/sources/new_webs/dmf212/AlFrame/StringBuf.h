/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: StringBuf.h,v 1.1.1.1 2005/09/01 02:36:38 edson Exp $ */

#ifndef h_StringBuf
#define h_StringBuf



#include "Dmf.h"


typedef struct StringBufTAG
{
   char_t* buf;
   
   SInt32 length;
   SInt32 capacity;
} StringBuf;


/*
 * Create a string buffer that knows how to append new strings to the end,
 * growing as appropriate. By keeping track of the current string length, we
 * can add strings to the end very efficiently.
 */
extern StringBuf* CreateStringBuf(SInt32 capacity);

extern void DestroyStringBuf(StringBuf* buf);


/*
 * re-initialize the string buffer so we start adding new strings at the
 * beginning.
 */
extern void ResetStringBuf(StringBuf* buf);


/*
 * return the pointer to the string held inside the string buf.
 */
extern char_t* GetString(StringBuf* buf);

/*
 * return the length of the string inside the stringBuf object (eliminating
 * the need to call strlen().
 */
extern SInt32 GetStringLength(StringBuf* buf);

/*
 * Add a new string onto the end of the string buffer. If necessary,
 * reallocates memory to make room for the new string.
 */
extern int AddString(StringBuf* buf, const char_t* str);


/*
 *  Adds a (potentially non-zero-terminated) buffer of char_t data to the end 
 * of the specified string buffer.
 */
extern int AddBufToString(StringBuf* buf, const char_t* str, SInt32 len);






#endif /* this MUST be the last line in this file.*/
