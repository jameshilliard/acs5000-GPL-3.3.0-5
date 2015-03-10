/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: Param.h,v 1.1.1.1 2005/09/01 02:36:38 edson Exp $ */


#ifndef h_Param
#define h_Param

#include "webs.h"

#include "Dmf.h"
#include "ParamCodes.h"



/* forward ref so we may refer to the param struct in the 
 * following typedefs...  
 */
struct ParamTAG; 

typedef int (*GetFunc)(const char_t* /*key*/, const char_t* /*index*/, 
 char_t** /*value*/, void* /*rawData*/, struct ParamTAG* /*param*/, 
 int /*request #*/);

typedef int(*SetFunc)(const char_t* /*key*/, const char_t* /*index*/, 
 char_t* /*value*/, void* /*rawData*/, struct ParamTAG* /*param*/, 
 int /*request #*/);

typedef int (*QueryFunc)(struct ParamTAG* /*param*/, int /*request #*/);

typedef int (*CommitFunc)(struct ParamTAG* /*param*/, int /*request #*/, char_t* /*urlQuery*/);



typedef struct ParamTAG
{
   /* parameter name as used in the ejScript. If the parameter space is
    * hierarchical, this key should only be the final part of the parameter
    * name. For example, if the full name of this parameter will be 
    * "level1.level2.keyName", the value of 'key' for this parameter should be
    * 'keyName'
    */
   const char_t* const key;

   /* enum indicating the type of this  value (see list of available types in
    * "ParamCodes.h". Used to demarcate hierarchy points (types kBeginSection
    * and kEndSection), also used for xml-rpc output.
    */
   enum RpcDataType dataType;  

   /* integer code indicating the minimum access  level required to read 
    * or modify this parameter.   If the value is zero, then the parameter 
    * may be  accessed without limit. If it is '-1', it inherits  its 
    * access control from its parent.  
    */
   int   readAccess;
   int   writeAccess;            

   /* standard or custom function to convert this  parameter to its string 
    * representation.   If NULL, this parameter is write only.  
    */
   GetFunc getFunc;              
   
   /* standard or custom function to convert this  parameter from string to 
    * its real  value.   If NULL, this parameter is read only.  
    */   
   SetFunc setFunc;              
                                    
  /* (optional) called before each time a  getFunc() is called. This allows 
   * us to  only make a trip to the device once per  ASP page request -- we 
   * pass in the  current request number and the query  function can do 
   * nothing if the request  number is equal to the last one it  handled. We 
   * install a custom URL  handler to track the request numbers.   if this 
   * is NULL, either we are using  a custom GetFunc that handles this  logic 
   * itself, or the data is not  volatile -- we can be certain that a  local 
   * copy of this parameter is what's  in the device (or there is no  
   * corresponding parameter in the device) 
   */ 
   QueryFunc queryFunc; 
   
   /*
    * lastQuery -- stores the request cycle number of the last time this
    * parameter was queried. Used to batch queries -- the query function
    * should be written so that if the request currently being processed is
    * equal to the last query processed, nothing happens, since we already
    * have current data.
    */
   int lastQuery;
   
  
   /* (optional) -- if present, called at the  end of the generic form handler. 
    * This  lets us accumulate multiple parameter  sets into a single 
    * structure, then  commit the entire structure at once.   if NULL, either 
    * there is no  corresponding parameter inside the  device, or the SetFunc 
    * for this  parameter is responsible for persisting  the value.  
    */
   CommitFunc commitFunc;        

   /* lastCommit -- stores the last request cycle that this parameter was
    * committed on. If the current request cycle is equal to the last request
    * cycle, the commit function should not do anything.
    */
   int lastCommit;
     

   /* if not NULL, the location of this  parameter in memory.  */ 
   void* dataStart;              
   
   /* if != 0, the data is in an array, and  this indicates the offset 
    * between  successive instances of this data type.  
    */
   unsigned int offset;          
   
   /* points to an integer containing the  first valid index number for 
    * this  parameter, or NULL if not indexed.  
    */
   const int* startIndex;        
   
   /* points to an integer containing one  after the last valid index 
    * number for  this parameter (so we can easily use a  simple '<' test 
    * to validate index ranges,  or NULL if the parameter is not indexed.  
    */
   const int* endIndex;          
   
   
   /* if lowerLimit < upperLimit, the built-in set  will perform a range 
    * check before doing  the set operation (where possible). On  numeric values, 
    * used to indicate valid  values, on string values, indicates valid  string 
    * lengths.    If lowerLimit >= upperLimit, we do not perform  any range 
    * checking.  
    */
   int lowerLimit;
   int upperLimit;               
                                     
   /* if we are in a system that's arranging things  hierarchically, this 
    * is the index of our parent  parameter in the hierarchy. Should always  
    * be set to -1 at compile time.  
    */
   int parent;                   
} Param;


extern Param parameterList[];
extern int parameterCount;


#endif
