/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: RpcUtil.h,v 1.1.1.1 2005/09/01 02:36:38 edson Exp $ */

#ifndef h_RpcUtil
#define h_RpcUtil

#include "Dmf.h"
#if defined(qXmlRpc) || defined(qSoap)

#include "StringBuf.h"

#ifdef qSoap
enum SoapState
{
   kNoState = 0,
   kEnvelope,
   kHeader,
   kBody
};
#endif




enum RpcNodeType
{
   kValueNode = 0,
   kStructNode,
   kArrayNode,
   kNullNode   /* placeholder, pointing to nothing. */
};

/*
 * Key to everything here is the RpcNode. This can point to:
 *    - an RpcValue struct (holds a single value)
 *    - an RpcStruct structure (holds 1 or more key/value pairs, which may
 *    nest
 *    - an RpcArray struct, which holds 1 or more values (which may nest).
 */


typedef struct RpcNodeTAG
{
   /*
    * we know which kind of struct we're pointing to by the type code
    * in the rpcType member.
    */
   enum RpcNodeType nodeType;

   /*
    * under xml-rpc, only struct members have names. Under SOAP, everything 
    * has a name.
    */
   char_t* name;

   /*
    * the nodeData member may point to either a:
    *  RpcValue  struct
    *  RpcStruct struct
    *  RpcArray  struct
    */
   void*             nodeData;

   /*
    * ...and we point to the next node in our list (if any)
    */
   struct RpcNodeTAG*    next;  

} RpcNode;





typedef struct RpcValueTAG
{
   /*
    * we always store values as a string
    */
   char_t*  value;
   /*
    * The enum (see paramcodes.h) tells what the type of this data is.
    */
   enum RpcDataType type;

} RpcValue;




typedef struct RpcStructTAG
{
   RpcNode*  head;
   RpcNode*  tail;
} RpcStruct;



typedef struct RpcArrayTAG
{
   RpcNode* head;
   RpcNode* tail;
   int size;

} RpcArray;


typedef struct RpcTreeTAG
{
   /*
    * We build the tree into an RpcArray internally.
    */
   RpcNode* root;


   /*
    * We maintain an array of pointers to RpcNodes to implement a simple
    * stack of containers. If calling code attempts to add a simple value
    * type, it is added to the 'current' container. If calling code adds a
    * struct or an array, we add a pointer to that node to the top of the
    * stack, and it becomes the current container. New nodes are then added to
    * the current container until either a new sub-container is added, or
    * calling code calls the CloseContainer() function. When a container is
    * closed, its pointer is popped from the stack, and the next-higher
    * container once again becomes the current container.
    */
   RpcNode** stack;

   SInt32 stackIndex;
   SInt32 stackSize;

} RpcTree;


/*
 * Any function wishing to handle an XML-RPC request must expose the following
 * interface, and be added to the system using the dmfXmlRpcAddFunction() call
 * (see below).
 */
typedef int (*RpcFunc)(webs_t wp, RpcTree* params, RpcTree* output);

/*
 * looks up the specified methodname in the 'funcs' symbol table, and if found,
 * invokes it, returning its output in the 'response' tree, and returning its
 * success/error code.
 */

extern int dmfRpcCallFunction(webs_t wp, sym_fd_t funcs, char_t* methodName, 
 RpcTree* params, RpcTree* response);



typedef struct RpcParseContextTAG
{
   StringBuf*  currentData;
#ifdef qSoap
   /* in SOAP, the data type is given as an attribute in the opening tag, not
    * as a separate tag as in xml-rpc, so we need to remember what type we're
    * dealing with.
    */
   enum RpcDataType currentType;
   enum SoapState currentState;
   char_t* currentTag;
   /* we keep a flag that is set to 1 when we handle a start tag, and zero 
      when we handle an end tag -- the character handler only appends new
      text when this flag is set. Prevents eating useless whitespace that
      expat sends us.
   */
   int started;
#endif
#ifdef qXmlRpc
   char_t* memberName;
#endif
   
   char_t*     methodName;

   /*
    * we assemble the parameters passed in with the xml-rpc call into an
    * RpcTree.
    */
   RpcTree* paramList;    
} RpcParseContext;



/*
 * Allocate memory for the context structure and initialize it.
 */
extern RpcParseContext* dmfCreateRpcContext(void);
/*
 * free any memory that may still be allocated for the context structure, then
 * free the memory for the structure itself.
 */
extern void dmfDestroyRpcContext(RpcParseContext* context);

/*
 * free any memory that this structure may still be holding on to, then NULL
 * out any pointers (make sure that everything is cleaned up.
 */
extern void dmfResetRpcContext(RpcParseContext* context);


/*****************************************************************************
 *
 *                          F U N C T I O N S
 *                          
 ****************************************************************************/

/*
 * Useful functions to retrieve correctly typed values from a value node.
 * Returns an error code if there's a type mismatch.
 */

extern int GetValueRaw(RpcNode* node, char_t** val);
extern int GetValueInteger(RpcNode* node, SInt32* val);
extern int GetValueBoolean(RpcNode* node, SInt32* val);
extern int GetValueDouble(RpcNode* node, double* val);
extern int GetValueString(RpcNode* node, char_t** val);

/*
 * converts the data from base64 and allocates memory for it and returns it
 * through the val pointer.
 */
extern int GetValueBase64(RpcNode* node, void** val);

/*
 * free all the memory associated with this Node. Note that since the nodeData
 * member may be the root of a sub-tree, we may need to free memory
 * recursively underneath this Node.
 */
extern void DestroyNode(RpcNode* node);

/*
 * If this node points to a value, returns the RpcValue struct. Otherwise
 * returns NULL.
 */
extern RpcValue* GetValueNodePtr(RpcNode* node);

extern RpcStruct* GetStructNodePtr(RpcNode* node);


extern RpcArray* GetArrayNodePtr(RpcNode* node);


/*
 * create a node that contains the given value. Allows you to skip a step and
 * just create the node directly.
 */
extern RpcNode* CreateValueNode(char_t* name, char_t* val, enum RpcDataType type);

extern RpcNode* CreateIntegerValueNode(char_t* name, SInt32 val);
extern RpcNode* CreateDoubleValueNode(char_t* name, double val);
extern RpcNode* CreateStringValueNode(char_t* name, char_t* val);
extern RpcNode* CreateBooleanValueNode(char_t* name, SInt32 val);


/*
 * free the existing name (if any), then copy the provided name into the 
 * node.
 */
extern void RenameNode(RpcNode* node, char_t* name);


/*
 * Create a new node that holds a structure member. You can build it up one
 * piece at a time by passing in a NULL pointer for the `val' parameter, then
 * add the actual member data by calling AddNodeToMember() (see below).
 */
extern RpcNode* CreateStructMemberNode(char_t* name, RpcNode* val);


/*
 * Create an empty structure node, ready to be filled with members (by calling
 * AddNodeToStruct() below
 */
extern RpcNode* CreateStructNode(char_t* name);


extern void AddNodeToStruct(RpcNode* structNode, RpcNode* memberNode);

/*extern void AddNodeToMember(RpcNode* memberNode, RpcNode* node);*/

/*
 * Use this to iterate through the structure -- call the first time with
 * 'member' set to NULL, then use the return value to retrieve the next
 * member. Returns NULL when it's run out of members.
 * Like this:
 * void WalkStruct(RpcNode* s)
 * {
 *    RpcNode* member = NULL;
 *    member = GetNextMember(s, member);
 *    while (NULL != member)
 *    {
 *       / * do something useful with the struct member... * /
 *       member = GetNextMember(s, member);
 *    }
 * }
 */
extern RpcNode* GetNextMember(RpcNode* structure, 
 RpcNode* member);


/*
 * Create an empty Array node, ready to be filled by calling AddNodeToArray()
 * (see below)
 */
extern RpcNode* CreateArrayNode(char_t* name);



extern void AddNodeToArray(RpcNode* arrayNode, RpcNode* node);


/*
 * Iterator interface to retrieve the next item in the array. 
 * Call with node = NULL to get the first item, then use the current node to
 * retrieve the next node. Returns NULL when the array is exhausted.
 */
extern RpcNode* GetNextArrayNode(RpcNode* arrayNode, RpcNode* node);


/*
 * create the tree builder struct and allocate memory for the stack.
 */
extern RpcTree* CreateTree(SInt32 size);

/*
 * destroy the tree & free all associated memory.
 */
extern void DestroyTree(RpcTree* tree);

/*
 * frees any nodes owned by the tree and empties the stack.
 */
extern void ResetTree(RpcTree* tree);


/*
 * Add a new node to the 'current' container. May cause the stack to be
 * reallocated if we are out of stack space.
 */
extern void AddTreeNode(RpcTree* tree, RpcNode* node);


/*
 * 'close' the current node -- the parent of the current node then becomes the
 * new current node, and any new items we find are added to that instead.
 */
extern void CloseTreeNode(RpcTree* tree);


extern int dmfRpcGetParam(webs_t wp, int parmIndex, char_t* index, 
 RpcTree* result);

/*
 * A simple debugging function to copy the input 'params' tree into the output
 * 'result' tree as is -- the xml output should be structurally identical to
 * the xml input. Useful in verifying our parsing code.
 */

extern int dmfRpcEchoFunc(webs_t wp, RpcTree* params, RpcTree* result);

#endif /* #if defined(qXmlRpc) || defined(qSoap) */
#endif /*#ifndef h_RpcUtil */
