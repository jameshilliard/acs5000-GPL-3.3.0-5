/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: Locals.c,v 1.1.1.1 2005/09/01 02:36:38 edson Exp $ */



#include "Locals.h"
#include "Parse.h"

#define kMaxLocalsLength   1024

/* 
 * we maintain a symbol table to keep variables that are interesting to
 * us, but not the device
 */
static sym_fd_t   sLocals = -1;
/*
 * ...and we also maintain a symbol table for ephemeral values that we do not 
 * want persisted across runs of the server. 
 */
static sym_fd_t   sEphemerals = -1;

/*
 * we maintain a table of callback functions that may be called when an
 * attempt is made to save a specific local variable. The table is not
 * actually created unless someone tries to register one of these callbacks.
 */
static sym_fd_t sLocalHooks = -1;

/*
 * we maintain a 'dirty' flag that's set to true whenever a local variable is
 * set. If it's zero, the dmfPersistLocals() function does nothing.
 */
static int sLocalsDirty = 0;

/*
 * we also maintain a single user-collision variable to prevent multiple
 * overlapping set() attempts from clobbering recently set values.
 */
static int sLastSet = 0;

/*
 * we store (a copy of) the name of the file to persist locals into..
 */
char_t*  sLocalFile = NULL;



extern int dmfInitLocals(int size)
{
   int retval = kDmfError;

   sLocals = symOpen(64);
   sEphemerals = symOpen(32);
   if ((sLocals >= 0) && (sEphemerals >= 0))
   {
      retval = kDmfSuccess;
   }
   return retval;

}

extern void dmfCloseLocals()
{
   symClose(sLocals);
   symClose(sEphemerals);
   if (-1 != sLocalHooks)
   {
      symClose(sLocalHooks);
   }
   bfreeSafe(B_L, sLocalFile);
}


extern int dmfKeyIsLocalVar(char_t* key)
{
   return (kLocalPrefix == key[0]);
}



extern int dmfRegisterLocalHook(char_t* key, LocalHookFunc hook)
{
   if (-1 == sLocalHooks)
   {
      sLocalHooks = symOpen(11);
   }
   symEnter(sLocalHooks, key, valueInteger((long) hook), 0);
   return kDmfSuccess;

}

static sym_fd_t dmfSelectLocalTable(char_t* key)
{
   int len = gstrlen(key);
   return (kEphemeralSuffix == key[len-1]) ? sEphemerals : sLocals;
}

static char_t* dmfMakeKeyName(char_t* key, char_t* index)
{
   /*
    * To simulate indexes in the local variables, we concatenate the key and
    * index together as "key_index". If the index pointer is NULL, then we
    * just dupe the key and return it. Else we return a new string that has
    * both key and index. 
    * It's the caller's responsibility to free the memory allocated by this
    * function.
    */
   char_t* retval = NULL;
   
   if (index != NULL)
   {
      fmtAlloc(&retval, BUF_MAX, T("%s_%s"), key, index);
   }
   else
   {
      retval = bstrdup(B_L, key);
   }
   return retval;
}

extern int dmfGetLocal(char_t* key, char_t* index, char_t** val)
{
   sym_t* symbol;
   char_t* realKey = dmfMakeKeyName(key, index);
   int retval = kDmfParameterName;
   sym_fd_t table = dmfSelectLocalTable(key);


   if (realKey)
   {
      symbol = symLookup(table, realKey);
      if (NULL != symbol)
      {
         /*
          * copy the value back into the pointer the user provided us with.
          * It's their responsibility to free it when it's no longer needed.
          */
         *val = bstrdup(B_L, symbol->content.value.string);
         retval = kDmfSuccess;
      }
      bfreeSafe(B_L, realKey);
   }

   return retval;
}


extern int dmfSetLocal(char_t* key, char_t* index, char_t* val, int request)
{
   if (0 != request && sLastSet > request)
   {
      return kDmfOldData;
   }

   /*sLastSet = request;*/

   if (-1 != sLocalHooks)
   {
      sym_t* sym = symLookup(sLocalHooks, key);
      if (NULL != sym)
      {
         LocalHookFunc f = (LocalHookFunc) sym->content.value.integer;
         if (NULL != f)
         {
            return (*f)(key, index, val);
         }
      }
   }

   return dmfSetLocalImpl(key, index, val);
}


extern int dmfSetLocalImpl(char_t* key, char_t* index, char_t* val)
{
   int retval = kDmfError;
   char_t* realKey = dmfMakeKeyName(key, index);
   sym_fd_t table = dmfSelectLocalTable(key);
   if (realKey)
   {
      symEnter(table, realKey, valueString(val, VALUE_ALLOCATE), 0);
      retval = kDmfSuccess;
      bfreeSafe(B_L, realKey);
      if (sLocals == table)
      {
         sLocalsDirty = 1;
      }
      retval = kDmfSuccess;
   }


   return retval;
}



    
extern int dmfDeleteLocal(char_t* key, char_t* index)
{
   int retval = kDmfError;
   char_t* realKey = dmfMakeKeyName(key, index);
   sym_fd_t table = dmfSelectLocalTable(key);
   if (realKey)
   {
      if (0 == symDelete(table, realKey))
      {
         retval = kDmfSuccess;
         if (sLocals == table)
         {
            sLocalsDirty = 1;
         }
      }
      bfreeSafe(B_L, realKey);
   }


   return retval;
   
}


extern int dmfPersistLocals(void)
{
   int retval = kDmfSuccess;
   if (NULL != sLocalFile && sLocalsDirty)
   {
      retval = dmfPersistSymbolTable(sLocals, sLocalFile);
      sLastSet = dmfGetRequestNumber();
      sLocalsDirty = 0;
   }
   return retval;
}


extern int dmfReloadLocals(char_t* fileName)
{
   int retval = kDmfError;
   bfreeSafe(B_L, sLocalFile);
   sLocalFile = bstrdup(B_L, fileName);
   retval = dmfReloadSymbolTable(sLocals, fileName, string);
   return retval;
}
      
         

