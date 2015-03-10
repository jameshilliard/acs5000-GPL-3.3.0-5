/* Copyright (c) 2002 Art & Logic, Inc. All Rights Reserved.         */
/* $Id: Param.c,v 1.1.1.1 2005/09/01 02:36:39 edson Exp $ */

#include "GetFuncs.h"
#include "SetFuncs.h"
#include "Param.h"
#include "UserFunc.h"


/*
 * each entry in the parameter table needs to have the following fields:
 */

#if 0
   {
      T(""),               /* key */
      kNoType,             /* dataType */
      -1,                  /* read access level */
      -1,                  /* write access level */
      NULL,                /* getFunc */
      NULL,                /* setFunc */
      NULL,                /* queryFunc */
      0,                   /* lastQuery */
      NULL,                /* commitFunc */
      0,                   /* last commit */
      NULL,                /* dataStart */
      0,                   /* offset */
      NULL,                /* startIndex */
      NULL,                /* endIndex */
      0,                   /* lowerLimit */
      0,                   /* upperLimit */
      -1,                  /* parent -- keep this -1 */
   },
#endif


extern Param parameterList[] = 
{
   {
      T(""),               /* key */
      kNoType,             /* dataType */
      -1,
      -1,
      NULL,                /* getFunc */
      NULL,                /* setFunc */
      NULL,                /* queryFunc */
      0,                   /* last query */
      NULL,                /* commitFunc */
      0,                   /* last commit */
      NULL,                /* dataStart */
      0,                   /* offset */
      NULL,                /* startIndex */
      NULL,                /* endIndex */
      0,                   /* lowerLimit */
      0,                   /* upperLimit */
      -1,                  /* parent -- keep this -1 */
   }
};

extern int parameterCount = sizeof(parameterList) / sizeof(parameterList[0]);


