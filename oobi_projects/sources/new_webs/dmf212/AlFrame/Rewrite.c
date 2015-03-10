/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: Rewrite.c,v 1.1.1.1 2005/09/01 02:36:38 edson Exp $ */


#include "Rewrite.h"


#ifdef qRewriteMarkups

extern char_t* dmfRewriteMarkups(char_t* buffer)
{
   char_t* outBuf = NULL;
   char_t* textStart = buffer;
   char_t* markupStart = NULL;
   char_t* markupEnd = NULL;
   char_t* theMarkup = NULL;
   char_t* tail = NULL;
   
   /* current length of the output buffer (in bytes, not char_t's */
   int len = 0;  
   
   /* bytes output */
   int outCount = 0; 

   int charCount = 0;

   int finished = 0;

   /* give ourselves a 50% size increase for markups. */
   len = gstrlen(buffer) * sizeof(char_t);
   len += (len / 2);
   /* don't forget to convert from characters to bytes (as needed) */
   outBuf = balloc(B_L, len);
   tail = outBuf;

   /*
    * The basic design is:
    * - save the start of plain text.
    * - look for the next occurrence of the '[[' delimiter
    *    - if found, write the plain text into the output buffer
    *    - look for the closing markup tag
    *    - rewrite the text between the markups and append to the out buf
    *    - set the start pointer to 1 char past the ']]'
    *    - loop.
    */
   while (!finished)
   {
      markupStart = gstrstr(textStart, kMarkupStart);
      if (NULL != markupStart)
      {
         /* 
          * first, put everything between textStart and markupStart onto the 
          * output buffer.
          */

         *markupStart = '\0';
         charCount = gstrlen(textStart);
         gstrcpy(tail, textStart);
         tail += charCount;

         /* 
          * move the markupStart pointer to the actual beginning of the 
          * markup token
          */
         markupStart += 2;

         /* 
          * then, search for the close markup 
          */
         markupEnd = gstrstr(markupStart, kMarkupEnd);
         if (NULL != markupEnd)
         {
            /*
             * we found the end of the markup -- format the string and 
             * add it to the end of the output buffer.
             */
            *markupEnd = '\0';
            charCount = fmtAlloc(&theMarkup, BUF_MAX, T("<%%get(\"%s\");%%>"), 
             markupStart);

            gstrcpy(tail, theMarkup);
            /* move the tail pointer */
            tail += charCount;

            /*
             * move the start pointer to just after the markup closer
             */
            textStart = markupEnd + 2;
         }
         else
         {
            /*
             * !!! this is an error -- we had an unclosed markup. 
             */
         }

      }
      else
      {
         /* there are no more markups between here and the end of this
            page. Copy everything that remains into the output buffer 
            (reallocating as needed) and get out of this loop.
         */
         gstrcpy(tail, textStart);
         finished = 1;
      }
   }

   return outBuf;
}

#endif   /*qRewriteMarkups */


