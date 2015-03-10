/* Framework code: (c) 2001 Art & Logic, Inc. All Rights Reserved. */
/* Project Specific code: (c) <<year>> <<company>> */
/* $Id: UserFunc.h,v 1.1.1.1 2005/09/01 02:36:39 edson Exp $ */

#ifndef h_UserFunc
#define h_UserFunc

#include "Dmf.h"


/*
 * UserFunc.h -- This file should be the home for any user-supplied
 * functions required for your project. First we declare some functions 
 * for initialization and shutdown for you to hook in any behavior that 
 * the stock framework does not provide.
 * The remainder of the file is for project-specific functions
 */

/*
 * FRAMEWORK FUNCTIONS
 */
extern const char* kUserFuncVersion;

   /*
    * dmfUserPreInit will be called from 'dmfInit()' before it's done 
    * initiailzation.
    */
extern void dmfUserPreInit(void);

   /*
    * dmfUserPostInit will be called from 'dmfInit()' just before it 
    * returns. This is a good place to set/initialize any 'local variables' 
    * that will be accessed using the 'dmfSetLocal'/'dmfGetLocal'
    * functions.
    */
extern void dmfUserPostInit(void);

   /*
    * dmfUserPreClose will be called from 'dmfClose()' before it does 
    * any of its cleanup. All of the dmframe framework code is still 
    * available at this point.
    */
extern void dmfUserPreClose(void);

   /*
    * dmfUserPostClose will be called from 'dmfClose()' just before it 
    * returns. None of the dmframe framework code can be called -- this
    * is your last opportunity to do any cleanup that may be needed for 
    * your system.
    */
extern void dmfUserPostClose(void);


/*
 * PROJECT-SPECIFIC FUNCTIONS/VARIABLES FOLLOW
 */


/*
 * CH03: Add the data structure to hold our local copies of the three fields
 * that make up the system information we will report. 
 */

#define kInfoStringLength 80
typedef struct SystemInfoTAG
{
   char_t   name[kInfoStringLength+1];
   char_t   description[kInfoStringLength+1];
   int      uptime;

} SystemInfo;

extern SystemInfo gSystemInfo;

/*
 * CH03: Add a query function to update the contents of the SystemInfo
 * structure
 */

extern int SystemInfoQuery(Param* param, int request);


#endif /* this must be the last line in this file. */


