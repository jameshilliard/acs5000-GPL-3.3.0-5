/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: RpcUtil.c,v 1.2 2006/01/10 23:51:50 erik Exp $ */

#include "RpcUtil.h"
#if defined(qXmlRpc) || defined(qSoap)
#include "Error.h"
#include "GetSet.h"
#include "Param.h"

/*#define qDebugNodes*/
#ifdef qDebugNodes
#define mTrace(txt) if (txt) OutputDebugString(txt);
#else
#define mTrace(txt) 
#endif
/*
 * create a new node structure to hold the specified data.
 */
static RpcNode* CreateNode(char_t* name, void* data, enum RpcNodeType type);


/*
 * Create and initialize the value structure.
 */
static RpcValue* CreateValue(char_t* val, enum RpcDataType type);

/*
 * destroy the value structure, freeing all memory.
 */
static void DestroyValue(RpcValue* val);



/* ../dmf212/AlFrame/RpcUtil.c:37: warning: `ValueString' declared `static' but never defined
 * return a string representation of this value (including the xml tags)
static void ValueString(StringBuf* str, RpcNode* val);
 */




/* ../dmf212/AlFrame/RpcUtil.c:45: warning: `MemberString' declared `static' but never defined
 * return a string representation of this member.
static void MemberString(StringBuf* str, RpcNode* member);
 */

static RpcStruct* CreateStruct(void);
static void DestroyStruct(RpcStruct* structure);

static void AddMember(RpcStruct* structure, RpcNode* member);
/* ../dmf212/AlFrame/RpcUtil.c:54: warning: `StructString' declared `static' but never defined
 * return a string representation of this struct.
static void StructString(StringBuf* str, RpcNode* structure);
 */


static RpcArray* CreateArray(void);
static void DestroyArray(RpcArray* array);

/* ../dmf212/AlFrame/RpcUtil.c:63: warning: `ArrayString' declared `static' but never defined
 * return a string representation of this array.
static void ArrayString(StringBuf* str, RpcNode* array);
 */

static int dmfRpcGetStruct(webs_t wp, char_t* name, int parmIndex, 
 RpcTree* result);

static int dmfRpcGetArray(webs_t wp, char_t* name, int parmIndex, int start, 
 int end, RpcTree* result);

extern RpcParseContext* dmfCreateRpcContext(void)
{
   RpcParseContext* context = (RpcParseContext*) balloc(B_L, 
    sizeof(RpcParseContext));
   if (NULL != context)
   {
      context->methodName = NULL;
      context->currentData = CreateStringBuf(128);
      context->paramList = CreateTree(16);
#ifdef qSoap
      context->currentType = kNoType;
      context->currentState = kNoState;
      context->currentTag = NULL;
      context->started = 0;
#endif
#ifdef qXmlRpc
      context->memberName = NULL;
#endif
   }

   return context;

}


extern void dmfDestroyRpcContext(RpcParseContext* context)
{
   if (NULL != context)
   {
      dmfResetRpcContext(context);
      DestroyStringBuf(context->currentData);
      DestroyTree(context->paramList);
      bfree(B_L, context);
   }
}

extern void dmfResetRpcContext(RpcParseContext* context)
{
   if (NULL != context)
   {
      bfreeSafe(B_L, context->methodName);
      context->methodName = NULL;

      ResetStringBuf(context->currentData);

      ResetTree(context->paramList);

#ifdef qSoap
      context->currentType = kNoType;
      context->currentState = kNoState;
      bfreeSafe(B_L, context->currentTag);
      context->currentTag = NULL;
      context->started = 0;
#endif
#ifdef qXmlRpc
      bfreeSafe(B_L, context->memberName);
      context->memberName = NULL;
#endif
   }

}



extern int dmfRpcCallFunction(webs_t wp, sym_fd_t funcs, char_t* methodName, 
 RpcTree* params, RpcTree* response)
{
   sym_t* symbol = NULL;
   RpcFunc func = NULL;
   int retval = kDmfError;
   if (-1 != funcs)
   {
      symbol = symLookup(funcs, methodName);
      if (symbol != NULL)
      {
         func = (RpcFunc) symbol->content.value.integer;
         if (func != NULL)
         {
            retval = (*func)(wp, params, response);
         }
      }
      else
      {
         dmfAddError(wp, kDmfNoSuchFunction, methodName, NULL, NULL);
      }
   }

   return retval;
}



extern int GetValueRaw(RpcNode* node, char_t** val)
{
   int retval = kDmfError;
   RpcValue* rpcVal = NULL;
   a_assert(node);
   if (node)
   {
      rpcVal = GetValueNodePtr(node);
      if (rpcVal)
      {
         retval = kDmfSuccess;
         *val = bstrdup(B_L, rpcVal->value);
      }
   }

   return retval;
}


extern int GetValueInteger(RpcNode* node, SInt32* val)
{
   int retval = kDmfError;
   RpcValue* rpcVal = NULL;
   a_assert(node);
   if (node)
   {
      rpcVal = GetValueNodePtr(node);
      if (rpcVal)
      {
         if (kInteger == rpcVal->type)
         {
            retval = kDmfSuccess;
            *val = gatoi(rpcVal->value);
         }
      }
   }
   return retval;
}

extern int GetValueBoolean(RpcNode* node, SInt32* val)
{
   int retval = kDmfError;
   RpcValue* rpcVal = NULL;
   a_assert(node);
   if (node)
   {
      rpcVal = GetValueNodePtr(node);
      if (rpcVal)
      {
         if (kBoolean == rpcVal->type)
         {
            retval = kDmfSuccess;
            *val = gatoi(rpcVal->value);
         }
      }
   }
   return retval;
}

extern int GetValueDouble(RpcNode* node, double* val)
{
   int retval = kDmfError;
   RpcValue* rpcVal = NULL;
   a_assert(node);
   if (node)
   {
      rpcVal = GetValueNodePtr(node);
      if (rpcVal)
      {
         if (kDouble == rpcVal->type)
         {
            retval = kDmfSuccess;
            *val = atof(rpcVal->value);
         }
      }
   }
   return retval;
}

extern int GetValueString(RpcNode* node, char_t** val)
{
   int retval = kDmfError;
   RpcValue* rpcVal = NULL;
   a_assert(node);
   if (node)
   {
      rpcVal = GetValueNodePtr(node);
      if (rpcVal)
      {
         if (kString == rpcVal->type)
         {
            retval = kDmfSuccess;
            *val = bstrdup(B_L, rpcVal->value);
         }
      }
   }

   return retval;
}


extern int GetValueBase64(RpcNode* node, void** val)
{
   int retval = kDmfError;
   return retval;
}



extern void RenameNode(RpcNode* node, char_t* name)
{
   bfreeSafe(B_L, node->name);
   node->name = NULL;
   if (NULL != name)
   {
      node->name = bstrdup(B_L, name);
   }

}


static RpcNode* CreateNode(char_t* name, void* data, enum RpcNodeType type)
{


   RpcNode* retval = (RpcNode*) balloc(B_L, sizeof(RpcNode));

   a_assert(data);
   if (NULL != retval)
   {
      mTrace("node: name = ");
      mTrace(name);
      mTrace("\n");
      retval->name = bstrdup(B_L, name);
      retval->nodeData = data;
      retval->nodeType = type;
      retval->next = NULL;   
   }
   return retval;
}


extern void DestroyNode(RpcNode* node)
{
   a_assert(node);
   if (node)
   {
      switch (node->nodeType)
      {
         case kValueNode:
         DestroyValue(node->nodeData);
         break;

         case kStructNode:
         DestroyStruct(node->nodeData);
         break;

         case kArrayNode:
         DestroyArray(node->nodeData);
         break;

         default:
         a_assert(0);
         break;
      }

      bfreeSafe(B_L, node->name);
      bfreeSafe(B_L, node);
   }
}

extern RpcValue* GetValueNodePtr(RpcNode* node)
{
   a_assert(node);
   return (RpcValue*)(kValueNode == node->nodeType) ? node->nodeData : NULL;
}

extern RpcStruct* GetStructNodePtr(RpcNode* node)
{
   a_assert(node);
   return (RpcStruct*)(kStructNode == node->nodeType) ? node->nodeData : NULL;
}



extern RpcArray* GetArrayNodePtr(RpcNode* node)
{
   a_assert(node);
   return (RpcArray*)(kArrayNode == node->nodeType) ? node->nodeData : NULL;
}


static RpcValue* CreateValue(char_t* val, enum RpcDataType type)
{

   RpcValue* retval = NULL;
   retval = (RpcValue*) balloc(B_L, sizeof(RpcValue));
   a_assert(val);

   if (retval)
   {
      retval->value = bstrdup(B_L, val);
      retval->type = type;
   }
   return retval;
}


extern RpcNode* CreateValueNode(char_t* name, char_t* val, enum RpcDataType type)
{
   RpcValue* newVal = CreateValue(val, type);
   RpcNode* newNode = NULL;
   if (newVal)
   {
      mTrace("Creating Value ");
      newNode = CreateNode(name, newVal, kValueNode);
   }
   return newNode;
}




extern RpcNode* CreateIntegerValueNode(char_t* name, SInt32 val)
{
   char_t* strVal = NULL;
   RpcNode* nodeVal = NULL;
   char_t   strBuf[32];

   fmtStatic(strBuf, 32, T("%d"), val);
   nodeVal = CreateValueNode(name, strVal, kInteger);

   return nodeVal;
}

extern RpcNode* CreateDoubleValueNode(char_t* name, double val)
{
   char_t strVal[32];
   RpcNode* nodeVal = NULL;

   gsprintf(strVal, T("%f"), val);
   nodeVal = CreateValueNode(name, strVal, kDouble);
   bfreeSafe(B_L, strVal);

   return nodeVal;
}

extern RpcNode* CreateStringValueNode(char_t* name, char_t* val)
{
   return CreateValueNode(name, val, kString);
}

extern RpcNode* CreateBooleanValueNode(char_t* name, SInt32 val)
{
   return CreateValueNode(name, (0 == val) ? T("0") : T("1"), kInteger);
}



static void DestroyValue(RpcValue* val)
{
   a_assert(val);

   if (val)
   {
      bfreeSafe(B_L, val->value);
      bfree(B_L, val);
   }
}



extern RpcNode* CreateStructMemberNode(char_t* name, RpcNode* val)
{
   RpcNode* newNode = NULL;
   mTrace("Creating Struct member ");
   if (NULL != val)
   {
      /* !!! ? */
      newNode = val;
   }
   else
   {
      newNode = CreateNode(name, NULL, kNullNode);
   }
   return newNode;
}


static RpcStruct* CreateStruct(void)
{
   RpcStruct* retval = (RpcStruct*) balloc(B_L, sizeof(RpcStruct));
   if (retval)
   {
      retval->head = NULL;
      retval->tail = NULL;
   }
   return retval;
}


extern RpcNode* CreateStructNode(char_t* name)
{
   RpcNode* newNode = NULL;
   RpcStruct* newStruct = CreateStruct();
   if (newStruct)
   {
      mTrace("Creating Struct ");
      newNode = CreateNode(name, newStruct, kStructNode);
   }
   return newNode;
}

extern void AddNodeToStruct(RpcNode* structNode, RpcNode* memberNode)
{
   RpcStruct* structPtr = GetStructNodePtr(structNode);
   a_assert(structPtr);
   a_assert(memberNode);
   if (structPtr && memberNode)
   {
      AddMember(structPtr, memberNode);
   }
}


extern void AddNodeToNode(RpcNode* node, RpcNode* newNode)
{
   /*
    * when we create structures, we sometimes add a 'null' placeholder node
    * that lets us come back and add the real value node later.
    * We do this by trading our value pointers, then deleting the new node
    * that was passed in to us.
    */

   if (kNullNode == node->nodeType && kNullNode != newNode->nodeType)
   {
      node->nodeType = newNode->nodeType;
      node->nodeData = newNode->nodeData;
      /*newNode->nodeType = kNullNode;*/
      newNode->nodeData = NULL;
      DestroyNode(newNode);
   }
   else
   {
      a_assert(0);
   }




}

static void DestroyStruct(RpcStruct* structure)
{
   RpcNode* current = NULL;
   RpcNode* next = NULL;
   if (structure)
   {
      /*
       * walk the linked list and destroy all the nodes.
       */
      current = structure->head;
      while (current != NULL)
      {
         next = current->next;
         DestroyNode(current);
         current = next;
      }
      bfreeSafe(B_L, structure);
   }
}



static void AddMember(RpcStruct* structure, RpcNode* member)
{
   if (NULL != structure && NULL != member)
   {
      /*
       * case 1 -- first member being added, both head & tail are NULL
       */
      if (NULL == structure->head)
      {
         structure->head = structure->tail = member;
      }
      else
      {
         /* regular case -- appending a member.*/
         structure->tail->next = member;
         structure->tail = member;
      }
   }
}


extern RpcNode* GetNextMember(RpcNode* structureNode, 
 RpcNode* memberNode)
{
   RpcNode* retval = NULL;
   RpcStruct* structure = GetStructNodePtr(structureNode);
   if (structure)
   {
      if (NULL == memberNode)
      {
         /*
          * they're requesting the head.
          */
         retval = structure->head;
      }
      else
      {
         retval = memberNode->next;
      }
   }
   return retval;
}



static RpcArray* CreateArray(void)
{
   RpcArray* retval = (RpcArray*) balloc(B_L, sizeof(RpcArray));
   if (retval)
   {
      retval->head = NULL;
      retval->tail = NULL;
      retval->size = 0;
   }
   return retval;
}


extern RpcNode* CreateArrayNode(char_t* name)
{
   RpcNode* newNode = NULL;
   RpcArray* newArray = CreateArray();
   if (newArray)
   {
      mTrace("Creating array ");
      newNode = CreateNode(name, newArray, kArrayNode);
   }
   return newNode;
}


static void DestroyArray(RpcArray* array)
{
   RpcNode* current = NULL;
   RpcNode* next = NULL;
   if (array)
   {
      current = array->head;
      while (current != NULL)
      {
         next = current->next;
         DestroyNode(current);
         current = next;
      }
      bfree(B_L, array);
   }
}

extern void AddNodeToArray(RpcNode* arrayNode, RpcNode* node)
{
   RpcArray* array = GetArrayNodePtr(arrayNode);
   if (array && node)
   {
      array->size++;
      /*
       * case 1, adding a node to an empty array
       */
      if (NULL == array->head)
      {
         array->head = array->tail = node;
      }
      else
      {
         /* case 2 -- appending a node */
         array->tail->next = node;
         array->tail = node;
      }
   }
}

extern RpcNode* GetNextArrayNode(RpcNode* arrayNode, RpcNode* node)
{
   RpcNode* retval = NULL;
   RpcArray* array = GetArrayNodePtr(arrayNode);
   if (array)
   {
      if (NULL == node)
      {
         /* they're requesting the first item */
         retval = array->head;
      }
      else
      {
         retval = node->next;
      }
   }
   
   return retval;
}




extern RpcTree* CreateTree(SInt32 size)
{
   RpcTree* tree = (RpcTree*) balloc(B_L, 
    sizeof(RpcTree));
   if (tree)
   {
      tree->root = CreateArrayNode(NULL);
      tree->stack = balloc(B_L, size * sizeof(RpcNode*));
      if (tree->root && tree->stack)
      {
         tree->stackIndex = 0;
         tree->stackSize = size;

         tree->stack[tree->stackIndex] = tree->root;
      }
      else
      {
         DestroyTree(tree);
         tree = NULL;
      }
   }
   return tree;
}

extern void DestroyTree(RpcTree* tree)
{
   if (tree)
   {
      DestroyNode(tree->root);
      bfreeSafe(B_L, tree->stack);
      bfreeSafe(B_L, tree);
   }
}

extern void ResetTree(RpcTree* tree)
{
   if (tree)
   {
      /* delete the existing tree, and build a new empty one in its place.*/
      DestroyNode(tree->root);
      tree->root = CreateArrayNode(NULL);
      tree->stackIndex = 0;
      tree->stack[tree->stackIndex] = tree->root;
   }
}

extern void AddTreeNode(RpcTree* tree, RpcNode* node)
{
   enum RpcNodeType newNodeType;
   SInt32 newSize = 0;
   RpcNode** newStack = NULL;
   RpcNode* current = NULL;

   a_assert(tree);
   a_assert(node);

   if (tree && node)
   {
      /* add this node to the current container.. */
      current = tree->stack[tree->stackIndex];
      switch (current->nodeType)
      {
         case kStructNode:
         mTrace("Adding node to struct\n");
         AddNodeToStruct(current, node);
         break;

         case kArrayNode:
         mTrace("Adding node to array\n");
         AddNodeToArray(current, node);
         break;

         case kNullNode:
         mTrace("Adding node to node\n");
         AddNodeToNode(current, node);
         node = current;
         break;

         /* testing -- */
         case kValueNode:
         mTrace("Error? Adding a value node?\n");
         /* pop up a level... */
         
         /* ..and try again */
         /*AddTreeNode(tree, node);*/
         break;


         default:
         /* not a container -- serious logic error here... */
         mTrace("ERROR -- adding node to a non-container\n");
         a_assert(0);
         break;
      }
      

      
      newNodeType = node->nodeType;
      if (newNodeType != kValueNode)
      /*if (1)  testing...*/
      {
        /* this node needs to become the current container */
        mTrace("New node becoming new container.\n");
        tree->stackIndex++;
        if (tree->stackIndex == tree->stackSize)
        {
           /* we need to expand the stack..we'll grow by 150%*/
           newSize = (tree->stackSize * 3) / 2;
           newStack = brealloc(B_L, tree->stack, newSize);
           if (newStack != NULL)
           {
              tree->stack = newStack;
              tree->stackSize = newSize;
           }
        }
        tree->stack[tree->stackIndex] = node;

      }
   }
}



extern void CloseTreeNode(RpcTree* tree)
{
   /*
    * set the index of the 'current' container node to the current node's
    * parent. If the current index is 0, there's a serious logical error.
    */
   a_assert(tree->stackIndex > 0);
   tree->stackIndex--;
   mTrace("Closing tree node.\n");
}



extern int dmfRpcGetParam(webs_t wp, int parmIndex, char_t* index, 
 RpcTree* result)
{
   Param* parm = NULL;
   char_t* value = NULL;
   RpcNode* node = NULL;
   int retval = -1;
   int alFrameCode = kDmfSuccess;
   if (parmIndex < 0 || parmIndex >= parameterCount)
   {
      return -1;
   }

   parm = &(parameterList[parmIndex]);
   switch (parm->dataType)
   {
      case kBeginSection:
      /* this is the beginning of a section, which we package as a struct. */
      return dmfRpcGetStruct(wp, (char_t*) parm->key, parmIndex + 1, result);
      break;

      case kEndSection:
      /* this is the end of a section, which finishes the current struct.*/
      break;

      default:
      /* this is a real data value. */
      if (NULL == index && (NULL != parm->startIndex && NULL != parm->endIndex))
      {
         return dmfRpcGetArray(wp, (char_t*) parm->key, parmIndex, 
          *(parm->startIndex), *(parm->endIndex), result);
      }
      else
      {
         /* this is a single data value */
         alFrameCode = dmfGetParamByIndex(parmIndex, (char_t*) parm->key, 
          index, &value, 256); /* !!! bypassing access level checking */
         if (kDmfSuccess == alFrameCode)
         {
            node = CreateValueNode((char_t*) parm->key, value, parm->dataType);
            bfreeSafe(B_L, value);
            AddTreeNode(result, node);
            retval = parmIndex + 1;
         }
         else
         {
            dmfAddError(wp, alFrameCode, (char_t*) parm->key, index, NULL);
         }
      }

   }
   return retval;
}

static int dmfRpcGetStruct(webs_t wp, char_t* name, int parmIndex, RpcTree* result)
{
   /*Param* parm = NULL; ../dmf212/AlFrame/RpcUtil.c:862: warning: unused variable `parm'*/
   int retval = parmIndex;
   int index;

   AddTreeNode(result, CreateStructNode(name));
   while (1)
   {
      index = retval;
      if (kEndSection == parameterList[index].dataType)
      {
         break;
      }
      /* add a struct member node to the tree */
      AddTreeNode(result, 
       CreateStructMemberNode((char_t*) parameterList[index].key, NULL));

      retval = dmfRpcGetParam(wp, index, NULL, result);
      /* ...and after we add the member node, we close this member out */
      CloseTreeNode(result);
      if (-1 == retval)
      {
         a_assert(0);
         break;
      }

   } 

   CloseTreeNode(result);
   return index + 1;

}

static int dmfRpcGetArray(webs_t wp, char_t* name, int parmIndex, int start, int end, 
 RpcTree* result)
{
   int i;
   char_t indexBuf[32];


   if (end > start)
   {
      AddTreeNode(result, CreateArrayNode(name));
      for (i = start; i < end; ++i)
      {
         fmtStatic(indexBuf, 32, T("%d"), i);
         dmfRpcGetParam(wp, parmIndex, indexBuf, result);
      }
      CloseTreeNode(result);
   }
   return parmIndex + 1;
}


static RpcNode* CopyNode(RpcNode* node);

static RpcNode* CopyArray(RpcNode* node)
{
   RpcNode* theArray = CreateArrayNode(node->name);
   RpcNode* item = GetNextArrayNode(node, NULL);
   RpcNode* itemClone = NULL;

   while (item != NULL)
   {
      itemClone = CopyNode(item);
      AddNodeToArray(theArray, itemClone);
      item = GetNextArrayNode(node, item);
   }

   return theArray;
}

static RpcNode* CopyStruct(RpcNode* node)
{
   RpcNode* theStruct = CreateStructNode(node->name);
   RpcNode* member = GetNextMember(node, NULL);
   RpcNode* memberClone = NULL;

   while (member != NULL)
   {
      memberClone = CopyNode(member);
      AddNodeToStruct(theStruct, memberClone);
      member = GetNextMember(node, member);
   }
   return theStruct;
}

static RpcNode* CopyValue(RpcNode* node)
{
   RpcValue* val = (RpcValue*) node->nodeData;
   return CreateValueNode(node->name, val->value, val->type);
}

static RpcNode* CopyNode(RpcNode* node)
{
   RpcNode* retval = NULL;
   switch (node->nodeType)
   {
      case kValueNode:
      retval = CopyValue(node);
      break;

      case kStructNode:
      retval = CopyStruct(node);
      break;

      case kArrayNode:
      retval = CopyArray(node);
      break;

      default:
      a_assert(0);
      break;
   }

   return retval;
}

extern int dmfRpcEchoFunc(webs_t wp, RpcTree* params, RpcTree* result)
{
   RpcNode* head = GetNextArrayNode(params->root, NULL);
   RpcNode* copy = CopyNode(head);
   dmfClearErrors(wp);

   AddTreeNode(result, copy);

   return kDmfSuccess;
}




#endif /* this must be the last line in this file! */
