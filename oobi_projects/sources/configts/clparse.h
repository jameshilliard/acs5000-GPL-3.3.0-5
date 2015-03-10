/******************************************************************************

          COPYRIGHT 2000 - 2002 Ki NETWORKS, Inc.  ALL RIGHTS RESERVED

*******************************************************************************

  RCS Revision Control Information:
   @(#)$Header: /usr/cvsroot/oobi_projects/sources/configts/clparse.h,v 1.1.1.1 2005/09/01 02:36:37 edson Exp $

   Revision:        $Revision: 1.1.1.1 $
   Created From:    $Source: /usr/cvsroot/oobi_projects/sources/configts/clparse.h,v $
   Last Updated:    $Date: 2005/09/01 02:36:37 $
   Current State:   $State: Exp $
   Checked Out By:  $Locker:  $

*******************************************************************************

                              clparse.h

   Description:

      This file defines the command line parsing API.

   Change Log:

     Date    Who  Rev   Reason for update
   --------- --- ------ -------------------------------------------------------
    9-Oct-02 bns  1.6   Add clexec(), clexit(), clscreensize(), and clpause()
    2-May-02 bns  1.5   Add CL_GOTO
    2-Feb-01 bns  1.4   Add CL_HOSTNAME token type
   15-Dec-00 bns  1.3   Add CL_INDEXED_KEYWORD token type, add index to token
   14-Jan-00 bns  1.2   Add clparse()
   27-Jan-00 bns  1.1   1.0.0: create from scratch

******************************************************************************/

#ifndef CLPARSE_H
#define CLPARSE_H  1

#include <stdio.h>

#define CL_END			0
#define CL_KEYWORD		1
#define CL_PATH			2
#define CL_NUMBER		3
#define CL_STRING		4
#define CL_EOL			5
#define CL_INDEXED_KEYWORD	6
#define CL_HOSTNAME		7
#define CL_GOTO			8

#define CL_HIDDEN	(1 << 0)
#define CL_NOREPEAT	(1 << 1)
#define CL_SAVEARG	(1 << 2)

typedef struct _cl_element
{
   unsigned int type;
   void*        param;
   void*        next;
   char*        help_text;
   unsigned int flags;
} CL_ELEMENT;

typedef CL_ELEMENT CL_STATE[];

typedef struct _cl_arg
{
   void*        userData;
   unsigned int index;
   union
   {
      unsigned int int_val;
      char*        string_val;
   } u;
} CL_ARG;

typedef void (CL_CALLBACK)(unsigned int nargs, CL_ARG* args);

#define CL_STATE_END	{CL_END, NULL, NULL}

#if defined __cplusplus
extern "C" {
#endif

/*
 * argparse
 *
 * Parse and execute an arg array.
 */
extern void argparse(
   int      argc,
   char*    argv[],
   CL_STATE begin_state,
   int      print_help);

/*
 * clparse
 *
 * Parse and execute a command line.
 */
extern void clparse(
   char*    cmdline,
   CL_STATE begin_state);

/*
 * clread
 *
 * Read a command (possibly multi-line).
 * Return 0 if successful, -1 if not
 */
extern int clread(
   FILE* infp,			/* File to read the command from */
   FILE* outfp,			/* File to print the prompts to or NULL */
   char* buffer,		/* Buffer to read the command into */
   int   buffer_size,		/* Size of the buffer */
   char* prompt,		/* Command prompt */
   char* continuation_prompt);	/* Continuation line prompt */

/*
 * clexec
 *
 * Get command line input, parse, and execute.
 * Repeat until end-of-file on the input or clexit() is called by
 * a processing function.
 */
extern void clexec(
   int      max_size,			/* Maximum command size */
   char*    prompt,			/* Command prompt */
   char*    continuation_prompt,	/* Continuation line prompt */
   CL_STATE begin_state);		/* Beginning parsing state */

/*
 * clexit
 *
 * Signal clexec to exit.
 */
extern void clexit(void);

/*
 * clscreenrows
 *
 * Return the number of rows on the terminal.
 */
extern int clscreenrows(void);

/*
 * clpause
 *
 * Print a message and wait for the user to press a key.
 * Return the key that was pressed, or 0 if no message was printed.
 */
extern char clpause(
   char* message);

#if defined __cplusplus
}
#endif

#endif /* CLPARSE_H */
