/* (c) 2003 Cyclades. All Rights Reserved. */
/* (c) 2003 Cyclades. All Rights Reserved. */
/* $Id: ResultCodes.h,v 1.6 2008/01/21 18:42:21 sergio Exp $ */

#ifdef qErrorStringTable
/* include this file again, but to define array rather than enum */
#undef h_ResultCodes
#endif

#ifndef h_ResultCodes
#define h_ResultCodes

#include <ErrorCodeMacros.h> /* define the enum macro or array macro */

/* This error table has mostly empty strings, except to detail replacement
parms.  The general error strings themselves are stored in language.* files.

Note that mAppError(id) MUST be used when returning or setting any of these
error codes! */

mErrorTableBegin(Result)
   mErrorTableEntry(kCycSuccess, T("Success"), T(""), kReplaceNone), 
   mErrorTableEntryLogin(kInvalidLogin, T("invalidLogin"), T(""), kReplaceNone), 
   mErrorTableEntryLogin(kLoginMustWait, T("loginMustWait"), T(""), kReplaceNone), 
   //[RK]Sep/16/04 - root password was wrong
   mErrorTableEntry(kInvalidRoot, T("invalidRoot"), T(""), kReplaceNone), 
   mErrorTableEntryLogin(kLoginRequired, T("loginRequired"), T(""), kReplaceNone), 
   mErrorTableEntryLogin(kTimeout, T("timeout"), T(""), kReplaceNone), 
   mErrorTableEntry(kAnotherAdminIsOn, T("anotherAdminIsOn"), T(""), kReplaceNone), 
   mErrorTableEntry(kInvalidCategory, T("invalidCategory"), T(""), kReplaceNone), 
   mErrorTableEntry(kInvalidKeyForCategory, T("invalidKeyForCategory"), T(""), kReplaceNone), 
   mErrorTableEntry(kCannotAccess, T("cannotAccess"), T(""), kReplaceNone), 
   mErrorTableEntry(kCannotSet, T("cannotSet"), T(""), kReplaceNone), 
   mErrorTableEntry(kCannotCancel, T("cannotCancel"), T(""), kReplaceNone), 
   mErrorTableEntry(kCannotSetRoField, T("cannotSetRoField"), T(""), kReplaceNone), 
   mErrorTableEntry(kSystemError, T("systemError"), T(""), kReplaceNone), 
   mErrorTableEntry(kFileWriteError, T("fileWriteError"), T(""), kReplaceNone), 
   mErrorTableEntry(kFileReadError, T("fileReadError"), T(""), kReplaceNone), 
   mErrorTableEntry(kFileStatError, T("fileStatError"), T(""), kReplaceNone), 
   mErrorTableEntry(kFileOpenError, T("fileOpenError"), T(""), kReplaceNone), 
   mErrorTableEntry(kMemAllocationFailure, T("memAllocationFailure"), T(""), kReplaceNone), 
   mErrorTableEntry(kMissingIpAddress, T("missingIpAddress"), T(""), kReplaceNone), 
   mErrorTableEntry(kCannotWriteToFlash, T("cannotWriteToFlash"), T(""), kReplaceNone), 
   mErrorTableEntry(kExceedSpaceAvail, T("exceedSpaceAvail"), T(""), kReplaceNone), 
   mErrorTableEntry(kPasswordNotMatch, T("passwordNotMatch"), T(""), kReplaceNone), 
   mErrorTableEntry(kChangePasswordFailed, T("changePasswordFailed"), T(""), kReplaceNone), 
   mErrorTableEntry(kAddUserFailed, T("addUserFailed"), T(""), kReplaceNone), 
   mErrorTableEntry(kAddRouteFailed, T("addRouteFailed"), T(""), kReplaceNone), 
   mErrorTableEntry(kMissingConnectionName, T("missingConnectionName"), T(""), kReplaceNone), 
   mErrorTableEntry(kCreateDirFailure, T("createDirFailure"), T(""), kReplaceNone), 
   mErrorTableEntry(kCheckSumCheckFailed, T("checkSumCheckFailed"), T(""), kReplaceNone), 
   mErrorTableEntry(kLoginFailed, T("LoginFailed"), T(""), kReplaceNone), 
   mErrorTableEntry(kFileDirUnavail, T("fileDirUnavail"), T(""), kReplaceNone),   //[GY]2006/Jun/20  BUG#7323
   mErrorTableEntry(kFileTransferred, T("fileTransferred"), T(""), kReplaceNone), 
   mErrorTableEntry(kFileTransferredFail, T("fileTransferredFail"), T(""), kReplaceNone), 
   mErrorTableEntry(kEditPermissionDenied, T("editPermissionDenied"), T(""), kReplaceNone), 
   mErrorTableEntry(kDeletePermissionDenied, T("deletePermissionDenied"), T(""), kReplaceNone), 
   mErrorTableEntry(kNoSocketPortConfigured, T("noSocketPortConfigured"), T(""), kReplaceNone), 
   mErrorTableEntry(kCommitPending, T("commitPending"), T(""), kReplaceNone), 
   mErrorTableEntry(kCycSidNotFound, T("cycSidNotFound"), T(""), kReplaceNone),	
   mErrorTableEntry(kWrongArgs, T("wrongArgs"), T(""), kReplaceNone),	


   /* others here!*/
   mErrorTableEntry(kInvalidPort, T("invalidPort"), T(""), kReplaceNone), 
   mErrorTableEntry(kCycLastError, T("cycLastError"), T(""), kReplaceNone),
   mErrorTableEntry(kFileRamSpaceError, T("fileRamSpaceError"), T(""), kReplaceNone),
   mErrorTableEntry(kFileCopyFlashError, T("fileCopyFlashError"), T(""), kReplaceNone),
   mErrorTableEntry(kFileWrongProductError, T("fileWrongProductError"), T(""), kReplaceNone),
   mErrorTableEnd(Result)
#endif
