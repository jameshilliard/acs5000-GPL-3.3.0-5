/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: Locals.h,v 1.1.1.1 2005/09/01 02:36:38 edson Exp $ */


#ifndef h_Locals
#define h_Locals

#include "Dmf.h"

/*
 * our 'local' variables are always prefixed with a leading underscore.
 */

#define kLocalPrefix '_'
#define kEphemeralSuffix kLocalPrefix

/*
 * A LocalHookFunc is a function callback that may be registered with the
 * locals engine and called before actually setting the local var. 
 * The hook function should call the dmfSetLocalImpl() function to actually
 * store the new value. The hook function allows programmers to trap the
 * attempt to set a local variable and perform some additional action before
 * storing the new value. 
 *
 * Use the dmfRegisterLocalHook() function to add an entry to the hook table.
 */
typedef int (*LocalHookFunc)(char_t*, char_t*, char_t*);

extern int dmfInitLocals(int size);

extern void dmfCloseLocals();


/*
 * Add the 'hook' function to the table of hook functions. If an attempt is
 * made to set the local variable named 'key', the framework first calls the
 * hook function, passing in the key, index, and value being set. It should
 * call dmfSetLocalImpl() to store the new value in the locals table.
 */
extern int dmfRegisterLocalHook(char_t* key, LocalHookFunc hook);




extern int dmfGetLocal(char_t* key, char_t* index, char_t** val);

extern int dmfSetLocal(char_t* key, char_t* index, char_t* val, int request);

extern int dmfSetLocalImpl(char_t* key, char_t* index, char_t* val);

/*
 * returns true if the specified key identifies a local variable.
 */
extern int dmfKeyIsLocalVar(char_t* key);


/*
 * Saves the entire contents of the table of local variables into the specified 
 * file.
 */
extern int dmfPersistLocals(void);

/*
 * attempts to load a database of local variables from the specified file
 * for re-use.
 */
extern int dmfReloadLocals(char_t* fileName);

/*
 * deletes the local variable 'key' from our table of local variables.
 * This function shold be used with great care.
 */
extern int dmfDeleteLocal(char_t* key, char_t* index);



#endif
