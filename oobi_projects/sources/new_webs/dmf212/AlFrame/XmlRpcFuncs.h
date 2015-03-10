/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: XmlRpcFuncs.h,v 1.1.1.1 2005/09/01 02:36:38 edson Exp $ */

#ifndef h_XmlRpcFuncs
#define h_XmlRpcFuncs

#include "Dmf.h"
#if defined(qXmlRpc) || defined(qSoap)


#include "RpcUtil.h"

/*
 * If there are errors, packages them all up and sends them back packaged like:
 * <?xml version="1.0"?> 
 *    <methodResponse>    
 *       <params>       
 *          <param>          
 *             <array>
 *                <data>
 *                   <value>
 *                      <struct>
 *                         <member>
 *                            <name>errorCode</name>
 *                            <value><string>errorName</string></value>
 *                         </member>
 *                         <member>
 *                            <name>errorString</name>
 *                            <value><string>Some error string
 *                            here...</string></value>
 *                         </member>
 *                      </struct>
 *                   </value>
 *                   <value>
 *                      <struct>
 *                         <member>
 *                            <name>errorCode</name>
 *                            <value><string>errorName</string></value>
 *                         </member>
 *                         <member>
 *                            <name>errorString</name>
 *                            <value><string>Some error string
 *                            here...</string></value>
 *                         </member>
 *                      </struct>
 *                   </value>
 *          </param>       
 *       </params>    
 *    </methodResponse>
 *
 *    ...and then clears the errors.
 *
 *    In SOAP (which shares this function) the output looks something like: 
 * 
 *   <SOAPENV:Envelope xmlns:SOAPENV="http://schemas.xmlsoap.org/soap/envelope/" 
 *    SOAPENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" 
 *    xmlns:SOAPENC="http://schemas.xmlsoap.org/soap/encoding/" 
 *    xmlns:xsi="http://www.w3.org/2001/XMLSchemainstance">
 *       <SOAPENV:Body>
 *          <d:getErrorsResponse xmlns:d="http://www.artlogic.com/dmf/">
 *             <Result>
 *                <errors SOAPENC:arrayType="xsd:urtype[3]" xsi:type="SOAPENC:Array">
 *                   <Item>
 *                      <errorCode xsi:type="xsd:string">GroupExists</errorCode> 
 *                      <errorString xsi:type="xsd:string">
 *                         `admin': Group already exists
 *                      </errorString> 
 *                   </Item>
 *                   <Item>
 *                      <errorCode xsi:type="xsd:string">GroupExists</errorCode> 
 *                      <errorString xsi:type="xsd:string">
 *                         `users': Group already exists
 *                      </errorString> 
 *                   </Item>
 *                   <Item>
 *                      <errorCode xsi:type="xsd:string">UserExists</errorCode> 
 *                      <errorString xsi:type="xsd:string">
 *                         `adminUser': User already exists
 *                      </errorString> 
 *                   </Item>
 *                </errors>
 *             </Result>
 *          </d:getErrorsResponse>
 *       </SOAPENV:Body>
 *     </SOAPENV:Envelope>
 *
 */
extern int dmfXmlRpcGetErrors(webs_t wp, RpcTree* params, RpcTree* result);

#endif

#ifdef qXmlRpc


/*
 * Function to get the value of a parameter from an XmlRpc call.
 * We expect this function to be called like:
 * get('parmName'[, index])
 * It returns the value of the specified parameter, or an error if the
 * parameter doesn't exist, etc.
 * Some unusual aspects of this function include:
 *
 *  - If the requested parameter is indexed, but no index is provided, this
 *  function will return the value of each indexed value between its
 *  minIndex and its maxIndex (as an XmlRpc array)
 *
 *  - If the requested parameter is a section name, we return all the
 *  values in that section as a struct. If there are indexed values in 
 *  this section, it packages them into an array. If there are other
 *  sections under this one, it nests them as structs inside the top level
 *  struct. This means that calling get() on the topmost section of our
 *  parameter list will return the entire current state of the device in a
 *  single call! 
 */
extern int dmfXmlRpcGet(webs_t wp, RpcTree* params, RpcTree* result);

/*
 * Function to set the value of a single (possibly indexed) parameter on the
 * device. For the time being (anyway) we explicitly do NOT support setting
 * multiple parameters at the same time (see our Get implementation, which
 * does support retrieving multiple parameters).
 *
 * We expect to be called as
 * set(parmName, [index, ] newValue);
 * If the set is successful, it will send back 
 *
 *  <?xml version="1.0"?>
 *   <methodResponse>
 *    <params>
 *     <param>
 *      <struct>
 *      <value><int>0</int></value>
 *     </param>
 *    </params>
 *   </methodResponse>
 *
 * Otherwise, it will return the appropriate fault structure (see the xml-rpc
 * spec).
 */
extern int dmfXmlRpcSet(webs_t wp, RpcTree* params, RpcTree* result);

/*
 * Function to query the current translation of a label key. The current
 * implementation only supports querying a single label per xml-rpc call, but
 * we can obviously extend this to query an array of labels on each call.
 */
#ifdef qLocalize
extern int dmfXmlRpcGetLabel(webs_t wp, RpcTree* params, RpcTree* result);
#endif



#endif // qXmlRpc

#endif // h_XmlRpcFuncs
