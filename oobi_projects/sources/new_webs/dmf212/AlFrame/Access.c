/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: Access.c,v 1.1.1.1 2005/09/01 02:36:38 edson Exp $ */



#include "webs.h"

#include "Access.h"
#include "Error.h"
#include "ErrorCodes.h"
#include "Parse.h"

static int sInitialized = 0;

static char_t* sUserDatabaseFile = NULL;

#ifdef qHierarchicalAccess
static char_t* sLevelMapFile = NULL;
static sym_fd_t sGroupMap = -1;
#endif


/*
 * SetGroupAccess -- assigns an integer access level to a given group name.
 * If the specified group has already been assigned an access level, this will
 * change the level to this new value. Returns kDmfSuccessif set successfully, 
 * kDmfError if not.
 */

static int dmfSetGroupAccess(char_t* groupName, int accessLevel);

/*
 * returns the integer access level associated with the specified group name.
 * Returns zero if there's no entry for the requested group.
 */
static int dmfGetGroupAccess(char_t* groupName);

/*
 * DeleteGroupAccess -- removes a group name from our internal mapping of
 * group names -> integer access levels. Called automatically from code that
 * deletes groups from the user management table.
 */
static int dmfDeleteGroupAccess(char_t* groupName);





extern int dmfInitAccess(char_t* databaseFilename)
{
   int retval = kDmfError;
#ifdef qHierarchicalAccess
   char_t* dotPos;   
#endif
   if (!sInitialized && (NULL != databaseFilename))
   {
      sUserDatabaseFile = bstrdup(B_L, databaseFilename);  
      if (0 == umOpen())
      {
         umRestore(sUserDatabaseFile);
#ifdef qHierarchicalAccess
         /*
          * initialize the group map with room for 16 group -> Access level
          * mappings. Increase if needed for your installation.
          */
         sGroupMap = symOpen(16);
         /* allocate fresh memory -- reserve room for our extension */
         sLevelMapFile = balloc(B_L, gstrlen(sUserDatabaseFile) + 4);
         if (NULL != sLevelMapFile)
         {
            /* look for the rightmost dot */
            gstrcpy(sLevelMapFile, sUserDatabaseFile);
            dotPos = gstrrchr(sLevelMapFile, '.');
            if (NULL != dotPos)
            {
               /* NULL out the dot and whatever follows...*/
               *(dotPos) = '\0';
            }
            gstrcat(sLevelMapFile, T(".lvl"));

            /* finally -- attempt to restore the level map database*/
            dmfReloadSymbolTable(sGroupMap, sLevelMapFile, integer);
         }

#endif
         sInitialized = 1;
         retval = kDmfSuccess;
      }
      
   }


   return retval;
}


extern int dmfCloseAccess(void)
{
   if (sInitialized)
   {
#ifdef qHierarchicalAccess
      symClose(sGroupMap);
      bfreeSafe(B_L, sLevelMapFile);
#endif
      bfree(B_L, sUserDatabaseFile);
      umClose();
      sInitialized = 0;
   }
   return kDmfSuccess;
}


extern int dmfCommitAccess(void)
{
   int retval = kDmfError;
   if (sInitialized)
   {
      if (0 == umCommit(sUserDatabaseFile))
      {
#ifdef qHierarchicalAccess
         retval = dmfPersistSymbolTable(sGroupMap, sLevelMapFile);
# else
         retval = kDmfSuccess;
#endif
      }
   }
   return retval;
}




extern bool_t dmfCanAccess(char_t* userGroup, char_t* urlGroup)
{
   bool_t retval = FALSE;
   sym_t* userAccess = NULL;
   sym_t* urlAccess = NULL;

   if (0 == gstrcmp(userGroup, urlGroup))
   {
      /* 
       * no need for a symbol table lookup, because we have an exact match.
       */
      retval = TRUE;
   }
#ifdef qHierarchicalAccess
   else if (-1 != sGroupMap)
   {
      /*
       * we need to find the access level settings for both groups and compare
       * them as integers.
       */
      userAccess = symLookup(sGroupMap, userGroup);
      urlAccess = symLookup(sGroupMap, urlGroup);
      if ((NULL != userAccess) && (NULL != urlAccess))
      {
         retval = (userAccess->content.value.shortint >= 
          urlAccess->content.value.shortint);
      }
   }
#endif

   return retval;   
}


extern int dmfUserAccessLevel(char_t* userName)
{
   int retval = 0;
#ifdef qHierarchicalAccess
   if (NULL != userName)
   {
      char_t* groupName = umGetUserGroup(userName);
      retval = dmfGetGroupAccess(groupName);
   }
#endif
   return retval;
}


static int dmfSetGroupAccess(char_t* groupName, int accessLevel)
{
   int retval = kDmfSuccess;
#ifdef qHierarchicalAccess
   sym_t* symbol;
   
   if (accessLevel > kMaxAccessLevel)
   {
      accessLevel = kMaxAccessLevel;
   }
   if (accessLevel < 0)
   {
      accessLevel = 0;
   }

   if (-1 != sGroupMap)
   {
      symbol = symEnter(sGroupMap, (char_t*) groupName, 
       valueInteger(accessLevel), 0);
      if (symbol == NULL)
      {
         retval = kDmfError;
      }
   }
#endif
   return retval;
}

static int dmfGetGroupAccess(char_t* groupName)
{
   int retval = 0;
#ifdef qHierarchicalAccess
   if (NULL != groupName)
   {
      sym_t* sym = symLookup(sGroupMap, groupName);
      if (NULL != sym)
      {
         retval = sym->content.value.integer;
      }
   }
#endif
   return retval;
}


static int dmfDeleteGroupAccess(char_t* groupName)
{
   int retval = kDmfSuccess;
#ifdef qHierarchicalAccess

   if (-1 != sGroupMap)
   {
      if (0 != symDelete(sGroupMap, groupName))
      {
         retval = kDmfError;
      }
   }

#endif
   return retval;


}

extern int dmfEjGetNextUser(int eid, webs_t wp, int argc, char_t **argv)
{
   char_t* name = NULL;
   char_t* nextName = NULL;
   ejArgs(argc, argv, T("%s"), &name);
   if (gstrlen(name) > 1)
   {
      /*
       * They have passed in a previous name, and are looking for the next 
       * user name.
       */
      nextName = umGetNextUser(name);
   }
   else
   {
      nextName = umGetFirstUser();
   }
   if (NULL == nextName)
   {
      ejSetResult(eid, T(""));
      dmfSetErrorCode(eid, kDmfIndexRange);
   }
   else
   {
      ejSetResult(eid, nextName);
      dmfSetErrorCode(eid, kDmfSuccess);
   }

   return 0;
}
      

extern int dmfEjGetUserName(int eid, webs_t wp, int argc, char_t **argv)
{
   if (NULL == wp->userName)
   {
      ejSetResult(eid, T(""));
      dmfSetErrorCode(eid, kDmfUserName);
   }
   else
   {
      ejSetResult(eid, wp->userName);
      dmfSetErrorCode(eid, kDmfSuccess);
   }

   return 0;
}


extern int dmfEjUserCanAccess(int eid, webs_t wp, int argc, char_t **argv)
{
   char_t* groupName = NULL;
   ejArgs(argc, argv, T("%s"), &groupName);

   if (NULL != wp->userName && 
    dmfCanAccess(umGetUserGroup(wp->userName), groupName))
   {
      ejSetResult(eid, T("1"));
      dmfSetErrorCode(eid, kDmfSuccess);
   }
   else
   {
      ejSetResult(eid, T("0"));
      dmfSetErrorCode(eid, kDmfAccessLevel);
   }
   return 0;


}


extern int dmfEjGetUserDetail(int eid, webs_t wp, int argc, char_t **argv)
{
   char_t* user = NULL;
   char_t* field = NULL;
   char_t* value = NULL;
   int retval = kDmfSuccess;

   if (ejArgs(argc, argv, T("%s %s"), &user, &field) >= 2)
   {
      if (0 == gstrcmp(field, T("password")))
      {
         value = umGetUserPassword(user);
      }
      else if (0 == gstrcmp(field, T("group")))
      {
         value = bstrdup(B_L, umGetUserGroup(user));
      }
      else if (0 == gstrcmp(field, T("enabled")))
      {
         if (umGetUserEnabled(user))
         {
            value = bstrdup(B_L, T("1"));
         }
         else
         {
            value = bstrdup(B_L, T("0"));
         }
      }
      else if (0 == gstrcmp(field, T("exists")))
      {
         if (umUserExists(user))
         {
            value = bstrdup(B_L, T("1"));
         }
         else
         {
            value = bstrdup(B_L, T("0"));
         }
      }
      else
      {
         retval = kDmfArgError;
      }
   }
   else
   {
      retval = kDmfArgCount;
   }
   if (NULL != value)
   {
      ejSetResult(eid, value);
      bfree(B_L, value);
   }
   else
   {
      ejSetResult(eid, T(""));
   }
   dmfSetErrorCode(eid, retval);
   
   return 0;
}

extern void dmfUserCommandHandler(CommandStruct* cmd)
{
   char_t* name = NULL;
   char_t* password = NULL;
   char_t* enabled = NULL;
   char_t* group = NULL;

   int enabledVal = 0;

   /*
    * look for the form variables that we're looking for...
    */
   
   name = websGetVar(cmd->wp, T("name"), NULL);
   if (NULL == name)
   {
      /* need a user name! */
   }
   password = websGetVar(cmd->wp, T("password"), NULL);
   enabled = websGetVar(cmd->wp, T("enabled"), T("off"));
   group = websGetVar(cmd->wp, T("group"), NULL);
   enabledVal = (0 == gstrcmp(T("on"), enabled));


   /*
    * dispatch based on the 'command' -- convert to upper case first
    */
   strupper(cmd->command);
   if (0 == gstrcmp(cmd->command, T("ADDUSER")))
   {
      dmfAddUser(cmd->wp, name, password, group, 0, enabledVal);
   }
   else if (0 == gstrcmp(cmd->command, T("MODIFYUSER")))
   {
      dmfModifyUser(cmd->wp, name, password, group, enabledVal);
   }
   else if (0 == gstrcmp(cmd->command, T("DELETEUSER")))
   {
      dmfDeleteUser(cmd->wp, name);
   }
   else
   {
      /* 
       * unsupported command for this form.
       */
      dmfAddError(cmd->wp, kDmfArgError, cmd->command, NULL, NULL);
   }

   /*
    * Write the changes out to the disk
    */
   if (NULL != sUserDatabaseFile)
   {
      umCommit(sUserDatabaseFile);
   }

}




extern int dmfEjGetGroup(int eid, webs_t wp, int argc, char_t **argv)
{
   char_t* name = NULL;
   char_t* nextName = NULL;
   int retval = kDmfSuccess;
   ejArgs(argc, argv, T("%s"), &name);
   if (gstrlen(name) > 1)
   {
      /*
       * They have passed in a previous name, and are looking for the next 
       * user name.
       */
      retval = kDmfUserGroup;
      if (umGroupExists(name))
      {
         retval = kDmfSuccess;
         nextName = umGetNextGroup(name);
      }
   }
   else
   {
      nextName = umGetFirstGroup();
   }

   if (NULL == nextName)
   {
      retval = kDmfIndexRange;
   }
   ejSetResult(eid, (nextName != NULL) ? nextName : T(""));
   dmfSetErrorCode(eid, retval);


   return kDmfSuccess;
}


extern int dmfEjGetGroupDetail(int eid, webs_t wp, int argc, char_t **argv)
{
   char_t* group = NULL;
   char_t* field = NULL;
   char_t* value = NULL;
   char_t tempString[32];
   short privilege = PRIV_NONE;
   int needSeparator = 0;
   int retval = kDmfSuccess;

   if (ejArgs(argc, argv, T("%s %s"), &group, &field) >= 2)
   {
      if (0 == gstrcmp(T("accessMethod"), field))
      {
         switch (umGetGroupAccessMethod(group))
         {
            case AM_NONE:
            value = bstrdup(B_L, T("NONE"));
            break;

            case AM_FULL:
            value = bstrdup(B_L, T("FULL"));
            break;

            case AM_BASIC:
            value = bstrdup(B_L, T("BASIC"));
            break;

            case AM_DIGEST:
            value = bstrdup(B_L, T("DIGEST"));
            break;

            case AM_INVALID:
            default:
            value = bstrdup(B_L, T("INVALID"));
            break;


         }
      }
      else if (0 == gstrcmp(T("enabled"), field))
      {
         if (umGetGroupEnabled(group))
         {
            value = bstrdup(B_L, T("1"));
         }
         else
         {
            value = bstrdup(B_L, T("0"));
         }
      }
      else if (0 == gstrcmp(T("privilege"), field))
      {
         privilege = umGetGroupPrivilege(group);
         if (PRIV_NONE == privilege)
         {
            value = bstrdup(B_L, T("NONE"));
         }
         else
         {
            /*
             * guarantee that the string is nulled...
             */
            gstrcpy(tempString, T("\0"));

            if (privilege & PRIV_READ)
            {
               gstrcat(tempString, T("READ"));
               needSeparator = 1;
            }
            if (privilege & PRIV_WRITE)
            {
               if (needSeparator)
               {
                  gstrcat(tempString, T("/"));
               }
               gstrcat(tempString, T("WRITE"));
               needSeparator = 1;
            }
            if (privilege & PRIV_ADMIN)
            {
               if (needSeparator)
               {
                  gstrcat(tempString, T("/"));
               }
               gstrcat(tempString, T("ADMIN"));
            }
            value = bstrdup(B_L, tempString);
         }
      }
      else if (0 == gstrcmp(T("exists"), field))
      {
         if (umGroupExists(group))
         {
            value = bstrdup(B_L, T("1"));
         }
         else
         {
            value = bstrdup(B_L, T("0"));
         }
      }
      else if (0 == gstrcmp(T("accessLevel"), field))
      {
         fmtAlloc(&value, BUF_MAX, "%d", dmfGetGroupAccess(group));
      }     
      else
      {
         /*
          * error...
          */
         retval = kDmfParameterName;
      }

      if (NULL != value)
      {
         /* give the result of the operation to the interpreter...*/
         ejSetResult(eid, value);
         bfree(B_L, value);
      }
   }
   else
   {
      /*
       * We didn't receive any parameters with the function.
       */
      retval = kDmfParameterMissing;

   }


   
   dmfSetErrorCode(eid, retval);

   return kDmfSuccess;
}


static int ValidateAddOrModifyGroup(webs_t wp, char_t* group, char_t* priv, 
 char_t* access, char_t* protect, char_t* enabled, int accessLevel)
{
   int retval = 1;
   /*
    * If any of the required parameters are NULL, we end up returning 0/false
    * instead.
    */
   if (NULL == priv)
   {
      dmfAddError(wp, kDmfGroupPrivilege, group, NULL, NULL);
      retval = 0;
   }
   if (NULL == access)
   {
      dmfAddError(wp, kDmfGroupAccess, group, NULL, NULL);
      retval = 0;
   }
   if (NULL == protect)
   {
      dmfAddError(wp, kDmfGroupProtection, group, NULL, NULL);
      retval = 0;
   }
   if (NULL == enabled)
   {
      dmfAddError(wp, kDmfGroupEnable, group, NULL, NULL);
      retval = 0;
   }

   if ((accessLevel < 0) || (accessLevel > 255))
   {
      dmfAddError(wp, kDmfGroupAccessLevel, group, NULL, NULL);
      retval = 0;
   }

   return retval;
}


      
extern void dmfGroupCommandHandler(CommandStruct* cmd)
{
   char_t* group = NULL;
   char_t* privilege = NULL;
   char_t* accessMethod = NULL;
   char_t* enabled = NULL;
   char_t* protect = NULL;
   char_t* accessLevel = NULL;
   char_t* key = NULL;
   char_t* val = NULL;
   
   int enabledVal = FALSE;
   short privilegeVal = PRIV_NONE;
   accessMeth_t accessVal = AM_NONE;
   bool_t protectVal = FALSE;
   int accessLevelVal = 0;
   
   dmfClearErrors(cmd->wp);
   /*
    * see below for correct handling of the privilege setting...
    * 
    * privilege = websGetVar("privilege", NULL);
    */

   group = websGetVar(cmd->wp, T("group"), NULL);
   accessMethod = websGetVar(cmd->wp, T("accessMethod"), T("NONE"));
   enabled = websGetVar(cmd->wp, T("enabled"), T("off"));
   protect = websGetVar(cmd->wp, T("protect"), T("off"));
   accessLevel = websGetVar(cmd->wp, T("accessLevel"), T("0"));

   /*
    * convert strings -> other types as needed...
    */
   strupper(accessMethod);
   if (0 == gstrcmp(T("NONE"), accessMethod))
   {
      accessVal = AM_NONE;
   }
   else if (0 == gstrcmp(T("FULL"), accessMethod))
   {
      accessVal = AM_FULL;
   }
   else if (0 == gstrcmp(T("BASIC"), accessMethod))
   {
      accessVal = AM_BASIC;
   }
   else if (0 == gstrcmp(T("DIGEST"), accessMethod))
   {
      accessVal = AM_DIGEST;
   }

   protectVal = (0 == gstrcmp(T("on"), protect));
   enabledVal = (0 == gstrcmp(T("on"), enabled));
   accessLevelVal = gatoi(accessLevel);
   
   /*
    * NOTE that since the privilege setting is a bitmap, we will get multiple 
    * privilege=XXXXXX settings in the query string.
    * Also note that this needs to be done last, since it trashes the 
    * query string.
    */
   while (dmfParseQueryString(&(cmd->queryString), &key, &val))
   {
      if (0 == gstrcmp(T("privilege"), key))
      {
         /*
          * convert the 'val' into a valid bitmap value...
          */
         strupper(val);
         if (0 == gstrcmp(T("NONE"), val))
         {
            privilegeVal |= PRIV_NONE;
         }
         else if (0 == gstrcmp(T("READ"), val))
         {
            privilegeVal |= PRIV_READ;
         }
         else if (0 == gstrcmp(T("WRITE"), val))
         {
            privilegeVal |= PRIV_WRITE;
         }
         else if (0 == gstrcmp(T("ADMIN"), val))
         {
            privilegeVal |= PRIV_ADMIN;
         }
         else
         {
            /*
             * Not a value we understand -- don't flag this as an error
             * (for now...)
             */
         }
      }
   }
   

   /*
    * dispatch on the command
    */
   if (NULL != group)
   {
      strupper(cmd->command);
      
      if (0 == gstrcmp(T("ADDGROUP"), cmd->command))
      {
         if (ValidateAddOrModifyGroup(cmd->wp, group, privilege, accessMethod, 
          protect, enabled, accessLevelVal))
         {
            dmfAddGroup(cmd->wp, group, privilegeVal, accessVal, protectVal, 
             enabledVal, accessLevelVal);
         }
      }
      else if (0 == gstrcmp(T("MODIFYGROUP"), cmd->command))
      {
         if (ValidateAddOrModifyGroup(cmd->wp, group, privilege, accessMethod, 
          protect, enabled, accessLevelVal))
         {
            dmfModifyGroup(cmd->wp, group, privilegeVal, accessVal, enabledVal, 
             accessLevelVal);
         }
      }
      else if (0 == gstrcmp(T("DELETEGROUP"), cmd->command))
      {
         dmfDeleteGroup(cmd->wp, group);
      }
   }  
   else
   {
      /*
       * we can't do ANYTHING without a group name here -- set an error.
       */
      
      dmfAddError(cmd->wp, kDmfGroupName, NULL, NULL, NULL);
   }
}



extern void dmfAddUser(webs_t wp, char_t* name, char_t* password, char_t* group, 
 int protected, int enabled)
{
   /*
    * first, let's make sure that this user doesn't already exist.
    */
   if (umUserExists(name))
   {
      dmfAddError(wp, kDmfUserExists, name, NULL, NULL);
   }
   else
   {
      if (0 == umAddUser(name, password, group, (bool_t) protected, 
       (bool_t) (!enabled)))
      {
         dmfCommitAccess();
      }
      else
      {
         /* report the error...*/
         dmfAddError(wp, kDmfUserAdd, name, NULL, NULL);
      }
   }
}

extern void dmfDeleteUser(webs_t wp, char_t* name)
{
   if (0 == umUserExists(name))
   {
      dmfAddError(wp, kDmfUserDoesNotExist, name, NULL, NULL);
   }
   else
   {
      if (0 == umDeleteUser(name))
      {
         dmfCommitAccess();
      }
      else
      {
         dmfAddError(wp, kDmfUserDelete, name, NULL, NULL);
      }
   }
}

extern void dmfModifyUser(webs_t wp, char_t* name, char_t* password, 
 char_t* group, int enabled)
{
   if (0 == umUserExists(name))
   {
      dmfAddError(wp, kDmfUserDoesNotExist, name, NULL, NULL);
   }
   else
   {
      if (0 != umSetUserPassword(name, password))
      {
         dmfAddError(wp, kDmfUserPassword, name, NULL, NULL);
      }
      if (0 != umSetUserGroup(name, group))
      {
         dmfAddError(wp, kDmfUserGroup, name, group, NULL);
      }
      if (0 != umSetUserEnabled(name, (bool_t) enabled))
      {
         dmfAddError(wp, (enabled ? kDmfUserEnable : kDmfUserDisable), 
          name, NULL, NULL);
      }
      dmfCommitAccess();
   }
}



extern void dmfAddGroup(webs_t wp, char_t* group, short privilege, 
 accessMeth_t am, bool_t protect, int enabled, int accessLevel)
{

   /*
    * don't do anything if the group already exists
    */
   if (umGroupExists(group))
   {
      dmfAddError(wp, kDmfGroupExists, group, NULL, NULL);
   }
   else
   {
      /*
       * add the group -- note that we have to invert our enabled parameter to
       * the API's disabled parameter.
       */
      if (0 == umAddGroup(group, privilege, am, protect, ((bool_t) !enabled)))
      {
         if (kDmfSuccess == dmfSetGroupAccess(group, accessLevel))
         {
            dmfCommitAccess();
         }
         else
         {
            dmfAddError(wp, kDmfGroupAccessLevel, group, NULL, NULL);
         }       
      }
      else
      {
         dmfAddError(wp, kDmfGroupAdd, group, NULL, NULL);
      }
   }
}


extern void dmfModifyGroup(webs_t wp, char_t* group, short privilege, 
 accessMeth_t am, int enabled, int accessLevel)
{

   if (umGroupExists(group))
   {
      if (!umSetGroupPrivilege(group, privilege))
      {
         dmfAddError(wp, kDmfGroupPrivilege, group, NULL, NULL);
      }
      if (!umSetGroupAccessMethod(group, am))
      {
         dmfAddError(wp, kDmfGroupAccess, group, NULL, NULL);
      }
      if (!umSetGroupEnabled(group, (bool_t) enabled))
      {
         dmfAddError(wp, kDmfGroupEnable, group, NULL, NULL);
      }
      if (!dmfSetGroupAccess(group, accessLevel))
      {
         dmfAddError(wp, kDmfGroupAccessLevel, group, NULL, NULL);
      }
      dmfCommitAccess();
   }
}
      

   
extern void dmfDeleteGroup(webs_t wp, char_t* group)
{

   /*
    * the group must exist before we can delete it...
    */
   if (umGroupExists(group))
   {
      if (0 == umDeleteGroup(group))
      {
         if (0 == dmfDeleteGroupAccess(group))
         {
            dmfCommitAccess();
         }
         else
         {
            dmfAddError(wp, kDmfGroupDelete, group, NULL, NULL);   
         }
      }
      else
      {
         dmfAddError(wp, kDmfGroupDelete, group, NULL, NULL);   
      }
   }
   else
   {
      dmfAddError(wp, kDmfGroupDoesNotExist, group, NULL, NULL);
   }
}
  
