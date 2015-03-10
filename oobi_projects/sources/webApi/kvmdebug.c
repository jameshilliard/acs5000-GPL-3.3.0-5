#include "kvmdebug.h"
#include "kvmdefs.h"
#include "kvmconfig.h"
#include "kvmd.h"
#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>

extern struct _kvm_global kvmgbl;

unsigned long set_debug(unsigned long level)
{
	kvmgbl.debuglevel = level;
	if (level > 0) {
		fprintf(stderr, "Setting debug level to %02X", 
				(unsigned int)kvmgbl.debuglevel);
		LOGDEBUG(level, "Debug level Set OK!");
	}
	return (kvmgbl.debuglevel);
}


void pmesg(unsigned long level, int line, const char *func, char* format, ...) {
        va_list args;
	char string_to_print[4096];
        if (level & kvmgbl.debuglevel) {
	        va_start(args, format);
	        vsprintf(string_to_print, format, args);
	        va_end(args);
		syslog(LOG_DEBUG, "%s[%d] - %s", func, line, string_to_print);
	}
}

