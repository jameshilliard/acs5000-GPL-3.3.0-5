/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: StringBuf.c,v 1.1.1.1 2005/09/01 02:36:38 edson Exp $ */

#include <stdarg.h>

#include "StringBuf.h"


extern StringBuf* CreateStringBuf(SInt32 capacity)
{
   StringBuf* retval = (StringBuf*) balloc(B_L, sizeof(StringBuf));
   if (retval)
   {
      retval->buf = (char_t*) balloc(B_L, sizeof(char_t) * capacity);
      retval->capacity = (NULL != retval->buf) ? capacity : 0;
      if (retval->capacity > 0)
      {
         ResetStringBuf(retval);
      }
   }
   return retval;
}

extern void DestroyStringBuf(StringBuf* buf)
{
   a_assert(NULL != buf);

   if (buf)
   {
      bfreeSafe(B_L, buf->buf);
      bfree(B_L, buf);
   }
}

extern void ResetStringBuf(StringBuf* buf)
{
   a_assert(NULL != buf);

   if (buf != NULL)
   {
      /*
       * reset things so the internal buffer is 'empty'. Note that the
       * capacity remains unchanged.
       */
      buf->length = 0;
      buf->buf[0] = '\0';
   }


}
extern char_t* GetString(StringBuf* buf)
{
   char_t* retval = NULL;
   a_assert(NULL != buf);

   if (buf != NULL)
   {
      retval = buf->buf;
      buf->buf[buf->length] = '\0';
      a_assert(buf->length != 0);
   }
   return retval;
}


extern SInt32 GetStringLength(StringBuf* buf)
{
   SInt32 len = 0;
   a_assert(NULL != buf);
   if (NULL != buf)
   {
      len = buf->length;
   }
   return len;
}

/*
extern int AddStrings(StringBuf* buf, ...)
{
   va_list args;
   va_start(args, buf);


   return kDmfSuccess;
}
*/

extern int AddString(StringBuf* buf, const char_t* str) 
{
   SInt32 len = gstrlen(str);
   a_assert(NULL != buf);
   return AddBufToString(buf, str, len);
}

extern int AddBufToString(StringBuf* buf, const char_t* str, SInt32 len) 
{

   SInt32 newCapacity = 0;
   SInt32 needed = 0;
   int retval = kDmfSuccess;

   a_assert(NULL != buf);
   a_assert(len > 0);
   if (buf && str)
   {
      needed = len + buf->length + 10;
      if (needed >= buf->capacity)
      {
         
         /*
          * I read someplace (comp.lang.c++, IIRC) that growing a string buffer 
          * by 150% ends up being more efficient than any other factor, in
          * general. So that's what we do here.
          */
         newCapacity = buf->capacity;
         while (newCapacity < needed)
         {
            newCapacity = ((newCapacity * 3) / 2) + 1;
         }
         
         /* grow the buffer in place. */
         buf->buf = brealloc(B_L, buf->buf, newCapacity);
         a_assert(NULL != buf->buf);
         buf->capacity = newCapacity;
      }
      /*
       * copy the new string onto the end of the buffer and update the length.
       */
      memcpy((buf->buf + buf->length), str, len);
      buf->length += len;
      /*
       * manually add the NULL terminator -- we were sure to reserve room for 
       * it above.
       */
   }

   return retval;
}

