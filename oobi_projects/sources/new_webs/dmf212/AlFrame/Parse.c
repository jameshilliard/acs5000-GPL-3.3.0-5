/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: Parse.c,v 1.1.1.1 2005/09/01 02:36:38 edson Exp $ */



#include "Parse.h"

extern int dmfParseGetArgs(int argc, char_t** argv, char_t** key, 
 char_t** index)
{
   char_t* parm1 = NULL;
   char_t* parm2 = NULL;
   int num = 0;
   int retval = kDmfSuccess;

   num = ejArgs(argc, argv, T("%s %s"), &parm1, &parm2);

   if (num < 1) 
   {
      retval =  kDmfArgCount;
   }
   else if (1 == num)
   {
      *key = parm1;
      *index = NULL;
   }
   else 
   {
      *key = parm1;
      *index = parm2;
   }
   return retval;
}



extern int dmfParseSetArgs(int argc, char_t** argv, char_t** key, char_t** index, 
 char_t** value)
{
   char_t* parm1 = NULL;
   char_t* parm2 = NULL;
   char_t* parm3 = NULL;
   int num = 0;

   num = ejArgs(argc, argv, T("%s %s %s"), &parm1, &parm2, &parm3);

   /*
    * we need at least 2 parameters -- key, value
    */
   if (num < 2) 
   {   
      return kDmfArgCount;
   }
   else if (2 == num)
   {
      *key = parm1;
      *index = NULL;
      *value = parm2;
   }
   else 
   {
      *key = parm1;
      *index = parm2;
      *value = parm3;
   }
   return kDmfSuccess;
}




extern int dmfParseKeyValueString(char_t** string, char_t** key, char_t** val, 
 char_t keyDelimiter, char_t valDelimiter)
{

   int retval = 0;
   char_t* endPtr = NULL;


   if (NULL != *string && 0 != gstrlen(*string))
   {
      /*
       * look for the '=' delimiter -- if we don't find it, we will return false,
       * because this is not a key=value string.
       */
      endPtr = gstrchr(*string, keyDelimiter);
      if (NULL != endPtr)
      {
         /*
          * we found the equal sign, let's set some pointers:
          * - key is the beginning of the string
          * - the '=' is replaced with a NULL
          * - we move the string pointer to the character after the '='
          */
         *key = *string;
         *endPtr = '\0';
         *val = *string = ++endPtr;
         /*
          * handle case where the last key has an empty value
          */
         if ('\0' != **val)
         {
            endPtr = gstrchr(*string, valDelimiter);
            if (NULL != endPtr)
            {
               *endPtr = '\0';
               *string = ++endPtr;
            }
            else
            {
               *string += gstrlen(*val);
            }
         }
         retval = 1;
      }
   }
   return retval;
}
         
       
extern int dmfParseQueryString(char_t** string, char_t** key, char_t** val)
{
   /*
    * On Entry: *string points to a char_t buffer that has data of the form :
    * "key1=val1&key2=val2&key3=val3"
    * There's been a request that it also support keys with empty values.,
    * e.g.:
    * "key1=&key2=val2&key3=val3"
    * On exit:
    * *key points to the first key, NULL terminated where the '=' char is.
    * *val points to the first value, NULL terminated where the '&' char is.
    * *string points to the char_t following the '&' char. If there are no
    * more key/value pairs left, *string is a NULL pointer,
    *
    * If there are no key/value pairs to return, the function returns zero,
    * otherwise it returns 1.
    */

   int retval = 0;
   if (dmfParseKeyValueString(string, key, val, '=', '&'))
   {
      if (NULL != *key)
      {
         websDecodeUrl(*key, *key, gstrlen(*key));
      }
      if (NULL != *val)
      {
         websDecodeUrl(*val, *val, gstrlen(*val));
      }

      retval = 1;
   }

   return retval;
}



