//-----------------------------------------------------------------------------
// Copyright 2007, Avocent Corporation
//-----------------------------------------------------------------------------
// FILE                 : dlog.c
// DESCRIPTION          : DLOG Shared Memory Implementation
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
#include "dlog.h"

#include <dirent.h>
#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <sys/un.h>

// Maximum Pattern Length
#define MAX_PATTERN_LEN         32

// Maximum Syslog Message Size
#define SYSLOG_MAX_SIZE         4096

// The location of the Syslog Unix datagram socket
#define SYSLOG_DEVICE           "/dev/log"

// The key by which the shared memory is allocated KEY = GSPDLOG on telephone
#define SHARED_MEMORY_KEY       4773564

// The amount of memory to allocate ( 1 page )
#define SHM_SIZE sysconf(_SC_PAGE_SIZE)

// Internally used to identify the pattern type
#define PATTERN_TYPE_WILDCARD 0
#define PATTERN_TYPE_SUFFIX   1
#define PATTERN_TYPE_PREFIX   2
#define PATTERN_TYPE_INFIX    3
#define PATTERN_TYPE_EXACT    4

// Structure used to represent a pattern internally
typedef struct
{
  // The Pattern Type (see PATTERN_TYPE_* constants)
  int type;
  
  // The Pattern Length (Only non-wildcard characters)
  int len;
  
  // The Patern minus wildcard characters
  char data[ MAX_PATTERN_LEN ];
} pattern;

// Structure used to represent an entry in the rules table
typedef struct
{
  // The Application/Module Pattern
  pattern app;
  
  // The Context Pattern
  pattern ctx;
  
  // The Log Level for this rule
  int level;
} rule_entry;

// Structure to represent a rule table.
typedef struct
{
  // A change count (incremented each time a rule is modified)
  int changecnt;
  
  // The number of valid rules
  int rulecnt;
  
  // The output flag value
  int outputFlag;
  
  // The minimum level that requires rule checks
  int minlevel;
  
  // The maximum number of rules
  int rulemax;
  
  // The actual rules
  rule_entry rule[];
} rule_table;

// Pointer to the shared global table (This table is shared by all processes)
static rule_table* global_table = NULL;

// Pointer to the local table (This table is a subset of the global_table)
static rule_table* local_table = NULL;

// Holds the name of the current process
static char proc_name[32];

// Holds the PID of the current process
static int proc_pid;

// Holds the file descriptor used to send Syslog Datagrams for this process
static int proc_syslog_fd = -1;

#define check_fd()\
	if ( proc_syslog_fd == -1 )\
	{\
		int tmpfd = socket( PF_UNIX, SOCK_DGRAM, 0 );\
		struct sockaddr_un syslog_addr = {\
			.sun_family = AF_UNIX,\
			.sun_path = SYSLOG_DEVICE\
		};\
		\
		if ( connect( tmpfd, \
									(struct sockaddr*)&syslog_addr, sizeof( struct sockaddr_un ) ) == -1 )\
		{\
			close( tmpfd );\
			\
			if ( errno == EPROTOTYPE )\
			{\
				\
				tmpfd = socket( PF_UNIX, SOCK_STREAM, 0 );\
				struct sockaddr_un syslog_addr = {\
					.sun_family = AF_UNIX,\
					.sun_path = SYSLOG_DEVICE\
				};\
				\
				if ( connect( tmpfd, \
											(struct sockaddr*)&syslog_addr, sizeof( struct sockaddr_un ) ) == -1 )\
				{\
					close( tmpfd );\
					tmpfd = -1;\
					printf( "Failed to connect using DGRAM and STREAM. Errno: %d\n" , errno );\
				}\
			}\
		}\
		proc_syslog_fd = tmpfd;\
	}

#define reopen_fd()\
{\
	int tmpfd = proc_syslog_fd;\
	proc_syslog_fd = -1;\
	if ( tmpfd != -1 )\
	{\
		close( tmpfd );\
	}\
	check_fd();\
}\
	

//-----------------------------------------------------------------------------
// FUNCTION     : init_table
//
// DESCRIPTION  : Initializes the given table structure to empty.
//
// PARAMETERS   : table - The table to initialize
//
// RETURNS      : Nothing
//
//-----------------------------------------------------------------------------
static void init_table( rule_table* table )
{
  int x;
  int rule_max = ( ( SHM_SIZE - offsetof( rule_table, rule ) ) / 
                   ( sizeof( rule_entry ) ) );

  bzero( table, SHM_SIZE );

  table->minlevel = DLOG_NONE;
  table->outputFlag = DLOG_OUTPUT_SYSLOG;
  table->rulemax = rule_max;
  for ( x = 0; x < rule_max; x++ )
  {
    table->rule[x].level = DLOG_NONE;
  }
}

//-----------------------------------------------------------------------------
// FUNCTION     : pid_to_name
//
// DESCRIPTION  : Convert a process PID to a name. Used internally to determine
//                the processes own name. This uses the /proc filesystem for
//                this
//
// PARAMETERS   : pid  - The PID of the process to obtain the name for
//                name - A buffer to output the process name to
//                max  - The maximum number of characters that may be stored
//                       in the name buffer
//
// RETURNS      : Nothing
//
//-----------------------------------------------------------------------------
static void pid_to_name(int pid, char* name, int max)
{
  if ( max == 0 )
  {
    return;
  }
  name[0] = 0;
  
  int ch;
  char filename[32];
  FILE *file;
  
  snprintf(filename, 32, "/proc/%d/cmdline", pid);
  file = fopen( filename, "r");
  if ( file )
  {
    int off = 0;
    while ( ( ch = fgetc( file ) ) != EOF )
    {  
      if ( ch == '/' )
      {
        off = 0;
      }
      else if ( ch == 0 )
      {
        break;
      }
      else if ( off < max - 1 )
      {
        name[ off++ ] = ch;
      }
    }
    name[ off ] = 0;
    fclose( file );
  }
}

//-----------------------------------------------------------------------------
// FUNCTION     : string_to_pattern
//
// DESCRIPTION  : Converts the given String to a pattern representation
//
// PARAMETERS   : pattern_str    - A String representing a pattern.
//                pattern_struct - A pointer to a patter struct to populate
//
// RETURNS      : Nothing.
//
//-----------------------------------------------------------------------------
static void string_to_pattern(const char* pattern_str, pattern* pattern_struct)
{
  int sl = strlen(pattern_str);
  bzero( pattern_struct, sizeof( *pattern_struct ) );

  if ( sl == 0 )
  {
      pattern_struct->type = PATTERN_TYPE_EXACT;
  }
  else if ( strcmp( pattern_str, "*" ) == 0 )
  {
      pattern_struct->type = PATTERN_TYPE_WILDCARD;
  }
  else if ( pattern_str[0] == '*' ) // SUFFIX or INFIX
  {
    if ( pattern_str[ sl - 1 ] == '*' ) // INFIX
    {
      pattern_struct->type = PATTERN_TYPE_INFIX;
      pattern_struct->len = sl - 2;
      strncpy(pattern_struct->data, pattern_str + 1 , sl - 2 );
    }
    else // SUFFIX
    {
      pattern_struct->type = PATTERN_TYPE_SUFFIX;
      pattern_struct->len = sl - 1;
      strcpy(pattern_struct->data, pattern_str + 1);
    }
  }
  else if ( pattern_str[ sl - 1 ] == '*' ) // PREFIX
  {
      pattern_struct->type = PATTERN_TYPE_PREFIX;
      pattern_struct->len = sl - 1;
      strncpy(pattern_struct->data, pattern_str, sl - 1);
  }
  else // EXACT
  {
      pattern_struct->type = PATTERN_TYPE_EXACT;
      pattern_struct->len = sl;
      strcpy( pattern_struct->data, pattern_str);
  }
}

//-----------------------------------------------------------------------------
// FUNCTION     : pattern_to_string
//
// DESCRIPTION  : Converts the given pattern to a String representation
//
// PARAMETERS   : pattern_struct - The pattern to convert
//                pattern_str    - A 32 byte buffer to output the pattern to
//
// RETURNS      : Nothing.
//
//-----------------------------------------------------------------------------
static void pattern_to_string(const pattern* pattern_struct, char* pattern_str )
{
  switch ( pattern_struct->type )
  {
    case PATTERN_TYPE_EXACT:
      sprintf( pattern_str, "%s", pattern_struct->data );
      break;
    case PATTERN_TYPE_WILDCARD:
      sprintf( pattern_str, "*" );
      break;
    case PATTERN_TYPE_INFIX:
      sprintf( pattern_str, "*%s*", pattern_struct->data );
      break;
    case PATTERN_TYPE_SUFFIX:
      sprintf( pattern_str, "*%s", pattern_struct->data );
      break;
    case PATTERN_TYPE_PREFIX:
      sprintf( pattern_str, "%s*", pattern_struct->data );
      break;
    default:
      pattern_str[0] = 0;
      break;
  }
}

//-----------------------------------------------------------------------------
// FUNCTION     : pattern_match
//
// DESCRIPTION  : Used internally to check is the given string matches the
//                given pattern.
//
// PARAMETERS   : pattern_struct - The pattern to compare the string against
//                pattern_str    - The string to compare with the pattern
//
// RETURNS      : TRUE(Non-zero) if the string matches the pattern
//
//-----------------------------------------------------------------------------
static int pattern_match ( const pattern* pattern_struct, 
                           const char *pattern_str)
{
  int slen;
  
  switch ( pattern_struct->type )
  {
    case PATTERN_TYPE_WILDCARD:
      return 1;
    case PATTERN_TYPE_SUFFIX:
      slen = strlen( pattern_str );
      return ( ( slen >= pattern_struct->len ) && 
               ( memcmp( pattern_str + slen - pattern_struct->len, 
                         pattern_struct->data, 
                         pattern_struct->len * sizeof( char ) ) == 0 ) );
    case PATTERN_TYPE_PREFIX:
      return ( ( strlen( pattern_str ) >= pattern_struct->len ) && 
               ( memcmp( pattern_str, pattern_struct->data, 
                         pattern_struct->len * sizeof( char ) ) == 0 ) );
    case PATTERN_TYPE_INFIX:
      return ( ( strlen( pattern_str ) >= pattern_struct->len ) && 
               ( strstr( pattern_str, pattern_struct->data ) != NULL ) );
    case PATTERN_TYPE_EXACT:
      return ( ( strlen( pattern_str ) == pattern_struct->len ) && 
               ( memcmp( pattern_str, pattern_struct->data, 
                         pattern_struct->len * sizeof( char ) ) == 0 ) );
    default:
      return ( 0 );
  }
} 

//-----------------------------------------------------------------------------
// FUNCTION     : get_global_rule_table
//
//
// DESCRIPTION  : Returns a pointer to the global rule table. If neccessary, it
//                will attach to the shared memory, and initialize the memory if
//                neccessary as well
//
// PARAMETERS   : None.
//
// RETURNS      : A pointer to the global rule table, or NULL if the attach
//                to shared memory failed.
//
//-----------------------------------------------------------------------------
static rule_table* get_global_rule_table()
{
  if ( global_table == NULL )
  {
    // SHARED MEM INIT
    key_t key = SHARED_MEMORY_KEY;
    int shmid;
    if ( ( shmid = shmget(key, SHM_SIZE, 0400 ) ) < 0 ) 
    {
      //create shared memory segment
      if ( ( shmid = shmget(key, SHM_SIZE, IPC_EXCL | IPC_CREAT | 0644 ) ) < 0 )
      {
        // Failed to create a new region
        return ( NULL );
      }
      rule_table* tmp = shmat( shmid, NULL, 0 );
      init_table( tmp );
      global_table = tmp;
      dlog_set_rule( "*", "*", DLOG_ERROR );
      
      struct shmid_ds perms;
      perms.shm_perm.uid = 0;
      perms.shm_perm.gid = 0;
      perms.shm_perm.mode = 0444;
      shmctl( shmid, IPC_SET,  &perms );
      
      dlog_refresh();
    }
    else
    {
      // Try Read/Write Attach
      rule_table* tmp = shmat( shmid, NULL, 0 );
      if ( (int)tmp == -1 ) // RW Failed.. try RO
      {
         tmp = shmat( shmid, NULL, SHM_RDONLY );
      }
      if ( (int)tmp != -1 ) // One of the attaches succeeded.. update the pointer
      {
         global_table = tmp;
      }
    }
  }
  
  return ( global_table );
}

//-----------------------------------------------------------------------------
// FUNCTION     : get_local_rule_table
//
// DESCRIPTION  : Returns the application local Rule Table. If neccessary
//                updates the local table from the global table.
//
// PARAMETERS   : None.
//
// RETURNS      : A pointer to the local rule table, or NULL if the table
//                could not be allocated.
//
//-----------------------------------------------------------------------------
static rule_table* get_local_rule_table()
{
  if ( local_table == NULL )
  {
		check_fd();
		
    proc_pid = getpid();
    pid_to_name( proc_pid, proc_name, sizeof( proc_name ) / sizeof( char ) );

    // If no local table... allocate one
    rule_table* tmp = (rule_table*)malloc( SHM_SIZE );
    if ( tmp == NULL )
    {
      return ( NULL );
    }
    init_table( tmp );
    local_table = tmp;
    
    // Global Table may also be NULL..
    if ( global_table == NULL )
    {
      get_global_rule_table(); // Force Load
      if ( global_table == NULL )
      {
         return ( NULL );
      }
    }
    
  }

  if ( local_table->changecnt != global_table->changecnt )
  {
    // Update change count first.. limit chances of double copies
    local_table->changecnt = global_table->changecnt;
    local_table->minlevel = DLOG_NONE;
    
    int rc = global_table->rulecnt;
    int lrc = 0;
    int x = 0;
    for ( x = 0; x < rc; x++ )
    {
      if ( global_table->rule[x].level != DLOG_NONE )
      {
        if ( pattern_match( &global_table->rule[x].app, proc_name ) )
        {
          // COPY RULE.. only level and context matter!
          local_table->rule[lrc].level = DLOG_NONE;
          memcpy( &local_table->rule[lrc].ctx, 
                  &global_table->rule[x].ctx, sizeof( pattern ) );
          local_table->rule[lrc].level = global_table->rule[x].level;
          if ( local_table->rule[lrc].ctx.type == PATTERN_TYPE_WILDCARD )
          {
            local_table->minlevel = local_table->rule[lrc].level;
            break;
          }
          lrc++;
        }
      }
    }
    
    local_table->rulecnt = lrc;
    for ( x = lrc; x < local_table->rulemax; x++ )
    {
      local_table->rule[x].level = DLOG_NONE;
    }
  }
  
  return ( local_table );
}

//-----------------------------------------------------------------------------
// FUNCTION     : writelog
//
// DESCRIPTION  : Outputs a log message to all appropriate output locations
//
// PARAMETERS   : level  - Log Level of message
//                app    - Application/module beign logged for
//                pid    - PID of process, -1 for unknown
//                ctx    - Context for Message
//                format - Message Format
//                va     - Format Arguments
//
// RETURNS      : Nothing.
//
//-----------------------------------------------------------------------------
static void writelog( const int level, const char* app, const int pid, 
                      const char* ctx, const char* format, va_list va )
{
  char buffer[ SYSLOG_MAX_SIZE ];
  int len,len2;
  if ( pid == -1 )
  {
    len = snprintf( buffer, SYSLOG_MAX_SIZE, "<%d>%s: [%s] ", 
                    level | SYSLOG_DLOG_FACILITY, app, ctx );
  }
  else
  {    
    len = snprintf( buffer, SYSLOG_MAX_SIZE, "<%d>%s(%d): [%s] ", 
                    level | SYSLOG_DLOG_FACILITY, app, pid, ctx );
  }
  if ( len < 0 )
  {
     len = 0;
  }
  
  if ( ( len > SYSLOG_MAX_SIZE - 1 ) ) 
  {
     len = (SYSLOG_MAX_SIZE - 1);
  }
  len2 = vsnprintf( buffer + len, SYSLOG_MAX_SIZE - len, format, va );
  if ( len2 < 0 )
  {
     len2 = snprintf( buffer + len, SYSLOG_MAX_SIZE - len, "Invalid Message Format? Format String: \"%s\"", format );
     if ( len2 >= 0 )
     {
       len += len2;
     }
  }
  else
  {
   len += len2;
  }
  if ( ( len > SYSLOG_MAX_SIZE - 1 ) ) 
  {
     len = (SYSLOG_MAX_SIZE - 1);
  }

  int outputFlag = global_table->outputFlag;
  if ( outputFlag & DLOG_OUTPUT_SYSLOG )
  {
	int firsttime = 1;  
    int res;
	int err = 0;
	check_fd();
    while ( (res = send( proc_syslog_fd, buffer, len + 1, 0 ) ) < 0 )
    {
		err = errno;
		if ( err == EMSGSIZE )
		{
			// Too LONG
			break;
		}
		else
		{
			if ( firsttime )
			{
				reopen_fd();
				firsttime = 0;
				err = 0;
			}
			else
			{
				// Connection Reattempt Already Tried Quit
				break;
			}
		}
    }
    if ( ( !firsttime ) && ( err ) )
    {
       fprintf( stderr, "Failed to send DLOG message to Syslog! Error: %d  FD: %d\n", err, proc_syslog_fd );
       if ( ! ( outputFlag & DLOG_OUTPUT_PRINTF ) )
       {
          if ( buffer[ len - 1 ] != '\n' )
          {
             if ( len == SYSLOG_MAX_SIZE - 1 )
             {
                buffer[ SYSLOG_MAX_SIZE - 2 ] = '\n';
             }
             else
             {
               len += snprintf( buffer + len, (SYSLOG_MAX_SIZE - 1) - len, "\n" ); 
             }
          }
          fprintf( stderr, "%s", buffer );
       }
    }
  }
  if ( outputFlag & DLOG_OUTPUT_PRINTF )
  {
		if ( buffer[ len - 1 ] != '\n' )
		{
       if ( len == SYSLOG_MAX_SIZE - 1 )
       {
          buffer[ SYSLOG_MAX_SIZE - 2 ] = '\n';
       }
       else
       {
			len += snprintf( buffer + len, (SYSLOG_MAX_SIZE - 1) - len, "\n" ); 
       }
		}
      printf( "%s", buffer );
  }
}

/*****************************************************/
/**************** Start of public API ****************/
/*****************************************************/

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
void dlog_reset()
{
  rule_table* tmp = get_global_rule_table();
  if ( tmp != NULL )
  {
    init_table( tmp );
    dlog_set_rule( "*", "*", DLOG_ERROR );
    dlog_refresh();
  }
}

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
void dlog_refresh()
{
  rule_table* table = get_global_rule_table();

  if ( table == NULL )
  {
    return;
  }
  int i;
    
  // Module name plus "/sys/module/" and "/dlog_level"
  char filename_buffer[ NAME_MAX + 32 ]; 
  DIR *dir = opendir( "/sys/module" );
  if ( dir )
  {
    struct dirent* ent;
    while ( ( ent = readdir( dir ) ) != NULL )
    {
      int modlevel = DLOG_NONE;
      
      // Try each rule against the module name
      for ( i=0; i < table->rulecnt; i++ )
      {
        int rlevel = table->rule[ i ].level;
        if ( rlevel == DLOG_NONE ) // No More Applicable Rules
        {
          continue;
        }
        
        // Check if module name matches the pattern
        if ( pattern_match( &table->rule[ i ].app, ent->d_name ) )
        {
          modlevel = rlevel;
          break;
        }
      }
    
      // Matches module name! Update dlog_level
      snprintf( filename_buffer, NAME_MAX + 32, 
                "/sys/module/%s/parameters/dlog_level", ent->d_name );
      
      // Verify dlog_level file exists
      FILE* file = fopen( filename_buffer, "r" );
      if ( file )
      {
        fclose( file );
        
        // Rewrite with new dlog level
        file = fopen( filename_buffer, "w" );
        if ( file )
        {
          char ch[32];
          snprintf( ch, 32, "%d", modlevel );
          fwrite( ch, strlen(ch), 1, file );
          fclose( file );
        }
      }
    }
    closedir( dir );
  }
}

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
int dlog_set_output( int flag )
{
  if ( ( ( flag & DLOG_OUTPUT_ALL ) == 0 ) || 
       ( ( flag & ~(DLOG_OUTPUT_ALL) ) != 0 ) )
  {
    return ( DLOG_ERROR_INVALID_OUTPUT );
  }
  rule_table* table = get_global_rule_table();
  if ( table == NULL )
  {
     return ( DLOG_ERROR_INIT );
  }
  table->outputFlag = flag;
  
  return ( DLOG_SUCCESS );
}

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
int dlog_get_output()
{
  rule_table* table = get_global_rule_table();
  if ( table == NULL )
  {
     return ( 0 );
  }
  return ( table->outputFlag );
}

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
int dlog_set_rule( const char* module_pattern_str, 
                   const char* context_pattern_str, const int level)
{
  rule_table* table = get_global_rule_table();

  if ( table == NULL )
  {
    return ( DLOG_ERROR_INIT );
  }

  if ( ( strlen( module_pattern_str ) >= MAX_PATTERN_LEN ) || 
       ( strlen( context_pattern_str ) >= MAX_PATTERN_LEN ) )
  {
    return ( DLOG_ERROR_PATTERN );
  }
  
  pattern parsed_module_pattern;
  pattern parsed_context_pattern;
  string_to_pattern( module_pattern_str, &parsed_module_pattern );
  string_to_pattern( context_pattern_str, &parsed_context_pattern );

  int success = 0;
  int x;
  
  for ( x = 0; x < table->rulemax; x++ )
  {
    rule_entry* currule = &table->rule[x];
    if ( ( currule->level != DLOG_NONE ) &&
         ( memcmp( &currule->app, 
                   &parsed_module_pattern, sizeof( pattern ) ) == 0 ) &&
         ( memcmp( &currule->ctx, 
                   &parsed_context_pattern, sizeof( pattern ) ) == 0 ) )
    {
      
      memmove( currule, currule + 1, 
               ( (table->rulemax - x) - 1 ) * sizeof( rule_entry ) );
      table->rulecnt--;
      
      table->rule[ table->rulemax - 1 ].level = DLOG_NONE;
      bzero( &table->rule[ table->rulemax - 1 ].app, 
             sizeof( table->rule[ table->rulemax - 1 ].app ) );
      bzero( &table->rule[ table->rulemax - 1 ].ctx, 
             sizeof( table->rule[ table->rulemax - 1 ].ctx ) );
      
    }
    
    if ( ( level > currule->level ) && ( success == 0 ) )
    {
      if ( table->rulecnt == table->rulemax )
      {
        return ( DLOG_ERROR_TABLE_FULL );
      }
      // INSERT
      memmove( currule + 1, currule, 
               ( (table->rulemax - x) - 1 ) * sizeof( rule_entry ) );
      table->rulecnt++;
      
      currule->level = DLOG_NONE;
      memcpy( &currule->app, &parsed_module_pattern, sizeof( pattern ) );
      memcpy( &currule->ctx, &parsed_context_pattern, sizeof( pattern ) );
      currule->level = level;
      
      success = 1;
    }
  }
  
  table->changecnt++;
  dlog_refresh();
  return ( ( level == DLOG_NONE ) ? 
                           DLOG_SUCCESS : 
                           ( success ? DLOG_SUCCESS : DLOG_ERROR_TABLE_FULL ) );
}

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
int dlog_get_rule( int index, char* module, char* context, int* log_level )
{
  rule_table* table = get_global_rule_table();

  if ( table == NULL )
  {
    return ( DLOG_ERROR_INIT );
  }
  
  if ( index < table->rulecnt )
  {
    pattern_to_string( &table->rule[ index ].app, module );
    pattern_to_string( &table->rule[ index ].ctx, context );
    *log_level = table->rule[ index ].level;
    
    return ( DLOG_SUCCESS );
  }
  else
  {
    return ( DLOG_ERROR_NO_SUCH_RULE );
  }
  
}

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
int dlog_get_rule_count()
{
  rule_table* tmp = get_global_rule_table();
  if ( tmp == NULL )
  {
    return ( 0 );
  }
  else
  {
    return ( tmp->rulecnt );
  }
}

//-----------------------------------------------------------------------------
// FUNCTION     : __dlog_should_log
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
// NOTE         : This function should not be called directly, and should
//                instead be called via the dlog_printf() MACRO. The macro adds
//                the ability for the context parameter to be NULL
//-----------------------------------------------------------------------------
int __dlog_should_log( const int level, const char* context )
{
  rule_table* table = get_local_rule_table();

  if ( table == NULL )
  {
    return ( 0 );
  }
  else if ( level <= table->minlevel )
  {
    return ( 1 );
  }
  else if ( table->rulecnt > 0 )
  {
    int i;
    
    for ( i=0; i < table->rulecnt; i++ )
    {
      int rlevel = table->rule[ i ].level;
      // No More Applicable Rules
      if ( ( rlevel != DLOG_NONE ) && ( level > rlevel ) ) 
      {
        break;
      }
      
      if ( pattern_match( &table->rule[ i ].ctx, context) )
      {
        return ( 1 );
      }
    }
  }
  
  return 0;
}

//-----------------------------------------------------------------------------
// FUNCTION     : __dlog_printf
//
// DESCRIPTION  : Logs a message to DLOG
//
// PARAMETERS   : level   - The log level of the message
//                context - The context for the message being logged.
//                format  - The format of the message to log
//                ...     - The parameters for the formatted message
//
// RETURNS      : Nothing
//
// NOTE         : This function should not be called directly, and should
//                instead be called via the dlog_printf() MACRO. The macro adds
//                the ability for the context parameter to be NULL
//-----------------------------------------------------------------------------
void __dlog_printf( const int level, const char* context, 
                    const char* format, ... )
{
  if ( dlog_should_log( level, context ) )
  {
    va_list ap;
    va_start(ap, format);
    writelog( level, proc_name, proc_pid, context, format, ap );
    va_end( ap );
  }
}


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
 void __dlog_printbuf( const int level, const char* context, const char* label, const unsigned char* buff, const int len )
{
    if ( dlog_should_log( level, context ) )
    {
		if ( len == 0 )
		{
			dlog_printf( level, context, "%s(01/01): Empty\n", label );
		}
		else
		{
			int linenum = 0;
			int linetot = ((len+15)/16);
			char hex1[25];
			char hex2[25];
			char ascii[17];
			int lineoff = 0;
			while ( lineoff < len )
			{
				int x;
				memset( hex1, 0, sizeof( hex1 ) );
				memset( hex2, 0, sizeof( hex2 ) );
				memset( ascii, 0, sizeof( ascii ) );
				
				for ( x = 0; x < 8; x++ )
				{
					if ( lineoff + x < len )
					{
						sprintf( hex1 + (x*3), "%02x ", *(buff + lineoff + x) );
						unsigned char ch = *(buff + lineoff + x);					
						ascii[ x ] = (( ch <= ' ' )||( ch > 126 ))?'.':ch;
					}
					if ( lineoff + 8 + x < len )
					{
						sprintf( hex2 + (x*3), "%02x ", *(buff + 8 + lineoff + x) );
						unsigned char ch = *(buff + 8 + lineoff + x);					
						ascii[ x + 8 ] = (( ch <= ' ' )||( ch > 126 ))?'.':ch;
					}
				}
				
				dlog_printf( level, context, "%s(%02d/%02d): %04x %-24s %-24s %-16s\n", label, ++linenum, linetot, lineoff, hex1, hex2, ascii );
				lineoff += 16;
			}
		}
    }
}


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
                          const char* context )
{
  rule_table* table = get_global_rule_table();

  if ( table == NULL )
  {
    return ( 0 );
  }
  else if ( table->rulecnt > 0 )
  {
    int i;

    for (i=0; i < table->rulecnt; i++)
    {
      int rlevel = table->rule[ i ].level;
      if ( ( rlevel != DLOG_NONE ) && ( level > rlevel ) ) 
      {
        break;
      }
      
      if ( ( pattern_match( &table->rule[ i ].app, app) ) &&
           ( pattern_match( &table->rule[ i ].ctx, context) ) )
      {
        return ( 1 );
      }
    }
  }
  
  return 0;
}

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
void dlog_full_printf( const int level, const char* app, const int pid, 
                       const char* context, const char* format, ... )
{
  if ( dlog_full_should_log( level, app, context ) )
  {
    va_list ap;
    va_start(ap, format);
    writelog( level, app, pid, context, format, ap );
    va_end( ap );
  }
}


