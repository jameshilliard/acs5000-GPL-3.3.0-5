/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */

#include "EjFuncs.h"
#include "Access.h"
#include "Error.h"
#include "GetSet.h"
#include "Locals.h"
#include "Parse.h"

extern int dmfEjGetRequestNumber(int eid, webs_t wp, int argc, char_t **argv)
{
   /*
    * there are no meaningful parameters to this -- if any are provided, we
    * just ignore them.
    */
   char_t num[32];

   fmtStatic(num, 32, T("%d"), dmfGetRequestNumber());
   ejSetResult(eid, num);

   dmfSetErrorCode(eid, kDmfSuccess);
   return 0;
   
}


int dmfEjGetQueryVal(int eid, webs_t wp, int argc, char_t **argv)
{
   char_t* var;
   char_t* defaultValue;
   int retval = kDmfError;

   if (ejArgs(argc, argv, T("%s %s"), &var, &defaultValue) >= 2)
   {
      ejSetResult(eid, websGetVar(wp, var, defaultValue));
      /* do NOT free the memory here...*/
      retval = kDmfSuccess;

   }
   else
   {
      retval = kDmfArgCount;
   }
   dmfSetErrorCode(eid, retval);
   return 0;
}



extern int dmfEjGetVal(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t* key = NULL;
   char_t* index = NULL;
   char_t* val = NULL;
   int retval = kDmfError;
   int accessLevel = dmfUserAccessLevel(wp->userName);

   retval = dmfParseGetArgs(argc, argv, &key, &index);
   if (kDmfSuccess == retval)
   {
      retval = dmfGetParam(key, index, &val, accessLevel);
      if (kDmfSuccess == retval)
      {
         ejSetResult(eid, val);
         bfreeSafe(B_L, val);
         retval = kDmfSuccess;
      }
   }
   dmfSetErrorCode(eid, retval);
   return 0;
}


extern int dmfEjGet(int eid, webs_t wp, int argc, char_t **argv)
{

   char_t* key = NULL;
   char_t* index = NULL;
   char_t* val = NULL;
   int result = kDmfSuccess;
   int accessLevel = dmfUserAccessLevel(wp->userName);
   int size;
   
   result = dmfParseGetArgs(argc, argv, &key, &index);
   if (kDmfSuccess == result)
   {
      result = dmfGetParam(key, index, &val, accessLevel);
      if (kDmfSuccess != result && kDmfLastError != result)
      {
         val = dmfFormatErrorString(result, key, index, NULL);
      }

   }
   else
   {
      val = dmfFormatErrorString(result, key, index, NULL);
   }

   	size = strlen(val);
	
	if (size >= (WEBS_BUFSIZE-50)) {
		char_t *partial, *aux=val;
		int psize;
		partial = balloc(B_L, WEBS_BUFSIZE);
		
		while (size > 0) {
			memset(partial, 0, WEBS_BUFSIZE);
			
			if (size > (WEBS_BUFSIZE-50)) {
				psize = WEBS_BUFSIZE-50;
				while (psize && (*(aux + psize) != '\n')) psize--;
				if (psize == 0) {
					psize = WEBS_BUFSIZE-50;
					while (psize && (*(aux + psize) != '>')) psize--;
					if (psize == 0) {
						psize = WEBS_BUFSIZE - 50;
					}
				}
			} else {
				psize = size;
			}
			strncpy(partial, aux, psize);
			websWrite(wp,T("%s"), partial);
			size -= psize;
			aux += psize;
		}
		bfreeSafe(B_L,partial);
	} else {
		websWrite(wp, T("%s"), val);
	}
	bfreeSafe(B_L, val);
   dmfSetErrorCode(eid, result);
   
   return 0;

}


extern int dmfEjSet(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t* key;
   char_t* index;
   char_t* val;
   int retval = kDmfError;
   int accessLevel = dmfUserAccessLevel(wp->userName);

   retval = dmfParseSetArgs(argc, argv, &key, &index, &val);
   if (kDmfError != retval)
   {
      retval = dmfSetParam(key, index, val, dmfGetRequestNumber(), accessLevel);
      /*
       * if the parameter we just set was a local (aka server) variable, we
       * persist it out to disk. Otherwise the next call is a no-op.
       */
      dmfPersistLocals();
   }
   dmfSetErrorCode(eid, retval);

   return 0;
}

#ifndef qLocalize
int dmfEjGetLabel(int eid, webs_t wp, int argc, char_t **argv)
{
#if 0
   int retval = kDmfError;
   int parmIndex = -1;
   char_t* key = NULL;
   char_t* index = NULL;

   retval = dmfParseGetArgs(argc, argv, &key, &index);
   if (kDmfSuccess == retval)
   {
      retval = kDmfParameterName;
      parmIndex = dmfFindParam(key);
      if (parmIndex > -1)
      {
         retval = kDmfSuccess;
         ejSetResult(eid, (char_t*) parameterList[parmIndex].label);
      }
   }
   if (retval != kDmfSuccess)
   {
      ejSetResult(eid, T(""));
   }
   dmfSetErrorCode(eid, retval);
#endif
   return 0;
}
#endif

int dmfEjDebugInfo(int eid, webs_t wp, int argc, char_t **argv)
{
   /* 
    * display the list of error numbers / codes / descriptions
    */
   int i;
   int index;
   int retval = kDmfSuccess;
   char_t* str = NULL;

   Param* parm = NULL;
#if 1
   websWrite(wp, T("<h1>Framework Error Numbers / Codes</h1>\n"));
   websWrite(wp, T("<table border=\"1\">\n"));
   websWrite(wp, T("<thead>\n<tr>"));
   websWrite(wp, T("<th>Integer value</th>\n"));
   websWrite(wp, T("<th>Name</th>\n"));
   websWrite(wp, T("<th>Message format</th>\n"));
   websWrite(wp, T("</tr>\n</thead>\n"));
   websWrite(wp, T("<tbody>\n"));
   for (i = kDmfSuccess; i < kDmfLastError; ++i)
   {
      str = dmfFormatErrorString(i, T("'parm1'"), T("'parm2'"), T("'parm3'"));
      websWrite(wp, T("<tr><td>%d</td><td>%s</td><td>%s</td></tr>\n"), 
        i, dmfGetErrorName(i), str);
      bfreeSafe(B_L, str);
   }

   websWrite(wp, T("</tbody>\n"));
   websWrite(wp, T("</table>\n"));
#endif 

   /*
    * we walk through the table of get-able parameters and return a web page
    * that lists all the parameters that may be retrieved and their 
    * current values.
    */

   websWrite(wp, 
    T("<h1>Debugging Info -- parameters/values</h1>\n"));

   websWrite(wp, T("<table>\n"));

   for (i = 0; i < parameterCount; ++i)
   {
      parm = &(parameterList[i]);
      if (kBeginSection == parm->dataType)
      {
      }
      else if (kEndSection == parm->dataType)
      {
      }
      else
      {
         /* this is an actual data parameter... */
         if (NULL != parm->startIndex && NULL != parm->endIndex)
         {
            for (index = *(parm->startIndex); index < *(parm->endIndex); ++index)
            {
               /* step through each index write out its current value */
            }
         }
         else
         {
            /* just do a regular get on this parameter, by index... */
         }

      }
   }


   websWrite(wp, T("</table>\n"));
   return retval;
}




int dmfEjMinVal(int eid, webs_t wp, int argc, char_t **argv)
{
   int x;
   int y;
   char_t str[32];

   int retval = kDmfError;

   if (ejArgs(argc, argv, T("%d %d"), &x, &y) >= 2)
   {
      /* x and y are the 2 integers passed to us. */
      fmtStatic(str, 32, T("%d"), (x < y ? x : y));
      ejSetResult(eid, str);
      retval = kDmfSuccess;
   }
   else
   {
      /*
       * !!! Add error handling support!'
       */
   }
   dmfSetErrorCode(eid, retval);
   return retval;
}



int dmfEjMaxVal(int eid, webs_t wp, int argc, char_t **argv)
{
   int x;
   int y;
   char_t str[32];

   int retval = kDmfError;

   if (ejArgs(argc, argv, T("%d %d"), &x, &y) >= 2)
   {
      /* x and y are the 2 integers passed to us. */
      fmtStatic(str, 32, T("%d"), (x < y ? y : x));
      ejSetResult(eid, str);
      retval = kDmfSuccess;
   }
   else
   {
      /*
       * !!! Add error handling support!'
       */
   }


   dmfSetErrorCode(eid, retval);
   return retval;
}


extern int dmfEjGetTime(int eid, webs_t wp, int argc, char_t **argv)
{
   char_t outputString[80];
   time_t now = time(0);
   char_t* reference;


   if (ejArgs(argc, argv, T("%s"), &reference) >= 1)
   {
      strupper(reference);
      if ( (0 == gstrcmp(T("GMT"), reference)) ||
       (0 == gstrcmp(T("UTC"), reference)) )
      {
         /*
          * convert local time to UTC (into a struct tm)
          * then back to a time_t value.
          */
         now = mktime(gmtime(&now));
      }
   }
   
   /* ../dmf212/AlFrame/EjFuncs.c:346: warning: int format, time_t arg (arg 3)
   gsprintf(outputString, T("%d"), now);*/
   gsprintf(outputString, T("%d"), (int)now);
   ejSetResult(eid, outputString);
   dmfSetErrorCode(eid, kDmfSuccess);
   return kDmfSuccess;
}
   

extern int dmfEjFormatTime(int eid, webs_t wp, int argc, char_t **argv)
{
   int time;
   time_t tTime;
   char_t* format;
   char_t outputString[80];
   struct tm* tmTime;
   int retval = kDmfError;

   if (ejArgs(argc, argv, T("%d %s"), &time, &format) >= 2)
   {
      tTime = (time_t) time;

      /*tmTime = gmtime(&tTime);*/
      tmTime = localtime(&tTime);

      gstrftime(outputString, 79, format, tmTime);
      ejSetResult(eid, outputString);
      retval = kDmfSuccess;
   }
   dmfSetErrorCode(eid, retval);
   return kDmfSuccess;
   
}

int dmfEjSplit(int eid, webs_t wp, int argc, char_t **argv)
{
   char_t* string;
   int index;
   char_t *sep;
   int retval = kDmfError;

   if (ejArgs(argc, argv, T("%s %d %s"), &string, &index, &sep) >= 3) {
      char_t *p, *q, buf[256];
      int i;
     
      p = string;

      for(i = 0; (i < index) && *p; i++){
         while(*p && (*p != sep[0])) p++;
         if(!*p) break;
	 p++;
      }

      q = buf;
      while(*p && (*p != sep[0]) && (q < buf+255)) *q++ = *p++;
      *q = 0;

      ejSetResult(eid, buf);
      /* do NOT free the memory here...*/
      retval = kDmfSuccess;

   } else {
      retval = kDmfArgCount;
   }
   dmfSetErrorCode(eid, retval);
   return 0;
}

