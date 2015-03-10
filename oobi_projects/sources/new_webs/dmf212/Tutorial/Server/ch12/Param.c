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
    * CH04: make 'system.info.name' and 'system.info.description' read/write-able
    */
   {
      T("system"),         /*  key */
      kBeginSection,       /*  dataType */
      0,                   /*  read access level */
      0,                   /*  write access level */
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
      T("info"),           /*  key */
      kBeginSection,       /*  dataType */
      -1,                  /* read access level */
      -1,                  /* write access level */
      NULL,                /* getFunc */
      NULL,                /* setFunc */
      SystemInfoQuery,     /*  queryFunc */
      0,                   /* lastQuery */
      SystemInfoCommit,    /* commitFunc */
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
      T("name"),           /*  key */
      kString,             /*  dataType */
      -1,                  /* read access level */
      -1,                  /* write access level */
      dmfGetString,        /*  getFunc */
      dmfSetStringFixed,   /* setFunc */
      NULL,                /* queryFunc */
      0,                   /* lastQuery */
      NULL,                /* commitFunc */
      0,                   /* last commit */
      &(gSystemInfo.name), /*  dataStart */
      0,                   /* offset */
      NULL,                /* startIndex */
      NULL,                /* endIndex */
      0,                   /* lowerLimit */
      kInfoStringLength,   /* upperLimit */
      -1,                  /* parent -- keep this -1 */
   },
   {
      T("description"),    /*  key */
      kString,             /*  dataType */
      -1,                  /* read access level */
      -1,                  /* write access level */
      dmfGetString,        /*  getFunc */
      dmfSetStringFixed,   /* setFunc */
      NULL,                /* queryFunc */
      0,                   /* lastQuery */
      NULL,                /* commitFunc */
      0,                   /* last commit */
      &(gSystemInfo.description), /*  dataStart */
      0,                   /* offset */
      NULL,                /* startIndex */
      NULL,                /* endIndex */
      0,                   /* lowerLimit */
      kInfoStringLength,   /* upperLimit */
      -1,                  /* parent -- keep this -1 */
   },
   {
      T("uptime"),         /*  key */
      kString,            /*  dataType */
      -1,                  /* read access level */
      -1,                  /* write access level */
      GetUptime,           /* !!! getFunc */
      NULL,                /* setFunc */
      NULL,                /* queryFunc */
      0,                   /* lastQuery */
      NULL,                /* commitFunc */
      0,                   /* last commit */
      &(gSystemInfo.uptime),  /*  dataStart */
      0,                   /* offset */
      NULL,                /* startIndex */
      NULL,                /* endIndex */
      0,                   /* lowerLimit */
      0,                   /* upperLimit */
      -1,                  /* parent -- keep this -1 */
   },
   {
      T("info"),           /*  key */
      kEndSection,         /*  dataType */
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
   /* CH05: Adding the 'values' subtree... */
   {
      T("values"),         /* key */
      kBeginSection,       /* dataType */
      -1,                  /* read access level */
      -1,                  /* write access level */
      NULL,                /* getFunc */
      NULL,                /* setFunc */
      ValuesQuery,         /* queryFunc */
      0,                   /* lastQuery */
      ValuesCommit,        /* commitFunc */
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
      T("value"),          /* key */
      kInteger,            /* dataType */
      -1,                  /* read access level */
      -1,                  /* write access level */
      dmfGetInt32,         /* getFunc */
      NULL,                /* setFunc */
      NULL,                /* queryFunc */
      0,                   /* lastQuery */
      NULL,                /* commitFunc */
      0,                   /* last commit */
      &(gValues.value),    /* dataStart */
      sizeof(int),         /* offset */
      &kZeroIndex,         /* startIndex */
      &(gValues.endIndex), /* endIndex */
      0,                   /* lowerLimit */
      0,                   /* upperLimit */
      -1,                  /* parent -- keep this -1 */
   },
   {
      T("lowerAlarm"),     /* key */
      kInteger,            /* dataType */
      -1,                  /* read access level */
      5,                  /* write access level */
      dmfGetInt32,         /* getFunc */
      dmfSetInt32,         /* setFunc */
      NULL,                /* queryFunc */
      0,                   /* lastQuery */
      NULL,                /* commitFunc */
      0,                   /* last commit */
      &(gValues.lowerAlarm), /* dataStart */
      sizeof(int),         /* offset */
      &kZeroIndex,         /* startIndex */
      &(gValues.endIndex), /* endIndex */
      -1000,               /* lowerLimit */
      1000,                /* upperLimit */
      -1,                  /* parent -- keep this -1 */
   },
   {
      T("upperAlarm"),     /* key */
      kInteger,            /* dataType */
      -1,                  /* read access level */
      5,                   /* write access level */
      dmfGetInt32,         /* getFunc */
      dmfSetInt32,         /* setFunc */
      NULL,                /* queryFunc */
      0,                   /* lastQuery */
      NULL,                /* commitFunc */
      0,                   /* last commit */
      &(gValues.upperAlarm), /* dataStart */
      sizeof(int),         /* offset */
      &kZeroIndex,         /* startIndex */
      &(gValues.endIndex), /* endIndex */
      -1000,               /* lowerLimit */
      1000,                /* upperLimit */
      -1,                  /* parent -- keep this -1 */
   },
   {
      T("label"),          /* key */
      kString,             /* dataType */
      -1,                  /* read access level */
      5,                   /* write access level */
      dmfGetString,        /* getFunc */
      dmfSetStringFixed,   /* setFunc */
      NULL,                /* queryFunc */
      0,                   /* lastQuery */
      NULL,                /* commitFunc */
      0,                   /* last commit */
      &(gValues.label),    /* dataStart */
      kMaxValueLabelLength+1, /* offset */
      &kZeroIndex,         /* startIndex */
      &(gValues.endIndex), /* endIndex */
      0,                   /* lowerLimit */
      kMaxValueLabelLength,/* upperLimit */
      -1,                  /* parent -- keep this -1 */
   },

   {
      T("values"),         /* key */
      kEndSection,         /* dataType */
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
      T("system"),         /*  key */
      kEndSection,         /*  dataType */
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


