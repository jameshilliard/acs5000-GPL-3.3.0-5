/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: GetSet.h,v 1.1.1.1 2005/09/01 02:36:38 edson Exp $ */


#ifndef h_GetSet
#define h_GetSet


#include "Dmf.h"


/*
 * alfGetParam -- used internally by both alfGet and dmfEjGet.
 * 'key' is the key to look up, val is a pointer to pointer where
 * the value is returned. Do NOT free this memory after you are 
 * done.
 * 
 * The 'index' parameter should be set to NULL if it's not being used to 
 * look up an indexed variable.
 *
 * The accessLevel parameter indicates the access level of the current user.
 * Valid access levels are from 0..255. If the access level is -1, we will not
 * validate access.
 */
extern int dmfGetParam(char_t* key, char_t* index, char_t** val, int accessLevel);

/*
 * internal implementation of dmfGetParam -- the first argument is the index
 * of the desired parameter in the parameterList array.
 *
 * The accessLevel parameter indicates the access level of the current user.
 * Valid access levels are from 0..255. If the access level is -1, we will not
 * validate access.
 */
extern int dmfGetParamByIndex(int parmIndex, char_t* key, char_t* index, 
 char_t** val, int accessLevel);

/*
 * alfSetParam -- Implementation used by dmfSet(). Presumably, 
 * it will also be useful to set a key, value directly (e.g., from a 
 * form handler. 
 * 
 *
 * The accessLevel parameter indicates the access level of the current user.
 * Valid access levels are from 0..255. If the access level is -1, we will not
 * validate access.
 */ 
extern int dmfSetParam(char_t* key, char_t* index, char_t* val, int request, 
 int accessLevel);


extern int dmfSetParamByIndex(int parmIndex, char_t* key, 
 char_t* index, char_t* val, int request, int accessLevel);

/*
 * dmfCommitParam -- if this parameter has an associated commit function, this
 * calls it. The commit function may choose to do nothing, depending on what
 * the value of 'request' is.
 */
extern int dmfCommitParam(char_t* key, int request, char_t* urlQuery);



#endif
