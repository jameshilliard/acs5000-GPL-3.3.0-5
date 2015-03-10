// Copyright (c) 2000 Art & Logic.  All rights reserved.
//     $Id: AlTypes.h,v 1.1.1.1 2005/09/01 02:36:38 edson Exp $

#ifndef h_AlTypes
#define h_AlTypes

typedef short bool;
#define true 1
#define false 0

#if defined(WIN32)
// Microsoft Visual C++ Version
typedef char               SInt8;
typedef unsigned char      UInt8;
typedef short              SInt16;
typedef unsigned short     UInt16;
typedef int                SInt32;
typedef unsigned int       UInt32;
typedef __int64            SInt64;
typedef unsigned __int64   UInt64;

/*#elif ... other OS types typedeffed here... */


#else /*default -- take a guess that we're on a 32-bit OS */
typedef char               SInt8;
typedef unsigned char      UInt8;
typedef short              SInt16;
typedef unsigned short     UInt16;
typedef int                SInt32;
typedef unsigned int       UInt32;

#endif

#define mArrayLength(a) a ? (sizeof(a) / sizeof(a[0])) : 0

#endif /* this must be the last line in this file. */
