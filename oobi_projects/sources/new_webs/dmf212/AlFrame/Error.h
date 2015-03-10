/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */

#ifndef h_Error
#define h_Error

#include "Dmf.h"
#include "ErrorCodes.h"

/*
 * By default, after calling one of our ejScript functions, a variable in the
 * ejScript with the following name will magically contain a text version of
 * the resulting error code (see 'ErrorCodes.h' for the table of error codes.
 */
#define kErrorCodeVariable T("__error__")
#define kSuccessVariable T("__success__")


/*
 * We also define several ephemeral local variables that may be accessed from
 * a page reporting an error code (using rich error pages). Change these to
 * change the variable names to use...
 */
#define kRichErrorCode  T("_errorCode_")
#define kRichErrorText  T("_errorText_")
#define kRichErrorMsg   T("_errorMessage_")
#define kRichErrorUrl   T("_errorUrl_")


/*
 * if qRichErrorPage is defined, this controls the mapping of error codes to 
 * file names -- in the default case, error code 404 would be handled by a
 * page named 'error404.asp'
 */

#define kErrorPageFormat T("error%d.asp")
#define kErrorDefaultPage T("error.asp")


/*
 * Errors that need to persist across pages are stored in the following 
 * structure. If session management is enabled, each user logged into the
 * server has their own error table.
 */
#define kStartingErrorCapacity 2

typedef struct ErrorsTAG
{

   char_t** errorTable;

   /*
    * ...and the current number of errors.
    */
   size_t errorCount;
   /*
    * How many errors can we hold without needing to resize?
    */
   size_t errorCapacity;

   /*
    * path of the page that is able to ask about errors...
    */
   char_t* errorPage;

} Errors;



extern int dmfEjGetLastErrorCode(int eid, webs_t wp, int argc, char_t** argv);

extern int dmfEjResetError(int eid, webs_t wp, int argc, char_t** argv);

extern int dmfEjGetErrorCount(int eid, webs_t wp, int argc, char_t** argv);

extern int dmfEjGetErrorMessageByIndex(int eid, webs_t wp, int argc, char_t** argv);

extern int dmfEjGetErrorNameByIndex(int eid, webs_t wp, int argc, char_t** argv);


extern Errors* dmfCreateErrors(void);

extern void dmfDestroyErrors(Errors* err);

extern void dmfClearErrors(webs_t wp);

extern char_t* dmfFormatErrorString(int code, char_t* parm1, char_t* parm2, 
 char_t* parm3);

extern void dmfAddError(webs_t wp, int code, char_t* parm1, char_t* parm2, 
 char_t* parm3);

/*
 * Create an app-level ErrorStrings list, like the DMF-level one called
 * kErrorStrings (found in ErrorCodes.h), then call this so that your
 * app-level errors can be returned just like DMF errors and accessed from
 * server-side code just like DMF errors.  Note that you must use mAppError(id)
 * (e.g., return mAppError(kBilboError).
 * lastErrorId refers to the code/id of the last error in your array..
 */
extern void dmfSetAppErrors(ErrorStrings* errorStrings, int lastErrorId);
#define mAppError(id) (id ? id + kDmfLastError : 0)

/*
 * To avoid displaying error lists on the wrong page (e.g., the user submits a
 * form with errors, then manually navigates to another page that looks for
 * errors), we also store the specified urlError page when handling a form
 * with errors. The functions to get error counts and error messages will act
 * as if there are no errors when those functions are being called from a page
 * other than the currently designated error page
 */
extern void dmfSetErrorPage(webs_t wp, char_t* errorPage);


/*
 * given a string in our internal delimited format, reformat it into a 
 * string that's suitable for human readable output. If the parameter 
 * 'alFrameCode' is not NULL, we set it to the integer code used by the 
 * framework.
 */
extern char_t* dmfFormatError(char_t* errorString, int* alFrameCode);

extern int dmfGetErrorCount(webs_t wp);


/*
 * returns the specified error name (from the internal table of error 
 * names, or "" if the code is not a valid error code.
 */
extern char_t* dmfGetErrorName(int code);

/*
 * looks up and returns the specified error message. If the parameter 
 * 'alFrameCode' is not NULL, the Dmf error code associated with this error 
 * is returned.
 */
extern int dmfGetErrorString(webs_t wp, char_t* index, int unFormatted, 
 int* alFrameCode,  char_t** errorString);

extern int dmfGetErrorCodeImpl(void);
 
/*
 * Sets the value of the current error code. The value may be retrieved
 * explicitly by calling the ejScript function 'getError();' or implicitly by
 * checking the value of the magic error code variable. By default, the name
 * of this variable is 'errorCode' -- you may change it for your server by
 * modifying the value of `kErrorCodeVariable' above.
 */
extern void dmfSetErrorCode(int eid, int code);



extern int dmfRichError(webs_t wp, int code, char_t* userMsg);
#endif
