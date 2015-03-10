/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: Localize.c,v 1.2 2006/01/10 23:51:50 erik Exp $ */



#include "Error.h"
#include "Localize.h"
#include "Locals.h"
#include "Parse.h"
#ifdef qLocalize

/*
 * Each language file is stored in its own text file named
 * 'language.<<code>>', like:
 * like:
 * language.en
 * language.fr
 *
 * The format of this file is one entry per line, like:
 * key=value. 
 *
 * Obviously, the 'key' used must not be translated.
 * :
 * key=value. 
 *
 * Obviously, the 'key' used must not be translated. Also, since the '=' sign
 * is used as a delimiter, it may not appear in the key.
 */
 
#define kLanguageVariable  T("_language")

#define kLanguageFileFormat T("%s/language.%s")
#define kLanguagePathLength 63 /* it should be something short like "../locale" */
static char_t sLanguagePath[kLanguagePathLength] = T(".");

#define kLanguageCodeLength 2
static char_t sCurrentLanguage[kLanguageCodeLength + 1] = T("en");
static sym_fd_t   sWordTable = -1;

extern int dmfSetLanguagePath(char_t* languagePath)
{
   int length = 0;
   gstrncpy(sLanguagePath, languagePath, kLanguagePathLength - 1);
   sLanguagePath[kLanguagePathLength - 1] = 0;
   length = gstrlen(sLanguagePath);
   /* Strip any trailing /: */
   if (0 == gstrncmp(T("/"), &(sLanguagePath[length - 1]), 1))
      sLanguagePath[length - 1] = 0;
   return kDmfSuccess; // is it worth the return?
}

extern int dmfSetLanguage(char_t* languageCode)
{
   int retval = kDmfArgError;
   if (kLanguageCodeLength == gstrlen(languageCode))
   {
      sym_fd_t tempTable = symOpen(128);
      char_t* fileName = NULL;
      if (-1 != tempTable)
      {
         languageCode = strlower(languageCode);
         fmtAlloc(&fileName, BUF_MAX, kLanguageFileFormat,
          sLanguagePath, languageCode);
         if (kDmfSuccess == dmfReloadSymbolTable(tempTable, fileName, string))
         {
            /* 
             * if we're replacing an existing table, make sure that we clean
             * up first.
             */
            dmfCloseLanguage();
            
            sWordTable = tempTable;
            gstrcpy(sCurrentLanguage, languageCode);
            dmfSetLocalImpl(kLanguageVariable, NULL, sCurrentLanguage);
            retval = kDmfSuccess;
         }
         bfreeSafe(B_L, fileName);
      }
   }
   return retval;

}

extern const char_t* dmfGetLanguage(void)
{
   return sCurrentLanguage;
}

static int SetLanguageCallback(char_t* key, char_t* index, char_t* val)
{
   return dmfSetLanguage(val);
}


extern int dmfLoadLanguage(void)
{
   int retval = kDmfError;
   char_t* lang = NULL;
   /*
    * try to get the server variable that tells us what language to use.
    */
   if (kDmfSuccess != dmfGetLocal(kLanguageVariable, NULL, &lang))
   {
      /*
       * not found, so default to english.
       */
      lang = bstrdup(B_L, T("en"));
   }
   /*
    * attempt to load the specified language file.
    */
   retval = dmfSetLanguage(lang);
   bfreeSafe(B_L, lang);

   /*
    * register with the locals engine, so any attempts in the future to change 
    * the language by setting the '_language' variable will be routed to our
    * dmfSetLanguage() function.
    */
   dmfRegisterLocalHook(kLanguageVariable, SetLanguageCallback);

   return retval;

}


extern void dmfCloseLanguage(void)
{
   if (sWordTable != -1)
   {
      symClose(sWordTable);
      sWordTable = -1;
   }

}


extern int dmfGetLabel(char_t* key, char_t** label)
{
   int retval = kDmfError;
   sym_t* symbol = NULL; 
   if (sWordTable != -1)
   {
      retval = kDmfParameterName;
      symbol = symLookup(sWordTable, key);
      if (NULL != symbol)
      {
         *label = symbol->content.value.string;
         retval = kDmfSuccess;
      }
   }
   return retval;
}

extern int dmfEjGetLabel(int eid, webs_t wp, int argc, char_t **argv)
{
   int retval = kDmfError;
   char_t* key = NULL;
   char_t* defaultVal = NULL;
   char_t* label = NULL;


   if (-1 != sWordTable)
   {
      retval = dmfParseGetArgs(argc, argv, &key, &defaultVal);
      if (kDmfSuccess == retval)
      {
         retval = dmfGetLabel(key, &label);
         if (kDmfSuccess == retval)
         {
            label = bstrdup(B_L, label);
         }
         else
         {
            if (NULL != defaultVal)
            {
               label = bstrdup(B_L, defaultVal);
            }
            else
            {
               label = dmfFormatErrorString(retval, key, defaultVal, NULL);
            }
         }

      }
      websWrite(wp, T("%s"), label);
      bfreeSafe(B_L, label);
   }
   dmfSetErrorCode(eid, retval);
   return 0;

}

extern int dmfEjGetLabelVal(int eid, webs_t wp, int argc, char_t **argv)
{
   int retval = kDmfError;
   char_t* key = NULL;
   char_t* defaultVal = NULL;
   char_t* label = NULL;
   /*sym_t* symbol = NULL; ../dmf212/AlFrame/Localize.c:200: warning: unused variable `symbol'*/

   if (-1 != sWordTable)
   {
      retval = dmfParseGetArgs(argc, argv, &key, &defaultVal);
      if (kDmfSuccess == retval)
      {
         retval = dmfGetLabel(key, &label);
         if (kDmfSuccess == retval)
         {
            ejSetResult(eid, label);
         }
         else
         {
            if (NULL != defaultVal)
            {
               ejSetResult(eid, defaultVal);
            }
            else
            {
               ejSetResult(eid, T(""));
            }
         }
      }
   }
   dmfSetErrorCode(eid, retval);
   return 0;
}

extern int dmfEjGetLanguageCode(int eid, webs_t wp, int argc, char_t **argv)
{
   char_t* defaultLang = NULL;
   char_t* suffix = NULL;
   char_t* value = sCurrentLanguage;
   /*int retval = kDmfSuccess; ../dmf212/AlFrame/Localize.c:234: warning: unused variable `retval'*/

   /*
    * we discard the return value of the parsing function -- if defaultLang is
    * NULL on return, we return the current language always. If suffix is not
    * NULL, we append the suffix if defaultLang is NOT null.
    */
   dmfParseGetArgs(argc, argv, &defaultLang, &suffix);

   if (NULL != defaultLang)
   {
      /* the language codes are ALWAYS lowercase. */
      defaultLang = strlower(defaultLang);
      if (0 == gstrcmp(defaultLang, sCurrentLanguage))
      {
         value = T("");       
      }
   }

   /*
    * write the language code (and optional suffix) directly to the user's
    * browser.
    */
   websWrite(wp, T("%s%s"), value, ((NULL != suffix) ? suffix : T("")));
   
   return 0;
}



#endif
