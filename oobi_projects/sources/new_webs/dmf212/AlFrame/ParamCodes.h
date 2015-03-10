/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: ParamCodes.h,v 1.2 2006/01/10 23:51:50 erik Exp $ */


#if defined(qXmlRpcTypes) || defined(qSoapTypes)
#undef h_ParamCodes
#endif

#ifndef h_ParamCodes
#define h_ParamCodes

#undef mParamTableBegin
#undef mParamTableEntry
#undef mParamTableEnd



#if defined(qXmlRpcTypes)
#define mParamTableBegin(name) const char_t* kXmlRpcTypes[] = {
#define mParamTableEntry(id, name, soapName) name
#define mParamTableEnd };
#undef qXmlRpcTypes


#elif defined (qSoapTypes)
#define mParamTableBegin(name) const char_t* kSoapTypes[] = {
#define mParamTableEntry(id, name, soapName) soapName
#define mParamTableEnd };
#undef qSoapTypes


#else
#ifdef qSoap
#endif
#define mParamTableBegin(name) enum name {
#define mParamTableEntry(id, name, soapName) id
#define mParamTableEnd };

#ifdef qXmlRpc
extern const char_t* kXmlRpcTypes[];

#endif

#ifdef qSoap
extern const char_t* kSoapTypes[];
#endif

#endif


mParamTableBegin(RpcDataType)
   mParamTableEntry(kNoType = 0, T(""), T("")),
   mParamTableEntry(kInteger, T("int"), T("xsd:int")),
   mParamTableEntry(kBoolean, T("boolean"), T("xsd:boolean")),
   mParamTableEntry(kString, T("string"), T("xsd:string")),
   mParamTableEntry(kDouble, T("double"), T("xsd:float")),
   mParamTableEntry(kDateTime, T("dateTime.iso8601"), T("xsd:timeInstant")),
   mParamTableEntry(kBase64, T("base64"), T("SOAP-ENC:base64")),
#ifdef qSoap
   /* SOAP Array type. Do not enter this type in your parameter table, it's 
    * only used internally by the framework when handling SOAP arrays.
    */
   mParamTableEntry(kSoapArray, T(""), T("SOAP-ENC:Array")),
#endif
   /* insert additional (private) data types here.... */
   mParamTableEntry(kBeginSection = 1000, T(""), T("")),
   mParamTableEntry(kEndSection, T(""), T(""))
mParamTableEnd 


#endif
/*../dmf212/AlFrame/ParamCodes.h:71:7: warning: no newline at end of file*/

