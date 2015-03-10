/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: UserFunc.c,v 1.1.1.1 2005/09/01 02:36:39 edson Exp $ */


#include <time.h>
#include "webs.h"

#include "Access.h"
#include "Locals.h"
#include "Session.h"
#include "UserFunc.h"

/*
 * FRAMEWORK CODE FOLLOWS
 *
 */


DataTest gTestStruct;
RandomTest gRandomTest;
static void InitTestData(void);

extern int kZeroIndex = 0;

void dmfUserPreInit(void)
{
   /*
    * Open the user database
    */
   dmfInitAccess(T("access.txt"));

   /*
    * Initialize the session management subsystem
    */
#ifdef qSession
   dmfInitSession(30, 0);
#endif

   /*
    * Initialize the parameter tree...
    */
   dmfInitParams(parameterList, parameterCount);

   /*
    * TODO: Things that you may want to change here for your project:
    * + location of web root directory
    * + name of default file
    * + server port
    */
}

void dmfUserPostInit(void)
{

#ifdef USER_MANAGEMENT_SUPPORT
   /*
    * TODO: Create user groups that must be present on the server.
    * NOTE that we use the dmfAddGroup() function -- this allows us to
    * associate an integer access level with each group as we create it.
    */
   dmfAddGroup(NULL, T("admin"), PRIV_READ | PRIV_WRITE | PRIV_ADMIN, 
    AM_BASIC, 1, 1, 10);

   dmfAddGroup(NULL, T("users"), PRIV_READ | PRIV_WRITE, 
    AM_BASIC, 1, 1, 1);


   
   /*
    * TODO: Create a default admin account on the server. You may want to
    * choose a different name for the builtin admin account, and will
    * definitely want to change the password.
    */
   dmfAddUser(NULL, T("adminUser"), T("changeThisPassword"), 
    T("admin"), 1, 1);
   
#endif
   /*
    * TODO: Configure access permissions for paths on the device as needed.
    */

   /* for our test, we password protect the /admin subdirectory. */
   umAddAccessLimit(T("/admin/"), AM_BASIC, 0, T("admin"));
   /*umAddAccessLimit(T("/goform/Dmf"), AM_BASIC, 0, T("admin"));*/


   InitTestData();
}

void dmfUserPreClose(void)
{
   /*
    * by default, this function does nothing.
    */
   /*dmfPersistLocals(kLocalsFile);*/
}

void dmfUserPostClose(void)
{
   /*
    * by default, this function does nothing.
    */
}




/*
 * PROJECT-SPECIFIC CODE FOLLOWS....
 *
 */


static void InitTestData(void)
{
   int i;
   int charsWritten;
   time_t localTime;



   gTestStruct.charVal = 1;
   gTestStruct.shortVal = 2;
   gTestStruct.longVal = 3;
   gTestStruct.byteVal = 4;
   gTestStruct.uShortVal = 5;
   gTestStruct.uLongVal = 6;
   for (i = 0; i < kByteArrayLen; ++i)
   {
      gTestStruct.byteArray[i] = i * 2;
   }
   gTestStruct.byteArrayLength = kByteArrayLen;

   gTestStruct.floatVal = 3.14159f;
   gTestStruct.doubleVal = 4.12310563;

   gstrcpy(gTestStruct.string1, T("123456789ABCDEF"));

   time(&localTime);
   charsWritten = fmtStatic(gTestStruct.string2, kString2Len, 
    T("%s"), ctime(&localTime));
   /* get rid of the newline that ctime sticks on the end...*/
   gTestStruct.string2[charsWritten-1] = '\0';

   srand(time(0));
   gRandomTest.requestCount = 0;
   

}

extern int DataTestQuery(Param* param, int request)
{
   static int sLastRequest = -1;
   time_t localTime;
   int charsWritten = 0;

   int retval = kDmfSuccess;

   if (request != sLastRequest)
   {
      if (request != 0)
      {
         sLastRequest = request;
      }
      time(&localTime);
      charsWritten = fmtStatic(gTestStruct.string2, kString2Len, 
       T("%s"), ctime(&localTime));
      /* get rid of the newline that ctime sticks on the end...*/
      gTestStruct.string2[charsWritten-1] = '\0';
   }

   return retval;

}


extern int DataTestCommit(Param* param, int request)
{
   // this test commit function is a no-op.
   return kDmfSuccess;

}


extern int RandomTestQuery(Param* param, int request)
{
   gRandomTest.requestCount++;
   gRandomTest.randVal = rand();

   return kDmfSuccess;
}
