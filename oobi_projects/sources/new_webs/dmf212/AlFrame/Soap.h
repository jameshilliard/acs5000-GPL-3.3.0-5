/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: Soap.h,v 1.2 2006/01/10 23:51:50 erik Exp $ */

#ifndef h_Soap
#define h_Soap

#include "Dmf.h"
#ifdef qSoap

#include "RpcUtil.h"

/*
 * An Implementation of the Simple Object Access Protocol specification 
 * (see <http://www.w3.org/TR/SOAP> for full details.
 */





/*
 * Initialize the Xml-Rpc service.
 */
extern int dmfInitSoap(void);

/*
 * Clean up the Xml-Rpc service.
 */
extern void dmfCloseSoap(void);


/*
 * "GoForms" handler that consumes XML-RPC messages (contained in the query
 * string, originating in the body of a POST message and attempts to use them
 * to call functions on the device.
 */
extern void dmfSoapHandler(webs_t wp, char_t *path, char_t *query);

/*
 * Adds the specified Xml-RPC handler function to the system. Note that we do
 * not support removing a function from the system once it's been added.
 */
extern int dmfSoapAddFunction(char_t* nameSpace, char_t* methodName, RpcFunc func);

#endif   /* ifdef qSoap */
#endif   /* #ifndef h_Soap */

/*../dmf212/AlFrame/Soap.h:46:30: warning: no newline at end of file*/

