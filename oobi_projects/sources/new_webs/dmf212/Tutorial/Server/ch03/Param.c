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
   /* 
    * CH03: add system.info parameters.
    */
   {
      T("system"),         /* !!! key */
      kBeginSection,       /* !!! dataType */
      0,                   /* !!! read access level */
      0,                   /* !!! write access level */
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
   },
   {
      T("info"),           /* !!! key */
      kBeginSection,       /* !!! dataType */
      -1,                  /* read access level */
      -1,                  /* write access level */
      NULL,                /* getFunc */
      NULL,                /* setFunc */
      SystemInfoQuery,     /* !!! queryFunc */
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
   {
      T("name"),           /* !!! key */
      kString,             /* !!! dataType */
      -1,                  /* read access level */
      -1,                  /* write access level */
      dmfGetString,        /* !!! getFunc */
      NULL,                /* setFunc */
      NULL,                /* queryFunc */
      0,                   /* lastQuery */
      NULL,                /* commitFunc */
      0,                   /* last commit */
      &(gSystemInfo.name), /* !!! dataStart */
      0,                   /* offset */
      NULL,                /* startIndex */
      NULL,                /* endIndex */
      0,                   /* lowerLimit */
      kInfoStringLength,   /* !!! upperLimit */
      -1,                  /* parent -- keep this -1 */
   },
   {
      T("description"),    /* !!! key */
      kString,             /* !!! dataType */
      -1,                  /* read access level */
      -1,                  /* write access level */
      dmfGetString,        /* !!! getFunc */
      NULL,                /* setFunc */
      NULL,                /* queryFunc */
      0,                   /* lastQuery */
      NULL,                /* commitFunc */
      0,                   /* last commit */
      &(gSystemInfo.description), /* !!! dataStart */
      0,                   /* offset */
      NULL,                /* startIndex */
      NULL,                /* endIndex */
      0,                   /* lowerLimit */
      0,                   /* upperLimit */
      -1,                  /* parent -- keep this -1 */
   },
   {
      T("uptime"),         /* !!! key */
      kInteger,            /* !!! dataType */
      -1,                  /* read access level */
      -1,                  /* write access level */
      dmfGetInt32,         /* !!! getFunc */
      NULL,                /* setFunc */
      NULL,                /* queryFunc */
      0,                   /* lastQuery */
      NULL,                /* commitFunc */
      0,                   /* last commit */
      &(gSystemInfo.uptime),  /* !!! dataStart */
      0,                   /* offset */
      NULL,                /* startIndex */
      NULL,                /* endIndex */
      0,                   /* lowerLimit */
      0,                   /* upperLimit */
      -1,                  /* parent -- keep this -1 */
   },
   {
      T("info"),           /* !!! key */
      kEndSection,         /* !!! dataType */
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
   {
      T("system"),         /* !!! key */
      kEndSection,         /* !!! dataType */
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
   }

};

extern int parameterCount = sizeof(parameterList) / sizeof(parameterList[0]);


