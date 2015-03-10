/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: UserFunc.c,v 1.1.1.1 2005/09/01 02:36:39 edson Exp $ */


#include "webs.h"

#include "Access.h"
#include "UserFunc.h"

/*
 * FRAMEWORK CODE FOLLOWS
 *
 */



void dmfUserPreInit(void)
{
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

