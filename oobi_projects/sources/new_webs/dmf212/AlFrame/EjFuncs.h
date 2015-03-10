/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: EjFuncs.h,v 1.1.1.1 2005/09/01 02:36:38 edson Exp $ */



#ifndef h_EjFuncs
#define h_EjFuncs

#include "Dmf.h"

/*
 * dmfGet -- ASP function to retrieve the current value of a key.
 * It will be registered as an ASP function as part of dmfInit.
 * The ASP function will be called like:
 * <% get("keyName"); %>.
 * When the page is served, the current value associated with 
 * 'keyName' will be written into the page being served. If there 
 * is no value for the requested key, <<!!! Hm -- what should we do?
 * Write in "keyName = UNKNOWN" ?? >>
 *
 * If two parameters are passed to the function, the second one will 
 * be used as an 'index' parameter, and the first will be 
 * used as the key. The index parameter may or may not have meaning 
 * to the underlying get function.
 *
 * Note that the framework makes no assumptions that the 'index' parameter is
 * an integer -- if it makes sense in your application, you may feel free to
 * use a textual index (like an associative array).
 */
extern int dmfEjGet(int eid, webs_t wp, int argc, char_t** argv);



/*
 * dmfSet -- ASP function to set the current value of a key.
 * It will be registered as an ASP function as part of dmfInit.
 * The ASP function will be called like:
 * <% set("keyName", "value"); %>
 * !!! Presumably, this can be called through ASP if a page is served with 
 * a query string.!!! 
 */ 
extern int dmfEjSet(int eid, webs_t wp, int argc, char_t** argv);

/*
 * alfEjGetVal -- The 'dmfEjGet' function is an ASP function that 
 * looks up a parameter and returns its value to the ejScript interpreter.
 * This is a similar function to dmfGet, except it lets you use the 
 * value returned in server-side ejScript code.
 */
extern int dmfEjGetVal(int eid, webs_t wp, int argc, char_t **argv);

#ifndef qLocalize
/*
 * dmfGetLabel -- ASP function to retrieve the label associated with a
 * parameter. 
 */
extern int dmfEjGetLabel(int eid, webs_t wp, int argc, char_t **argv);
#endif

/*
 * dmfGetLastErrorCode -- ejScript function that returns the current (most
 * recent) error code as a string (see the table in ErrorCodes.h).
 * called as 
 * <% code = getLastError();%>
 * Expects no arguments.
 */
extern int dmfEjGetLastErrorCode(int eid, webs_t wp, int argc, char_t** argv);

/*
 * dmfGetErrorCount -- ejScript function to return the current number of
 * errors that need to be reported. 
 * called as 
 * <% count = getErrorCount(); %>
 * Expects no arguments.
 */
extern int dmfEjGetErrorCount(int eid, webs_t wp, int argc, char_t** argv);

/*
 * dmfGetErrorMessage -- ejScript function to retrieve a specific error
 * set by a previous form page (or call to Set())
 * called as:
 * getErrorMessage(index [, notFormatted]);
 * where index is 0..(error count - 1).
 * If the notFormatted argument is 1, returns the internal representation of
 * the error string, which is keyword/value pairs like so:
 * "code=numericErrorCode&error=errorText&key=keyName&index=indexVal&val=Value"
 * otherwise, formats the error message for semi-useful output. 
 * May set the current error code for this page           
 *
 * example:
 * <% 
 *    count = getErrorCount();
 *    for (i = 0; i < count; i++)
 *    {
 *       msg = getErrorMessage(i, 0);
 *       write(msg);
 *    }
 * %>
 */
extern int dmfEjGetErrorMessageByIndex(int eid, webs_t wp, int argc, char_t** argv);

/* dmfEjGetErrorNameByIndex -- ejScript function to retrieve the error name
 * (errorName in ErrorStrings struct) of the error identified by index, where
 * index is between 0 and GetErrorCount().
 *
 * example:
 * <% 
 *    count = getErrorCount();
 *    for (i = 0; i < count; i++)
 *    {
 *       name = getErrorName(i);
 *       getLabel(name); // output the associated error string in language.*
 *    }
 * %>
 */
extern int dmfEjGetErrorNameByIndex(int eid, webs_t wp, int argc, char_t** argv);

/*
 * returns the current time at the server as number of seconds since 
 * the epoch
 * Called as:
 * <% var now = getTime(reference);%>
 * If 'reference' is "UTC" or "GMT", the local time will be converted to
 * UTC/GMT. Otherwise (including if 'reference' blank) the local time at the 
 * server will be returned.
 */
extern int dmfEjGetTime(int eid, webs_t wp, int argc, char_t **argv);

/*
 * Exposes the functionality of strftime to ASP code. Available only in 
 * an ejScript version right now. 
 *
 * from ASP code, call FormatTime('timeSinceEpoch', "format");
 * 
 * The format argument consists of one or more codes; as in printf, the
 * formatting codes are preceded by a percent sign (%). Characters that do not
 * begin with % are copied unchanged to strDest. The LC_TIME category of the
 * current locale affects the output formatting of strftime.(For more
 * information on LC_TIME, see setlocale.) The formatting codes for strftime
 * are listed below:
 *
 * %a  Abbreviated weekday name
 *
 * %A  Full weekday name
 *
 * %b  Abbreviated month name
 *
 * %B  Full month name
 *
 * %c  Date and time representation appropriate for locale
 *
 * %d  Day of month as decimal number (01  31)
 *
 * %H  Hour in 24-hour format (00  23)
 *
 * %I  Hour in 12-hour format (01  12)
 *
 * %j  Day of year as decimal number (001  366)
 *
 * %m  Month as decimal number (01  12)
 *
 * %M  Minute as decimal number (00  59)
 *
 * %p  Current locales A.M./P.M. indicator for 12-hour clock
 *
 * %S  Second as decimal number (00  59)
 *
 * %U  Week of year as decimal number, with Sunday as 
 * first day of week (00 * 53)
 *
 * %w  Weekday as decimal number (0  6; Sunday is 0)
 *
 * %W  Week of year as decimal number, with Monday 
 * as first day of week (00 * 53)
 *
 * %x  Date representation for current locale
 *
 * %X  Time representation for current locale
 *
 * %y  Year without century, as decimal number (00  99)
 *
 * %Y  Year with century, as decimal number
 *
 * %z, %Z  Time-zone name or abbreviation; no characters 
 * if time zone is unknown
 *
 * %%  Percent sign
 *
 * As in the printf function, the # flag may prefix any formatting code.
 * In that case, the meaning of the format code is changed as follows.
 *
 * Format Code Meaning 
 * %#a, %#A, %#b, %#B, %#p, %#X, %#z, %#Z, %#% # flag is ignored. 
 * 
 * %#c Long date and time representation, appropriate for current
 * locale. For example: Tuesday, March 14, 1995, 12:41:29. 
 * 
 * %#x Long date representation, appropriate to current locale. For
 * example: Tuesday, March 14, 1995. 
 * 
 * %#d, %#H, %#I, %#j, %#m, %#M, %#S, %#U, %#w, %#W, %#y, %#Y 
 */ 

extern int dmfEjFormatTime(int eid, webs_t wp, int argc, char_t **argv);



/*
 * Little Utility ASP functions. 
 */

/*
 * dmfMinVal -- called from ejScript as min(x, y);
 * returns the lesser value of the two parameters.
 */
extern int dmfEjMinVal(int eid, webs_t wp, int argc, char_t **argv);


/*
 * dmfMaxVal -- called from ejScript as max(x, y);
 * returns the larger of the two parameters.
 */
extern int dmfEjMaxVal(int eid, webs_t wp, int argc, char_t **argv);

/*
 * dmfGetQueryVal -- called from ejScript as 
 * var val = getQueryVal("queryKey", "defaultVal");
 * If the specified query string is present (or any of the 
 * standard CGI variables), will return its value, otherwise will
 * return the specified default.
 */
extern int dmfEjGetQueryVal(int eid, webs_t wp, int argc, char_t **argv);

/*
 * dmfDebugInfo -- prints out the entire list of parameters that 
 * may be retrieved from the device and their current
 * values.
 */
extern int dmfEjDebugInfo(int eid, webs_t wp, int argc, char_t **argv);

/*
 * returns the specified cookie value (if present) to the ejScript 
 * interpreter.
 * Called as:
 * <% var val = getCookieVal("cookieName"); %>
 * if no parameter is passed to the function, returns the entire
 * set of active cookies.
 * if no cookie has been set for the specified cookie name, returns 
 * "" (an empty string)
 */
extern int dmfEjGetCookieVal(int eid, webs_t wp, int argc, char_t **argv);



/*
 * ASP function to return the request number of the page currently being 
 * served.
 */
extern int dmfEjGetRequestNumber(int eid, webs_t wp, int argc, char_t **argv);

/*
 * ASP function to find out the authentication type being used
 * 
 * Called as:
 * <% var val = getAuthType(); %>
 *
 * Return codes:
 *   0 -> authtype != none
 *   1 -> authtype == none
 */
extern int dmfEjSplit(int eid, webs_t wp, int argc, char_t **argv);
#endif


