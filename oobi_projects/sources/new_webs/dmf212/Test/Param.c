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
      -1,
      -1,
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
      T("test"),               /* key */
      kBeginSection,             /* dataType */
      0,
      0,
      NULL,                /* getFunc */
      NULL,                /* setFunc */
      DataTestQuery,                /* queryFunc */
      0,
      DataTestCommit,                /* commitFunc */
      0,
      NULL,                /* dataStart */
      0,                   /* offset */
      NULL,                /* startIndex */
      NULL,                /* endIndex */
      0,                   /* lowerLimit */
      0,                   /* upperLimit */
      -1,                /* parent -- keep this -1 */
   },
   {
      T("struct"),               /* key */
      kBeginSection,             /* dataType */
      -1,
      -1,
      NULL,                /* getFunc */
      NULL,                /* setFunc */
      NULL,                /* queryFunc */
      0,
      NULL,                /* commitFunc */
      0,
      NULL,                /* dataStart */
      0,                   /* offset */
      NULL,                /* startIndex */
      NULL,                /* endIndex */
      0,                   /* lowerLimit */
      0,                   /* upperLimit */
      -1,                /* parent -- keep this -1 */
   },
   {
      T("char"),               /* key */
      kInteger,             /* dataType */
      -1,
      -1,
      dmfGetInt8,                /* getFunc */
      dmfSetInt8,         /* setFunc */
      NULL,                /* queryFunc */
      0,
      NULL,                /* commitFunc */
      0,
      &(gTestStruct.charVal),                /* dataStart */
      0,                   /* offset */
      NULL,                /* startIndex */
      NULL,                /* endIndex */
      0,                   /* lowerLimit */
      0,                   /* upperLimit */
      -1,                /* parent -- keep this -1 */
   },
   {
      T("short"),               /* key */
      kInteger,             /* dataType */
      -1,
      -1,
      dmfGetInt16,                /* getFunc */
      dmfSetInt16,         /* setFunc */
      NULL,                /* queryFunc */
      0,
      NULL,                /* commitFunc */
      0,
      &(gTestStruct.shortVal),                /* dataStart */
      0,                   /* offset */
      NULL,                /* startIndex */
      NULL,                /* endIndex */
      0,                   /* lowerLimit */
      0,                   /* upperLimit */
      -1,                /* parent -- keep this -1 */
   },
   {
      T("long"),               /* key */
      kInteger,             /* dataType */
      -1,
      -1,
      dmfGetInt32,                /* getFunc */
      dmfSetInt32,         /* setFunc */
      NULL,                /* queryFunc */
      0,
      NULL,                /* commitFunc */
      0,
      &(gTestStruct.longVal),                /* dataStart */
      0,                   /* offset */
      NULL,                /* startIndex */
      NULL,                /* endIndex */
      0,                   /* lowerLimit */
      100,                   /* upperLimit */
      -1,                /* parent -- keep this -1 */
   },
   {
      T("double"),               /* key */
      kDouble,             /* dataType */
      -1,
      -1,
      dmfGetDouble,                /* getFunc */
      dmfSetDouble,                /* setFunc */
      NULL,                /* queryFunc */
      0,
      NULL,                /* commitFunc */
      0,
      &(gTestStruct.doubleVal),                /* dataStart */
      0,                   /* offset */
      NULL,                /* startIndex */
      NULL,                /* endIndex */
      0,                   /* lowerLimit */
      0,                   /* upperLimit */
      -1,                  /* parent -- keep this -1 */
   },
   {
      T("string1"),               /* key */
      kString,             /* dataType */
      -1,
      -1,
      dmfGetString,                /* getFunc */
      dmfSetStringFixed,                /* setFunc */
      NULL,                /* queryFunc */
      0,
      NULL,                /* commitFunc */
      0,
      &(gTestStruct.string1),                /* dataStart */
      0,                   /* offset */
      NULL,                /* startIndex */
      NULL,                /* endIndex */
      0,                   /* lowerLimit */
      kString1Len,                   /* upperLimit */
      -1,                  /* parent -- keep this -1 */
   },
   {
      T("string2"),               /* key */
      kString,             /* dataType */
      -1,
      -1,
      dmfGetString,                /* getFunc */
      dmfSetStringDynamic,                /* setFunc */
      NULL,                /* queryFunc */
      0,
      NULL,                /* commitFunc */
      0,
      &(gTestStruct.string2),                /* dataStart */
      0,                   /* offset */
      NULL,                /* startIndex */
      NULL,                /* endIndex */
      0,                   /* lowerLimit */
      kString1Len,                   /* upperLimit */
      -1,                  /* parent -- keep this -1 */
   },

   {
      T("bytes"),               /* key */
      kInteger,             /* dataType */
      -1,
      -1,
      dmfGetUInt8,                /* getFunc */
      dmfSetUInt8,                /* setFunc */
      NULL,                /* queryFunc */
      0,
      NULL,                /* commitFunc */
      0,
      &(gTestStruct.byteArray),                /* dataStart */
      sizeof(UInt8),                   /* offset */
      &kZeroIndex,                /* startIndex */
      &(gTestStruct.byteArrayLength),                /* endIndex */
      0,                   /* lowerLimit */
      0,                   /* upperLimit */
      -1,                  /* parent -- keep this -1 */
   },
   {
      T(""),               /* key */
      kEndSection,             /* dataType */
      -1,
      -1,
      NULL,                /* getFunc */
      NULL,                /* setFunc */
      NULL,                /* queryFunc */
      0,
      NULL,                /* commitFunc */
      0,
      NULL,                /* dataStart */
      0,                   /* offset */
      NULL,                /* startIndex */
      NULL,                /* endIndex */
      0,                   /* lowerLimit */
      0,                   /* upperLimit */
      -1,                /* parent -- keep this -1 */
   },
   {
      T("random"),               /* key */
      kBeginSection,             /* dataType */
      -1,
      -1,
      NULL,                /* getFunc */
      NULL,                /* setFunc */
      RandomTestQuery,     /* queryFunc */
      0,
      NULL,                /* commitFunc */
      0,
      NULL,                /* dataStart */
      0,                   /* offset */
      NULL,                /* startIndex */
      NULL,                /* endIndex */
      0,                   /* lowerLimit */
      0,                   /* upperLimit */
      -1,                /* parent -- keep this -1 */
   },
   {
      T("val"),               /* key */
      kInteger,             /* dataType */
      -1,
      -1,
      dmfGetInt32,                /* getFunc */
      NULL,         /* setFunc */
      NULL,                /* queryFunc */
      0,
      NULL,                /* commitFunc */
      0,
      &(gRandomTest.randVal),                /* dataStart */
      0,                   /* offset */
      NULL,                /* startIndex */
      NULL,                /* endIndex */
      0,                   /* lowerLimit */
      0,                   /* upperLimit */
      -1,                /* parent -- keep this -1 */
   },
   {
      T("count"),               /* key */
      kInteger,             /* dataType */
      -1,
      -1,
      dmfGetInt32,                /* getFunc */
      NULL,         /* setFunc */
      NULL,                /* queryFunc */
      0,
      NULL,                /* commitFunc */
      0,
      &(gRandomTest.requestCount),                /* dataStart */
      0,                   /* offset */
      NULL,                /* startIndex */
      NULL,                /* endIndex */
      0,                   /* lowerLimit */
      0,                   /* upperLimit */
      -1,                /* parent -- keep this -1 */
   },
   {
      T(""),               /* key */
      kEndSection,             /* dataType */
      -1,
      -1,
      NULL,                /* getFunc */
      NULL,                /* setFunc */
      NULL,                /* queryFunc */
      0,
      NULL,                /* commitFunc */
      0,
      NULL,                /* dataStart */
      0,                   /* offset */
      NULL,                /* startIndex */
      NULL,                /* endIndex */
      0,                   /* lowerLimit */
      0,                   /* upperLimit */
      -1,                /* parent -- keep this -1 */
   }, 
   {
      T(""),               /* key */
      kEndSection,             /* dataType */
      -1,
      -1,
      NULL,                /* getFunc */
      NULL,                /* setFunc */
      NULL,                /* queryFunc */
      0,
      NULL,                /* commitFunc */
      0,
      NULL,                /* dataStart */
      0,                   /* offset */
      NULL,                /* startIndex */
      NULL,                /* endIndex */
      0,                   /* lowerLimit */
      0,                   /* upperLimit */
      -1,                /* parent -- keep this -1 */
   }
};

extern int parameterCount = sizeof(parameterList) / sizeof(parameterList[0]);


