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
#define  kCurrentChapter   T("ch04")


static void InitSystemInfo(void);


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
      
}

void dmfUserPostInit(void)
{
   char_t* newRootWeb = NULL;
#ifdef USER_MANAGEMENT_SUPPORT
   /*
    * TODO: Create user groups that must be present on the server.
    * NOTE that we use the dmfAddGroup() function -- this allows us to
    * associate an integer access level with each group as we create it.
    */

   
   /*
    * TODO: Create a default admin account on the server. You may want to
    * choose a different name for the builtin admin account, and will
    * definitely want to change the password.
    */

#endif
   /*
    * TODO: Configure access permissions for paths on the device as needed.
    */

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
