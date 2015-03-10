/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: Functions.c,v 1.2 2006/01/10 23:51:50 erik Exp $ */

#include "webs.h"

#include "Functions.h"
#include "UserFunc.h"


const char* kFunctionsVersion = "$Revision: 1.2 $";


CmdFuncEntry cmdFunctionTable[] = 
{
   {"", NULL}

};

int cmdTableLength = sizeof(cmdFunctionTable) / sizeof(cmdFunctionTable[0]);



