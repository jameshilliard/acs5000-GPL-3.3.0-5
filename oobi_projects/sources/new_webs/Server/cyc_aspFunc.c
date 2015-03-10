/*****************************************************************
* File: cyc_aspFunc.c
*
* Copyright (C) 2003 Cyclades Corporation
*
* www.cyclades.com
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version
* 2 of the License, or (at your option) any later version.
*
* Description: Implements all Cyclades WMI aspFunc's
*
****************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "uemf.h"
#include "EjFuncs.h"
#include "wsIntrn.h"
#include "Access.h"
#include "Error.h"
#include "GetSet.h"
#include "Locals.h"
#include "Parse.h"


#ifdef USER_MANAGEMENT_SUPPORT
#include "um.h"
#endif

#ifdef qDmf
#include "../AlFrame/Dmf.h"
#endif

#include "webApi.h"

extern char_t kSidKey[];

static int dmfEjGetUserBio(int eid, webs_t wp, int argc, char_t **argv)
{
	int sid;
	int val = 0;
	char_t num[32];

	if (argc > 0 && ((sid = atoi(argv[0])) > 0)) {
       val = CycUserBio(sid);
	}
    
	fmtStatic (num, 32, T("%d"), val);
	ejSetResult(eid, num);

	return (0);
}

static int dmfEjGetNewSid(int eid, webs_t wp, int argc, char_t **argv)
{
	int sid;
	int val = 0;
	char_t num[32];

	if (argc > 0 && ((sid = atoi(argv[0])) > 0)) {
       val = CycGetNewSid(sid);
	}
    
	fmtStatic (num, 32, T("%d"), val);
	ejSetResult(eid, num);
	dmfSetParam(kSidKey, NULL, num, 0, -1);

	return (0);
}

void cycSetAspFunc(void)
{
      /*warning: passing arg 2 of `websAspDefine' from incompatible pointer type
	warning: passing arg 2 of `websAspDefine' from incompatible pointer type 
      websAspDefine(T("getUserBio"), (AspFunc) dmfEjGetUserBio);
      websAspDefine(T("getNewSid"), (AspFunc) dmfEjGetNewSid);*/
      websAspDefine(T("getUserBio"), dmfEjGetUserBio);
      websAspDefine(T("getNewSid"), dmfEjGetNewSid);
}

