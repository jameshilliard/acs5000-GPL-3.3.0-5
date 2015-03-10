//-----------------------------------------------------------------------------
// Copyright 2007, Avocent Corporation
//-----------------------------------------------------------------------------
// FILE                 : dlogcfg.c
// DESCRIPTION          : DLOG Configuration Utility
//
//-----------------------------------------------------------------------------
// AUTHOR               : Eduardo Fernandez
// DATE                 : April 18, 2007
//
//-----------------------------------------------------------------------------
// HISTORY
//
// DATE         PROGRAMMER              MODIFICATIONS
//
//-----------------------------------------------------------------------------
#include "dlog.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//-----------------------------------------------------------------------------
// FUNCTION     : get_log_level
//
// DESCRIPTION  : Converts the given string into a numerical log level
//
// PARAMETERS   : str - The log level as a String. This may be in the form
//                      of the DLOG_* constants, numerical values, or
//                      the printed level name.
//
// RETURNS      : The converted log level, or -2 if conversion failed.
//
//-----------------------------------------------------------------------------
static int get_log_level( char* str )
{
  char* inval;
  int level = strtol( str, &inval, 10 );
  if ( inval == str + strlen( str ) )
  {
    if ( ( level >= -1 ) && ( level <= 7 ) )
    {
      return ( level );
    }
  }
  
  if ( strncasecmp( "dlog_Em", str, 7 ) == 0 ) return ( 0 );
  if ( strncasecmp( "dlog_A", str, 6 ) == 0 ) return ( 1 );
  if ( strncasecmp( "dlog_C", str, 6 ) == 0 ) return ( 2 );
  if ( strncasecmp( "dlog_Er", str, 7 ) == 0 ) return ( 3 );
  if ( strncasecmp( "dlog_W", str, 6 ) == 0 ) return ( 4 );
  if ( strncasecmp( "dlog_Not", str, 8 ) == 0 ) return ( 5 );
  if ( strncasecmp( "dlog_I", str, 6 ) == 0 ) return ( 6 );
  if ( strncasecmp( "dlog_D", str, 6 ) == 0 ) return ( 7 );
  if ( strncasecmp( "dlog_Non", str, 8 ) == 0 ) return ( -1 );
  if ( strncasecmp( "Em", str, 2 ) == 0 ) return ( 0 );
  if ( strncasecmp( "A", str, 1 ) == 0 ) return ( 1 );
  if ( strncasecmp( "C", str, 1 ) == 0 ) return ( 2 );
  if ( strncasecmp( "Er", str, 2 ) == 0 ) return ( 3 );
  if ( strncasecmp( "W", str, 1 ) == 0 ) return ( 4 );
  if ( strncasecmp( "Not", str, 3 ) == 0 ) return ( 5 );
  if ( strncasecmp( "I", str, 1 ) == 0 ) return ( 6 );
  if ( strncasecmp( "De", str, 2 ) == 0 ) return ( 7 );
  if ( strncasecmp( "Non", str, 3 ) == 0 ) return ( -1 );
  
  return ( -2 );
}

//-----------------------------------------------------------------------------
// FUNCTION     : get_log_level_name
//
// DESCRIPTION  : Returns the String representation of the given log level
//
// PARAMETERS   : level - The log level to convert
//
// RETURNS      : The string representing the log level or Unknown if the value
//                is not valid.
//
//-----------------------------------------------------------------------------
static char* get_log_level_name( int level )
{
  switch ( level )
  {
    case DLOG_EMERGENCY:
      return "Emergency";
    case DLOG_ALERT:
      return "Alert";
    case DLOG_CRITICAL:
      return "Critical";
    case DLOG_ERROR:
      return "Error";
    case DLOG_WARNING:
      return "Warning";
    case DLOG_NOTICE:
      return "Notice";
    case DLOG_INFO:
      return "Info";
    case DLOG_DEBUG:
      return "Debug";
    case DLOG_NONE:    
      return "None";
    default: 
      return "Unknown";
  }
}

//-----------------------------------------------------------------------------
// FUNCTION     : print_rules
//
// DESCRIPTION  : Output the current DLOG state
//
// PARAMETERS   : None
//
// RETURNS      : Nothing
//
//-----------------------------------------------------------------------------
static void print_rules()
{
  char* outLoc;
  int val = dlog_get_output();
  if ( ( val & DLOG_OUTPUT_ALL ) == DLOG_OUTPUT_ALL )
    outLoc = "All";
  else if ( ( val & DLOG_OUTPUT_SYSLOG ) == DLOG_OUTPUT_SYSLOG )
    outLoc = "Syslog";
  else if ( ( val & DLOG_OUTPUT_PRINTF ) == DLOG_OUTPUT_PRINTF )
    outLoc = "Printf";
  else
    outLoc = "Unknown";
  printf( "Outputting To: %s\n", outLoc );
  printf( "\n");
  printf( "Module                           "
          "Context                          "
          "Level\n" );
  printf( "-------------------------------- "
          "-------------------------------- "
          "------------\n" );
  int x;
  for ( x = 0; x < dlog_get_rule_count(); x++ )
  {
    int level;
    char module[32], context[32];
    if ( dlog_get_rule( x, module, context, &level ) == 0 )
    {
      printf( "%-32s %-32s %s(%d)\n", 
              module, context, get_log_level_name( level ), level );
    }
    
  }
}

//-----------------------------------------------------------------------------
// FUNCTION     : get_error_msg
//
// DESCRIPTION  : Converts the given error code into an error message
//
// PARAMETERS   : val - The error code
//
// RETURNS      : The error message associated with the given error code, or
//                "Unknown" if the error code is not known
//
//-----------------------------------------------------------------------------
static char* get_error_msg( int val )
{
  switch ( val )
  {
    case DLOG_SUCCESS: 
      return "No Error";
    case DLOG_ERROR_INIT: 
      return "Failed to Initialize Shared Memory";
    case DLOG_ERROR_PATTERN: 
      return "Pattern was invalid";
    case DLOG_ERROR_TABLE_FULL: 
      return "Rule Table is Full";
    case DLOG_ERROR_NO_SUCH_RULE: 
      return "No Such Rule";
    case DLOG_ERROR_INVALID_OUTPUT: 
      return "Invalid Output Option";
    default: 
      return "Unknown";
  }
}

//-----------------------------------------------------------------------------
// FUNCTION     : main
//
// DESCRIPTION  : The main function. This parses the command line and
//                performs teh requested action
//
// PARAMETERS   : argc - The parameter count
//                argv - The parameters
//
// RETURNS      : The program result code. Always 0.
//
//-----------------------------------------------------------------------------
int main( int argc, char** argv )
{
  int printusage = 0;
  
  if ( argc > 1 )
  {
    if ( strcasecmp( argv[1], "print" ) == 0 )
    {
      if ( argc != 2 )
      {
        printusage = 1;
      }
      else
      {
        print_rules();
      }      
    }
    else if ( strcasecmp( argv[1], "refresh" ) == 0 )
    {
      if ( argc != 2 )
      {
        printusage = 1;
      }
      else
      {
        dlog_refresh();
        printf( "Application/Modules Refreshed.\n\n" );
        print_rules();
      }      
    }
    else if ( strcasecmp( argv[1], "reset" ) == 0 )
    {
      if ( argc != 2 )
      {
        printusage = 1;
      }
      else
      {
        dlog_reset();
        printf( "Rules Reset.\n\n" );
        print_rules();
      }      
    }
    else if ( strcasecmp( argv[1], "output" ) == 0 )
    {
      if ( argc != 3 )
      {
        printusage = 1;
      }
      else
      {
        if ( strcasecmp( argv[2], "printf" ) == 0 )
        {
          dlog_set_output( DLOG_OUTPUT_PRINTF );
        }
        else if ( strcasecmp( argv[2], "syslog" ) == 0 )
        {
          dlog_set_output( DLOG_OUTPUT_SYSLOG );
        }
        else if ( strcasecmp( argv[2], "all" ) == 0 )
        {
          dlog_set_output( DLOG_OUTPUT_ALL );
        }
        else
        {
          printf( "Failed to Modify Output. Error: Invalid Output Option.\n" );
          printf( "Possible Values: syslog, printf, all\n\n" );
        }
        print_rules();
      }      
    }
    else if ( strcasecmp( argv[1], "add" ) == 0 )
    {
      if ( argc != 5 )
      {
        printusage = 1;
      }
      else
      {
        int level = get_log_level( argv[4] );
        if ( level == -2 )
        {
          printf( "Failed to Add/Modify Rule. Error: Invalid Level.\n" );
          printf( "Possible Values: None, Emergency, Alert, Critical, Error, "
                  "Warning, Notice, Info, Debug\n\n" ); 
        }
        else
        {
          int ret = dlog_set_rule( argv[2], argv[3], level );
          if( ret == DLOG_SUCCESS )
          {
            printf( "Rule Added/Modified.\n\n" );
          }
          else
          {
            printf( "Failed to Add/Modify Rule. Error: %s.\n\n", 
                    get_error_msg( ret ) ); 
          }
        }
        print_rules();
      }      
    }
    else if ( strcasecmp( argv[1], "del" ) == 0 )
    {
      if ( argc != 4 )
      {
        printusage = 1;
      }
      else
      {
        int ret = dlog_set_rule( argv[2], argv[3], DLOG_NONE );
        if( ret == DLOG_SUCCESS )
        {
          printf( "Rule Deleted.\n\n" );
        }
        else
        {
          printf( "Failed to Delete Rule. Error: %s.\n\n", 
                  get_error_msg( ret ) ); 
        }
        print_rules();
      }      
    }
    else
    {
      printusage = 1;
    }
  }
  else
  {
    printusage = 1;
  }
  
  if ( printusage )
  {
    printf( "This utility may be used to modify or "
            "view the current DLOG settings.\n" );
    printf( "\n" );
    printf( "      To Print Rules: %s print\n", argv[0] );
    printf( "    To Refresh Rules: %s refresh\n", argv[0] );
    printf( "      To Reset Rules: %s reset\n", argv[0] );
    printf( "To Set Output Method: %s output [syslog|printf|all]\n", argv[0] );
    printf( "To Add/Modify a Rule: %s add "
            "MODULE_PATTERN CONTEXT_PATTERN LOG_LEVEL\n", argv[0] );
    printf( "    To Remove a Rule: %s del "
            "MODULE_PATTERN CONTEXT_PATTERN\n", argv[0] );
  }

  return ( 0 );
}
