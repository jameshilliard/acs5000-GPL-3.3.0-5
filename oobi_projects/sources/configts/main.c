/******************************************************************************
  Copyright (c) 2001 - 2002 Ki NETWORKS, Inc..  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************

  RCS Revision Control Information:
   @(#)$Header: /usr/cvsroot/oobi_projects/sources/configts/main.c,v 1.1.1.1 2005/09/01 02:36:37 edson Exp $

   Revision:        $Revision: 1.1.1.1 $
   Created From:    $Source: /usr/cvsroot/oobi_projects/sources/configts/main.c,v $
   Last Updated:    $Date: 2005/09/01 02:36:37 $
   Current State:   $State: Exp $
   Checked Out By:  $Locker:  $

*******************************************************************************

                                main.c

   Description:

      This file contains the Cyclades control program main program.
      Arguments in [] are optional
      Arguments in <> are required


      The command syntax in user interactive mode is:

         
         exit
         help
         quit
         modify port <number> tty      <string>
         modify port <number> protocol <string>
         modify port <number> interval <number>       poll interval
         modify port <number> authtype <string>
         modify port <number> speed <number>
         modify port <number> datasize <number>
         modify port <number> stopbits <number>
         modify port <number> parity <string>
         modify port <number> socket <number>          port ip number
         modify port <number> break <string>
         
           or
  
         modify port <number> tty <string> protocol <string> 

           or

         modify port <number> tty <string> protocol <string>  interval <number> authtype <string>  speed <number> datasize <number>  stopbits <number>  parity <string>  socket <number> break <string>

if string has special character such as ~ need to enclose them in ""
when issuing from command line else the shell will barf on them

tsxcp modify port 35 break "~~~"

           example  in command line mode:

/bin/tsxcp modify port 35 tty ttyS35 protocol socket_server interval 1 authtype none speed 19200 datasize 7 stopbits 2 parity even socket 3035 break HARDW

or


/bin/tsxcp modify port 2 tty ttyS2 protocol socket_ssh interval 0 authtype none  speed 9600 datasize 8 stopbits 1  parity none socket 2002 break "~break"

if want to modify a range of ports or a set of ports at the same time, 

example:
/bin/tsxcp modify port 1-10 speed 19200

will modify all ports 1 to 10 with speed 19200


/bin/tsxcp modify port 1,3,5,7  speed 19200

will modify ports 1 3 5 and 7 with speed 19200


   Change Log:

     Date    Who  Rev   Reason for update
   --------- --- ------ -------------------------------------------------------
   30-Oct-02 bqc  1.6   1.2.0: support Lantronix
   23-Oct-02 bqc  1.6   1.1.2: add clear line/logout port command
                               add port range capability
   17-Oct-02 bns  1.5   1.1.1: make ? and tab work in interactive input
   10-Oct-02 bns  1.4   1.1.0: call clexec() rather than clread() and clparse()
   01-Oct-02 bqc  1.2   1.0.1: add cisco commands syntax
   23-Sep-02 bqc  1.1   1.0.0: create from kidscp

******************************************************************************/

#define MY_REV	"1.2.0"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

#include "clparse.h"


#ifdef CYCLADES_TS
#define LOCK_FILE "/var/lock/LCK..pslave"
#define PSLAVE_CONF "/etc/portslave/pslave.conf"
#endif

#ifdef LANTRONIX_TS
#define LOCK_FILE  "/var/lock/LCK..lci"
#define INETD_CONF "/etc/inetd.conf"
#define LWIP_CONF  "/etc/lci/lwip_serial.conf"
#define LCI_CONF   "/etc/lci/lci.conf"
#define DEFAULT_USER_CONF "/lci/default.user.conf"
#define NOBODY_CONF       "/lci/users/nobody.conf"
#endif

#define FALSE 0
#define TRUE  ~FALSE
#define MAXHOSTNAMELEN 128
#define KZERO(cp,n)  memset ((void *) (cp), (int) 0, (size_t) n)
#define BSTRCPY(x,y) (void)kstrncpy((char *)&(x)[0], (char *)(y), sizeof(x) - 1)

#ifdef CYCLADES_TS
#ifdef CYCLADES_KVM
#include "kvmdefs.h"
#include "kvmcfg.h"
#include "kvmconfig.h"
#include "kvmuser.h"
#endif
#endif

/**************************           
* Externally Defined Data *
**************************/

/******************
* Forward Modules *
******************/
static CL_CALLBACK HelpCB;
static CL_CALLBACK ExitCB;
static CL_CALLBACK ModifyPortCB;
static CL_CALLBACK ModifyConfCB;
static CL_CALLBACK ConfigureEtherCB;
extern CL_STATE modify_port_number;
static CL_CALLBACK ShowVersionCB;
static CL_CALLBACK WriteCB;
static CL_CALLBACK LogoutPortCB;
#ifdef CYCLADES_TS
static CL_STATE config_conf_state;
static CL_STATE config_ether_state;
#ifdef CYCLADES_KVM
static CL_CALLBACK ModifyKVMConfCB;
static CL_CALLBACK ModifyKVMSlaveCB;
static CL_CALLBACK ModifyKVMPortCB;
static CL_CALLBACK ModifyKVMUserCB;
static CL_STATE config_kvmconf_state;
static CL_STATE config_kvmslave_name;
static CL_STATE config_kvmport_number;
static CL_STATE config_kvmuser_name;
static CL_STATE config_kvmslave_name;
#endif
#endif

/*****************************
* Externally Referenced Data *
*****************************/

/***************
* Private Data *
***************/
static char host[MAXHOSTNAMELEN];
static char myName[MAXHOSTNAMELEN];
static int exit_status;
static int inetd_pid = 0;


/**************
* Static Data *
**************/
#define NUM_ELEMENTS(x)     (sizeof (x)/sizeof (x)[0])

#define NUM_PORT_PARAM 59 /* there are only 10 params but allow make 1 extra */
#define NUM_ETHER_PARAM 4 /* there are only 3 params but allow make 1 extra */
#define NUM_CONF_PARAM 6
#define TOKEN_SIZE   40  /* the string s18.poll_interval */   
#define COMMAND_SIZE 64  /* the string s18.poll_interval 0 */ 

typedef struct _port_param 
{
   char token[COMMAND_SIZE];
   char command[COMMAND_SIZE];
   int  token_len;
} PORT_PARAM;

typedef struct _port_list
{
   int                   number; /* port number */
   struct _port_list*    next;
} PORT_LIST;

static PORT_LIST *port_head = NULL;

#ifndef CYCLADES_TS
char *help_string = "\n\
   exit \n\
   help \n\
   modify port <number> tty <string> \n\
   modify port <number> protocol <string> \n\
   modify port <number> pollinterval <number> \n\
   modify port <number> authtype <string> \n\
   modify port <number> speed <number>    \n\
   modify port <number> datasize <number> \n\
   modify port <number> stopbits <number> \n\
   modify port <number> parity <string>   \n\
   modify port <number> socket <number>  \n\
   modify port <number> break <string> \n\
     - or - \n\
   modify port <number> tty <string> protocol <string> \n\
     - or -\n\
   modify port <number> tty <string> protocol <string>  pollinterval <number> authtype <string>  speed <number> datasize <number>  stopbits <number>  parity <string>  socket <number> break <string> \n\
     - or - \n\
   configure line <number> tty <string> \n\
   configure line <number> protocol <string> \n\
   configure line <number> pollinterval <number> \n\
   configure line <number> authtype <string> \n\
   configure line <number> speed <number>    \n\
   configure line <number> datasize <number> \n\
   configure line <number> stopbits <number> \n\
   configure line <number> parity <string>   \n\
   configure line <number> socket <number>  \n\
   configure line <number> break <string> \n\
     - or -\n\
   configure line <number> tty <string> protocol <string>  pollinterval <number> authtype <string>  speed <number> datasize <number>  stopbits <number>  parity <string>  socket <number> break <string> \n\
   configure ether ip <string> \n\
   configure ether mask <string> \n\
   configure ether mtu <string> \n\
    - or - \n\
   configure ether ip <string> mask <string> mtu <string> \n\
   logout port <number> \n\
   clear line <number> \n\
   quit \n\
   write \n\
   show version \n\n";
#else
char *help_string = "\n\
SYNOPSIS \n\
   configure line <port number> [options1] \n\
   configure ether [options2] \n\
   configure conf [options3] \n\
"
#if CYCLADES_KVM
"  configure kvm conf [options4]  \n\
   configure kvm slave <slave name>[options5] \n\
   configure kvm server <port number> [options6]  \n\
   configure kvm user <username> [options7] \n\
"
#endif
"   clear line <port number> \n\
   exit \n\
   quit \n\
   write \n\
   help \n\
   show version \n\
OPTIONS 1 \n\
   accthost1 <string> \n\
   accthost2 <string> \n\
   adminusers <string> \n\
   alarm <number> \n\
   xml_alarm <number> \n\
   web_WinEMS <number> \n\
   translation <string> \n\
   authhost1 <string> \n\
   authhost2 <string> \n\
   authtype <string> \n\
   auto_input <string> \n\
   auto_output <string> \n\
   break <string> \n\
   databuffering <number> \n\
   datasize <number> \n\
   dbmenu <number> \n\
   dbmode <string> \n\
   dbtimestamp <number> \n\
   dcd <number> \n\
   dtr_reset <number> \n\
   escape <string> \n\
   flow <string> \n\
   host <string> \n\
   idletimeout <number> \n\
   ipno <string> \n\
   issue <string> \n\
   lf <number> \n\
   modbus <string> \n\
   multiplesess <string> \n\
   parity <string>   \n\
   pmkey <string> \n\
   pmnumofoutlets <number> \n\
   pmoutlet <string> \n\
   pmtype <string> \n\
   pmusers <string> \n\
   pollinterval <number> \n\
   prompt <string> \n\
   protocol <string> \n\
   retries <number> \n\
   secret <string> \n\
   alias <string> \n\
   pool_ipno <string> \n\
   pool_socket <number>  \n\
   pool_alias <string> \n\
   sniffmode <string> \n\
   socket <number>  \n\
   speed <number>    \n\
   stopbits <number> \n\
   sttycmd <string> \n\
   syslogdb <number> \n\
   syslogsess <number> \n\
   telnetclientmode <number> \n\
   term <string> \n\
   timeout <number> \n\
   tty <string> \n\
   txinterval <number> \n\
   userauto <string> \n\
   users <string> \n\
OPTIONS 2 \n\
   ip <string> \n\
   mask <string> \n\
   mtu <string> \n\
OPTIONS 3 \n\
   dbfacility <number> \n\
   facility <number> \n\
   group <string> \n\
   locallogins <number> \n\
   nfsdb <string> \n\
"
#if CYCLADES_KVM
"OPTIONS 4 \n\
   authtype <string> \n\
   authdownlocal <string> \n\
   idletimeout <number> \n\
   facility <number> \n\
   authhost1 <string> \n\
   authhost2 <string> \n\
   accthost1 <string> \n\
   accthost2 <string> \n\
   authtimeout <number> \n\
   authretries <number> \n\
   authsecret <string> \n\
   group <string> \n\
OPTIONS 5 \n\
   remote <number> \n\
   local <number> \n\
   nports <number> \n\
OPTIONS 6 \n\
   port <string> \n\
   pmoutlet <string> \n\ 
   alias <string> \n\
OPTIONS 7 \n\
   portaccess <string> <string> \n\
   escapeseq <string> \n\
   powerseq <string> \n\
   srvsync <string> \n\
   videoconf <string> \n\
   cycleseq <string> \n\
   cycletime <number> \n\
"
#endif
"EXAMPLES \n\
   configure line <number> tty <string> protocol <string>  pollinterval <number> authtype <string>... \n\
   configure ether ip <string> mask <string> mtu <string> \n\n";
#endif

static char* kstrncpy (
   char* d,
   char* s,
   int   n)
{
   if (d == NULL || s == NULL)
      return d;

   for ( ; *s && n; n--)
      *d++ = *s++;

   *d = '\0';

   return d;
}
/*
 * myhostname
 *
 * Return a pointer to a static array containing the fully-qualified
 * local host name.
 * Return "localhost" if the name can't be determined.
 */
static char* myhostname(void)
{
   static char buffer[MAXHOSTNAMELEN];
   int         status;
   int         start;

   if (buffer[0] == '\0')
   {
      status = gethostname (buffer, sizeof buffer);
      if (status == -1)
         BSTRCPY(buffer, "localhost");  /* Fall back and punt */
      else
      {
         buffer[sizeof buffer - 1] = '\0';
      }
   }
   return buffer;
}

/*
 * exit
 */

static CL_STATE exit_state =
{
   {CL_EOL, (void*) ExitCB, NULL, "End of line"},
   CL_STATE_END
};

/*
 * help
 */

static CL_STATE help_state =
{
   {CL_STRING, (void*) 0, help_state, "Command to get help on"},
   {CL_EOL, (void*) HelpCB, NULL, "End of line"},
   CL_STATE_END
};

static CL_STATE show_version_state =
{
   {CL_EOL, (void*) ShowVersionCB, NULL, "End of line"},
   CL_STATE_END
};

static CL_STATE show_state =
{
   {CL_KEYWORD, "version", show_version_state, "Show program version"},
   CL_STATE_END
};

static CL_STATE write_state =
{
   {CL_EOL, (void*) WriteCB, NULL, "End of line"},
   CL_STATE_END
};

static CL_STATE modify_port_state =
{
   {CL_STRING, (void*)0, modify_port_number, "Port Number"},
   CL_STATE_END
};

static CL_STATE modify_port_tty =
{
   {CL_STRING, (void*) 1, modify_port_number, "ttyS<port number>"},
   CL_STATE_END
};


static CL_STATE modify_port_protocol =
{
   {CL_STRING, (void*) 2, modify_port_number, "Port Protocol: socket_server, socket_ssh,..."},
   CL_STATE_END
};

static CL_STATE modify_port_pollinterval =
{
   {CL_NUMBER, (void*) 3, modify_port_number, "Poll Interval: Time in milliseconds"},
   CL_STATE_END
};

static CL_STATE modify_port_authtype =
{
   {CL_STRING, (void*) 4, modify_port_number, "Authentication Type: none, local, radius,..."},
   CL_STATE_END
};

static CL_STATE modify_port_speed =
{
   {CL_NUMBER, (void*) 5, modify_port_number, "Port Speed: 9600, 19200,..."},
   CL_STATE_END
};

static CL_STATE modify_port_datasize =
{
   {CL_NUMBER, (void*) 6, modify_port_number, "Port Data Size"},
   CL_STATE_END
};

static CL_STATE modify_port_stopbits =
{
   {CL_NUMBER, (void*) 7, modify_port_number, "Stop Bits"},
   CL_STATE_END
};

static CL_STATE modify_port_parity =
{
   {CL_STRING, (void*) 8, modify_port_number, "Parity: none, even, odd"},
   CL_STATE_END
};

static CL_STATE modify_port_socket =
{
   {CL_NUMBER, (void*) 9, modify_port_number, "Socket Number"},
   CL_STATE_END
};

static CL_STATE modify_port_break =
{
   {CL_STRING, (void*) 10, modify_port_number, "Break Sequence"},
   CL_STATE_END
};

static CL_STATE modify_port_flow =
{
   {CL_STRING, (void*) 11, modify_port_number, "Flow: none, hard, soft"},
   CL_STATE_END
};

static CL_STATE modify_port_dcd =
{
   {CL_NUMBER, (void*) 12, modify_port_number, "Dcd: 0, 1"},
   CL_STATE_END
};

static CL_STATE modify_port_dtr_reset =
{
   {CL_NUMBER, (void*) 13, modify_port_number, "DTR Reset"},
   CL_STATE_END
};

static CL_STATE modify_port_authhost1 =
{
   {CL_STRING, (void*) 14, modify_port_number, "Authentication Host 1: IP address"},
   CL_STATE_END
};

static CL_STATE modify_port_accthost1 =
{
   {CL_STRING, (void*) 15, modify_port_number, "Accounting Host 1: IP address"},
   CL_STATE_END
};

static CL_STATE modify_port_authhost2 =
{
   {CL_STRING, (void*) 16, modify_port_number, "Authentication Host 2: IP address"},
   CL_STATE_END
};

static CL_STATE modify_port_accthost2 =
{
   {CL_STRING, (void*) 17, modify_port_number, "Accounting Host 2: IP address"},
   CL_STATE_END
};

static CL_STATE modify_port_timeout =
{
   {CL_NUMBER, (void*) 18, modify_port_number, "Time Out: Time in seconds"},
   CL_STATE_END
};

static CL_STATE modify_port_retries =
{
   {CL_NUMBER, (void*) 19, modify_port_number, "Authenication Host Retries"},
   CL_STATE_END
};

static CL_STATE modify_port_secret =
{
   {CL_STRING, (void*) 20, modify_port_number, "Radius Shared Secret"},
   CL_STATE_END
};

static CL_STATE modify_port_line_feed =
{
   {CL_NUMBER, (void*) 21, modify_port_number, "Line Feed Suppression: 0, 1"},
   CL_STATE_END
};

static CL_STATE modify_port_auto_answer_input =
{
   {CL_STRING, (void*) 22, modify_port_number, "Auto Answer Input"},
   CL_STATE_END
};

static CL_STATE modify_port_auto_answer_output =
{
   {CL_STRING, (void*) 23, modify_port_number, "Auto Answer Output"},
   CL_STATE_END
};

static CL_STATE modify_port_alarm =
{
   {CL_NUMBER, (void*) 24, modify_port_number, "Alarm"},
   CL_STATE_END
};

static CL_STATE modify_port_databuffering =
{
   {CL_NUMBER, (void*) 25, modify_port_number, "Data Buffering"},
   CL_STATE_END
};

static CL_STATE modify_port_syslogdb =
{
   {CL_NUMBER, (void*) 26, modify_port_number, "Syslog Data Buffering"},
   CL_STATE_END
};

static CL_STATE modify_port_syslog_sess =
{
   {CL_NUMBER, (void*) 27, modify_port_number, "Syslog Session: 0, 1"},
   CL_STATE_END
};

static CL_STATE modify_port_dbmenu =
{
   {CL_NUMBER, (void*) 28, modify_port_number, "Data Buffering Menu: 0, 1, 2, 3"},
   CL_STATE_END
};

static CL_STATE modify_port_dbtimestamp =
{
   {CL_NUMBER, (void*) 29, modify_port_number, "Data Buffering TimeStamp: 0, 1"},
   CL_STATE_END
};

static CL_STATE modify_port_dbmode =
{
   {CL_STRING, (void*) 30, modify_port_number, "Data Buffering Mode: cir, lin"},
   CL_STATE_END
};

static CL_STATE modify_port_sniffmode =
{
   {CL_STRING, (void*) 31, modify_port_number, "Sniff Mode: in, out, i/o"},
   CL_STATE_END
};

static CL_STATE modify_port_escape_char =
{
   {CL_STRING, (void*) 32, modify_port_number, "Escape Char: ^a,...,^z"},
   CL_STATE_END
};

static CL_STATE modify_port_multiple_sess =
{
   {CL_STRING, (void*) 33, modify_port_number, "Multiple Session: yes, no"},
   CL_STATE_END
};

static CL_STATE modify_port_host =
{
   {CL_STRING, (void*) 34, modify_port_number, "Host: IP address"},
   CL_STATE_END
};

static CL_STATE modify_port_term =
{
   {CL_STRING, (void*) 35, modify_port_number, "Terminal Type"},
   CL_STATE_END
};

static CL_STATE modify_web_WinEMS =
{
   {CL_NUMBER, (void*) 56, modify_port_number, "web-WinEMS: Enable Windows EMS"},
   CL_STATE_END
};

static CL_STATE modify_xml_alarm =
{
   {CL_NUMBER, (void*) 57, modify_port_number, "XML-ALARM: Enable xml alarms for Windows EMS"},
   CL_STATE_END
};

static CL_STATE modify_translation =
{
   {CL_STRING, (void*) 58, modify_port_number, "translation: Enable F-KEY translation to terminal type vt-utf8"},
   CL_STATE_END
};

static CL_STATE modify_port_pool_ipno =
{
   {CL_STRING, (void*) 53, modify_port_number, "POOL_IPNO: IP address"},
   CL_STATE_END
};

static CL_STATE modify_port_pool_socket =
{
   {CL_NUMBER, (void*) 55, modify_port_number, "Serial port pool's TCP port number"},
   CL_STATE_END
};

static CL_STATE modify_port_pool_alias =
{
   {CL_STRING, (void*) 54, modify_port_number, "Serial port pool's Serverfarm"},
   CL_STATE_END
};

static CL_STATE modify_port_alias =
{
   {CL_STRING, (void*) 36, modify_port_number, "Serverfarm"},
   CL_STATE_END
};

static CL_STATE modify_port_txinterval =
{
   {CL_NUMBER, (void*) 37, modify_port_number, "Tx Interval: Time in milliseconds"},
   CL_STATE_END
};

static CL_STATE modify_port_idletimeout =
{
   {CL_NUMBER, (void*) 38, modify_port_number, "Idle Timeout: Time in minutes"},
   CL_STATE_END
};

static CL_STATE modify_port_ipno =
{
   {CL_STRING, (void*) 39, modify_port_number, "IPNO: IP address"},
   CL_STATE_END
};

static CL_STATE modify_port_modbus =
{
   {CL_STRING, (void*) 40, modify_port_number, "Modbus: ascii, rtu,..."},
   CL_STATE_END
};

static CL_STATE modify_port_userauto =
{
   {CL_STRING, (void*) 41, modify_port_number, "User Auto"},
   CL_STATE_END
};

static CL_STATE modify_port_users =
{
   {CL_STRING, (void*) 42, modify_port_number, "Port Users: List of names. Do not put space after commas."},
   CL_STATE_END
};

static CL_STATE modify_port_adminusers =
{
   {CL_STRING, (void*) 43, modify_port_number, "Admin Users: List of names. Do not put space after commas."},
   CL_STATE_END
};

static CL_STATE modify_port_sttycmd =
{
   {CL_STRING, (void*) 44, modify_port_number, "Stty Settings"},
   CL_STATE_END
};

static CL_STATE modify_port_issue =
{
   {CL_STRING, (void*) 45, modify_port_number, "Issue"},
   CL_STATE_END
};

static CL_STATE modify_port_prompt =
{
   {CL_STRING, (void*) 46, modify_port_number, "Terminal Prompt"},
   CL_STATE_END
};

static CL_STATE modify_port_telnetclientmode =
{
   {CL_NUMBER, (void*) 47, modify_port_number, "Telnet Client Mode: 0, 1"},
   CL_STATE_END
};

static CL_STATE modify_port_pmkey =
{
   {CL_STRING, (void*) 48, modify_port_number, "Power Management Key"},
   CL_STATE_END
};

static CL_STATE modify_port_pmNumOfOutlets =
{
   {CL_NUMBER, (void*) 49, modify_port_number, "Power Management Number of Outlets"},
   CL_STATE_END
};

static CL_STATE modify_port_pmoutlet =
{
   {CL_STRING, (void*) 50, modify_port_number, "Power Management Outlet"},
   CL_STATE_END
};

static CL_STATE modify_port_pmtype =
{
   {CL_STRING, (void*) 51, modify_port_number, "Power Management Type"},
   CL_STATE_END
};

static CL_STATE modify_port_pmusers =
{
   {CL_STRING, (void*) 52, modify_port_number, "Power Management Users"},
   CL_STATE_END
};

CL_STATE modify_port_number =
{
#ifdef CYCLADES_TS
   {CL_KEYWORD, "accthost1", modify_port_accthost1, "First Accounting Host", CL_NOREPEAT},
   {CL_KEYWORD, "accthost2", modify_port_accthost2, "Second Accounting Host", CL_NOREPEAT},
   {CL_KEYWORD, "adminusers", modify_port_adminusers, "Admin Users", CL_NOREPEAT},
   {CL_KEYWORD, "alarm", modify_port_alarm, "Syslog-ng Alarm", CL_NOREPEAT},
   {CL_KEYWORD, "authhost1", modify_port_authhost1, "First Authentication Host", CL_NOREPEAT},
   {CL_KEYWORD, "authhost2", modify_port_authhost2, "Second Authentication Host", CL_NOREPEAT},
   {CL_KEYWORD, "authtype", modify_port_authtype, "Authorization", CL_NOREPEAT},
   {CL_KEYWORD, "auto_input", modify_port_auto_answer_input, "Auto Answer Input", CL_NOREPEAT},
   {CL_KEYWORD, "auto_output", modify_port_auto_answer_output, "Auto Answer Output", CL_NOREPEAT},
   {CL_KEYWORD, "break", modify_port_break, "Break Sequence", CL_NOREPEAT},
   {CL_KEYWORD, "databuffering", modify_port_databuffering, "Data Buffering", CL_NOREPEAT},
   {CL_KEYWORD, "datasize", modify_port_datasize, "Data Size", CL_NOREPEAT},
   {CL_KEYWORD, "dbmenu", modify_port_dbmenu, "Data Buffering Menu", CL_NOREPEAT},
   {CL_KEYWORD, "dbmode", modify_port_dbmode, "Data Buffering Mode", CL_NOREPEAT},
   {CL_KEYWORD, "dbtimestamp", modify_port_dbtimestamp, "Data Buffering Time Stamp", CL_NOREPEAT},
   {CL_KEYWORD, "dcd", modify_port_dcd, "Dcd", CL_NOREPEAT},
   {CL_KEYWORD, "dtr_reset", modify_port_dtr_reset, "DTR_reset", CL_NOREPEAT},
   {CL_KEYWORD, "escape", modify_port_escape_char, "Escape Character", CL_NOREPEAT},
	{CL_KEYWORD, "flow", modify_port_flow, "Flow", CL_NOREPEAT},
   {CL_KEYWORD, "host", modify_port_host, "Host", CL_NOREPEAT},
   {CL_KEYWORD, "idletimeout", modify_port_idletimeout, "Idle Time Out", CL_NOREPEAT},
   {CL_KEYWORD, "ipno", modify_port_ipno, "IP Address of Serial Port", CL_NOREPEAT},
   {CL_KEYWORD, "issue", modify_port_issue, "Terminal Header", CL_NOREPEAT},
   {CL_KEYWORD, "lf", modify_port_line_feed, "Line Feed Suppression", CL_NOREPEAT},
   {CL_KEYWORD, "modbus", modify_port_modbus, "Modbus Smode", CL_NOREPEAT},
   {CL_KEYWORD, "multiplesess", modify_port_multiple_sess, "Multiple Sessions", CL_NOREPEAT},
   {CL_KEYWORD, "parity", modify_port_parity, "Parity", CL_NOREPEAT},
   {CL_KEYWORD, "pmkey", modify_port_pmkey, "Power Management Key", CL_NOREPEAT},
   {CL_KEYWORD, "pmnumofoutlets", modify_port_pmNumOfOutlets, "Power Management Number of Outlets", CL_NOREPEAT},
   {CL_KEYWORD, "pmoutlet", modify_port_pmoutlet, "Power Management Outlet", CL_NOREPEAT},
   {CL_KEYWORD, "pmtype", modify_port_pmtype, "Power Management Type", CL_NOREPEAT},
   {CL_KEYWORD, "pmusers", modify_port_pmusers, "Power Management Users", CL_NOREPEAT},
   {CL_KEYWORD, "pollinterval", modify_port_pollinterval, "Interval", CL_NOREPEAT},
   {CL_KEYWORD, "prompt", modify_port_prompt, "Terminal Prompt", CL_NOREPEAT},
   {CL_KEYWORD, "protocol", modify_port_protocol, "Protocol", CL_NOREPEAT},
   {CL_KEYWORD, "retries", modify_port_retries, "Number of Tries to Connect to the Authentication Host", CL_NOREPEAT},
   {CL_KEYWORD, "secret", modify_port_secret, "Shared Secret used by Radius", CL_NOREPEAT},
   {CL_KEYWORD, "alias", modify_port_alias, "Alias for Server", CL_NOREPEAT},
   {CL_KEYWORD, "web_WinEMS ", modify_web_WinEMS, "Enable Windows EMS", CL_NOREPEAT},
   {CL_KEYWORD, "xml_alarm ", modify_xml_alarm, "Enable xml alarms for Windows EMS", CL_NOREPEAT},
   {CL_KEYWORD, "translation ", modify_translation, "Enable F-KEY translation to terminal vt-utf8", CL_NOREPEAT},
   {CL_KEYWORD, "pool_alias", modify_port_pool_alias, "Alias for Serial port pool", CL_NOREPEAT},
   {CL_KEYWORD, "pool_ipno", modify_port_pool_ipno, "IP address for Serial port pool", CL_NOREPEAT},
   {CL_KEYWORD, "pool_socket_port", modify_port_pool_socket, "TCP port number for Serial port pool", CL_NOREPEAT},
   {CL_KEYWORD, "sniffmode", modify_port_sniffmode, "Sniff Mode", CL_NOREPEAT},
   {CL_KEYWORD, "socket", modify_port_socket, "Socket Number", CL_NOREPEAT},
   {CL_KEYWORD, "speed", modify_port_speed, "Speed", CL_NOREPEAT},
   {CL_KEYWORD, "stopbits", modify_port_stopbits, "Stop Bits", CL_NOREPEAT},
   {CL_KEYWORD, "sttycmd", modify_port_sttycmd, "Stty Settings", CL_NOREPEAT},
   {CL_KEYWORD, "syslogdb", modify_port_syslogdb, "Syslog Buffering", CL_NOREPEAT},
   {CL_KEYWORD, "syslogsess", modify_port_syslog_sess, "Syslog Session", CL_NOREPEAT},
   {CL_KEYWORD, "telnetclientmode", modify_port_telnetclientmode, "Telnet Client Mode", CL_NOREPEAT},
   {CL_KEYWORD, "term", modify_port_term, "Terminal Type", CL_NOREPEAT},
   {CL_KEYWORD, "timeout", modify_port_timeout, "Radius Time Out per Query in Seconds", CL_NOREPEAT},
   {CL_KEYWORD, "tty", modify_port_tty, "TTY name", CL_NOREPEAT},
   {CL_KEYWORD, "txinterval", modify_port_txinterval, "Transmission Interval", CL_NOREPEAT},
   {CL_KEYWORD, "userauto", modify_port_userauto, "User Auto", CL_NOREPEAT},
   {CL_KEYWORD, "users", modify_port_users, "Users", CL_NOREPEAT},
#else
   {CL_KEYWORD, "speed", modify_port_speed, "Speed", CL_NOREPEAT},
   {CL_KEYWORD, "datasize", modify_port_datasize, "Data Size", CL_NOREPEAT},
   {CL_KEYWORD, "stopbits", modify_port_stopbits, "Stop Bits", CL_NOREPEAT},
   {CL_KEYWORD, "parity", modify_port_parity, "Parity", CL_NOREPEAT},
   {CL_KEYWORD, "socket", modify_port_socket, "Socket Number", CL_NOREPEAT},
#endif
   {CL_EOL, (void*) ModifyPortCB, NULL, "End of line"},
   CL_STATE_END
};

#ifdef CYCLADES_TS
static CL_STATE modify_state =
{
   {CL_KEYWORD, "port", modify_port_state, "Modify port parameters"},
   CL_STATE_END
};
#endif


CL_STATE config_line_number =
{
#ifdef CYCLADES_TS
   {CL_KEYWORD, "accthost1", modify_port_accthost1, "First Accounting Host", CL_NOREPEAT},
   {CL_KEYWORD, "accthost2", modify_port_accthost2, "Second Accounting Host", CL_NOREPEAT},
   {CL_KEYWORD, "adminusers", modify_port_adminusers, "Admin Users", CL_NOREPEAT},
   {CL_KEYWORD, "alarm", modify_port_alarm, "Syslog-ng Alarm", CL_NOREPEAT},
   {CL_KEYWORD, "authhost1", modify_port_authhost1, "First Authentication Host", CL_NOREPEAT},
   {CL_KEYWORD, "authhost2", modify_port_authhost2, "Second Authentication Host", CL_NOREPEAT},
   {CL_KEYWORD, "authtype", modify_port_authtype, "Authorization", CL_NOREPEAT},
   {CL_KEYWORD, "auto_input", modify_port_auto_answer_input, "Auto Answer Input", CL_NOREPEAT},
   {CL_KEYWORD, "auto_output", modify_port_auto_answer_output, "Auto Answer Output", CL_NOREPEAT},
   {CL_KEYWORD, "break", modify_port_break, "Break Sequence", CL_NOREPEAT},
   {CL_KEYWORD, "databuffering", modify_port_databuffering, "Data Buffering", CL_NOREPEAT},
   {CL_KEYWORD, "datasize", modify_port_datasize, "Data Size", CL_NOREPEAT},
   {CL_KEYWORD, "dbmenu", modify_port_dbmenu, "Data Buffering Menu", CL_NOREPEAT},
   {CL_KEYWORD, "dbmode", modify_port_dbmode, "Data Buffering Mode", CL_NOREPEAT},
   {CL_KEYWORD, "dbtimestamp", modify_port_dbtimestamp, "Data Buffering Time Stamp", CL_NOREPEAT},
   {CL_KEYWORD, "dcd", modify_port_dcd, "Dcd", CL_NOREPEAT},
   {CL_KEYWORD, "dtr_reset", modify_port_dtr_reset, "DTR_reset", CL_NOREPEAT},
   {CL_KEYWORD, "escape", modify_port_escape_char, "Escape Character", CL_NOREPEAT},
	{CL_KEYWORD, "flow", modify_port_flow, "Flow", CL_NOREPEAT},
   {CL_KEYWORD, "host", modify_port_host, "Host", CL_NOREPEAT},
   {CL_KEYWORD, "idletimeout", modify_port_idletimeout, "Idle Time Out", CL_NOREPEAT},
   {CL_KEYWORD, "ipno", modify_port_ipno, "IP Address of Serial Port", CL_NOREPEAT},
   {CL_KEYWORD, "issue", modify_port_issue, "Terminal Header", CL_NOREPEAT},
   {CL_KEYWORD, "lf", modify_port_line_feed, "Line Feed Suppression", CL_NOREPEAT},
   {CL_KEYWORD, "modbus", modify_port_modbus, "Modbus Smode", CL_NOREPEAT},
   {CL_KEYWORD, "multiplesess", modify_port_multiple_sess, "Multiple Sessions", CL_NOREPEAT},
   {CL_KEYWORD, "parity", modify_port_parity, "Parity", CL_NOREPEAT},
   {CL_KEYWORD, "pmkey", modify_port_pmkey, "Power Management Key", CL_NOREPEAT},
   {CL_KEYWORD, "pmnumofoutlets", modify_port_pmNumOfOutlets, "Power Management Number of Outlets", CL_NOREPEAT},
   {CL_KEYWORD, "pmoutlet", modify_port_pmoutlet, "Power Management Outlet", CL_NOREPEAT},
   {CL_KEYWORD, "pmtype", modify_port_pmtype, "Power Management Type", CL_NOREPEAT},
   {CL_KEYWORD, "pmusers", modify_port_pmusers, "Power Management Users", CL_NOREPEAT},
   {CL_KEYWORD, "pollinterval", modify_port_pollinterval, "Interval", CL_NOREPEAT},
   {CL_KEYWORD, "prompt", modify_port_prompt, "Terminal Prompt", CL_NOREPEAT},
   {CL_KEYWORD, "protocol", modify_port_protocol, "Protocol", CL_NOREPEAT},
   {CL_KEYWORD, "retries", modify_port_retries, "Number of Tries to Connect to the Authentication Host", CL_NOREPEAT},
   {CL_KEYWORD, "secret", modify_port_secret, "Shared Secret used by Radius", CL_NOREPEAT},
   {CL_KEYWORD, "alias", modify_port_alias, "Alias for Server", CL_NOREPEAT},
   {CL_KEYWORD, "web_WinEMS ", modify_web_WinEMS, "Enable Windows EMS", CL_NOREPEAT},
   {CL_KEYWORD, "xml_alarm ", modify_xml_alarm, "Enable xml alarms for Windows EMS", CL_NOREPEAT},
   {CL_KEYWORD, "translation ", modify_translation, "Enable F-KEY translation to terminal vt-utf8", CL_NOREPEAT},
   {CL_KEYWORD, "pool_alias", modify_port_pool_alias, "Alias for Serial port pool", CL_NOREPEAT},
   {CL_KEYWORD, "pool_ipno", modify_port_pool_ipno, "IP address for Serial port pool", CL_NOREPEAT},
   {CL_KEYWORD, "pool_socket_port", modify_port_pool_socket, "TCP port number for Serial port pool", CL_NOREPEAT},
   {CL_KEYWORD, "sniffmode", modify_port_sniffmode, "Sniff Mode", CL_NOREPEAT},
   {CL_KEYWORD, "socket", modify_port_socket, "Socket Number", CL_NOREPEAT},
   {CL_KEYWORD, "speed", modify_port_speed, "Speed", CL_NOREPEAT},
   {CL_KEYWORD, "stopbits", modify_port_stopbits, "Stop Bits", CL_NOREPEAT},
   {CL_KEYWORD, "sttycmd", modify_port_sttycmd, "Stty Settings", CL_NOREPEAT},
   {CL_KEYWORD, "syslogdb", modify_port_syslogdb, "Syslog Buffering", CL_NOREPEAT},
   {CL_KEYWORD, "syslogsess", modify_port_syslog_sess, "Syslog Session", CL_NOREPEAT},
   {CL_KEYWORD, "telnetclientmode", modify_port_telnetclientmode, "Telnet Client Mode", CL_NOREPEAT},
   {CL_KEYWORD, "term", modify_port_term, "Terminal Type", CL_NOREPEAT},
   {CL_KEYWORD, "timeout", modify_port_timeout, "Radius Time Out per Query in Seconds", CL_NOREPEAT},
   {CL_KEYWORD, "tty", modify_port_tty, "TTY name", CL_NOREPEAT},
   {CL_KEYWORD, "txinterval", modify_port_txinterval, "Transmission Interval", CL_NOREPEAT},
   {CL_KEYWORD, "userauto", modify_port_userauto, "User Auto", CL_NOREPEAT},
   {CL_KEYWORD, "users", modify_port_users, "Users", CL_NOREPEAT},
#else
   {CL_KEYWORD, "speed", modify_port_speed, "Speed", CL_NOREPEAT},
   {CL_KEYWORD, "datasize", modify_port_datasize, "Data Size", CL_NOREPEAT},
   {CL_KEYWORD, "stopbits", modify_port_stopbits, "Stop Bits", CL_NOREPEAT},
   {CL_KEYWORD, "parity", modify_port_parity, "Parity", CL_NOREPEAT},
   {CL_KEYWORD, "socket", modify_port_socket, "Socket Number", CL_NOREPEAT},
#endif
   {CL_EOL, (void*) ModifyPortCB, NULL, "End of line"},
   CL_STATE_END
};

static CL_STATE config_line_state =
{
   {CL_STRING, (void*)0, config_line_number, "Line Number"},
   CL_STATE_END
};

#ifdef CYCLADES_TS

static CL_STATE config_eth_ip_state =
{
   {CL_STRING, (void*) 1, config_ether_state, "IP address"},
   CL_STATE_END
};

static CL_STATE config_eth_mask_state =
{
   {CL_STRING, (void*) 2, config_ether_state, "IP mask"},
   CL_STATE_END
};

static CL_STATE config_eth_mtu_state =
{
   {CL_NUMBER, (void*) 3, config_ether_state, "Ether MTU"},
   CL_STATE_END
};

static CL_STATE config_ether_state =
{
   {CL_KEYWORD, "ip", config_eth_ip_state, "Configure ip address"},
   {CL_KEYWORD, "mask", config_eth_mask_state, "Configure bit mask"},
   {CL_KEYWORD, "mtu", config_eth_mtu_state, "Configure ethernet mtu"},
   {CL_EOL, (void*) ConfigureEtherCB, NULL, "End of line"},
   CL_STATE_END
};

static CL_STATE modify_conf_locallogins =
{
   {CL_NUMBER, (void*) 1, config_conf_state, "Local Logins: 0, 1"},
   CL_STATE_END
};

static CL_STATE modify_conf_nfs_databuffering =
{
   {CL_STRING, (void*) 2, config_conf_state, "NFS Data Buffering"},
   CL_STATE_END
};

static CL_STATE modify_conf_facility =
{
   {CL_NUMBER, (void*) 3, config_conf_state, "Syslog Facility: 1 - 8"},
   CL_STATE_END
};

static CL_STATE modify_conf_dbfacility =
{
   {CL_NUMBER, (void*) 4, config_conf_state, "Data Buffering Facility: 0 - 7"},
   CL_STATE_END
};

static CL_STATE modify_conf_group =
{
   {CL_STRING, (void*) 5, config_conf_state, "Group Users: [group:user1,user2,...]"},
   CL_STATE_END
};

static CL_STATE config_conf_state =
{
   {CL_KEYWORD, "dbfacility", modify_conf_dbfacility, "Data Buffering Facility"},
   {CL_KEYWORD, "facility", modify_conf_facility, "Syslog Facility"},
   {CL_KEYWORD, "group", modify_conf_group, "Group"},
   {CL_KEYWORD, "locallogins", modify_conf_locallogins, "Local Logins"},
   {CL_KEYWORD, "nfsdb", modify_conf_nfs_databuffering, "NFS Data Buffering"},
   {CL_EOL, (void*) ModifyConfCB, NULL, "End of line"},
   CL_STATE_END
};

#ifdef CYCLADES_KVM
static CL_STATE modify_kvm_authtype =
{
   {CL_STRING, (void*) CLI_KVM_AUTHTYPE, config_kvmconf_state, "Authentication type: none, local, radius..."},
   CL_STATE_END
};

static CL_STATE modify_kvm_authdownlocal =
{
   {CL_STRING, (void*) CLI_KVM_AUTHDOWNLOCAL, config_kvmconf_state, "Authentication down to local (yes or no)."},
   CL_STATE_END
};

static CL_STATE modify_kvm_idletimeout =
{
   {CL_NUMBER, (void*) CLI_KVM_IDLETIMEOUT, config_kvmconf_state, "Idle timeout, in seconds."},
   CL_STATE_END
};

static CL_STATE modify_kvm_facility =
{
   {CL_NUMBER, (void*) CLI_KVM_FACILITY, config_kvmconf_state, "Syslog facility: 0 to 7."},
   CL_STATE_END
};

static CL_STATE modify_kvm_authhost1 =
{
   {CL_STRING, (void*) CLI_KVM_AUTHHOST1, config_kvmconf_state, "Authentication server 1 (IP address or name)"},
   CL_STATE_END
};

static CL_STATE modify_kvm_authhost2 =
{
   {CL_STRING, (void*) CLI_KVM_AUTHHOST2, config_kvmconf_state, "Authentication server 2 (IP address or name)"},
   CL_STATE_END
};

static CL_STATE modify_kvm_accthost1 =
{
   {CL_STRING, (void*) CLI_KVM_ACCTHOST1, config_kvmconf_state, "Acconting server 1 (IP address or name)"},
   CL_STATE_END
};

static CL_STATE modify_kvm_accthost2 =
{
   {CL_STRING, (void*) CLI_KVM_ACCTHOST2, config_kvmconf_state, "Accounting server 2 (IP address or name)"},
   CL_STATE_END
};

static CL_STATE modify_kvm_authtimeout =
{
   {CL_NUMBER, (void*) CLI_KVM_AUTHTIMEOUT, config_kvmconf_state, "Authentication timeout (in minutes)"},
   CL_STATE_END
};

static CL_STATE modify_kvm_authretries =
{
   {CL_NUMBER, (void*) CLI_KVM_AUTHRETRIES, config_kvmconf_state, "Authentication number of retries"},
   CL_STATE_END
};

static CL_STATE modify_kvm_authsecret =
{
   {CL_STRING, (void*) CLI_KVM_AUTHSECRET, config_kvmconf_state, "Authentication secret"},
   CL_STATE_END
};

//static CL_STATE modify_kvm_group =
//{
//   {CL_STRING, (void*) CLI_KVM_GROUP, config_kvmconf_state, "Port Groups"},
//   CL_STATE_END
//};

static CL_STATE modify_kvm_kbdtype =
{
   {CL_STRING, (void*) CLI_KVM_KBDTYPE, config_kvmconf_state, "Keyboard type"},
   CL_STATE_END
};

static CL_STATE config_kvmconf_state =
{
   {CL_KEYWORD, "authtype", modify_kvm_authtype, "KVM authentication type"},
  // {CL_KEYWORD, "authdownlocal", modify_kvm_authdownlocal, "KVM authentication down to local"},
   {CL_KEYWORD, "idletimeout", modify_kvm_idletimeout, "KVM idle timeout"},
   {CL_KEYWORD, "facility", modify_kvm_facility, "KVM syslog facility"},
/*
   {CL_KEYWORD, "authhost1", modify_kvm_authhost1, "KVM first authentication server"},
   {CL_KEYWORD, "authhost2", modify_kvm_authhost2, "KVM second authentication server"},
   {CL_KEYWORD, "accthost1", modify_kvm_accthost1, "KVM first accounting server"},
   {CL_KEYWORD, "accthost2", modify_kvm_accthost2, "KVM second accounting server"},
   {CL_KEYWORD, "authtimeout", modify_kvm_authtimeout, "KVM authentication timeout"},
   {CL_KEYWORD, "authretries", modify_kvm_authretries, "KVM authentication number of retries"},
   {CL_KEYWORD, "authsecret", modify_kvm_authsecret, "KVM authentication secret"},
*/
//   {CL_KEYWORD, "group", modify_kvm_group, "KVM port groups"},
   {CL_KEYWORD, "kbdtype", modify_kvm_kbdtype, "KVM keyboard type"},
   {CL_EOL, (void*) ModifyKVMConfCB, NULL, "End of line"},
   CL_STATE_END
};

static CL_STATE modify_kvm_port =
{
   {CL_STRING, (void*) (CLI_KVM_PORT + 1), config_kvmport_number, "Physical port: <slave name>[.<port>]"},
   CL_STATE_END
};

static CL_STATE modify_kvm_pmoutlet =
{   {CL_STRING, (void*) (CLI_KVM_PMOUTLET + 1), config_kvmport_number, "Outlet number(s) : [<slave name>.]<outlet>"},
   CL_STATE_END
};

static CL_STATE modify_kvm_alias =
{   {CL_STRING, (void*) (CLI_KVM_ALIAS + 1), config_kvmport_number, "Server name "},
   CL_STATE_END
};

static CL_STATE config_kvmport_number =
{
   {CL_KEYWORD, "port", modify_kvm_port, "KVM server physical port"},
   {CL_KEYWORD, "pmoutlet", modify_kvm_pmoutlet, "KVM server outlet(s)"},
   {CL_KEYWORD, "alias", modify_kvm_alias, "KVM server name"},
   {CL_EOL, (void*) ModifyKVMPortCB, NULL, "End of line"},
   CL_STATE_END
};

static CL_STATE config_kvmport_state =
{
   {CL_NUMBER, (void *)0, config_kvmport_number, "KVM server number"},
   CL_STATE_END
};

static CL_STATE modify_kvmuser_access =
{
   {CL_STRING, (void *)CLI_KVM_ACCESS_PERM, config_kvmuser_name, "Permissions for the KVM port: ro/rw[|cfg][pw]"},
   CL_STATE_END
};

static CL_STATE config_kvmuser_access =
{
   {CL_STRING, (void *)CLI_KVM_ACCESS_PORT, modify_kvmuser_access, "KVM server number"},
   CL_STATE_END
};

static CL_STATE modify_kvmuser_escapeseq =
{
   {CL_STRING, (void *)CLI_KVM_ESCAPESEQ, config_kvmuser_name, "Escape key sequence (Ctrl=^)"},
   CL_STATE_END
};

static CL_STATE modify_kvmuser_powerseq =
{
   {CL_STRING, (void *)CLI_KVM_POWERSEQ, config_kvmuser_name, "Power Management key sequence (Ctrl=^)"},
   CL_STATE_END
};

//static CL_STATE modify_kvmuser_rpswitchseq =
//{
//   {CL_STRING, (void *)CLI_KVM_RPSWITCHSEQ, config_kvmuser_name, "RP switch key sequence (Ctrl=^)"},
//   CL_STATE_END
//};
//
static CL_STATE modify_kvmuser_srvsync =
{
   {CL_STRING, (void *)CLI_KVM_SRVSYNCSEQ, config_kvmuser_name, "Mouse/Keyb. synchronization key sequence (Ctrl=^)"},
   CL_STATE_END
};

static CL_STATE modify_kvmuser_videoconf =
{
   {CL_STRING, (void *)CLI_KVM_VIDEOCONFSEQ, config_kvmuser_name, "Video configuration key sequence (Ctrl=^)"},
   CL_STATE_END
};
//
static CL_STATE modify_kvmuser_cycleseq =
{
   {CL_STRING, (void *)CLI_KVM_SWITCHNEXTSEQ, config_kvmuser_name, "Cycle key sequence (Ctrl=^)"},
   CL_STATE_END
};

static CL_STATE modify_kvmuser_cycletime =
{
   {CL_NUMBER, (void *)CLI_KVM_CYCLETIME, config_kvmuser_name, "Cycle time (in seconds)"},
   CL_STATE_END
};

static CL_STATE config_kvmuser_name =
{
   {CL_KEYWORD, "portaccess", config_kvmuser_access, "KVM access permissions"},
   {CL_KEYWORD, "escapeseq", modify_kvmuser_escapeseq, "KVM escape sequence"},
   {CL_KEYWORD, "powerseq", modify_kvmuser_powerseq, "KVM power management sequence"},
//   {CL_KEYWORD, "rpswitchseq", modify_kvmuser_rpswitchseq, "KVM RP switch sequence"},
   {CL_KEYWORD, "srvsync", modify_kvmuser_srvsync, "KVM mouse/keyb. synchronization sequence"},
   {CL_KEYWORD, "videoconf", modify_kvmuser_videoconf, "KVM video configuration sequence"},
   {CL_KEYWORD, "cycleseq", modify_kvmuser_cycleseq, "KVM cycle sequence"},
   {CL_KEYWORD, "cycletime", modify_kvmuser_cycletime, "KVM cycle time"},
   {CL_EOL, (void*) ModifyKVMUserCB, NULL, "End of line"},
   CL_STATE_END
};

static CL_STATE config_kvmuser_state =
{
   {CL_STRING, (void *)CLI_KVM_USERNAME, config_kvmuser_name, "KVM user name"},
   CL_STATE_END
};

static CL_STATE modify_kvmslave_remote =
{
   {CL_NUMBER, (void *)CLI_KVM_SLAVEREMOTE, config_kvmslave_name, "Port number"},
   CL_STATE_END
};

static CL_STATE modify_kvmslave_local =
{
   {CL_NUMBER, (void *)CLI_KVM_SLAVELOCAL, config_kvmslave_name, "Port number"},
   CL_STATE_END
};

static CL_STATE modify_kvmslave_nports =
{
   {CL_NUMBER, (void *)CLI_KVM_SLAVENPORTS, config_kvmslave_name, "Number of servers"},
   CL_STATE_END
};

static CL_STATE config_kvmslave_name =
{
   {CL_KEYWORD, "remote", modify_kvmslave_remote, "Port connected to the remote station"},
   {CL_KEYWORD, "local", modify_kvmslave_local, "Port connected to the local station"},
   {CL_KEYWORD, "nports", modify_kvmslave_nports, "Number of servers to be configured for this slave"},
   {CL_EOL, (void *)ModifyKVMSlaveCB, NULL, "End of line"},
   CL_STATE_END
};

static CL_STATE config_kvmslave_state =
{
   {CL_STRING, (void *)CLI_KVM_SLAVENAME, config_kvmslave_name, "Slave name"},
   CL_STATE_END
};

static CL_STATE config_kvm_state =
{
   {CL_KEYWORD, "conf", config_kvmconf_state, "Configure KVM conf parameters"},
   {CL_KEYWORD, "slave", config_kvmslave_state, "Configure KVM slave connections"},
   {CL_KEYWORD, "server", config_kvmport_state, "Configure KVM server parameters"},
   {CL_KEYWORD, "user", config_kvmuser_state, "Configure KVM user profiles"},
   CL_STATE_END
};
#endif
#endif

static CL_STATE configure_state =
{
   {CL_KEYWORD, "line", config_line_state, "Configure line parameters"},
#ifdef CYCLADES_TS
   {CL_KEYWORD, "ether", config_ether_state, "Configure ethernet parameters"},
	{CL_KEYWORD, "conf", config_conf_state, "Configure conf parameters"},
#ifdef CYCLADES_KVM
	{CL_KEYWORD, "kvm", config_kvm_state, "Configure KVM parameters"},
#endif
#endif
   CL_STATE_END
};

static CL_STATE logout_port_number =
{
   {CL_EOL, (void*) LogoutPortCB, NULL, "End of line"},
   CL_STATE_END
};

#ifndef CYCLADES_TS
static CL_STATE logout_port_state =
{
   {CL_STRING, (void*)0, logout_port_number, "Terminal server port number"},
   CL_STATE_END
};

static CL_STATE logout_state =
{
   {CL_KEYWORD, "port", logout_port_state, "Logout a port"},
   CL_STATE_END
};
#endif

static CL_STATE clear_line_state =
{
   {CL_STRING, (void*)0, logout_port_number, "Terminal server line number"},
   CL_STATE_END
};

static CL_STATE clear_state =
{
   {CL_KEYWORD, "line", clear_line_state, "Clearing/Resetting a line"},
   CL_STATE_END
};


static CL_STATE begin_state =
{
   {CL_KEYWORD, "clear", clear_state, "Clearing a line"},
   {CL_KEYWORD, "configure", configure_state, "Configure parameters"},
   {CL_KEYWORD, "exit", exit_state, "Exit"},
   {CL_KEYWORD, "help", help_state, "Display program help"},
#ifndef CYCLADES_TS
   {CL_KEYWORD, "logout", logout_state, "Log out a port"},
   {CL_KEYWORD, "modify", modify_state, "Modify characteristics"},
#endif
   {CL_KEYWORD, "quit", exit_state, "Exit"},
   {CL_KEYWORD, "show", show_state, "Show characteristics"},
   {CL_KEYWORD, "write", write_state, "Apply changes and write to flash"},
   CL_STATE_END
};

/*******************
* External Modules *
*******************/
extern void find_kinet_root(
   char* argv[]);

extern char* build_kinet_path(
   char* dir,
   char* file);

extern int ki_isroot(void);

extern void getuserinfo(
   char* user,
   int   user_len,
   char* host,
   int   host_len);

extern char* kstrchr(
   char* s,
   char  c);

extern char* kstrsave(
   char* s);

extern void kstrfree(
   char* s);

extern char* kstrncpy (
   char* d,
   char* s,
   int   n);

typedef struct _config_template {
   char *token;
   char *command;
} CONFIG_TEMPL;

CONFIG_TEMPL  ttytempl[] = 
{
   {"",                  ""}, /* nothing for the case '0' in ModifyPortCB */
   {"s%d.tty",						"s%d.tty ttyS%d"},
   {"s%d.protocol",				"s%d.protocol %s"},
   {"s%d.poll_interval",		"s%d.poll_interval %d"},
   {"s%d.authtype",				"s%d.authtype %s"},
   {"s%d.speed",					"s%d.speed %d"},
   {"s%d.datasize",				"s%d.datasize %d"},
   {"s%d.stopbits",				"s%d.stopbits %d"},
   {"s%d.parity",					"s%d.parity %s"},
   {"s%d.socket_port",			"s%d.socket_port %d"},
   {"s%d.break_sequence",		"s%d.break_sequence %s"},
	{"s%d.flow",               "s%d.flow %s"},
   {"s%d.dcd",                "s%d.dcd %d"},
   {"s%d.DTR_reset",          "s%d.DTR_reset %d"},
   {"s%d.authhost1",          "s%d.authhost1 %s"},
   {"s%d.accthost1",          "s%d.accthost1 %s"},
   {"s%d.authhost2",          "s%d.authhost2 %s"},
   {"s%d.accthost2",          "s%d.accthost2 %s"},
   {"s%d.radtimeout",         "s%d.radtimeout %d"},
   {"s%d.radretries",         "s%d.radretries %d"},
   {"s%d.secret",             "s%d.secret %s"},
   {"s%d.lf_suppress",        "s%d.lf_suppress %d"},
   {"s%d.auto_answer_input",  "s%d.auto_answer_input %s"},
   {"s%d.auto_answer_output", "s%d.auto_answer_output %s"},
   {"s%d.alarm",              "s%d.alarm %d"},
   {"s%d.data_buffering",     "s%d.data_buffering %d"},
   {"s%d.syslog_buffering",   "s%d.syslog_buffering %d"},
   {"s%d.syslog_sess",        "s%d.syslog_sess %d"},
   {"s%d.dont_show_DBmenu",   "s%d.dont_show_DBmenu %d"},
   {"s%d.DB_timestamp",       "s%d.DB_timestamp %d"},
   {"s%d.DB_mode",            "s%d.DB_mode %s"},
   {"s%d.sniff_mode",         "s%d.sniff_mode %s"},
   {"s%d.escape_char",        "s%d.escape_char %s"},
   {"s%d.multiple_sessions",  "s%d.multiple_sessions %s"},
   {"s%d.host",               "s%d.host %s"},
   {"s%d.term",               "s%d.term %s"},
   {"s%d.alias",         "s%d.alias %s"},
   {"s%d.tx_interval",        "s%d.tx_interval %d"},
   {"s%d.idletimeout",        "s%d.idletimeout %d"},
   {"s%d.ipno",               "s%d.ipno %s"},
   {"s%d.modbus_smode",       "s%d.modbus_smode %s"},
   {"s%d.userauto",           "s%d.userauto %s"},
   {"s%d.users",              "s%d.users %s"},
   {"s%d.admin_users",        "s%d.admin_users %s"},
   {"s%d.sttyCmd",            "s%d.sttyCmd %s"},
   {"s%d.issue",           	"s%d.issue %s"},
   {"s%d.prompt",          	"s%d.prompt %s"},
   {"s%d.telnet_client_mode", "s%d.telnet_client_mode %d"},
   {"s%d.pmkey", 					"s%d.pmkey %s"},
   {"s%d.pmNumOfOutlets", 		"s%d.pmNumOfOutlets %d"},
   {"s%d.pmoutlet", 				"s%d.pmoutlet %s"},
   {"s%d.pmtype", 				"s%d.pmtype %s"},
   {"s%d.pmusers", 				"s%d.pmusers %s"},
   {"s%d.pool_ipno",               "s%d.pool_ipno %s"},
   {"s%d.pool_alias",         "s%d.pool_alias %s"},
   {"s%d.pool_socket_port",			"s%d.pool_socket_port %d"},
   {"s%d.web_WinEMS",			"s%d.web_WinEMS %d"},
   {"s%d.xml_alarm",			"s%d.xml_alarm %d"},
   {"s%d.translation",			"s%d.translation %s"},
   {NULL, NULL}
};


/*
 * main
 *
 * Main program
 */
int main(
   int   argc,
   char* argv[])
{
   int     status;
   char    prompt[MAXHOSTNAMELEN + 8];

   strcpy(host, myhostname());

   exit_status = 0;
   if (argc > 1)
   {
      argparse(argc - 1, &argv[1],  begin_state, FALSE);
   }
   else
   {
      if (strstr(argv[0], "configts"))
         sprintf(prompt, "configts@%s%s", host, ">>");
      else
         sprintf(prompt, "config@%s%s", host, ">>");

      clexec(512, prompt, ">", begin_state);
   }

   exit(exit_status);
}


static void HelpCB(
   unsigned int nargs,
   CL_ARG*      args)
{
   printf("%s", help_string);
}


static void ShowVersionCB(
   unsigned int nargs,
   CL_ARG*      args)
{
   puts(MY_REV);
}

static void WriteCB(
   unsigned int nargs,
   CL_ARG*      args)
{
#ifdef CYCLADES_TS
   system("signal_ras hup");
   system("saveconf");
#endif

#ifdef LANTRONIX_TS
   system("/lic/bin/SAVE");
#endif
}

static void ExitCB(
   unsigned int nargs,
   CL_ARG*      args)
{
   clexit();
}

/*
 * remove and recreate the lock file
 */
static int lock_it ()
{
   int fd;

   fd = open(LOCK_FILE, O_RDWR|O_CREAT, 0600);
   if (fd < 0)
   {
      fprintf(stderr, "fail to create lock file \n");
      return 0;
   }
   else
   {
      close (fd);
      return 0;
   }
}


/* 
 * attempt to lock the config file
 */

static int lock_config()
{
   struct stat finfo;
   time_t now;
   int i;

   now = time ((time_t *)NULL);
   i = stat (LOCK_FILE, &finfo);

   if (i == 0)
   {
      if (finfo.st_mtime > 0)
      {
         if (now > finfo.st_mtime + 60)
         {
            /* lock is more than 60 seconds old so delete it & create new one*/
            unlink (LOCK_FILE);
            return (lock_it ());
         }
         else /* lock is not more than 60 seconds old so wait some more*/
         {
            return -1;
         }
      }
   }
   else
   {
      return (lock_it ());
   }
}

/*
 * get a line from the buffer and return the pointer to the next line
 * return NULL if no more line
 */
static char * ki_getline (
   char *buffer, 
   char *line,
   int  size)
{
   char *cp, *buf;

   cp = line;
   buf = buffer;
   line[0] = '\0'; /* incase there is nothing to scan */
   if (!buf || *buf == '\0')
      return ((char *)0);

   --size;
   while (size && *buf && *buf != '\n')
   {
      if  (*buf != '\r')
         *cp++ = *buf++;
      --size;
   }
   *cp = '\0';
   if (!*buf)
   {
      /* here (cp  > line) so we must have read a partial line */
         return buffer; /* return the pointer to the original buffer */
   }
   while (*buf && *buf == '\r')
      ++buf; /* skip over the \r */
   if (*buf == '\n')
      ++buf; /* skip over the \n of the line we read in*/
   
   return (buf); /* return point to next line */
}




static void init_portcmd(
   PORT_PARAM *port,
   int size)
{
   int i;

   for (i=0; i<size; ++i)
      KZERO (&port[i], sizeof (PORT_PARAM));

}

/*
 * check to see if string has the - or the ',' character in it
 */
static int is_range (
   char *str)
{
   if (strchr (str, '-'))
      return 1;
   else if (strchr (str, ','))
      return 1;
   else 
      return 0;
}

/*
 * add_port_entry
 *
 * Add an element to the list of ports
 */
static void add_port_entry (
   PORT_LIST** head,
   PORT_LIST*  port)
{
   PORT_LIST** ins;

   for (ins = head; *ins; ins = &(*ins)->next)
      if (port->number < (*ins)->number) 
         break;

   port->next = *ins;
   *ins       = port;
}

static void free_port_list (
   PORT_LIST** head)
{
   PORT_LIST* port;

   while (*head)
   {
      port = *head;
      *head  = port->next;
      (void) free (port);
   }
}

static int add_to_list (
   PORT_LIST **head,
   int pnum)
{
   PORT_LIST *plist;

   plist = (PORT_LIST *)malloc (sizeof (PORT_LIST));
   if (!plist)
   {
      printf("Out of Memory\n");
      free_port_list (head);
      return -1;
   }
   KZERO (plist, sizeof (PORT_LIST));
   plist->number = pnum;
   add_port_entry (head, plist);
   return 0;
}

/*
 * create a list of ports
 */
static int create_port_list (
   PORT_LIST** head,
   char *str)
{
   int startnum, endnum, i, status;
   char start[32], end[32];
   char *cp;

   if (cp = strchr (str, '-'))
   {
      sscanf (str, "%[^-]", start);
      sscanf (cp+1, "%[^ \t]", end);
      startnum = atoi (start);
      endnum = atoi (end);
      if (startnum >= endnum)
      {
         printf("Invalid range\n");
         return -1;
      }
      for (i=startnum; i<=endnum; ++i)
      {
         status = add_to_list (head, i);
         if (status < 0)
            return -1;
      }
   }
   else if (cp = strtok (str, ","))
   {
      while (cp)
      {
         i = atoi (cp);
         status = add_to_list (head, i);
         if (status < 0)
            return -1;
         cp = strtok(NULL, ",");
      }
   }
   return 0;

}


#ifdef CYCLADES_TS

/*
 * look throught the portchar list of command and substitue the -1
 * for the port number pnum
 */
static int substitute_port (
   int pnum,
   PORT_PARAM *in,
   PORT_PARAM *out,
   int size)
{
   int i, ipport;
   char *to, *from, *cp;

   if (size > NUM_PORT_PARAM)
      return -1;

   for (i=0; i<size; ++i)
   {
      if (in[i].command[0])
      {
         strcpy (out[i].token, in[i].token);
         out[i].token_len = in[i].token_len;

         /* copy the command and substitute -1 for the real port number */
         to = out[i].command;
         from = in[i].command;
         while (*from)
         {
            if ((*from == '-') && (*(from + 1) == '1'))
            {
               sprintf(to, "%d", pnum);
               to += strlen (to);
               from += 2;
            }
            else
               *to++ = *from++;
         }
         /* check to see if the command is for configuring a ip port number */
         if (cp = strstr (out[i].command , "socket_port"))
         {
            cp += strlen ("socket_port");
            cp++; /* go past the space and point to the number */
            sscanf (cp, "%d", &ipport); /* get the base ip port number input */
            ipport += pnum;            /* calucate the actual ipport number */
            sprintf(cp, "%d", ipport); /* write it back out */
         }
      }
   }
   return 0;
}
/*
 * check to see if any of the param in the portchar array 
 * match the line  (not counting the value )
 * ie if the line is 
 *   s18.protocol socket_ssh
 * and the portchar has a command
 *   S18.protocol socket_server
 * then there is a match
 * return 0 with we find the pattern
 * -1 if not
 */
static int check4pattern (
   char *line,
   PORT_PARAM *portchar,
   FILE *fp,
   int size)
{
   int i;
   char *a, *b, tempLine[80], tempCommand[80];

   /* Parse line to obtain just the label. Jen 10.25.02 */
   /* e.g  s18.protocol
    * by stopping at first space in the string
    */
   strcpy(tempLine, line);
   b = tempLine;
   while (*b && isspace(*b) == 0) 
      b++;
   *b = 0; /* terminate string at the first space found*/

   for (i=0; i < size; ++i)
   {
      if ((portchar[i].command[0]) &&
          (strcmp (tempLine, portchar[i].token) == 0))
      {
			/* This takes care of conf.group. Looks for the same group.
            Syntax for conf.group is group:users1,users2.
            If found, replace with new value */
         if (strcmp(tempLine, "conf.group") == 0) {
            strcpy(tempCommand, portchar[i].command);
            a = tempCommand;
            while (*a && isspace(*a)==0) {
               a++;
            }
            a = strtok(a, ":");
            if (strstr (line, a)== NULL) {
               return -1;
            }
         }
         fprintf(fp, "%s\n", portchar[i].command);
         portchar[i].command[0] = '\0'; /* done with this command */
         return 0;
      }
   }
   return -1;

}

/*
 * modify the /etc/portslave/pslave.conf file
 * with the value in its input args
 */
static int modify_pslave_conf (
   PORT_PARAM *portchar,
   int size)
{

   FILE *fp;
   int status, i;
   char *buf, *s, *cp;
   struct stat finfo;
   char line[128];

   status = lock_config(); /* try to lock the file */
   if (status < 0)
      return -1;

   if (stat(PSLAVE_CONF, &finfo)) 
   {
      fprintf (stderr, "failed to stat %s errno %d\n", PSLAVE_CONF, errno);
      unlink (LOCK_FILE);
      return -2;
   }

   /* allocate buffer big enough to read in the whole file */
   buf = (char *)malloc(finfo.st_size+1);
   if (!buf) 
   {
      fprintf (stderr, "malloc failed\n");
      unlink (LOCK_FILE);
      return -3;
   }

   fp = fopen (PSLAVE_CONF, "r");
   if (!fp) 
   {
      fprintf (stderr, "failed to open %s errno %d\n", PSLAVE_CONF, errno);
      free(buf);
      unlink (LOCK_FILE);
      return -4;
   }

   fread(buf, 1, finfo.st_size, fp);
   *(buf + finfo.st_size) = 0;
   fclose(fp);

   fp = fopen (PSLAVE_CONF, "w"); /* now open to write it */
   if (!fp) 
   {
      free(buf);
      unlink (LOCK_FILE);
      return -5;
   }

   s = buf;
   while (s && s[0])
   {
      s =  ki_getline (s, line, sizeof line); /* read a line from buf*/
      if (line[0] == '#') /* comment line , write it back out */
      { /* dont modify commented out line, just add at the end */
         fprintf(fp, "%s\n", line);
      }
      else if (line[0] == '\0') /* empty line, write it back out */
         fprintf(fp, "\n");
      else
      {
         status = check4pattern (line, portchar, fp, size);
   /* did not find a match to write it back out */
         if (status < 0)
            fprintf(fp, "%s\n", line);
      }
   }
   /* now write out any pattern that we did not find a match to */
   for (i=0; i<size; ++i)
   {
      if (portchar[i].command[0])
         fprintf(fp, "%s\n", portchar[i].command);
   }

   fclose(fp);
   unlink (LOCK_FILE);
   return 0;

}
/*
 * callback parse arguments from the command line and call routine to
 * modify the config files
 */
static void ModifyPortCB(
   unsigned int nargs,
   CL_ARG*      args)
{
   int i, idx, port, udata, status;
   char*   errmsg;
   PORT_PARAM portcmd[NUM_PORT_PARAM]; /* there are 10 parameters to change */
   PORT_PARAM realcmd[NUM_PORT_PARAM]; /* for use with range of port */
   PORT_LIST** plist;

   port = -1;
   idx = 0;
   init_portcmd (portcmd,  NUM_ELEMENTS(portcmd));

   for (i = 0; i < nargs; ++i)
   {
      if (idx == NUM_PORT_PARAM) /* reach maximum */
         break;

      udata = (int)args[i].userData;

      if ((udata >= 1) && (udata < NUM_PORT_PARAM))
      {
         if (ttytempl[udata].token)
         {
            sprintf(portcmd[idx].token, ttytempl[udata].token, port); 
            portcmd[idx].token_len = strlen (portcmd[idx].token);
         }
      }
      switch (udata)
      {
         case 0: /* port number */
            if (!is_range (args[i].u.string_val))
               port = atoi (args[i].u.string_val);
            else
            {
               status = create_port_list (&port_head, args[i].u.string_val);
               if (status < 0)
                  return;
               port = -1; /* special flag */
            }
	 break;

         case 1: /* tty name */
            sprintf(portcmd[idx++].command, ttytempl[udata].command, 
                                                             port, port);
            
            /* 
             * dont use the input param args[i].u.string_val, hard code
             * the tty name because it is always this name
             */
	 break;
			         /* Parameters with String Values */
         case 2: case 4: case 8: case 10: case 11: case 20: case 22: case 23:
         case 30: case 31: case 32: case 33: case 34: case 35: case 36:
         case 39: case 40: case 41: case 42: case 43: case 45: case 46:
			case 48: case 49: case 50: case 51: case 54: case 53: case 58:
         	sprintf(portcmd[idx++].command, ttytempl[udata].command,
                                                  port, args[i].u.string_val);
		  break;
                  /* Parameters with Numeric Values */
         case 3: case 5: case 6: case 7: case 9: case 12: case 13:
         case 14: case 15: case 16: case 17: case 18: case 19:
         case 21: case 24: case 25: case 26: case 27: case 28: case 29:
         case 37: case 38: case 44: case 47: case 52: case 55: case 56: case 57:
         	sprintf(portcmd[idx++].command, ttytempl[udata].command,
                                                  port, args[i].u.int_val);
		  break;

      }
   }

   if ((port < 0) && !port_head)
   {
      fprintf(stderr, "Need positive port number\n");
      printf("%s\n", help_string);
      exit_status = -1;
      return;
   }
   else if ((port == -1) && port_head)
   {
      for (plist = &port_head; *plist; plist = &(*plist)->next)
      {
         init_portcmd (realcmd,  NUM_ELEMENTS(realcmd));
         status = substitute_port ((*plist)->number, portcmd, realcmd, NUM_ELEMENTS(portcmd));
         if (status < 0)
            return;

         exit_status = modify_pslave_conf (realcmd, NUM_ELEMENTS(realcmd));
      }
      free_port_list (&port_head);
   }
   else
      exit_status = modify_pslave_conf (portcmd, NUM_ELEMENTS(portcmd));

}

static void ModifyConfCB (
   unsigned int nargs,
   CL_ARG*      args)
{
   int i, idx, port, udata;
   PORT_PARAM portcmd[NUM_CONF_PARAM]; /* there are 10 parameters to change */

   port = -1;
   idx = 0;
   init_portcmd (portcmd,  NUM_ELEMENTS(portcmd));

   for (i = 0; i < nargs; ++i)
   {
      if (idx == NUM_CONF_PARAM) /* reach maximum */
          break;

      udata = (int)args[i].userData;

      switch (udata)
      {

         case 1: /* Local Logins */
               sprintf(portcmd[idx].token, "conf.locallogins");
               portcmd[idx].token_len = strlen (portcmd[idx].token);
               sprintf(portcmd[idx++].command, "conf.locallogins %d", args[i].u.string_val);
               break;

         case 2: /* NFS db */
               sprintf(portcmd[idx].token, "conf.nfs_data_buffering");
               portcmd[idx].token_len = strlen (portcmd[idx].token);
               sprintf(portcmd[idx++].command, "conf.nfs_data_buffering %s", args[i].u.string_val);
               break;

         case 3: /* Facility */
               sprintf(portcmd[idx].token, "conf.facility");
               portcmd[idx].token_len = strlen (portcmd[idx].token);
               sprintf(portcmd[idx++].command, "conf.facility %d", args[i].u.string_val);
               break;

			case 4: /* DB Facility */
               sprintf(portcmd[idx].token, "conf.DB_facility");
               portcmd[idx].token_len = strlen (portcmd[idx].token);
               sprintf(portcmd[idx++].command, "conf.DB_facility %d", args[i].u.string_val);
               break;

         case 5: /* Group */
               sprintf(portcmd[idx].token, "conf.group");
               portcmd[idx].token_len = strlen (portcmd[idx].token);
               sprintf(portcmd[idx++].command, "conf.group %s", args[i].u.string_val);
               break;

      }
   }
   exit_status = modify_pslave_conf (portcmd, NUM_ELEMENTS(portcmd));
}

#ifdef CYCLADES_KVM
extern struct kvm_conf_def conf_cfg[], term_cfg[], user_cfg[];

static void ModifyKVMConfCB (
   unsigned int nargs,
   CL_ARG*      args)
{
   int i, udata;
   unsigned char aux[8];

   if (kvmreadcfg() < 0) {
      exit_status = -4;
      return;
   }
   for (i = 0; i < nargs; ++i)
   {
      udata = (int)args[i].userData;
      switch (udata) {
         case CLI_KVM_IDLETIMEOUT:
         case CLI_KVM_FACILITY:
         case CLI_KVM_AUTHTIMEOUT:
         case CLI_KVM_AUTHRETRIES:
	    sprintf(aux, "%d", args[i].u.int_val);
            kvm_setconfig ((char *)&kvmconf, &conf_cfg[udata], aux);
            break;
	 default:
            kvm_setconfig ((char *)&kvmconf, &conf_cfg[udata], 
               args[i].u.string_val);
            break;
      }
   }
   if (kvmwritecfg() < 0) {
      exit_status = -5;
      return;
   }
}

static void ModifyKVMPortCB (
   unsigned int nargs,
   CL_ARG*      args)
{
   int i, port = -1, udata;

   if (kvmreadcfg() < 0) {
      exit_status = -4;
      return;
   }
   for (i = 0; i < nargs; ++i)
   {
      udata = (int)args[i].userData;
      if (udata == 0) {
         port = args[i].u.int_val;
         continue;
      }
      if (port <= 0 || port > 256) {
         continue;
      }
   }
   if (kvmwritecfg() < 0) {
      exit_status = -5;
      return;
   }
}

/*****************************************************************
 * cli_set_acclist - This routine is used by the CLI to set a 
 *	parameter in the user access list
 ****************************************************************/ 
static int cli_set_acclist(struct _user_config *puser, unsigned char idx,
	unsigned char *a1, unsigned char *a2)
{
	if (idx == CLI_KVM_ACCESS_PORT) {
		if (!a1) return -1;
		if (!a2) return -1;
		return set_portaccess(puser, a1, a2);
	}
	if (!a1 || kvm_setconfig((char *)puser, &user_cfg[idx - CLI_KVM_ESCAPESEQ], 
		a1) < 0) {
		return -1;
	}
	return 0;
}

static void ModifyKVMUserCB (
   unsigned int nargs,
   CL_ARG*      args)
{
   int i, udata;
   struct _user_config kvmuser;
   unsigned char *username = NULL, *port = NULL;
   unsigned char aux[8];

   for (i = 0; i < nargs; ++i)
   {
      udata = (int)args[i].userData;
      if (udata == CLI_KVM_USERNAME) {
         username = args[i].u.string_val;
         if (kvm_read_user_acclist(&kvmuser, username) < 0) {
            exit_status = -4;
            return;
         }
         continue;
      }
      if (! username) {
         continue;
      }
      if (udata == CLI_KVM_ACCESS_PORT) {
         port = args[i].u.string_val;
         continue;
      }
      if (udata == CLI_KVM_ACCESS_PERM) {
         if (!port) {
            continue;
         }
         if (cli_set_acclist(&kvmuser, CLI_KVM_ACCESS_PORT, port, 
            args[i].u.string_val) < 0) {
            // ?????
         };
         continue;
      }
      switch (udata) {
      case CLI_KVM_CYCLETIME:
	 sprintf(aux, "%d", args[i].u.int_val);
         cli_set_acclist(&kvmuser, udata, aux, NULL);
         break;
      default:
         cli_set_acclist(&kvmuser, udata, args[i].u.string_val, NULL);
         break;
      }
   }
   if (username) {
      if (kvm_write_user_acclist(&kvmuser, username) < 0) {
         exit_status = -5;
         return;
      }
   }
}

static void ModifyKVMSlaveCB (
   unsigned int nargs,
   CL_ARG*      args)
{
   int i, j = 0, udata, nports = 0;
   unsigned char *slavename = NULL, port[2];
   kvmreadslavelist();
   if (kvmreadcfg() < 0) {
      exit_status = -4;
      return;
   }
   port[0] = port[1] = 0;
   for (i = 0; i < nargs; ++i) {
      udata = (int)args[i].userData;
      if (udata == CLI_KVM_SLAVENAME) {
         slavename = args[i].u.string_val;
	 for (j = 0; j < NUMPORTS; j++) {
            if (kvm_slave[j].ports[0] == 0) continue;
	    if (strcmp(slavename, kvm_slave[j].name) == 0) break;
	 }
      } else if (! slavename) {
      } else if (udata == CLI_KVM_SLAVEREMOTE) {
         port[0] = args[i].u.int_val;
      } else if (udata == CLI_KVM_SLAVELOCAL) {
         port[1] = args[i].u.int_val;
      } else if (udata == CLI_KVM_SLAVENPORTS) {
         nports = args[i].u.int_val;
      }
   }
   if (slavename) {
      if (j == NUMPORTS && port[0] != 0) { /* Add slave */
         if ((j = kvmaddslaveentry(slavename, port, nports)) < 0) {
            exit_status = -6;
            return;
         }
      } else if (j < NUMPORTS && port[0] == 0) { /* Delete slave */
         if (kvmdelslaveentry(j) < 0) {
            exit_status = -6;
            return;
         }
      } else if (j < NUMPORTS && port[0] != 0) { /* Edit slave */
printf("Edit j = %d port[0] = %d port[1] = %d", j, port[0], port[1]);
         if (kvmchgslaveentry(j, port, nports) < 0) {
            exit_status = -6;
            return;
         }
      }
   }
   kvmwriteslavelist();
   if (kvmwritecfg() < 0) {
      exit_status = -5;
      return;
   }
}
#endif
#endif


#ifdef LANTRONIX_TS

/*
 * modify the /etc/inetd.conf to have the correct ipport number
 */
static int modify_inetd_conf (
   int ipport)
{
   FILE *fp;
   int status, i, val, found;
   char *buf, *s;
   struct stat finfo;
   char line[128], tmp[128], data[128];

   status = lock_config(); /* try to lock the file */
   if (status < 0)
      return -1;

   if (stat(INETD_CONF, &finfo)) 
   {
      fprintf (stderr, "failed to stat file %s errno %d\n", INETD_CONF, errno);
      unlink (LOCK_FILE);
      return -2;
   }

   /* allocate buffer big enough to read in the whole file */
   buf = (char *)malloc(finfo.st_size+1);
   if (!buf) 
   {
      fprintf (stderr, "malloc failed\n");
      unlink (LOCK_FILE);
      return -3;
   }

   fp = fopen (INETD_CONF, "r");
   if (!fp) 
   {
      fprintf (stderr, "failed to open %s errno %d\n", INETD_CONF, errno);
      free(buf);
      unlink (LOCK_FILE);
      return -4;
   }

   fread(buf, 1, finfo.st_size, fp);
   *(buf + finfo.st_size) = 0;
   fclose(fp);

   fp = fopen (INETD_CONF, "w"); /* now open to write it */
   if (!fp) 
   {
      free(buf);
      unlink (LOCK_FILE);
      return -5;
   }

   sprintf(data, 
"%d   stream  tcp     nowait  root    /usr/sbin/tcpd  in.telnetd", ipport);

   s = buf;
   found  = FALSE;
   while (s && s[0])
   {
      s =  ki_getline (s, line, sizeof line); /* read a line from buf*/
      if (line[0] == '#') /* comment line , write it back out */
      { /* dont modify commented out line, just add at the end */
         fprintf(fp, "%s\n", line);
      }
      else if (line[0] == '\0') /* empty line, write it back out */
         fprintf(fp, "\n");
             /* found our pattern , so just write back everything else*/
      else if (found) 
         fprintf(fp, "%s\n", line);
      else
      {
         i = sscanf (line, "%d %s", &val, tmp);
         if (i != 2)
            fprintf(fp, "%s\n", line);
         else if (val == ipport) /* found it already there, all done */
         {
            fprintf(fp, "%s\n", line);
            found = TRUE;
         }
         else
            fprintf(fp, "%s\n", line);
      }
   }
        /* did not find the line we are looking for so write one at the end */
   if (!found) 
      fprintf(fp, "%s\n", data);

   fclose(fp);
   unlink (LOCK_FILE);

   if (!found) 
   {
      /* we have added entry, send SIGHUP to the inetd daemon */
      if (inetd_pid)
         kill (inetd_pid, SIGHUP);
   }
   
   return 0;

}

/*
 * modify the /etc/lci/lwip_serial.conf to have the correct ipport number
 */
static int modify_lwip_conf (
   int port,
   int ipport)
{
   FILE *fp;
   int status, i, pval, ipval, found, n;
   char *buf, *s;
   struct stat finfo;
   char line[128], tmp[128], data[128];

   status = lock_config(); /* try to lock the file */
   if (status < 0)
      return -1;

   if (stat(LWIP_CONF, &finfo)) 
   {
      fprintf (stderr, "failed to stat file %s errno %d\n", LWIP_CONF, errno);
      unlink (LOCK_FILE);
      return -2;
   }

   /* allocate buffer big enough to read in the whole file */
   buf = (char *)malloc(finfo.st_size+1);
   if (!buf) 
   {
      fprintf (stderr, "malloc failed\n");
      unlink (LOCK_FILE);
      return -3;
   }

   fp = fopen (LWIP_CONF, "r");
   if (!fp) 
   {
      fprintf (stderr, "failed to open %s errno %d\n", LWIP_CONF, errno);
      free(buf);
      unlink (LOCK_FILE);
      return -4;
   }

   fread(buf, 1, finfo.st_size, fp);
   *(buf + finfo.st_size) = 0;
   fclose(fp);

   fp = fopen (LWIP_CONF, "w"); /* now open to write it */
   if (!fp) 
   {
      free(buf);
      unlink (LOCK_FILE);
      return -5;
   }

   sprintf(data, 
"%d           0.0.0.0      %d         0", port, ipport);

   s = buf;
   found  = FALSE;
   while (s && s[0])
   {
      s =  ki_getline (s, line, sizeof line); /* read a line from buf*/
      if (line[0] == '#') /* comment line , write it back out */
      { /* dont modify commented out line, just add at the end */
         fprintf(fp, "%s\n", line);
      }
      else if (line[0] == '\0') /* empty line, write it back out */
         fprintf(fp, "\n");
             /* found our pattern , so just write back everything else*/
      else if (found) 
         fprintf(fp, "%s\n", line);
      else
      {
         i = sscanf (line, "%d %s %d %d", &pval, tmp, &ipval, &n);
         if (i != 4)
            fprintf(fp, "%s\n", line);
         else if ((port == pval) && (ipport == ipval))
                 /* found it already there, all done */
         {
            if (n == 0) /* no authentication */
            {
               fprintf(fp, "%s\n", line);
            }
            else
            {
               fprintf(fp, "%s\n", data);
            }
            found = TRUE;
         }
         else if (port == pval) /* ipport value is different */
         {
            fprintf(fp, "%s\n", data);
            found = TRUE;
         }
         else
            fprintf(fp, "%s\n", line);
      }
   }
        /* did not find the line we are looking for so write one at the end */
   if (!found) 
      fprintf(fp, "%s\n", data);

   fclose(fp);
   unlink (LOCK_FILE);
   return 0;

}

static int modify_nobody_conf (void)
{
   char buf[128];
   struct stat finfo;

   if (stat(NOBODY_CONF, &finfo)) 
   {
      sprintf(buf, "cp %s %s", DEFAULT_USER_CONF, NOBODY_CONF);
      system (buf); 
   }
   return 0;
}

static void modify_lci_conf (
   int port,
   int speed,
   int dsize,
   int stopbits,
   char *parity)
{
   char buf[256], tmp[32];

   strcpy(buf, "devices ");
   if (speed > 0)
   {
      sprintf(tmp, "--baud=%d ", speed);
      strcat (buf, tmp);
   }
   if (dsize > 0)
   {
      sprintf(tmp, "--data=%d ", dsize);
      strcat (buf, tmp);
   }
   if (stopbits > 0)
   {
      sprintf(tmp, "--stop=%d ", stopbits);
      strcat (buf, tmp);
   }
   if (parity[0])
   {
      sprintf(tmp, "--parity=%s ", parity);
      strcat (buf, tmp);
   }
   sprintf(tmp, "%d", port); /* port number at the end */
   strcat (buf, tmp);
   system (buf); 
}

/*
 * modify various config files
 */
static int modify_conf (
   int port,
   int ipport,
   int speed,
   int dsize,
   int stopbits,
   char *parity)
{

   if (ipport > 0)
   {
      modify_inetd_conf (ipport);
      modify_lwip_conf (port, ipport);
   }
   if ((speed > 0) || (dsize > 0) || (stopbits > 0) || parity[0])
   {
      modify_lci_conf (port, speed, dsize, stopbits, parity);
   }
   modify_nobody_conf();
}

static void get_inetd_pid(void)
{
   char buf[80], *cp;
   FILE *fp;

   fp =  fopen("/var/run/inetd.pid", "r");
   if (!fp)
      return;
   fgets (buf, sizeof buf, fp); /* read in the pid */
   if (buf[0])
   {
      cp = strchr (buf, '\n');
      if (cp)
         *cp = '\0';
      inetd_pid = atoi (buf);
   }
   fclose (fp);
}
/*
 * callback parse arguments from the command line and call routine to
 * modify the config files
 */ 
static void ModifyPortCB(
   unsigned int nargs,
   CL_ARG*      args)
{
   int i, idx, port, udata, status;
   char*   errmsg;
   PORT_LIST** plist;
   int speed, dsize, stopbits, ipport;
   char parity[8];

   port = -1;
   speed = dsize = stopbits = ipport = 0;
   KZERO (parity, sizeof parity);

   for (i = 0; i < nargs; ++i)
   {
      if (idx == NUM_PORT_PARAM) /* reach maximum */
         break;

      udata = (int)args[i].userData;

      switch (udata)
      {
         case 0: /* port number */
            if (!is_range (args[i].u.string_val))
               port = atoi (args[i].u.string_val);
            else
            {
               status = create_port_list (&port_head, args[i].u.string_val);
               if (status < 0)
                  return;
               port = -1; /* special flag */
            }
	 break;


         case 5:  /* speed */
            speed = args[i].u.int_val;
	 break;

         case 6: /* data size */
            dsize = args[i].u.int_val;
	 break;

         case 7:  /* stop bits */
            stopbits = args[i].u.int_val;
	 break;

         case 8: /* parity */
            strcpy (parity, args[i].u.string_val);
	 break;

         case 9:  /* ip port number */
            ipport = args[i].u.int_val;
	 break;

      }
   }
   if (ipport > 0) /* need to get inet pid */
      get_inetd_pid();

   if ((port < 0) && !port_head)
   {
      fprintf(stderr, "Need positive port number\n");
      printf("%s\n", help_string);
      exit_status = -1;
      return;
   }
   else if ((port == -1) && port_head) /* port range */
   {
      for (plist = &port_head; *plist; plist = &(*plist)->next)
      {
         exit_status = modify_conf ((*plist)->number, ipport, speed, dsize, stopbits, parity);
      }
      free_port_list (&port_head);
   }
   else
      exit_status = modify_conf (port, ipport, speed, dsize, stopbits, parity);
}
#endif



#ifdef CYCLADES_TS
static void ConfigureEtherCB (
   unsigned int nargs,
   CL_ARG*      args)
{
   int i, idx, port, udata;
   char*   errmsg;
   PORT_PARAM portcmd[NUM_ETHER_PARAM]; /* there are 10 parameters to change */

   port = -1;
   idx = 0;
   init_portcmd (portcmd,  NUM_ELEMENTS(portcmd));

   for (i = 0; i < nargs; ++i)
   {
      if (idx == NUM_ETHER_PARAM) /* reach maximum */
         break;

      udata = (int)args[i].userData;

      switch (udata)
      {

         case 1: /* ipaddress */
            sprintf(portcmd[idx].token, "conf.eth_ip");
            portcmd[idx].token_len = strlen (portcmd[idx].token);
            sprintf(portcmd[idx++].command, "conf.eth_ip %s", args[i].u.string_val);
            
            /* 
             * dont use the input param args[i].u.string_val, hard code
             * the tty name because it is always this name
             */
	 break;

         case 2: /* mask */
            sprintf(portcmd[idx].token, "conf.eth_mask");
            portcmd[idx].token_len = strlen (portcmd[idx].token);
            sprintf(portcmd[idx++].command, "conf.eth_mask %s", args[i].u.string_val);
	 break;

         case 3: /* MTU */
            sprintf(portcmd[idx].token, "conf.eth_mtu");
            portcmd[idx].token_len = strlen (portcmd[idx].token);
            sprintf(portcmd[idx++].command, "conf.eth_mtu %d", args[i].u.string_val);
	 break;

      }
   }

   exit_status = modify_pslave_conf (portcmd, NUM_ELEMENTS(portcmd));

}
#endif

/*
 * kill -9 the process that has the port open
 * the process pattern is in *pat
 * char * line has the line output from the ps ax command
 */
static int kill_proc (
   char *line, 
   char *pat)
{
   int pid = -1;
   char buf[128], tmp[16];

#ifdef CYCLADES_TS
   sscanf (line, "%d %s", &pid, buf);
#endif
#ifdef LANTRONIX_TS
   sscanf (line, "%s %d %s", tmp, &pid, buf);
#endif
   if (pid > 0)
   {
      kill (pid, SIGINT);
      return 1;
   }
   return 0;
   
}

/*
 * check to see if the process in pat1 or pat2 is present in the line buffer
 */

static int check4process (
   char *line,
   char *pat1,
   char *pat2)
{
   int got_it = 0;

   if (strstr (line, pat1))
   {
     got_it = kill_proc (line, pat1);
   }
   else if (pat2)
   {
     if (strstr (line, pat2))
        got_it = kill_proc (line, pat2);
   }
   return got_it;
}

/*
 * issue a ps ax | grep "RW_ssh ttySn"
 * or ps ax | grep "RW_srv ttySn"
 * where n is the port number
 * and kill -9 the process id of the process 
 */
static void LogoutPortCB (
   unsigned int nargs,
   CL_ARG*      args)
{
   int status;
   int port, got_it;
   char pat1[64], pat2[64];
   char line[128];
   FILE *fp;
   PORT_LIST *p;
   PORT_LIST** plist;

   if (!is_range (args[0].u.string_val))
      port = atoi (args[0].u.string_val);
   else
   {
      status = create_port_list (&port_head, args[0].u.string_val);
      if (status < 0)
         return;
      port = -1; /* special flag */
   }
   strcpy (line, "ps -ef");

   fp = popen (line, "r");
   if (!fp)
   {
      if (port_head)
         free_port_list (&port_head);
      return;
   }
   if (port > 0)
   {
#ifdef CYCLADES_TS
      sprintf(pat1, "RW_srv ttyS%d", port);
      sprintf(pat2, "RW_ssh ttyS%d", port);
#endif
#ifdef LANTRONIX_TS
      sprintf(pat1, "direct %d", port);
#endif
   }
   while ( fgets (line, sizeof line, fp)) 
   {
      if (port_head)
      {
         for (plist = &port_head; *plist; plist = &(*plist)->next)
         {
#ifdef CYCLADES_TS
            sprintf(pat1, "RW_srv ttyS%d", (*plist)->number);
            sprintf(pat2, "RW_ssh ttyS%d", (*plist)->number);
#endif
#ifdef LANTRONIX_TS
            sprintf(pat1, "direct %d", (*plist)->number);
#endif
            got_it = check4process (line, pat1, pat2);
            if (got_it)
            {
               /* 
                * free this port, so the port list get smaller so we
                * have less to search through in the next line of the ps ax
                * output
                */
               p = *plist;
               *plist = p->next;
               free (p);
               break; /* out of the for loop, go to next line */
            }
         }
      }
      else
      {
         got_it = check4process (line, pat1, pat2);
         if (got_it)
            break; /* out of the while, all done */
      }
   }
   if (port_head)
      free_port_list (&port_head);
   fclose (fp);
}
