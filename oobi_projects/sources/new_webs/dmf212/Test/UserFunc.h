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
 * PROJECT-SPECIFIC FUNCTIONS/VARIABLES FOLLOW
 */


#define kString1Len  16
#define kString2Len  128
#define kByteArrayLen   32

typedef struct DataTestTAG
{
   SInt8    charVal;
   SInt16   shortVal;
   SInt32   longVal;
   UInt8    byteVal;
   UInt8    byteArray[kByteArrayLen];
   int      byteArrayLength;
   UInt16   uShortVal;
   UInt32   uLongVal;

   float    floatVal;
   double   doubleVal;

   char_t   string1[kString1Len];
   char_t   string2[kString2Len];
} DataTest;

typedef struct RandomTestTAG
{
   SInt32   randVal;
   SInt32   requestCount;
} RandomTest;


extern DataTest   gTestStruct;
extern RandomTest gRandomTest;

extern int  kZeroIndex;

extern int DataTestQuery(Param* param, int request);
extern int DataTestCommit(Param* param, int request);

extern int RandomTestQuery(Param* param, int request);
/*extern int DataTestCommit(Param* param, int request);*/




#endif

