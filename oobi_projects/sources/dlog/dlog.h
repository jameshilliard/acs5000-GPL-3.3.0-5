//-----------------------------------------------------------------------------
// Copyright 2007, Avocent Corporation
//-----------------------------------------------------------------------------
// FILE                 : dlog.h
// DESCRIPTION          : DLOG Header File
//
//-----------------------------------------------------------------------------
// AUTHOR               : Jonathan Ibarra & Eduardo Fernandez
// DATE                 : April 18, 2007
//
//-----------------------------------------------------------------------------
// HISTORY
//
// DATE         PROGRAMMER              MODIFICATIONS
//
//-----------------------------------------------------------------------------

//
// dlog.h may be use in kernel modules or in user-space programs. When used in
// kernel modules, there are two ways to use it.
//
// Single-file kernel modules: Modules consisting of a single .c file, need only
// #include <dlog.h> and everything should jsut work
//
// Multi-file kernel modules: Modules consisting of multiple .c files need to
// #define DLOG_EXTERN
// #include <dlog.h>
// in all files making use of dlog, or in a common header file
// One of the C files should then 
// #include <dlog.h> 
// without the DLOG_EXTERN define. This will create the kernel param in a single
// place.
//
// NOTE: It is not neccessary to #undef DLOG_EXTERN, as the fist #include will
// undefine it for you.
//
#ifdef __KERNEL__
#ifndef __DLOG_H_DLOG_LEVEL
#define __DLOG_H_DLOG_LEVEL
// Define the dlog_level module parameter
#ifndef DLOG_EXTERN
int dlog_level = 3; // DLOG_ERROR .. set this way so we don't need the constant declared yet
module_param( dlog_level, int, S_IRUGO | S_IWUSR );
#endif
#endif
#endif

#ifndef __DLOG_H
#define __DLOG_H

// Function Result Codes
#define DLOG_SUCCESS               0
#define DLOG_ERROR_INIT           -1
#define DLOG_ERROR_PATTERN        -2
#define DLOG_ERROR_TABLE_FULL     -3
#define DLOG_ERROR_NO_SUCH_RULE   -4
#define DLOG_ERROR_INVALID_OUTPUT -5

// The Output Flag Options (useful only in userspace, but declared amongst all
//   the other #define constants
#define DLOG_OUTPUT_PRINTF      (0x01 << 0)
#define DLOG_OUTPUT_SYSLOG      (0x01 << 1)
#define DLOG_OUTPUT_ALL         ( DLOG_OUTPUT_PRINTF | DLOG_OUTPUT_SYSLOG )

// These Log Levels should match Syslog. Since this may be used in kernel where
//  syslog.h shoudl not be loaded.. we'll redefine here
#define DLOG_EMERGENCY 0
#define DLOG_ALERT     ( DLOG_EMERGENCY + 1 )
#define DLOG_CRITICAL  ( DLOG_EMERGENCY + 2 )
#define DLOG_ERROR     ( DLOG_EMERGENCY + 3 )
#define DLOG_WARNING   ( DLOG_EMERGENCY + 4 )
#define DLOG_NOTICE    ( DLOG_EMERGENCY + 5 )
#define DLOG_INFO      ( DLOG_EMERGENCY + 6 )
#define DLOG_DEBUG     ( DLOG_EMERGENCY + 7 )
#define DLOG_NONE      ( DLOG_EMERGENCY - 1 )

// Again taken from syslog.h
#define SYSLOG_DLOG_FACILITY    (22<<3)

// Conditional DLOG Calls are defined here. If DLOG_DEBUG_ENABLED is defined,
//   then the macros are mapped to the real macros/functions.. if not defined
//   the macros that return void are mapped to empty strings, those that return
//   booleans are mapped to false/0.
#ifdef DLOG_DEBUG_ENABLED
  #define dlog_debug_printf( level, ctx, format, arg... ) \
          dlog_printf( level, ctx, format, ##arg )
  #define dlog_debug_printbuf( level, ctx, label, buff, len ) \
          dlog_printbuf( level, ctx, format, label, buff, len )
  #define dlog_debug_full_printf( level, ctx, format, arg... ) \
          dlog_full_printf( level, ctx, format, ##arg )
  #define dlog_debug_should_log( level, ctx ) \
          dlog_should_log( level, ctx )
  #define dlog_debug_full_should_log( level, app, ctx ) \
          dlog_full_should_log( level, app, ctx )
#else
  #define dlog_debug_printf( level, ctx, format, arg... )
  #define dlog_debug_full_printf( level, ctx, format, arg... )
  #define dlog_debug_should_log( level, ctx ) ( 0 )
  #define dlog_debug_full_should_log( level, app, ctx ) ( 0 )
#endif

// Define the kernel space API
#ifdef __KERNEL__

#ifdef DLOG_EXTERN
extern int dlog_level;
#undef DLOG_EXTERN
#endif

// Define should log to simply compare log level to the module param. Note
//   that ctx is passed for API level compatibility with the userspace API, 
//   although it is not used for filtering in kernel space
#define dlog_should_log( level, ctx ) ( level <= dlog_level )

// Define what dlog_printf should do in kernel space. Esentially, check if it
//   should log, then if it should, use printk to output the message to syslog
#define dlog_printf( level, ctx, format, arg... ) \
        if ( dlog_should_log( level, ctx ) ) \
        { \
          printk( "<%d>" KBUILD_BASENAME "[%s(%i)]: [%s] " format,\
                  level, \
                  current->comm, current->pid, \
                  ((ctx==NULL)?__func__:ctx), \
                  ##arg ); \
          if ( format[ strlen( format ) - 1 ] != '\n' ) printk( "\n" ); \
        }

// Define what dlog_printbuf should do in kernel space. Esentially, check if it
//   should log, then if it should, use printk to output the message to syslog
#define dlog_printbuf( level, ctx, label, buff, len ) \
    if ( dlog_should_log( level, context ) )\
    {\
		if ( len == 0 )\
		{\
			dlog_printf( level, context, "%s(01/01): Empty\n", label );\
		}\
		else\
		{\
			int linenum = 0;\
			int linetot = ((len+15)/16);\
			char hex1[25];\
			char hex2[25];\
			char ascii[17];\
			int lineoff = 0;\
			while ( lineoff < len )\
			{\
				int x;\
				memset( hex1, 0, sizeof( hex1 ) );\
				memset( hex2, 0, sizeof( hex2 ) );\
				memset( ascii, 0, sizeof( ascii ) );\
				\
				for ( x = 0; x < 8; x++ )\
				{\
					if ( lineoff + x < len )\
					{\
						sprintf( hex1 + (x * 3), "%02x ", *(buff + lineoff + x) );\
						unsigned char ch = *(buff + lineoff + x);\
						ascii[ x ] = (( ch <= ' ' )||( ch > 126 ))?'.':ch;\
					}\
					if ( lineoff + 8 + x < len )\
					{\
						sprintf( hex2 + 8 + (x * 3), "%02x ", *(buff + 8 + lineoff + x) );\
						unsigned char ch = *(buff + 8 + lineoff + x);\
						ascii[ x + 8 ] = (( ch <= ' ' )||( ch > 126 ))?'.':ch;\
					}\
				}\
				\
				dlog_printf( level, context, "%s(%02d/%02d): %04x %-24s %-24s %-16s\n", label, ++linenum, linetot, lineoff, hex1, hex2, ascii );\
				lineoff += 16;\
			}\
		}\
    }

#else

// Define the userspace API
#include <stdlib.h>

//-----------------------------------------------------------------------------
// FUNCTION     : dlog_reset
//
// DESCRIPTION  : Resets the shared DLOG state. This clears any user set rules
//                and output options back to defaults
//
// PARAMETERS   : None
//
// RETURNS      : Nothing
//
//-----------------------------------------------------------------------------
void dlog_reset();

//-----------------------------------------------------------------------------
// FUNCTION     : dlog_refresh
//
// DESCRIPTION  : Forces an update of the kernel space DLOG users. This should
//                be performed after an insmod of a DLOG enabled module.
//
// PARAMETERS   : None
//
// RETURNS      : Nothing
//
//-----------------------------------------------------------------------------
void dlog_refresh();


//-----------------------------------------------------------------------------
// FUNCTION     : dlog_set_output
//
// DESCRIPTION  : Configure where log's should be output
//
// PARAMETERS   : outputFlags - A bitmap of DLOG_OUTPUT* flags indicating
//                              where logs should be outputed.
//
// RETURNS      : DLOG_SUCCESS or a DLOG_ERROR_*
//
//-----------------------------------------------------------------------------
int dlog_set_output( int outputFlags );

//-----------------------------------------------------------------------------
// FUNCTION     : dlog_get_output
//
// DESCRIPTION  : Return the state of the output flag
//
// PARAMETERS   : None
//
// RETURNS      : The current state of the output flags
//
//-----------------------------------------------------------------------------
int dlog_get_output();


//-----------------------------------------------------------------------------
// FUNCTION     : dlog_set_rule
//
// DESCRIPTION  : Add/modify a logging rule
//
// PARAMETERS   : module_pattern  - Module pattern to add/modify
//                context_pattern - Context pattern to add/modify
//                log_level       - Log level for the rule, or DLOG_NONE to
//                                  remove.
//
// RETURNS      : DLOG_SUCCESS or a DLOG_ERROR_*
//
//-----------------------------------------------------------------------------
int dlog_set_rule( const char* module_pattern, const char* context_pattern, 
                   const int log_level);

//-----------------------------------------------------------------------------
// FUNCTION     : dlog_get_rule
//
// DESCRIPTION  : Obtain a rule from the rule table
//
// PARAMETERS   : index           - The index of the rule to get
//                module_pattern  - 32 byte buffer for module value
//                context_pattern - 32 byte buffer for context value
//                log_level       - Location to output rule log level
//
// RETURNS      : DLOG_SUCCESS or a DLOG_ERROR_*
//
//-----------------------------------------------------------------------------
int dlog_get_rule( const int index, char* module_pattern, 
                   char* context_pattern, int* log_level);

//-----------------------------------------------------------------------------
// FUNCTION     : dlog_get_rule_count
//
// DESCRIPTION  : Obtains the number of rules in the rule table.
//
// PARAMETERS   : None
//
// RETURNS      : The number of rules in the rule table
//
//-----------------------------------------------------------------------------
int dlog_get_rule_count();


//-----------------------------------------------------------------------------
// FUNCTION     : dlog_should_log
//
// DESCRIPTION  : Determines if a log message with the given level, 
//                and context should be logged. 
//
// PARAMETERS   : level   - The log level to test
//                context - The context string to test, NULL to substitute
//                          function name
//
// RETURNS      : TRUE(non-zero) if the message would be logged.
//
//-----------------------------------------------------------------------------
#define dlog_should_log( level, context ) \
        __dlog_should_log( level, ((context==NULL)?__func__:context) )
int __dlog_should_log( const int level, const char* context);

//-----------------------------------------------------------------------------
// FUNCTION     : dlog_printf
//
// DESCRIPTION  : Logs a message to DLOG
//
// PARAMETERS   : level   - The log level of the message
//                context - The context for the message being logged, a value of
//                          NULL will cause the function name to be used as the 
//                          context.
//                format  - The format of the message to log
//                arg...  - The parameters for the formatted message
//
// RETURNS      : Nothing
//
//-----------------------------------------------------------------------------
#define dlog_printf( level, context, format, arg... ) \
        __dlog_printf( level, ((context==NULL)?__func__:context), format, ##arg)
__attribute__ ((format (printf, 3, 4)))
void __dlog_printf( const int level, const char* context, 
                    const char* format, ... );

//-----------------------------------------------------------------------------
// FUNCTION     : __dlog_printbuf
//
// DESCRIPTION  : Logs a char buffer to DLOG
//
// PARAMETERS   : level   - The log level of the message
//                context - The context for the message being logged.
//                label   - The label for the buffer
//                buffer  - The actual buffer
//                len     - The number of bytes in the buffer
//
// RETURNS      : Nothing
//
// NOTE         : This function should not be called directly, and should
//                instead be called via the dlog_printbuf() MACRO. The macro adds
//                the ability for the context parameter to be NULL
//-----------------------------------------------------------------------------
#define dlog_printbuf( level, context, label, buff, len ) \
        __dlog_printbuf( level, ((context==NULL)?__func__:context), label, buff, len )
void __dlog_printbuf( const int level, const char* context, const char* label, const unsigned char* buff, const int len );

//-----------------------------------------------------------------------------
// FUNCTION     : dlog_full_should_log
//
// DESCRIPTION  : Determines if a log message with the given level, 
//                module/application and context should be logged. This is
//                a surrogate application's analog to dlog_should_log
//
// PARAMETERS   : level   - The log level to test
//                app     - The application/module name to test
//                context - The context string to test
//
// RETURNS      : TRUE(non-zero) if the message would be logged.
//
//-----------------------------------------------------------------------------
int dlog_full_should_log( const int level, const char* app, 
                          const char* context );

//-----------------------------------------------------------------------------
// FUNCTION     : dlog_full_printf
//
// DESCRIPTION  : Logs a message to DLOG, allows for a surrogate to log on
//                behalf of another module/application
//
// PARAMETERS   : level   - The log level of the message
//                app     - The name of the application/module logging
//                pid     - The pid of the application/module logging or -1 if 
//                          unknown
//                context - The context for the message being logged
//                format  - The format of the message to log
//                arg...  - The parameters for the formatted message
//
// RETURNS      : Nothing
//
//-----------------------------------------------------------------------------
__attribute__ ((format (printf, 5, 6)))
void dlog_full_printf( const int level, const char* app, const int pid, 
                       const char* context, const char* format, ... );

#endif // KERNEL_H

#endif // __DLOG_H
