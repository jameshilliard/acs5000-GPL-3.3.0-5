/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: Access.h,v 1.1.1.1 2005/09/01 02:36:38 edson Exp $ */



#ifndef h_Access
#define h_Access


#include "um.h"

#include "Dmf.h"

/*
 * Access.c -- Utility functions to expose the GoAhead Server's user
 * management facilities to ejScript/ GoForms.
 */


#define kMaxAccessLevel 255


/*
 * dmfInitAccess -- initialize the user management system.
 * Called automatically by the framework's init code.
 */

extern int dmfInitAccess(char_t* databaseFilename);

/*
 * dmfCloseAccess -- shut the user management system down.
 * Called by the framework's shutdown code.
 */

extern int dmfCloseAccess(void);




/*
 * dmfCommitAccess -- writes the current user database back to the user file
 * that we are using. Called after each add/delete of a user or group.
 */
extern int dmfCommitAccess(void);

/* dmfCanAccess -- function called by umUserCanAccessUrl when the server is
 * compiled with qHierarchicalAccess defined. We use an internal symbol table
 * to map group names to 8-bit access levels. If the user's access level is
 * greater than or equal to the URL's access level, we return true. Else, we
 * return false. If either the user group or URL group is not found in the
 * symbol table, we assert (in a debug build) and return false in a release
 * build.
 *
 * If it's not already done, make sure that umCanAccessUrl is modified as
 * follows, and make sure to add a prototype for this function in um.c (or
 * #include "Dmf/Access.h" to make it visible there.)
      if (group && *group) {
      #ifdef qHierarchicalAccess
         
          * If we are compiling with the hierarchical access extensions, we
          * instead call the user-provided function that checks to see whether
          * the current user's access level is greater than or equal to the
          * access level required for this URL.
          
         return dmfCanAccess(usergroup, group);

      #else
         if (usergroup && (gstrcmp(group, usergroup) != 0)) {
            return FALSE;
      #endif

 *
 * 
 */
extern bool_t dmfCanAccess(char_t* userGroup, char_t* urlGroup);

/*
 * Given a user name, returns the integer access level associated with that
 * user's group. Returns zero if compiled without Hierarchical Access enabled, or if 
 * a NULL user name is passed in.
 */
extern int dmfUserAccessLevel(char_t* userName);

/*
 * Adds a user to the user database. 
 * name = user name
 * password = user password
 * group = name of group to add this user to
 * protected = "1" -- protected against deletion
 *             "0" -- not protected
 * enabled =   "1" -- user is enabled
 *             "0" -- user is disabled
 */
extern void dmfAddUser(webs_t wp, char_t* name, char_t* password, char_t* group, 
 int protected, int enabled);

/*
 * Deletes the user from the database.
 */
extern void dmfDeleteUser(webs_t wp, char_t* name);

/*
 * Parameters same as for AddUser, but the user must already exist.
 */
extern void dmfModifyUser(webs_t wp, char_t* name, char_t* password, char_t* group, 
 int enabled);


/*
 * dmfGetNextUser -- sets the ejScript return buffer to hold the user name of the 
 * specified user. Called from script as getNextUser("name");
 * If name == "", returns the first user, otherwise returns the next user in 
 * the user table.
 * Loop like this:
 * <%
 *    var user = "";
 *    for(;__success__;)
 *    {
 *       user = getNextUser(user);
 *       if (user != "")
 *       {
 *          // do something with the user....
 *       }
 *       / * else, we're done, and the magic __success__ variable will go false,
 *          kicking us out of the loop.
 *        * /
 *    }
 *       
 */       
extern int dmfEjGetNextUser(int eid, webs_t wp, int argc, char_t **argv);

/*
 * dmfGetUserDetail -- requests a detail field for the specified user.
 * Available fields are:
 * * password (string)
 * * group (string)
 * * enabled ("1" = enabled "0" = disabled)
 * * "exists" -- "1" == this user already exists. "0" = no such user.
 * called from script like:
 * var detail = getUserDetail("username", "field");
 * 
 */

extern int dmfEjGetUserDetail(int eid, webs_t wp, int argc, char_t **argv);


/*
 * handles forms designed to add/delete/modify users.
 * looks for the following form fields:
 * command = "ADDUSER" | "MODIFYUSER" | "DELETEUSER"
 * name = string
 * password = string
 * enabled = "1" = enabled, "0" = disabled
 * group = string (name of a group that exists)
 */
extern void dmfUserCommandHandler(CommandStruct* cmd);

/*
 *  Add the new group named 'group'.
 *  privilege is a bitmap using the constants:
 *
 *  PRIV_NONE - user has no privileges
 *  PRIV_READ - user can read files
 *  PRIV_WRITE - user can execute CGIs
 *  PRIV_ADMIN - user can administrate 
 *  
 *  am is one of:
 *
 *  AM_NONE - Group has no access to anything at all
 *  AM_FULL - Group has access all pages, no authorization required
 *  AM_BASIC - group requires at least basic authentication,
 *  AM_DIGEST - group requires at least digest authentication,
 *  (Note that goAhead also supports a method called "AM_INVALID" -- we 
 *  do not support this.
 *
 *  protect -- "on" = group protected against deletion, (absent = group not protected.
 *  enabled -- 1 = the group is enabled 0 = the group is disabled.
 *  Note that goAhead's underlying API creates users by stating whether
 *  they're enabled, but groups by stating whetaher they're disabled. We
 *  wrap their API so that everything is in terms of being ENabled.
 *
 *  accessLevel -- alFrame extension, using our sGroupMap. This assigns an
 *  integer access level (0..255) for this group name. Has no effect unless
 *  the server has been compiled with the qHierarchicalAccess option enabled.
 */  
extern void dmfAddGroup(webs_t wp, char_t* group, short privilege, accessMeth_t am, 
 bool_t protect, int enabled, int accessLevel);

/*
 * Parameters as for Addgroup()
 */
extern void dmfModifyGroup(webs_t wp, char_t* group, short privilege, accessMeth_t am, 
 int enabled, int accessLevel);

/*
 * Delete the group named 'group'
 */
extern void dmfDeleteGroup(webs_t wp, char_t* group);

/*
 * Return the name of the current user, or if we are not logged in, an 
 * empty string. When called from ejScript, takes no parameters.
 * called as 
 * name = userName();
 * If there is no user name, sets the ejScript error code to "UserName".
 */
extern int dmfEjGetUserName(int eid, webs_t wp, int argc, char_t **argv);


/*
 * called from ejScript as userCanAccess("groupName");
 * Return '1' if the current user is allowed to access elements from the 
 * specified group, or '0' if they may not. can be used as a boolean.
 * If the server is compiled with hierarchical access enabled, follows those
 * rules, otherwise requires an exact match (like the GoAhead WebServer 
 * default behavior).
 */
extern int dmfEjUserCanAccess(int eid, webs_t wp, int argc, char_t **argv);

/*
 * See dmfGetUser for comments -- we use the same style of loop syntax to 
 * iterate through the available groups.
 * called from script as:
 * var groupName = getGroup("groupName");
 * calling getGroup("") returns the first group name
 */
extern int dmfEjGetGroup(int eid, webs_t wp, int argc, char_t **argv);


/*
 * returns a detail field for a specified group.
 * Available fields are:
 * * "accessMethod" -- "NONE" -- no access allowed
 *                 "FULL" -- access everything without requiring auth
 *                 "BASIC" -- requires at least basic auth
 *                 "DIGEST" -- requires digest auth
 *  * "enabled" -- "1" = enabled. "0" = disabled
 *  * "privilege" -- "NONE" = no privileges
 *                   "READ" = read only
 *                   "WRITE" = can call CGI's / forms
 *                   "ADMIN" = can administrate
 *                   priviliges will be combined with '/':
 *                   READ/WRITE
 *  * "exists" -- "1" if the group already exists, "0" if it does not. 
 *  * "level" -- returns the integer access code (0..255) associated with this 
 *                access group. 
 */
extern int dmfEjGetGroupDetail(int eid, webs_t wp, int argc, char_t **argv);

/*
 * Handles forms designed to add/delete/modify access groups.
 * looks for the following form fields:
 * command -- "ADDGROUP" | "MODIFYGROUP" | "DELETEGROUP"
 * name -- group name
 * privilege -- "NONE" | "READ" | "WRITE" | "ADMIN" (may be combined  --
 *                                           "READ/WRITE")
 * accessMethod -- "NONE" -- no access allowed
 *                 "FULL" -- access everything without requiring auth
 *                 "BASIC" -- requires at least basic auth
 *                 "DIGEST" -- requires digest auth
 * protect --  "on" = protected against deletion 
 *             (not present) = not protected
 * enabled -- "on" = group is enabled
 *            (not present) = group is disabled
 * accessLevel -- 0..255 -- sets the integer access level for this group.
 */
extern void dmfGroupCommandHandler(CommandStruct* cmd);




#endif

