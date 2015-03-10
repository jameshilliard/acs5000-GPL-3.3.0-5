/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: UserFunc.c,v 1.1.1.1 2005/09/01 02:36:39 edson Exp $ */


#include "webs.h"

#include "Access.h"
#include "Locals.h"
#include "UserFunc.h"



/*
 * In the dmfUserPostInit() function  for each chapter, we set a server
 * variable named '_chapter_' that ASP code may use to make sure that it
 * always uses the correct ASP files to correspond with the version of the
 * server executable that's actually running.
 */
#define  kChapterKey T("_chapter_")

/*
 * In each chapter's UserFunc.c file, set this to the current chapter number
 * (as a string: 'chXX')
 */
#define  kCurrentChapter   T("ch09")


static void InitSystemInfo(void);

/* CH05 -- initialize the simulator's 'gValues' structure.*/
static void InitValues(void);


void dmfUserPreInit(void)
{
   /*
    * TODO: Things that you may want to change here for your project:
    * + location of web root directory
    * + name of default file
    * + server port
    */

   /*
    * Initialize the DMF's parameter tree.
    */
   dmfInitParams(parameterList, parameterCount);

   /*
    * CH03: Since this tutorial is not connected to a real device, we need to
    * initialize our SystemInfo data structure. In a real implementation, the
    * SystemInfo structure would be filled by executing the associated Query
    * function, which would retrieve the required data from the device and
    * store it in the gSystemInfo structure.
    */
   InitSystemInfo();

   /*
    * CH05: Initialize the simulator 'gValues' structure, since our tutorial
    * is not connected to a real device.
    */
   InitValues();

      
}

void dmfUserPostInit(void)
{
   char_t* newRootWeb = NULL;
#ifdef USER_MANAGEMENT_SUPPORT
   /* CH06: Open the user database...*/
   dmfInitAccess(T("users.txt"));


   /*
    * TODO: Create user groups that must be present on the server.
    * NOTE that we use the dmfAddGroup() function -- this allows us to
    * associate an integer access level with each group as we create it.
    */
   /* CH06: Add the three access control groups that our application will use:
    *    - users (lowest level of access, access level = 1)
    *    - tech  (medium access, access level = 5)
    *    - admin (full access, access level = 10)
    */

   dmfAddGroup(NULL, T("users"), PRIV_READ | PRIV_WRITE, 
    AM_BASIC, 1, 1, 1);
   
   dmfAddGroup(NULL, T("tech"), PRIV_READ | PRIV_WRITE, 
    AM_BASIC, 1, 1, 5);
   
   dmfAddGroup(NULL, T("admin"), PRIV_READ | PRIV_WRITE | PRIV_ADMIN, 
    AM_BASIC, 1, 1, 10);



   
   /*
    * TODO: Create a default admin account on the server. You may want to
    * choose a different name for the builtin admin account, and will
    * definitely want to change the password.
    */

   /* CH06: Add a predefined user with 'admin' access. This user will be able
    * to add regular user accounts to the system.
    */
   dmfAddUser(NULL, T("adminUser"), T("changeThisPassword"), 
    T("admin"), 1, 1);

#endif
   /*
    * TODO: Configure access permissions for paths on the device as needed.
    */
   
   /* CH06: Our server has four areas that use varying levels of access
    * control:
    *    - the root '/' directory (and any other directory not otherwise 
    *    controlled are accessible to any client.
    *    - the /user/ directory and its subdirectories are accessible by any
    *    logged-in user from the group 'users' or a group with a higher level
    *    of access control.
    *    - the /tech/ directory and its subdirectories are accessible by any
    *    logged-in user from the group 'tech' or 'admin'.
    *    - the /admin/ directory and its subdirectories are accessible by any
    *    logged-in user belonging to the 'admin' group.
    */
   umAddAccessLimit(T("/user/"), AM_BASIC, 0, T("users"));
   umAddAccessLimit(T("/tech/"), AM_BASIC, 0, T("tech"));
   umAddAccessLimit(T("/admin/"), AM_BASIC, 0, T("admin"));
   umAddAccessLimit(T("/goform/Dmf"), AM_BASIC, 0, T("users"));

   

   /*
    * set the ephemeral server variable that says what chapter we are...
    */
   dmfSetLocal(kChapterKey, NULL, kCurrentChapter, 0);


   fmtAlloc(&newRootWeb, BUF_MAX, "%s/%s", websGetDefaultDir(), kCurrentChapter);
   websSetDefaultDir(newRootWeb);
   bfreeSafe(B_L, newRootWeb);
   
}

void dmfUserPreClose(void)
{
   /*
    * by default, this function does nothing.
    */
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

/*
 * CH03: Adding system.info.* parameters...
 */

SystemInfo gSystemInfo;

#define kDefaultName T("DMF Tutorial")
#define kDefaultDescription T("Imaginary Pseudo Device")

static int sStartTime = 0;

static void InitSystemInfo(void)
{
   if (0 == sStartTime)
   {
      sStartTime = time(NULL);
   }
   gstrcpy(gSystemInfo.name, kDefaultName);
   gstrcpy(gSystemInfo.description, kDefaultDescription);
   gSystemInfo.uptime = 0;

}

extern int SystemInfoQuery(Param* param, int request)
{
   /*
    * CH03: In a real implementation, the Query function would retrieve
    * current data from the device using some sort of inter-process
    * communication. Since we're just simulating here, we leave the 'name' and
    * 'description' fields alone, and update the 'uptime' field.
    */
   gSystemInfo.uptime = time(NULL) - sStartTime;

   return kDmfSuccess;
}

extern int SystemInfoCommit(Param* param, int request)
{
   /*
    * CH04: This commit function will be called after any of the members of
    * the gSystemInfo structure have succesfully updated by a user. In a real
    * system, the new values would be sent to your device using the
    * appropriate IPC mechanism.
    *
    * In this simulation, we do nothing.
    */

   return kDmfSuccess;
}


/* CH05: */
Values   gValues;
const int kZeroIndex = 0;

static void InitValues(void)
{
   int i = 0;

   gValues.endIndex = kMaxValueIndex;

   for (i = 0; i < kMaxValueIndex; ++i)
   {
      /* 
       * create random values scaled between 0..120
       */
      gValues.value[i] = (int) (((rand() * 1.0) / RAND_MAX) * 120);
      gValues.lowerAlarm[i] = 0;
      gValues.upperAlarm[i] = 100;
      fmtStatic((gValues.label[i]), kMaxValueLabelLength, "Label %02d", i);      
   }
}


static int RandomWalker(int prevVal)
{
   /*
    * we create a random value with a triangle distribution centered around
    * zero ranging between -5..+5, and return the previous value + our new 
    * random noise value.
    */
   const int kRange = 3;

   /* 
    * create two random numbers between 0 and 1
    */
   double val1 = (rand() * 1.0) / RAND_MAX;
   double val2 = (rand() * 1.0) / RAND_MAX;
   /*
    * average the numbers to create a triangle distribution, then slide it down so the
    * distribution centers around zero.
    */
   double offset = ((val1 + val2) * 0.5) - 0.5;

   /*
    * convert to an integer 'noise' value scaled between -kRange and +kRange
    */
   int noise = (int) (kRange * 2 * offset);
    
   return prevVal + noise;

}

extern int ValuesQuery(Param* param, int request)
{
   /*
    * CH05: In a real implementation, the Query function would retrieve
    * current data from the device using some sort of inter-process
    * communication.  Here, we create fresh random data for each of the 
    * 'value' points in the structure.
    */
   int i = 0;
   for (i = 0; i < kMaxValueIndex; ++i)
   {
      gValues.value[i] = RandomWalker(gValues.value[i]);
   }

   return kDmfSuccess;
}

/*
 * Commit function to send current state of the 'gValues' structure to the
 * device.
 */
extern int ValuesCommit(Param* param, int request)
{

   return kDmfSuccess;
}

