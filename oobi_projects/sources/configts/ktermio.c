/******************************************************************************

         COPYRIGHT 1989 - 1994 KI RESEARCH, INC.  ALL RIGHTS RESERVED
         COPYRIGHT 1994 - 2002 Ki NETWORKS, Inc.  ALL RIGHTS RESERVED

*******************************************************************************

  RCS Revision Control Information:
   @(#)$Header: /usr/cvsroot/oobi_projects/sources/configts/ktermio.c,v 1.1.1.1 2005/09/01 02:36:37 edson Exp $

   Revision:        $Revision: 1.1.1.1 $
   Created From:    $Source: /usr/cvsroot/oobi_projects/sources/configts/ktermio.c,v $
   Last Updated:    $Date: 2005/09/01 02:36:37 $
   Current State:   $State: Exp $
   Checked Out By:  $Locker:  $

*******************************************************************************

                              ktermio.c

   Description:

      This file contains terminal and pty handling routines.

   For systems that do not support an O/S routine to locate and/or open
   tty/pty's, we will look for them ourselves.  The tty/pty device names
   are built and searched in the following order:

      /dev/tty[p-z][0-9a-f] for the first 176 pseudo terminals
      /dev/tty[k-o][0-9a-f] for the next   80 pseudo terminals
      /dev/tty[P-Z][0-9a-f] for the next  176 pseudo terminals
      /dev/tty[K-O][0-9a-f] for the next   80 pseudo terminals
      /dev/tty[p-z][g-v]    for the next  176 pseudo terminals
      /dev/tty[k-o][g-v]    for the next   80 pseudo terminals
      /dev/tty[P-Z][g-v]    for the next  176 pseudo terminals
      /dev/tty[K-O][g-v]    for the next   80 pseudo terminals

   giving a maximum of 1024 pseudo terminals.  Not all ports may
   support this total number.  This ordering was taken from the
   Gould system ordering.  Gould (and BSD4.3 in specific) supplies
   a routine called 'ptypair()' which returns two file descriptors
   and two strings which contain the devices names opened.  We
   don't want to open the tty though. So we don't want to call
   that routine.  Sequent supplies a routine called 'getpseudotty()'
   which returns pointers to the device names to use.  We'll use
   that routine for Sequent.


   Routines:

      k_set_echo              -- Set echo mode on stdin
      k_set_no_echo           -- Set no-echo mode on stdin
      k_fset_echo             -- Set echo mode
      k_fset_no_echo          -- Set no-echo mode
      loose_control_tty       -- Lose the controling tty
      set_non_blocking        -- Set a file descriptor to non-blocking
      exec_daemon             -- Run a program in a background process
      put_tty_in_raw_mode     -- Put stdin into "raw" mode
      put_tty_in_cooked_mode  -- Put stdin into "cooked" mode
                                 if it was in "raw" mode
      put_fd_in_raw_mode      -- Put fd into "raw" mode
      put_fd_in_cooked_mode   -- Put fd into "cooked" mode
      k_getapty               -- Find and open free pty
      k_set_baud              -- Set I/O Baudrate

   Change Log:

     Date    Who  Rev   Reason for update
   --------- --- ------ -------------------------------------------------------
    9-Oct-02 bns  1.54  Include sys/tty.h for Tru64
    9-Oct-02 bns  1.53  Add put_tty_in_raw_input_mode(), get_tty_cntl_chars(),
                        and get_tty_rows()
   18-Apr-02 bns  1.52  Linux changes
   17-Apr-02 bns  1.51  OpenBSD changes
    4-Oct-01 bns  1.49  Share windows daemon log files for read
   15-Feb-01 bqc  1.48  put back k_getapty() for svr4 (solaris)
   04-Dec-00 bqc  1.47  put back k_getapty() for IBMR2
   18-Apr-00 bns  1.46  Don't use TIOCNOTTY for Solaris
   15-Mar-00 bns  1.45  Linux changes
   14-Mar-00 bns  1.44  DUnix changes
   11-Jan-00 bns  1.43  Fix k_getapty on SGI to return error correctly
   30-Nov-99 bns  1.42  Remove NOUAF flag from VMS sys$creprc call
   23-Sep-99 bns  1.41  Create mailboxes without names so a new one always gets
                        created
   23-Sep-99 ebh  1.40  Modified exec_daemon on VMS so mailboxes for
                        communicating with the created loginout.exe process
                        are named uniquely for each process. 
   18-Aug-99 bns  1.37  Do more setup for VMS exec_daemon
   04-Aug-99 bqc  1.36  restore sgi's old k_getapty code back
   30-Jul-99 bns  1.35  Add close_tty, read_tty
   13-Jul-99 bns  1.34  Fix Unix code broken by the VMS port
    9-Jul-99 bns  1.33  Fix return value of k_set_parity
    9-Jul-99 bns  1.32  add k_getapty and k_freepty for win32, remove dead code
    9-Jul-99 bns  1.31  Fix Unix compiler warning
    8-Jul-99 bns  1.30  Fix Unix code broken by the VMS port
    8-Jul-99 bns  1.29  Fix Unix code broken by the VMS port
    8-Jul-99 bns  1.28  Fix Unix code broken by the VMS port
    8-Jul-99 bns  1.27  Fix Unix code broken by the VMS port
    8-Jul-99 bns  1.26  Add some functions for VMS's sake
   24-Jun-99 bns  1.25  Fix status message from exec_daemon on VMS
   17-Jun-99 bns  1.24  Create log and error files if they don't exist on win32
                        in exec_daemon
   16-Jun-99 ebh  1.23  Changed the creprc call on VMS to create processes
                        with base priority of 4 instead of zero.
   16-Jun-99 ebh  1.21  Changed exec_daemon to use executable name as procname
                        on VMS, instead of KIBG_#.  Still makes sure procname
                        is unique and adds _### if necessary.
   10-Jun-99 bns  1.20  Change exec_daemon to return pid
   10-Jun-99 bns  1.19  Fix merged log/error files on VMS
    9-Jun-99 bns  1.18  Fix typo in win32 code
    9-Jun-99 bns  1.17  Add exec_daemon for VMS and win32
   27-May-99 bns  1.16  Implement become_a_daemon for VMS
   26-Apr-99 bqc  1.15  Fix Win32 echo_on() echo_off()
   19-Apr-99 bns  1.14  Fix Win32 compiler warnings
   16-Apr-99 bqc  1.12  add echo_off() echo_on() 
   13-Apr-99 ebh  1.11  Stubbed out all functions for VMS platform with error
                        return values so I can write the function definitions
                        as they are needed. 
   26-Mar-99 bns  1.9   #ifdef on __osf__ instead of __alpha
   19-Nov-98 bns  1.8   Linux change
    6-Oct-98 bns  1.7   Use Get/SetCommState instead of Get/SetCommConfig
   29-Jun-98 ebh  1.12? (revision says 1.5)
                        Changed CreateProcess call in become_a_daemon on NT
                        platform to not DETACH_PROCESS.  Instead it sets the
                        desktop to "winsta0/default" and hides the console
                        window that would otherwise open. 
   16-Apr-98 drs  1.11  Changed function return type to remove warning that 
                        ANSI compiler acc was returning. 
   19-Mar-98 bns  1.10  Remove include of windows.h
   20-Feb-98 bns  1.9   Add k_set_baud, k_set_parity, and k_set_csize for NT
   27-Jan-98 bqc  1.8   solaris now uses termio for tty raw/cooked
   18-Nov-97 bqc  1.7   add k_set_csize() and k_set_parity()
    7-Aug-97 bns  1.6   Change directory to root in become_a_daemon for NT
   27-Jun-97 bns  1.5   Fix become_a_daemon for NT
   29-May-97 bqc  1.4   NT, fix become_a_daemon()
   27-May-97 bns  1.3   Windows NT changes
   09-May-95 bqc  1.2  fix purify uninit mem read in k_set_baud()
   28-Jul-93 bqc  10.34 LAT V5.18.22. ncr3000 k_getapty to return /dev/ptsxxx
   19-Jul-93 dew  10.35 LAT V5.18.21. Using O_NOCTTY for opening pty's
   26-Mar-93 bqc  10.34 unisys_svr4 may not need setsid
   17-Feb-93 dew  10.33 Solaris 2.0 has BSD defined. This has set_non_blocking
                        do the wrong thing.  We want to use O_NONBLOCK. Fixing
                        code so it handles this correctly.
   27-Jul-92 dew  10.31 ti1500 doesn't declare strsave()
   12-Jun-92 jmp  10.29 OS/2 2.0 edits
   27-May-92  jmp  10.28: OS/2 2.0 edits
   08-May-92  bqc  10.26: ti1500 edits
   15-Apr-92  dew  10.25 Sun4 took long time to figure out that there where no
                   available ptys.  5 seconds! Long enough to cause connections
                   to get dropped.  Change made for all ports which don't have
                   a system call to get next available pty.
    4-Mar-92  dew  10.24: HP/UX 8.0.7 edits @Hill AFB. stdin/stdout nonblocking 
                   aren't the same as for AF_UNIX.
   30-Jan-92  bqc  10.23: harris edits for k_getapty
   30-Jan-92  bqc  10.22: hpux set_non_blocking() to use SET_NONBLOCKING
   30-Jan-92  dew  10.21: Adding cefghij to MAJOR_CH for hpux
   21-Jan-92  bqc  10.20: Harris edits
   19-Dec-91  dew  10.19: Adding SGI Irix 4.0.1 _getpty() support
   30-Oct-91  nxs  declare strsave() for _IBMR2
   11-Oct-91  bqc  CRAY edits
              dew  There was a ^ L in the file from bqc.  Removed.
    8-Oct-91  dew  Generic BSD put_tty/fd_in_cooked_mode was bad.  We had a
                   line of code from the raw section. This resulted in the
                   routine leaving the tty/fd in raw mode.
   31-Aug-91  bqc  tandem edits
   22-Aug-91  bqc  svr4 edits, loose_contro_tty now call setsid()
    5-Aug-91  dew  Changes to put_fd_in_raw_mode for sysV
   31-Jul-91  jmp  Change to put_fd_in_raw_mode for IBMR2
   23-Jul-91  jmp  OS/2 edits
   26-Apr-91  bqc  hpux edits, loose_contro_tty now call setsid()
   26-Mar-91  bqc  Sequent k_freetty never did anything!!. Should free the mem.
   22-Mar-91  dew  The Mips edits need to use MIPS not mips since other ports
                   that are Mips CPU based don't have this ability. Adding 
                   support for SGI to use master pty's.  SGI doesn't normally
                   create the master devices.  We'll do it ourselves.
   21-Mar-91  bqc  Mips has its own k_getapty now, since it has name for pty
   20-Feb-91  bqc  Put the k_getapty for sco back in
   29-Jan-91  bqc  Comment out the k_getapty for sco since it did not work well
   23-Jan-91  dew  Change for SunOS 4.1
   09-Jan-91  dew  Adding support for Arix Stream PTY's and USE_POLL
   02-Jan-91  bqc  Sequent ptx edits
   18-Dec-90  bqc  Add k_getapty() for sco
   12-Oct-90  bqc  stellar edits
   12-Oct-90  jmp  Arix support
    5-Oct-90  dew  Moving ardent utmp stuff to rmut.c
   28-Sep-90  bqc  Fix for sequent k_getapty() finally made in here
   19-Sep-90  bqc  Stellar edits
   11-Sep-90  jmp  Stellar edits
   07-Sep-90  bqc  sco nedds setsid() also
   23-Jul-90  bqc  SunOS 4.1 is posix compliant. Fix loose_control_tty
   18-Jul-90  bqc  Fix bug in k_getapty for IBMR2
   11-Jul-90  bqc  MIPS OS 4.50 loose_control_tty needs to call setsid()
    6-Jul-90  dew  Removing (char *) cast in calls to ioctl(). This bombs on
                   on AOS/VS and shouldn't be necessary.
   07-May-90  bqc  Fix bug in loose_control_tty for IBMR2
   25-Jan-90  dew  Integrating with KiNET build
   15-Jan-90  bqc  IBMR2 changes
   30-Nov-89  bqc  MIPS debugging
   28-Nov-89  dew  Missing defined(mips) for pty stuff
    9-Nov-89  dew  Integrating with Sequent
   18-Oct-89  dew  Integrating with AOS/VS
   29-Sep-89  dew  Adding k_set_baud
   26-Sep-89  dew  SGI edits
   22-Sep-89  dew  Adding k_fset_echo and k_fset_no_echo
   14-Sep-89  jsc  Adding #include's for Ardent
   11-Sep-89  bqc  Add code for strict sysV sytems.
    7-Sep-89  dew  Created from LAT, SETHOST, and DVTA code.
                   Portions Copyright Ardent Computer.
                   Portions designed by dew and bqc.

******************************************************************************/

#undef DID_IT

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#if defined(sgi)
# include <sys/sysmacros.h>
# include <sys/stat.h>
#endif /* sgi */


#if defined(VMS)
# define DID_IT
# include <starlet.h>
# include <descrip.h>
# include <prcdef.h>
# include <iodef.h>
# include <dvidef.h>
# include <cmbdef.h>
# include <ssdef.h>
# include <ttdef.h>
# include <tt2def.h>
# include <dcdef.h>
#endif /* VMS */

#if defined(win32)
# define DID_IT
# include <process.h>
#endif /* win32 */

#if defined(DID_IT)
# undef DID_IT
#else /* default */
# include <unistd.h>
# if defined(__osf__) || defined(openbsd)
#  if defined(__osf__)
#   include <sys/tty.h>
#  endif
#  define TERM_CHARS	struct termios
#  define GET_TERM(x,y)	tcgetattr(x, y)
#  define SET_TERM(x,y)	tcsetattr(x, 0, y)
# else /* not osf nor openbsd */
#  if defined(linux)
#   include <termio.h>
#  else /* not linux */
#   include <sys/termio.h>
#  endif /* linux */

#  define TERM_CHARS	struct termio
#  define GET_TERM(x,y)	ioctl(x, TCGETA, y)
#  define SET_TERM(x,y)	ioctl(x, TCSETA, y)
# endif /* osf or openbsd */
#endif /* default */

#include <fcntl.h>
#include <time.h>

#if defined(VMS)
# define DID_IT
# define SUCCESSFUL(x)	(((x) & 1) != 0)
#endif /* VMS */

#if defined(win32)
# define DID_IT
#endif /* win32 */

#if defined(DID_IT)
# undef DID_IT
#else /* default */
#ifdef hpux
#define TTY_PATH  "/dev/pty/"
#define PTY_PATH  "/dev/ptym/"
#define MAJOR_CH           "pqrstuvwxyzabcefghijklmnoPQRSTUVWXYZKLMNO"
#else
#define TTY_PATH  "/dev/"
#define PTY_PATH  "/dev/"
#endif /* hpux */

#define TTY_NAME  "tty__"
#define PTY_NAME  "pty__"
#define TTY_LEN    (sizeof(TTY_PATH)+sizeof(TTY_NAME)-2) /* Don't count <NUL> */
#define PTY_LEN    (sizeof(PTY_PATH)+sizeof(PTY_NAME)-2) /* Don't count <NUL> */

#if ! defined(MAJOR_CH)
#define MAJOR_CH           "pqrstuvwxyzklmnoPQRSTUVWXYZKLMNO"
#endif /* !MAJOR_CH */

#ifdef O_NOCTTY
# define KI_NOCTTY	O_NOCTTY
#else
# define KI_NOCTTY	0
#endif
#endif /* default */

#ifndef TRUE
# define TRUE 1
#endif /* not TRUE */

#ifndef FALSE
# define FALSE 0
#endif /* not FALSE */

/***************
* Private Data *
***************/
#if defined(VMS)
# define DID_IT
static unsigned short ttyin_channel  = 0;
static unsigned short ttyout_channel = 0;
static unsigned int   old_mode[3];
static unsigned int   old_speed;
static unsigned int   old_parity;
static unsigned int   old_fill;
static unsigned int   current_mode[3];
static unsigned int   current_speed;
static unsigned int   current_parity;
static unsigned int   current_fill;
#endif /* VMS */

#if defined(win32)
# define DID_IT
static HANDLE ttyin_handle  = 0;
static HANDLE ttyout_handle = 0;
static DWORD  old_mode;
static DWORD  current_mode;
#endif /* win32 */

#if defined(DID_IT)
# undef DID_IT
#else /* default */
static int    ttyin_fd  = -1;
static int    ttyout_fd = -1;

static TERM_CHARS old_mode;
static TERM_CHARS current_mode;

static char major_ch[]  = MAJOR_CH;
static char minor_ch1[] = "0123456789abcdef";
static char minor_ch2[] = "ghijklmnopqrstuv";
static TERM_CHARS cooked_mode;
static int           raw_fd = -1;
static unsigned int  nrow;
#endif /* default */

/*******************
* External Modules *
*******************/
extern char* kstrrchr(
   char* s,
   char  c);


#if ! defined(VMS) && ! defined(WIN32)
/*
 * new_size
 *
 * Signal handler for when terminal window changes size.
 *
 */
static void new_size(
   int sig)
{
   char*          tptr;
   int            status;
   struct winsize winsize;

   signal(SIGWINCH, new_size);

   status = ioctl(ttyout_fd, TIOCGWINSZ, (char*) &winsize);
   if (status == 0)
      nrow = winsize.ws_row;

   if (nrow == 0)
   {
      tptr = getenv("LINES");
      if (tptr != NULL)
	 nrow = atoi(tptr);
   }

   /* Last resort */
   if (nrow == 0)
      nrow = 24;
}
#endif /* not VMS nor WIN32 */

/*
 * TTYSetup
 *
 * Return FALSE if setup successful or already done, TRUE if not.
 */
static int TTYSetup(void)
{
#if defined(VMS)
#define DID_IT
   int            status;
   unsigned short iosb[4];
   $DESCRIPTOR(input_desc, "SYS$INPUT");
   $DESCRIPTOR(output_desc, "SYS$OUTPUT");
#endif /* VMS */

#if defined(win32)
# define DID_IT
#endif /* win32 */

#if defined(DID_IT)
# undef DID_IT
#else /* default */
#endif /* default */

#if defined(VMS)
#define DID_IT
   int            status;
   unsigned short iosb[4];
   $DESCRIPTOR(input_desc, "SYS$INPUT");
   $DESCRIPTOR(output_desc, "SYS$OUTPUT");

   if (ttyin_channel)
      return FALSE;

   status = sys$assign(&input_desc, &ttyin_channel, NULL, NULL);
   if (! SUCCESSFUL(status))
      return TRUE;

   status = sys$assign(&output_desc, &ttyout_channel, NULL, NULL);
   if (! SUCCESSFUL(status))
   {
      sys$dassgn(ttyin_channel);
      ttyin_channel = 0;
      return TRUE;
   }

   status = sys$qiow(0, ttyin_channel, IO$_SENSEMODE, iosb, NULL, NULL,
                     old_mode, sizeof old_mode, 0, 0, 0, 0);
   if (! SUCCESSFUL(status) || ! SUCCESSFUL(iosb[0])
       || (old_mode[0] & 0xFF) != DC$_TERM)
   {
      sys$dassgn(ttyin_channel);
      sys$dassgn(ttyout_channel);
      ttyin_channel = 0;
      ttyout_channel = 0;
      return TRUE;
   }

   old_speed  = iosb[1];
   old_fill   = iosb[2];
   old_parity = iosb[3] << 4;

   MOVB(old_mode, current_mode, sizeof current_mode);
   current_speed  = old_speed;
   current_fill   = old_fill;
   current_parity = old_parity;

   nrow = (current_mode[1] >> 24) & 0xFF;
#endif /* VMS */

#if defined(win32)
# define DID_IT
   if (ttyin_handle)
      return FALSE;

   ttyin_handle  = GetStdHandle(STD_INPUT_HANDLE);
   ttyout_handle = GetStdHandle(STD_OUTPUT_HANDLE);

   GetConsoleMode(ttyin_handle, &old_mode);
   current_mode = old_mode;
#endif /* win32 */

#if defined(DID_IT)
# undef DID_IT
#else /* default */
   if (ttyin_fd != -1)
      return FALSE;

   ttyin_fd = fileno(stdin);

   if (! isatty(ttyin_fd))
   {
      ttyin_fd = -1;
      return TRUE;
   }

   ttyout_fd = fileno(stderr);
   GET_TERM(ttyin_fd, &old_mode);
   current_mode = old_mode;

   new_size(0); /* Doesn't use the argument, but compilers complain
		   if it's not there. */
#endif /* default */

   return FALSE;
}


/*
 * put_tty_in_raw_mode
 */
void put_tty_in_raw_mode(void)
{
#if defined(VMS)
#define DID_IT
   int            status;
   unsigned short iosb[4];

   if (TTYSetup())
      return;

   current_mode[1] |= TTM_NOECHO | TTM_ESCAPE;
   current_mode[2]  = (current_mode[2] & ~TT2M_LOCALECHO) | TT2M_PASTHRU;
   status = sys$qiow(0, ttyin_channel, IO$_SETMODE, iosb, NULL, NULL,
                     current_mode, sizeof current_mode, current_speed,
                     current_fill, current_parity, 0);
#endif /* VMS */

#if defined(win32)
# define DID_IT
   if (TTYSetup())
      return;

   current_mode = 0;
   SetConsoleMode(ttyin_handle, current_mode);
#endif /* win32 */

#if defined(DID_IT)
# undef DID_IT
#else /* default */
   if (TTYSetup())
      return;

   current_mode.c_lflag &= ~(ECHO | ECHOE | ECHOK | ICANON | ISIG);
   current_mode.c_iflag  =  0;
   current_mode.c_oflag  =  0;
   current_mode.c_cc[VMIN]  =  1; /* 4 is VMIN , min 1 character on input*/
   current_mode.c_cc[VTIME]  =  0; /* 5 is VTIME */

   SET_TERM(ttyin_fd, &current_mode);
#endif /* default */
}

/*
 * put_tty_in_cooked_mode
 */
void put_tty_in_cooked_mode(void)
{
#if defined(VMS)
#define DID_IT
   int            status;
   unsigned short iosb[4];

   if (TTYSetup())
      return;

   status = sys$qiow(0, ttyin_channel, IO$_SETMODE, iosb, NULL, NULL,
                     old_mode, sizeof old_mode, old_speed, old_fill,
                     old_parity, 0);
#endif /* VMS */

#if defined(win32)
# define DID_IT
   if (TTYSetup())
      return;

   SetConsoleMode(ttyin_handle, old_mode);
#endif /* win32 */

#if defined(DID_IT)
# undef DID_IT
#else /* default */
   if (TTYSetup())
      return;

   SET_TERM(ttyin_fd, &old_mode);
#endif /* default */
}



/*
 * k_set_echo
 */
void k_set_echo(void)
{
#if defined(VMS)
# define DID_IT
   echo_on();
#endif /* VMS */

#if defined(win32)
# define DID_IT
   echo_on();
#endif /* win32 */

#if defined(DID_IT)
# undef DID_IT
#else /* default */
   if (TTYSetup())
      return;

   current_mode.c_lflag |= ECHO;

   SET_TERM(ttyin_fd, &current_mode);
#endif /* default */
}

/*
 * k_set_no_echo
 */
void k_set_no_echo(void)
{
#if defined(VMS)
# define DID_IT
   echo_off();
#endif /* VMS */

#if defined(win32)
# define DID_IT
   echo_off();
#endif /* win32 */

#if defined(DID_IT)
# undef DID_IT
#else /* default */
   if (TTYSetup())
      return;

   current_mode.c_lflag &= ~ECHO;

   SET_TERM(ttyin_fd, &current_mode);
#endif /* default */
}

/*
 * write_tty
 */
void write_tty(
   char* buffer,
   int   size)
{
#if defined(VMS)
#define DID_IT
   int            status;
   unsigned short iosb[4];

   if (TTYSetup())
      return;

   status = sys$qiow(0, ttyout_channel, IO$_WRITEVBLK, iosb, NULL, NULL,
                     buffer, size, 0, 0, 0, 0);
#endif /* VMS */

#if defined(win32)
# define DID_IT
   DWORD nc;

   if (TTYSetup())
      return;

   WriteConsole(ttyout_handle, buffer, size, &nc, NULL);
#endif /* win32 */

#if defined(DID_IT)
# undef DID_IT
#else /* default */
   if (TTYSetup())
      return;

   write(ttyout_fd, buffer, size);
#endif /* default */
}

/*
 * read_tty
 */
int read_tty(
   char* buffer,
   int   size)
{
   int bytes;

#if defined(VMS)
#define DID_IT
   int            status;
   unsigned short iosb[4];
#endif /* VMS */

#if defined(win32)
# define DID_IT
   BOOL  ok;
   DWORD nc;
#endif /* win32 */

#if defined(DID_IT)
# undef DID_IT
#else /* default */
#endif /* default */

   bytes = 0;

   if (TTYSetup())
      return 0;

#if defined(VMS)
#define DID_IT
   status = sys$qiow(0, ttyin_channel, IO$_READVBLK, iosb, NULL, NULL,
                     buffer, size, 0, 0, 0, 0);
   if (SUCCESSFUL(status) && SUCCESSFUL(iosb[0]))
      bytes = iosb[1] + iosb[3];
#endif /* VMS */

#if defined(win32)
# define DID_IT
   ok = ReadConsole(ttyin_handle, buffer, size, &nc, NULL);
   if (ok)
      bytes = nc;
#endif /* win32 */

#if defined(DID_IT)
# undef DID_IT
#else /* default */
   bytes = read (ttyin_fd, buffer, size);
#endif /* default */

   return bytes;
}


/*
 * put_fd_in_raw_mode
 */
void put_fd_in_raw_mode(
   int fd)
{
#if defined(VMS)
#define DID_IT
   unsigned short channel;
   unsigned int   mode[3];
   unsigned int   speed;
   unsigned int   parity;
   unsigned int   fill;
   int            status;
   unsigned short iosb[4];

   channel = decc$get_sdc(fd);
   if (channel == 0)
      return;

   status = sys$qiow(0, channel, IO$_SENSEMODE, iosb, NULL, NULL,
                     mode, sizeof mode, 0, 0, 0, 0);
   if (! SUCCESSFUL(status) || ! SUCCESSFUL(iosb[0])
       || (mode[0] & 0xFF) != DC$_TERM)
      return;

   speed  = iosb[1];
   fill   = iosb[2];
   parity = iosb[3];

   mode[1] |= TTM_NOECHO;
   mode[2]  = (current_mode[2] & ~TT2M_LOCALECHO) | TT2M_PASTHRU;
   status = sys$qiow(0, channel, IO$_SETMODE, iosb, NULL, NULL,
                     mode, sizeof mode, speed, fill, parity, 0);
#endif /* VMS */

#if defined(win32)
# define DID_IT
   setmode(fd, O_BINARY);
#endif /* win32 */

#if defined(DID_IT)
# undef DID_IT
#else /* default */
   TERM_CHARS ntty;

   raw_fd = -1;

   if (GET_TERM(fd, &cooked_mode) != 0)
      return;

   raw_fd = fd;
   ntty   = cooked_mode;

   ntty.c_lflag &= ~(ECHO | ECHOE | ECHOK | ICANON | ISIG);
   ntty.c_iflag  =  0;
   ntty.c_oflag  =  0;
   ntty.c_cc[VMIN]  =  1;
   ntty.c_cc[VTIME]  =  0;

   SET_TERM(fd, &ntty);
#endif /* default */
}

/*
 * put_fd_in_cooked_mode
 */
void put_fd_in_cooked_mode(
   int fd)
{
#if defined(VMS)
#define DID_IT
   unsigned short channel;
   unsigned int   mode[3];
   unsigned int   speed;
   unsigned int   parity;
   unsigned int   fill;
   int            status;
   unsigned short iosb[4];

   channel = decc$get_sdc(fd);
   if (channel == 0)
      return;

   status = sys$qiow(0, channel, IO$_SENSEMODE, iosb, NULL, NULL,
                     mode, sizeof mode, 0, 0, 0, 0);
   if (! SUCCESSFUL(status) || ! SUCCESSFUL(iosb[0])
       || (mode[0] & 0xFF) != DC$_TERM)
      return;

   speed  = iosb[1];
   fill   = iosb[2];
   parity = iosb[3];

   mode[1] &= ~TTM_NOECHO;
   mode[2] &= ~TT2M_PASTHRU;
   status = sys$qiow(0, channel, IO$_SETMODE, iosb, NULL, NULL,
                     mode, sizeof mode, speed, fill, parity, 0);
#endif /* VMS */

#if defined(win32)
# define DID_IT
   setmode(fd, O_TEXT);
#endif /* win32 */

#if defined(DID_IT)
# undef DID_IT
#else /* default */
   TERM_CHARS ntty;

   if (fd == raw_fd)
   {
      SET_TERM(fd, &cooked_mode);
   }
   else
   {
      if (GET_TERM(fd, &ntty) != 0)
         return;

      ntty.c_lflag |= (ECHO | ECHOE | ECHOK | ICANON | ISIG);
      ntty.c_iflag  =  IXON | ICRNL | INLCR | INPCK | IGNPAR | BRKINT;
      ntty.c_oflag  =  ONLCR | OPOST;
      ntty.c_cc[VMIN]  =  '\004';
      ntty.c_cc[VTIME]  =  0;

      SET_TERM(fd, &ntty);
   }
#endif /* default */
}

/*
 * k_set_baud
 * this is for a fd not ttyin_fd to TTYSetup() has not been called
 * therefore 'current_mode' is not initialized
 */
void k_set_baud(
   int fd,
   int cb)
{
#if defined(VMS)
#define DID_IT
   unsigned short channel;
   unsigned int   mode[3];
   unsigned int   speed;
   unsigned int   parity;
   unsigned int   fill;
   int            status;
   unsigned short iosb[4];

   channel = decc$get_sdc(fd);
   if (channel == 0)
      return;

   status = sys$qiow(0, channel, IO$_SENSEMODE, iosb, NULL, NULL,
                     mode, sizeof mode, 0, 0, 0, 0);
   if (! SUCCESSFUL(status) || ! SUCCESSFUL(iosb[0])
       || (mode[0] & 0xFF) != DC$_TERM)
      return;

   speed  = iosb[1];
   fill   = iosb[2];
   parity = iosb[3];


   mode[2] |= TT2M_SETSPEED;
   speed    = cb;

   status = sys$qiow(0, channel, IO$_SETMODE, iosb, NULL, NULL,
                     mode, sizeof mode, speed, fill, parity, 0);
#endif /* VMS */

#if defined(win32)
# define DID_IT
   BOOL       success;
   DCB        dcb;

   success = GetCommState((HANDLE) fd, &dcb);
   if (! success)
      return;

   dcb.BaudRate = cb;

   success = SetCommState((HANDLE) fd, &dcb);
#endif /* win32 */

#if defined(DID_IT)
# undef DID_IT
#else /* default */
   TERM_CHARS ntio;

   bzero(&ntio, sizeof ntio); /* purify uninit mem read fix */
   if (GET_TERM(fd, &ntio) < 0)
      return;

#if defined(__osf__) || defined(openbsd)
   ntio.c_ispeed = cb;
   ntio.c_ospeed = cb;
#else /* not osf nor openbsd */
   ntio.c_cflag &= ~CBAUD;
   ntio.c_cflag |= cb;
#endif /* osf or openbsd */

   SET_TERM(fd, &ntio);
#endif /* default */
}

/*
 * k_set_csize
 */
void k_set_csize(
   int fd,
   int cz)
{
#if defined(VMS)
#define DID_IT
   unsigned short channel;
   unsigned int   mode[3];
   unsigned int   speed;
   unsigned int   parity;
   unsigned int   fill;
   int            status;
   unsigned short iosb[4];

   channel = decc$get_sdc(fd);
   if (channel == 0)
      return;

   status = sys$qiow(0, channel, IO$_SENSEMODE, iosb, NULL, NULL,
                     mode, sizeof mode, 0, 0, 0, 0);
   if (! SUCCESSFUL(status) || ! SUCCESSFUL(iosb[0])
       || (mode[0] & 0xFF) != DC$_TERM)
      return;

   speed  = iosb[1];
   fill   = iosb[2];
   parity = iosb[3];

   mode[1] &= ~TTM_EIGHTBIT;
   mode[1] |= cz;
   status = sys$qiow(0, channel, IO$_SETMODE, iosb, NULL, NULL,
                     mode, sizeof mode, speed, fill, parity, 0);
#endif /* VMS */

#if defined(win32)
# define DID_IT
   BOOL       success;
   DCB        dcb;
   DWORD      size;

   success = GetCommState((HANDLE) fd, &dcb);
   if (! success)
      return;

   dcb.ByteSize = cz;

   success = SetCommState((HANDLE) fd, &dcb);
#endif /* win32 */

#if defined(DID_IT)
# undef DID_IT
#else /* default */
   TERM_CHARS ntio;

   bzero(&ntio, sizeof ntio); /* purify uninit mem read fix */

   if (GET_TERM(fd, &ntio) < 0)
      return;

   ntio.c_cflag &= ~CSIZE;
   ntio.c_cflag |= cz;

   SET_TERM(fd, &ntio);
#endif /* default */
}

/*
 * k_set_parity
 */
void k_set_parity(
   int fd,
   int p)
{
#if defined(VMS)
#define DID_IT
   unsigned short channel;
   unsigned int   mode[3];
   unsigned int   speed;
   unsigned int   parity;
   unsigned int   fill;
   int            status;
   unsigned short iosb[4];

   channel = decc$get_sdc(fd);
   if (channel == 0)
      return;

   status = sys$qiow(0, channel, IO$_SENSEMODE, iosb, NULL, NULL,
                     mode, sizeof mode, 0, 0, 0, 0);
   if (! SUCCESSFUL(status) || ! SUCCESSFUL(iosb[0])
       || (mode[0] & 0xFF) != DC$_TERM)
      return;

   speed  = iosb[1];
   fill   = iosb[2];
   parity = iosb[3];

   parity &= ~(TTM_PARITY | TTM_ODD);
   parity |= p | TTM_ALTRPAR;
   status = sys$qiow(0, channel, IO$_SETMODE, iosb, NULL, NULL,
                     mode, sizeof mode, speed, fill, parity, 0);
#endif /* VMS */

#if defined(win32)
# define DID_IT
   BOOL       success;
   DCB        dcb;
   DWORD      size;

   success = GetCommState((HANDLE) fd, &dcb);
   if (! success)
      return;

   dcb.fParity = (p != NOPARITY);
   dcb.Parity  = p;

   success = SetCommState((HANDLE) fd, &dcb);
#endif /* win32 */

#if defined(DID_IT)
# undef DID_IT
#else /* default */
   TERM_CHARS ntio;

   bzero(&ntio, sizeof ntio); /* purify uninit mem read fix */
   if (GET_TERM(fd, &ntio) < 0)
      return;

   ntio.c_cflag &= ~(PARENB|PARODD);
   ntio.c_cflag |= p;

   SET_TERM(fd, &ntio);
#endif /* default */
}

/*
 * k_fset_echo
 */
void k_fset_echo(
   int fd)
{
#if defined(VMS)
# define DID_IT
   unsigned short channel;
   unsigned int   mode[3];
   unsigned int   speed;
   unsigned int   parity;
   unsigned int   fill;
   int            status;
   unsigned short iosb[4];

   channel = decc$get_sdc(fd);
   if (channel == 0)
      return;

   status = sys$qiow(0, channel, IO$_SENSEMODE, iosb, NULL, NULL,
                     mode, sizeof mode, 0, 0, 0, 0);
   if (! SUCCESSFUL(status) || ! SUCCESSFUL(iosb[0])
       || (mode[0] & 0xFF) != DC$_TERM)
      return;

   speed  = iosb[1];
   fill   = iosb[2];
   parity = iosb[3];

   mode[1] &= ~TTM_NOECHO;
   status = sys$qiow(0, channel, IO$_SETMODE, iosb, NULL, NULL,
                     mode, sizeof mode, speed, fill, parity, 0);
#endif /* VMS */

#if defined(win32)
# define DID_IT
#endif /* win32 */

#if defined(DID_IT)
# undef DID_IT
#else /* default */
   TERM_CHARS term;

   if (GET_TERM(fd, &term) < 0)
      return;

   term.c_lflag |= ECHO;

   SET_TERM(fd, &term);
#endif /* default */
}

/*
 * k_fset_no_echo
 */
void k_fset_no_echo(
   int fd)
{
#if defined(VMS)
# define DID_IT
   unsigned short channel;
   unsigned int   mode[3];
   unsigned int   speed;
   unsigned int   parity;
   unsigned int   fill;
   int            status;
   unsigned short iosb[4];

   channel = decc$get_sdc(fd);
   if (channel == 0)
      return;

   status = sys$qiow(0, channel, IO$_SENSEMODE, iosb, NULL, NULL,
                     mode, sizeof mode, 0, 0, 0, 0);
   if (! SUCCESSFUL(status) || ! SUCCESSFUL(iosb[0])
       || (mode[0] & 0xFF) != DC$_TERM)
      return;

   speed  = iosb[1];
   fill   = iosb[2];
   parity = iosb[3];

   mode[1] |= TTM_NOECHO;
   mode[2] &= ~TT2M_LOCALECHO;
   status = sys$qiow(0, channel, IO$_SETMODE, iosb, NULL, NULL,
                     mode, sizeof mode, speed, fill, parity, 0);
#endif /* VMS */

#if defined(win32)
# define DID_IT
#endif /* win32 */

#if defined(DID_IT)
# undef DID_IT
#else /* default */
   TERM_CHARS term;

   if (GET_TERM(fd, &term) < 0)
      return;

   term.c_lflag &= ~ECHO;

   SET_TERM(fd, &term);
#endif /* default */
}

/*
 * put the tty in non blocking, this is for climd tty device
 */
void TTYNonBlocking (
   int tty_fd)
{
#ifndef win32
   fcntl(tty_fd,F_SETFL,fcntl(tty_fd,F_GETFL,0)|O_NONBLOCK);
#endif
}

/*
 * TTYRawMode
 * put the tty in raw mode, but has XON (output flowconsole)
 * and XOFF (input flow control) and IXANY (any char will restart output *
 */
void TTYRawMode(
   int fd)
{
#if defined(VMS)
#define DID_IT
   unsigned short channel;
   unsigned int   mode[3];
   unsigned int   speed;
   unsigned int   parity;
   unsigned int   fill;
   int            status;
   unsigned short iosb[4];

   channel = decc$get_sdc(fd);
   if (channel == 0)
      return;

   status = sys$qiow(0, channel, IO$_SENSEMODE, iosb, NULL, NULL,
                     mode, sizeof mode, 0, 0, 0, 0);
   if (! SUCCESSFUL(status) || ! SUCCESSFUL(iosb[0])
       || (mode[0] & 0xFF) != DC$_TERM)
      return;

   speed  = iosb[1];
   fill   = iosb[2];
   parity = iosb[3];

   mode[1] |= TTM_NOECHO;
   mode[2]  = (current_mode[2] & ~TT2M_LOCALECHO) | TT2M_PASTHRU;
   status = sys$qiow(0, channel, IO$_SETMODE, iosb, NULL, NULL,
                     mode, sizeof mode, speed, fill, parity, 0);
#endif /* VMS */

#if defined(win32)
# define DID_IT
   setmode(fd, O_BINARY);
#endif /* win32 */

#if defined(DID_IT)
# undef DID_IT
#else /* default */
   TERM_CHARS ntty;

   raw_fd = -1;

   if (GET_TERM(fd, &cooked_mode) != 0)
      return;

   raw_fd = fd;
   ntty   = cooked_mode;

   ntty.c_lflag &= ~(ECHO | ECHOE | ECHOK | ICANON | ISIG);
   ntty.c_iflag  =  (IXON | IXOFF | IXANY);
   ntty.c_oflag  =  0;
   ntty.c_cc[VMIN]  =  1; /* 1 character in queue will send it back */
   ntty.c_cc[VTIME]  =  0; /* no delay before sending thatback */

   SET_TERM(fd, &ntty);
#endif /* default */
}

/*
 * put_tty_in_raw_input_mode
 *
 * Put the tty in raw input mode, but has XON (output flowconsole)
 * and XOFF (input flow control) and IXANY (any char will restart output.
 */
void put_tty_in_raw_input_mode(void)
{
#if defined(VMS)
#define DID_IT
   unsigned short channel;
   int            status;
   unsigned short iosb[4];
#endif /* VMS */

#if defined(win32)
# define DID_IT
#endif /* win32 */

#if defined(DID_IT)
# undef DID_IT
#else /* default */
#endif /* default */

   if (TTYSetup())
      return;

#if defined(VMS)
#define DID_IT
   current_mode[1] |= TTM_NOECHO;
   current_mode[2]  = (old_mode[2] & ~TT2M_LOCALECHO) | TT2M_PASTHRU;
   status = sys$qiow(0, channel, IO$_SETMODE, iosb, NULL, NULL,
		     current_mode, sizeof current_mode, current_speed,
		     current_fill, current_parity, 0);
#endif /* VMS */

#if defined(win32)
# define DID_IT
   current_mode = 0;
   SetConsoleMode(ttyin_handle, current_mode);
#endif /* win32 */

#if defined(DID_IT)
# undef DID_IT
#else /* default */
   current_mode.c_lflag &= ~(ECHO | ECHOE | ECHOK | ICANON | ISIG);
   current_mode.c_cc[VMIN]  =  1; /* 1 character in queue will send it back */
   current_mode.c_cc[VTIME]  =  0; /* no delay before sending thatback */

   SET_TERM(0, &current_mode);
#endif /* default */
}

/*
 * get_tty_cntl_chars
 *
 * Return the control characters set up for the terminal
 */
void get_tty_cntl_chars(
   char* eolChar,
   char* eraseChar,
   char* killChar,
   char* eofChar)
{
  if (TTYSetup())
     return;

#if defined(VMS)
#define DID_IT
  *eolChar = '\r';
  *eraseChar = '\177';
  *killChar = '\025';
  *eofChar = '\032';
#endif /* VMS */

#if defined(win32)
# define DID_IT
  *eolChar = '\r';
  *eraseChar = '\010';
  *killChar = '\025';
  *eofChar = '\032';
#endif /* win32 */

#if defined(DID_IT)
# undef DID_IT
#else /* default */
  *eolChar = '\n';
  *eraseChar = current_mode.c_cc[VERASE];
  *killChar = current_mode.c_cc[VKILL];
  *eofChar = old_mode.c_cc[VEOF];	/* Must use old mode because VMIN
					   overwrites VEOF */
#endif /* default */
}

/*
 * get_tty_rows
 *
 * Return the number of rows on the terminal
 */
unsigned int get_tty_rows(void)
{
#if defined(VMS)
#define DID_IT
   int            status;
   unsigned short iosb[4];
   unsigned int   mode[3];
#endif /* VMS */

#if defined(win32)
# define DID_IT
   CONSOLE_SCREEN_BUFFER_INFO csbi;
#endif /* win32 */

#if defined(DID_IT)
# undef DID_IT
#else /* default */
#endif /* default */

#if defined(VMS)
#define DID_IT
   status = sys$qiow(0, ttyout_channel, IO$_SENSEMODE, iosb, NULL, NULL,
                     mode, sizeof mode, 0, 0, 0, 0);
   if (SUCCESSFUL(status) && SUCCESSFUL(iosb[0])
       && (mode[0] & 0xFF) == DC$_TERM)
      return (mode[1] >> 24) & 0xFF;

   return 24;
#endif /* VMS */

#if defined(win32)
# define DID_IT
   GetConsoleScreenBufferInfo(ttyout_handle, &csbi);
   return csbi.dwSize.X;
#endif /* win32 */

#if defined(DID_IT)
# undef DID_IT
#else /* default */
   return nrow;
#endif /* default */
}

/*
 * k_set_stopbits
 */
void k_set_stopbits(
   int fd,
   int stop)
{
#if defined(VMS)
/* BUG need code here for vms */
#define DID_IT
#endif

#if defined(win32)
/* BUG need code here for win32 */
# define DID_IT
#endif

#ifdef outxxx
#if defined(VMS)
#define DID_IT
   unsigned short channel;
   unsigned int   mode[3];
   unsigned int   speed;
   unsigned int   parity;
   unsigned int   fill;
   int            status;
   unsigned short iosb[4];

   channel = decc$get_sdc(fd);
   if (channel == 0)
      return;

   status = sys$qiow(0, channel, IO$_SENSEMODE, iosb, NULL, NULL,
                     mode, sizeof mode, 0, 0, 0, 0);
   if (! SUCCESSFUL(status) || ! SUCCESSFUL(iosb[0])
       || (mode[0] & 0xFF) != DC$_TERM)
      return;

   speed  = iosb[1];
   fill   = iosb[2];
   parity = iosb[3];

   mode[1] &= ~TTM_EIGHTBIT;
   mode[1] |= cz;
   status = sys$qiow(0, channel, IO$_SETMODE, iosb, NULL, NULL,
                     mode, sizeof mode, speed, fill, parity, 0);
#endif /* VMS */

#if defined(win32)
# define DID_IT
   BOOL       success;
   DCB        dcb;
   DWORD      size;

   success = GetCommState((HANDLE) fd, &dcb);
   if (! success)
      return;

   dcb.ByteSize = cz;

   success = SetCommState((HANDLE) fd, &dcb);
#endif /* win32 */
#endif

#if defined(DID_IT)
# undef DID_IT
#else /* default */
   TERM_CHARS ntio;

   bzero(&ntio, sizeof ntio); /* purify uninit mem read fix */

   if (GET_TERM(fd, &ntio) < 0)
      return;

   ntio.c_cflag &= ~CSTOPB;
   ntio.c_cflag |= stop;

   SET_TERM(fd, &ntio);
#endif /* default */
}
