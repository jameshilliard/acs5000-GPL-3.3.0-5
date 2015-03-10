/*****************************************************************
* File: debug.h
*
* Copyright (C) 2006 Avocent Corporation
*
* www.avocent.com
*
* Description:
*     This file describes the prototypes and definitions used 
*     for system debugging.
*
* Date: 05/31/2006
* Author: Helio Fujimoto <helio.fujimoto@avocent.com>
* Date: 06/30/2006
* Author: Daniel Belz <daniel.belz@avocent.com>
* 	Importing the kvmd debug style
*
****************************************************************/
#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>

enum {
	CMSG = 0,
	READ_ACL = 1,
	READ_CONFIG,  	// 2
	PARSE_OUTLET,  	// 3
	MANAGE_ACL,  	// 4
	SHM_OPERATIONS,	// 5
	CORE,  		// 6
	IPDU_THREAD,  	// 7
	CORE_THREADS,  	// 8
	IPDULIB,  	// 9
	READ_OG,  	// 10
	SHM_DEBUG,  	// 11
	PMCLIB,		// 12 generic functions on pmclib.c
	PMCLIB_SHOW,	// 13 show functions on pmc_show.c
	PMCLIB_SEND,	// 14 build functions on pmc_build.c
	PMCLIB_VALIDATE, // 15 validate functions on pmc_validate.c
	CMD_XCHANGE,     // 16 commands exchange between client, core and ipduthread
	TRACE		// 17
};


#ifdef DEBUG
#define debug(level, P...) pmesg(level, __LINE__, __FUNCTION__ , ## P)

#define dodebug(fmt, arg...)		\
	debugprintf(fmt, ##arg);
#define dotrace(hdr, name, buf, size)	\
	if (debugLevel == TRACE || debugLevel == 0xFF) { \
		traceprintf(hdr, name, buf, size); \
	}

extern unsigned long debugLevel;
extern void debugprintf(const char *fmt, ...);
extern void traceprintf(const char *header, char *name, char *buf, int size);
extern void pmesg(unsigned long level, int line, const char *func, char* format, ...);

#else

#define debug(level, P...) 
#define dodebug(fmt, arg...)
#define dotrace(hdr, name, buf, size)

#endif

#endif //DEBUG_H
