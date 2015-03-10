/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: Dmf.h,v 1.1.1.1 2005/09/01 02:36:38 edson Exp $ */



#ifndef h_Dmf
#define h_Dmf

#ifdef __cplusplus
extern "C" {
#endif


#include "ejIntrn.h"
#include "emfdb.h"
#include "webs.h"


/*
 * Configuration Macros.
 *
 * The DMF uses a set of compiler macros to include or omit major feature
 * sets. Most of these only impact the code within the DMF itself, and may be
 * controlled by editing the lines of source below. However, some change the
 * behavior of the GoAhead WebServer source. To avoid the necessity of
 * including this header file in the GoAhead source, those macros must be
 * defined from the command line when compiling the GoAhead source.
 *
 * Command Line Macros:
 * qDmf -- must be defined to cause the GoAhead code to call our framework's
 *    entry points.
 * qHierarchicalAccess -- must be defined to use the DMF's hierarchical access
 *    control.
 * qRichErrorPage -- must be defined to use the DMF's enhanced error page
 *    mechanism.
 *
 * Macros that may be changed in this header file:   
 * Uncomment a line to include the feature, comment it to remove.
 * Note that if you require multiple builds with different feature sets, it
 * may be most convenient in your situation to instead use all these macros at
 * the command line to simplify project maintenance. 
 * Since these macros may affect the behavior of other #include files used in 
 * the project, we list them here, before the framework include files.
 */

/* XML-RPC support */
#define qXmlRpc

/* SOAP support  */
#define qSoap

/* SNMP inheritance support (future) */
/*#define qSnmp*/

/* SMTP outbound mail support (future) */
/*# define qSmtp */

/* Include Localization code */
#define qLocalize

/* symbol tables for server variables and language localization are compiled
 * into the image, not read from disk (Future) */
/* #define qCompiledSymbols */

/* Use session management (timeouts & per-user error tracking) */
#define qSession



/*
 *    All DMF functions should be named with the 3-letter module prefix
 *    'dmf'  -- like:
 *    int dmfSomeImaginaryFunction(char_t* name);
 */   
#include "AlTypes.h"
#include "ErrorCodes.h"
#include "Param.h"
   


  

/*
 * UNICODE defines -- The list of macros in GoAhead's file uemf.h provides an
 * easy way to provide support for either single byte or double byte
 * characters by defining the macro "UNICODE". However, that list doesn't
 * provide everything that the DMF requires, so we add a few here. Note that
 * we use the GoAhead naming style here, not Art & Logic naming style --
 * function names are prefixed with a lowercase 'g'.
 */
#ifdef UNICODE
   
#define gstrftime wcsftime
#define gfopen    _wfopen
#define gfprintf  fwprintf
#define gfgets    fgetws   
#else

#define gstrftime strftime   
#define gfopen    fopen
#define gfprintf  fprintf
#define gfgets    fgets
#endif   





/*
 * dmfInit -- any initialization required for this module should 
 * be performed in this function. Additionally, a call to this function
 * must be placed in the initWebs() function in main.c
 * NOTE that this will need to be done per-platform, as each platform
 * has its own version of main.c. Also note that Dmf.h will need to 
 * be included in main.c
 *
 *
 * returns: 0 on success
 *          -1 on failure
 */
extern int dmfInit(void);

/*
 * alfUserPreInit will be called from 'dmfInit()' before it's done 
 * initialization.
 *
 * NOTE -- This function must be defined in your project-specific code,
 * typically in your UserFunc.c file.
 */
extern void dmfUserPreInit(void);

/*
 * alfUserPostInit will be called from 'dmfInit()' just before it 
 * returns. This is a good place to set/initialize any 'local variables' 
 * that will be accessed using the 'alfSetLocal'/'dmfGetLocal'
 * functions.
 *
 * NOTE -- This function must be defined in your project-specific code,
 * typically in your UserFunc.c file.
 */
extern void dmfUserPostInit(void);

/*
 * alfUserPreClose will be called from 'dmfClose()' before it does 
 * any of its cleanup. All of the Dmf framework code is still 
 * available at this point.
 *
 * NOTE -- This function must be defined in your project-specific code,
 * typically in your UserFunc.c file.
 */
extern void dmfUserPreClose(void);

/*
 * alfUserPostClose will be called from 'dmfClose()' just before it 
 * returns. None of the Dmf framework code can be called -- this
 * is your last opportunity to do any cleanup that may be needed for 
 * your system.
 *
 * NOTE -- This function must be defined in your project-specific code,
 * typically in your UserFunc.c file.
 */
extern void dmfUserPostClose(void);




/*
 * dmfInitParams -- this function must be called from a project's
 * dmfUserPreInit() or dmfUserPostInit() functions (found in that project's
 * UserFunc.c file, typically.)
 * This function converts a flat array of Param structures into the tree
 * structure that we use internally.
 */
extern void dmfInitParams(Param* parameters, int parameterCount);


/*
 * dmfClose -- Called from the main() function, after the main 
 * loop has finished. Anything that needs cleaning up in this
 * module gets cleaned up here.
 */
extern int dmfClose(void);


/*
 * If user code calls this function from its dmfUserPreInit() function, 
 * the framework will initialize the user management system, reloading the
 * specified filename as the user database.
 */
extern int dmfSetAccessFile(char_t* filename);



/*
 * adds the specified error code and strings to the list of errors 
 */
extern void dmfAddError(webs_t wp, int code, char_t* key, char_t* index, 
 char_t* val);


   /*
    * sets the local variable kDmfErrorCode to the appropriate string
    * constant.
    */
extern void dmfSetErrorCode(int eid, int code);




/*
 * Utility function to get a pointer to a Param data structure given its key
 * name. Mostly used in the {Get|Set}Param functions, but available elsewhere
 * as well, should the need arise.
 * given a key, returns either a pointer to our Param struct representing the
 * key, or NULL if there's no parameter by that name in the system.
 */

extern int dmfFindParam(char_t* key);


extern Param* dmfGetParamPtr(int parmIndex);


/*
 * Utility function to access the request number of the page currently being
 * served.
 */
extern int dmfGetRequestNumber(void);

/*
 * Increments the request number, returns the new value.
 */
extern int dmfIncrementRequestNumber(void);


/*
 * !!! TODO: Document this.
 */

typedef struct CommandStructTAG
{
   char_t* command;  /* name of the current command*/
   char_t* queryString; /* the entire current query string */
   webs_t  wp;       /* the web server itself (used for
                      * websWrite(), etc...
                      */
} CommandStruct;

typedef int (*CmdFunc)(CommandStruct* command);



extern int dmfCommand(CommandStruct* cmd);


/*
 * write the specified symbol table into a file, using keyword/value pair
 * format:
 * key1=value1
 * key2=value2
 *
 * etc.
 * Note that the symbol tables are NOT maintained in a sorted order, so one
 * shouldn't be expecting sorted output.
 */
extern int dmfPersistSymbolTable(sym_fd_t table, char_t* fileName);

/*
 * Attempts to convert the specified file back into a symbol table. Assumes
 * that each line of the file is in the format:
 * key1=value1
 * key2=value2
 *
 * etc.
 * If not, results are undefined.
 *
 * the 'type' parameter is either string or integer (no quotes, it's an enum
 * value)
 */
extern int dmfReloadSymbolTable(sym_fd_t table, char_t* fileName, int type);


/*
 * typedef for the GoAhead 'ASP Function'. For some reason, they don't 
 * typedef this, but  instead repeat the signature literally wherever 
 * it's used.
 */
typedef int (*AspFunc)(int ejid, void* wp, int argc, char_t **argv);



#ifdef __cplusplus
}
#endif

#endif   /* this must be the final line in this file. */
