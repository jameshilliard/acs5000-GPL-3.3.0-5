/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: Parse.h,v 1.2 2006/01/10 23:51:50 erik Exp $ */


#ifndef h_Parse
#define h_Parse

#include "Dmf.h"

/*
 * Utility function to extract key/value pairs from a query string. 
 * NOTE that it destroys the query string as it goes along, so 
 * make sure that you're working with a copy of the string if you 
 * will need it again later.
 * 
 * returns zero when either *key or *value are NULL.
 * Usage:
 * while (dmfParseQueryString(&str, &key, &val))
 * {
 *    (* do something with key & val *)
 * }
 * 
 * Used internally by our generic form handler. Useful in command 
 * functions or custom user-supplied form handlers.
 */
extern int dmfParseQueryString(char_t** string, char_t** key, char_t** val);


/*
 * Utility function to parse set args -- Handles the following cases:
 * set("key", "val"); 
 * set("key", "index", "val");
 */
extern int dmfParseSetArgs(int argc, char_t** argv, char_t** key, char_t** index, 
 char_t** value);

/*
 * Utility function used by the framework to parse out arguments to a call that
 * gets something from the framework -- if only one argument is passed, it is the 
 * key that we're seeking. If a second argument is provided, it is an index value.
 */
extern int dmfParseGetArgs(int argc, char_t** argv, char_t** key, 
 char_t** index);

/*
 * Utility function to parse strings containing a list of delimited keyword
 * / value pairs. For example, URL query strings are encoded as 
 * key1=value1&key2=value2 (etc.).
 * This function strips the first key and value out of the string pointed
 * to by '*string', returning them in *key and *val. On return, *string
 * points to the first character after the value delimiter. 
 * returns 1 if it was able to find a delimited key (at least -- a key
 * without a matching value is NOT an error case) 
 * returns 0 if it did not find a delimited keyword, or if the string
 * started out empty.
 */
extern int dmfParseKeyValueString(char_t** string, char_t** key, char_t** val, 
 char_t keyDelimiter, char_t valDelimiter);


#endif
/*../dmf212/AlFrame/Parse.h:61:7: warning: no newline at end of file*/

