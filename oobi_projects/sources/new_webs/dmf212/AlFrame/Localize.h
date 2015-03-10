/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: Localize.h,v 1.1.1.1 2005/09/01 02:36:38 edson Exp $ */



#ifndef h_Localize
#define h_Localize




#include "Dmf.h"

#ifdef qLocalize

/*
 * Populate the internal database with key/values for the currently selected
 * language. 
 * We do NOT support per-user language, or look at
 * incoming HTTP headers to decide what language to use.
 *
 * The 'languageCode' parameter is a two character code as found in  
 * ISO 639 -- http://ftp.ics.uci.edu/pub/ietf/http/related/iso639.txt
 *
 * If the requested language database is not found, returns kDmfError,
 * otherwise returns kDmfSuccess.
 */

extern int dmfSetLanguage(char_t* langCode);

/*
 * Set the path to the language.* files.  With or without trailing '/'.
 * Typically, this will be something like "../locale/".
 */
extern int dmfSetLanguagePath(char_t* languagePath);

/*
 * return the current two-character language code.
 */
extern const char_t* dmfGetLanguage(void);

/*
 * Load the language database for the 'current' language into our internal
 * format, ready for use.
 */

extern int dmfLoadLanguage(void);

/*
 * Close the language database (i.e. when the server is shutting down...
 */
extern void dmfCloseLanguage(void);

/*
 * Function to actually look up the translated value associcated with a key.
 * Used internally by both dmfEjgetLabel and dmfEjGetLabelVal. The value that
 * gets returned in **label on success is a pointer to the internal symbol
 * table entry -- do NOT delete this.
 */
extern int dmfGetLabel(char_t* key, char_t** label);

/*
 * dmfGetLabel -- ASP function to retrieve the label associated with a
 * parameter. Writes the value directly to the user's browser, or writes the
 * appropriate error message in its place.
 * Called from ejScript as:
 * <% getLabel("my.parm"); write(" = "); get("my.parm"); %>
 */
extern int dmfEjGetLabel(int eid, webs_t wp, int argc, char_t **argv);


/*
 * dmfGetLabelVal -- ASP function to retrieve the label associated with a
 * parameter. Returns the label to the ejScript interpreter and sets the
 * __error__ variable appropriately.
 */
extern int dmfEjGetLabelVal(int eid, webs_t wp, int argc, char_t **argv);

/*
 * dmfEjGetLanguageCode -- returns the code of the currently selected
 * language. If called with arguments, the first argument is the 'default'
 * language code. If the current language is the default language, the
 * function returns nothing. If a second argument is provided and the current
 * language is not the default language, the second argument is appended to
 * the language code before being returned.
 *
 * This may be used as a utility to assemble language-specific paths at
 * runtime, like:
 * <img src="/images/<%getLanguage("en", "/");%>myImage.gif" (...etc...)
 * which will expand to 
 * <img src="images/myImage.gif" (...etc...)
 * if the current language is english, but if the current language is Scots
 * Gaelic, will instead expand to:
 * <img src="images/gd/myImage.gif" (...etc...)
 */
extern int dmfEjGetLanguageCode(int eid, webs_t wp, int argc, char_t **argv);



#endif   /* qLocalize */

#endif   /* h_Localize */
