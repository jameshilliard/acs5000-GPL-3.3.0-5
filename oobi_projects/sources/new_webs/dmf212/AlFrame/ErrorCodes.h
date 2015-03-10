/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: ErrorCodes.h,v 1.1.1.1 2005/09/01 02:36:38 edson Exp $ */



#ifdef qErrorStringTable
/* include this file again, but w/ structs rather than enums (See ErrorCodeMacros.h)*/
#undef h_ErrorCodes
#endif

#ifndef h_ErrorCodes
#define h_ErrorCodes

#include "ErrorCodeMacros.h"

mErrorTableBegin(DmfCodes)
   mErrorTableEntry(kDmfSuccess, T("Success"), T(""), kReplaceNone),
   
   /* non-specific error */
   mErrorTableEntry(kDmfError, T("Error"), T(""), kReplaceNone), 

   /* attempt to call a function that doesn't exist */
   mErrorTableEntry(kDmfNoSuchFunction, T("FunctionName"),
    T("'%s'"), kParm1),
   
   /* parameter name required, not present */
   mErrorTableEntry(kDmfParameterMissing, T("ParameterMissing"), 
    T(""), kReplaceNone),   

   /* index is required, not present */
   mErrorTableEntry(kDmfIndexMissing, T("IndexMissing"), 
    T("'%s'"), kParm1),   

   /* value is required, not present: */
   mErrorTableEntry(kDmfValueMissing, T("ValueMissing"), 
    T("'%s'"), kParm1),   
   
   /* specified index is out of range (key, index)*/
   mErrorTableEntry(kDmfIndexRange, T("IndexRange"), 
    T("%s[%s]"), kParm1 | kParm2), 

   /* data is below lower range (key, val)*/
   mErrorTableEntry(kDmfDataLowerRange, T("DataLowerRange"), 
   T("%s: %s"), kParm1 | kParm3),

   /* data above upper range (key, val) */
   mErrorTableEntry(kDmfDataUpperRange, T("DataUpperRange"), 
    T("%s: %s"), kParm1 | kParm3),
   
   /* too many/not enough arguments to a function */
   mErrorTableEntry(kDmfArgCount, T("ArgumentCount"), 
    T(""), kReplaceNone),
   
   /* an argument was of the wrong type */
   mErrorTableEntry(kDmfArgError, T("ArgumentError"),  
    T("'%s'"), kParm1),
  
   /* index provided, not needed */
   mErrorTableEntry(kDmfNotIndexed, T("NotIndexed"), 
    T("'%s'"), kParm1),    
   
   /* attempt to submit a form using old data */
   mErrorTableEntry(kDmfOldData, T("OldData"), 
    T("'%s'"), kParm1),     
   
   /* the specified parameter name does not exist */    
   mErrorTableEntry(kDmfParameterName, T("ParameterName"), 
    T("'%s'"), kParm1), 
   
   /* the parameter may not be written to */
   mErrorTableEntry(kDmfReadOnly, T("ReadOnly"), 
    T("'%s'"), kParm1),         
   
   /* the specified parameter may not be read (?) */
   mErrorTableEntry(kDmfWriteOnly, T("WriteOnly"), 
    T("'%s'"), kParm1),

   /* User Access codes follow... */
   mErrorTableEntry(kDmfAccessLevel, T("AccessLevel"), 
    T(""), kReplaceNone),

   /* Login required... */
   mErrorTableEntryLogin(kDmfLoginRequired, T("LoginRequired"), 
    T(""), kReplaceNone),

   /* missing user name */
   mErrorTableEntry(kDmfUserName, T("UserName"),
    T(""), kReplaceNone),
   
   /* user already exists */
   mErrorTableEntry(kDmfUserExists, T("UserExists"), 
    T("'%s'"), kParm1),
   
   /* user doesn't exist */
   mErrorTableEntry(kDmfUserDoesNotExist, T("UserDoesNotExist"), 
    T("'%s'"), kParm1),
   
   /* error adding user */
   mErrorTableEntry(kDmfUserAdd, T("UserAdd"), 
    T("'%s'"), kParm1),
   
   /* error deleting user */
   mErrorTableEntry(kDmfUserDelete, T("UserDelete"), 
    T("'%s'"), kParm1),
   
   /* error changing password */
   mErrorTableEntry(kDmfUserPassword, T("UserPassword"), 
    T("'%s'"), kParm1),
   
   /* error changing user group */
   mErrorTableEntry(kDmfUserGroup, T("UserGroup"), 
    T("'%s'"), kParm1 ),
   
   /* error enabling user */
   mErrorTableEntry(kDmfUserEnable, T("UserEnable"), 
    T("'%s'"), kParm1),
   
   /* error disabling user */
   mErrorTableEntry(kDmfUserDisable, T("UserDisable"),
    T("'%s'"), kParm1),
   
   /* the specified group already exists */
   mErrorTableEntry(kDmfGroupExists, T("GroupExists"), 
    T("'%s'"), kParm1),
   
   /* the group does not exist. */
   mErrorTableEntry(kDmfGroupDoesNotExist, T("GroupDoesNotExist"), 
    T("'%s'"), kParm1),
   
   /* error adding the new group */
   mErrorTableEntry(kDmfGroupAdd, T("GroupAdd"), 
    T("'%s'"), kParm1),
   
   /* error deleting a group */
   mErrorTableEntry(kDmfGroupDelete, T("GroupDelete"), 
    T("'%s'"), kParm1),
   
   /* missing / invalid group name */
   mErrorTableEntry(kDmfGroupName, T("GroupName"), 
    T(""), kReplaceNone),
   
   /* missing / invalid  privilege setting */
   mErrorTableEntry(kDmfGroupPrivilege, T("GroupPrivilege"), 
    T("'%s'"), kParm1),
   
   /* missing / invalid group access setting */
   mErrorTableEntry(kDmfGroupAccess, T("GroupAccess"), 
    T("'%s'"), kParm1),
   
   /* missing / invalid protection setting */
   mErrorTableEntry(kDmfGroupProtection, T("GroupProtection"), 
    T("'%s'"), kParm1),
   
   /* missing / invalid group enable */
   mErrorTableEntry(kDmfGroupEnable, T("GroupEnable"), 
    T("'%s'"), kParm1),
   
   /* missing / invalid group access level */
   mErrorTableEntry(kDmfGroupAccessLevel, T("GroupLevel"), 
    T("'%s'"), kParm1),

   /* error in parsing XML-RPC message */
   mErrorTableEntry(kDmfXmlRpcParse, T("XmlParse"), 
    T(""), kReplaceNone),

   /* the last XML-RPC function call resulted in multiple errors.*/
   mErrorTableEntry(kDmfXmlRpcMultiError, T("RpcErrors"), 
    T(""), kReplaceNone),

   /* internal framework error */
   mErrorTableEntry(kDmfInternalError, T("InternalError"), 
    T(""), kReplaceNone),

   /* ************************************************************************
    * this is the last error index -- it must be the final entry in the table.
    */
   mErrorTableEntry(kDmfLastError, T("LastError"), T(""), kReplaceNone)
mErrorTableEnd(DmfCodes)




#endif
