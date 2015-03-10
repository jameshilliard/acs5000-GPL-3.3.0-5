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
   @(#)$Header: /usr/cvsroot/oobi_projects/sources/configts/clparse.c,v 1.1.1.1 2005/09/01 02:36:37 edson Exp $

   Revision:        $Revision: 1.1.1.1 $
   Created From:    $Source: /usr/cvsroot/oobi_projects/sources/configts/clparse.c,v $
   Last Updated:    $Date: 2005/09/01 02:36:37 $
   Current State:   $State: Exp $
   Checked Out By:  $Locker:  $

*******************************************************************************

                                clparse.c

   Description:

      This file contains the function clparse.

   Change Log:

     Date    Who  Rev   Reason for update
   --------- --- ------ -------------------------------------------------------
   17-Oct-02 bns  1.4   Fix PPC compile warning
    9-Oct-02 bns  1.16  Allow quoting in interactive commands
    9-Oct-02 bns  1.16  Call read_tty() rather than read()
    9-Oct-02 bns  1.15  Add clexec(), clexit(), clscreensize(), and clpause()
    2-May-02 bns  1.14  Add CL_GOTO
   17-Sep-01 bns  1.13  Fix unrecognized commands
   14-Sep-01 bns  1.12  Fix memory leak
   14-Sep-01 bns  1.11  Fix buffer underflow (?)
    3-Jul-01 bns  1.10  Fix CL_HOSTNAME that crashes
    2-Jul-01 bns  1.9   Declare kstrchr()
    2-Jul-01 bns  1.8   Include kisocket.h
    2-Jul-01 bns  1.7   Change CL_HOSTNAME to not try to translate the host
                        name, just to check its syntax
   20-Mar-01 bns  1.6   Recognize escaped characters outside of strings
    2-Feb-01 bns  1.5   Add CL_HOSTNAME token type
   15-Dec-00 bns  1.4   Add CL_INDEXED_KEYWORD token type, add index to token
   14-Jan-00 bns  1.3   Add clparse()
   25-Feb-00 bns  1.2   Define strncasecmp for Windows
   25-Feb-00 bns  1.1   1.0.0: create from scratch

******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "clparse.h"

#define MAXHOSTNAMELEN 128
#define ACTION_EXECUTE	1
#define ACTION_HELP	2
#define ACTION_COMPLETE	3

#define NUM_ELEMENTS(x)	(sizeof (x)/sizeof (x)[0])

#if defined(win32)
# define strncasecmp(x,y,z)	strnicmp(x,y,z)
#endif /* win32 */

#ifndef TRUE
# define TRUE 1
#endif /* not TRUE */

#ifndef FALSE
# define FALSE 0
#endif /* not FALSE */

/**********************
*  Macros 	      *
**********************/

/********
* Types *
********/

/**************************
* Externally Defined Data *
**************************/

/*****************************
* Externally Referenced Data *
*****************************/

/***************
* Private Data *
***************/
static int        done;
static char*      completeString;
static int        completeLength;
static int        isIncomplete;
static int        isInteractive;

/******************
* Forward Modules *
******************/

/*******************
* External Modules *
*******************/
extern void put_tty_in_raw_input_mode(void);

extern void put_tty_in_cooked_mode(void);

extern void get_tty_cntl_chars(
   char* eolChar,
   char* eraseChar,
   char* killChar,
   char* eofChar);

extern unsigned int get_tty_rows(void);

extern int read_tty(
   char* buffer,
   int   size);


/*
 * PrintHelp
 *
 * Print the help text for a state, given an initial string to match.
 */
static void PrintHelp(
   CL_STATE     currentState,
   char*        initialString,
   unsigned int historyDepth,
   CL_ELEMENT** commandHistory)
{
   CL_ELEMENT*  thisState;
   char*        cp;
   int          len;
   unsigned int i;
   static char  buffer[128];

   len = strlen(initialString);

   for (thisState = currentState; thisState->type != CL_END; ++thisState)
   {
      while (thisState->type == CL_GOTO)
	 thisState = thisState->next;

      if (thisState->flags & CL_HIDDEN)
	 continue;

      if (thisState->flags & CL_NOREPEAT)
      {
	 for (i = 0; i < historyDepth; ++i)
	    if (thisState == commandHistory[i])
	       break;

	 if (i < historyDepth)
	    continue;
      }

      cp = NULL;

      switch (thisState->type)
      {
      case CL_KEYWORD:
	 if (len == 0
	     || strncasecmp((char*) thisState->param, initialString, len) == 0)
	    cp = (char*) thisState->param;
	 break;

      case CL_PATH:
	 cp = "<file name>";
	 break;

      case CL_NUMBER:
	 cp = "<unsigned number>";
	 break;

      case CL_STRING:
	 cp = "<string>";
	 break;

      case CL_EOL:
         if (len == 0)
	    cp = "<return>";
	 break;

      case CL_INDEXED_KEYWORD:
	 if (len == 0
	     || strncasecmp((char*) thisState->param, initialString, len) == 0)
	 {
	    strcpy(buffer, (char*) thisState->param);
	    strcat(buffer, "<n>");
	    cp = buffer;
	 }

	 break;
      }

      if (cp)
         printf("   %-25s   %s\n", cp, thisState->help_text);
   }
}

/*
 * AutoComplete
 *
 * Automatically complete a command as far as possible.
 * Return FALSE if there is one completion, TRUE if not. 
 */
static int AutoComplete(
   CL_STATE     currentState,
   char*        initialString,
   unsigned int historyDepth,
   CL_ELEMENT** commandHistory,
   char**       matchingString,
   int*         matchingLength)
{
   CL_ELEMENT*  thisState;
   char*        cp;
   int          len;
   unsigned int i;
   char         buffer[128];
   char*        completionString;
   int          completionLength;
   int          numMatches;

   len        = strlen(initialString);
   numMatches = 0;

   for (thisState = currentState; thisState->type != CL_END; ++thisState)
   {
      while (thisState->type == CL_GOTO)
	 thisState = thisState->next;

      if (thisState->flags & CL_HIDDEN)
	 continue;

      if (thisState->flags & CL_NOREPEAT)
      {
	 for (i = 0; i < historyDepth; ++i)
	    if (thisState == commandHistory[i])
	       break;

	 if (i < historyDepth)
	    continue;
      }

      cp = NULL;

      switch (thisState->type)
      {
      case CL_KEYWORD:
	 if (len == 0
	     || strncasecmp((char*) thisState->param, initialString, len) == 0)
	 {
	    if (++numMatches == 1)
	    {
	       completionString = (char*) thisState->param + len;
	       completionLength = strlen(completionString);
	    }
	    else
	    {
	       for (i = 0; i < completionLength; ++i)
		  if (*((char*) thisState->param + len + i)
		      != completionString[i])
		     break;

	       completionLength = i;
	    }
	 }
	       
	 break;

      case CL_PATH:
	 *matchingString = "";
	 *matchingLength = 0;
	 return TRUE;

      case CL_NUMBER:
	 *matchingString = "";
	 *matchingLength = 0;
	 return TRUE;

      case CL_STRING:
	 *matchingString = "";
	 *matchingLength = 0;
	 return TRUE;

      case CL_EOL:
	 *matchingString = "";
	 *matchingLength = 0;
	 return (len != 0);

      case CL_INDEXED_KEYWORD:
	 if (len == 0
	     || strncasecmp((char*) thisState->param, initialString, len) == 0)
	 {
	    if (++numMatches == 1)
	    {
	       completionString = (char*) thisState->param + len;
	       completionLength = strlen(completionString);
	       ++numMatches;
	    }
	    else
	    {
	       for (i = 0; i < completionLength; ++i)
		  if (*((char*) thisState->param + len + i)
		      != completionString[i])
		     break;

	       completionLength = i;
	    }
	 }

	 break;

      case CL_HOSTNAME:
	 *matchingString = "";
	 *matchingLength = 0;
	 return TRUE;
      }
   }

   if (numMatches > 0)
   {
      *matchingString = completionString;
      *matchingLength = completionLength;
   }

   return (numMatches != 1);
}

/*
 * ParseArgs
 *
 * Parse and execute an arg array.
 */
static void ParseArgs(
   int      argc,
   char*    argv[],
   CL_STATE begin_state,
   int      action,
   int      isEndInWord)
{
   int          nargs;
   CL_ELEMENT*  currentState;
   CL_ELEMENT*  thisState;
   CL_ELEMENT*  lastState;
   CL_ELEMENT*  firstMatch;
   CL_ARG       args[100];
   char*        next_char;
   int          len;
   unsigned int i;
   int          argi;
   int          limit;
   unsigned int historyDepth;
   CL_ELEMENT*  commandHistory[100];
   unsigned int index;
   int          isAmbiguous;
   unsigned int val;
	char *x, *t;

   currentState = begin_state;
   lastState    = NULL;
   historyDepth = 0;
   nargs        = 0;
   index        = 0;

   limit = argc;
   if (action != ACTION_EXECUTE && isEndInWord)
      limit = argc - 1;

   for (argi = 0; argi < limit; ++argi)
   {
      len          = strlen(argv[argi]);
      firstMatch   = NULL;
      isAmbiguous  = FALSE;

      for (thisState = currentState; thisState->type != CL_END; ++thisState)
      {
	 if (thisState->type == CL_GOTO)
	 {
	    firstMatch = thisState;
	    currentState = thisState->next;
	    break;
	 }

	 i = historyDepth;

	 if (thisState->flags & CL_NOREPEAT)
	    for (i = 0; i < historyDepth; ++i)
	       if (thisState == commandHistory[i])
		  break;

	 if (i >= historyDepth)
	 {
	    switch (thisState->type)
	    {
	    case CL_KEYWORD:
	       if (len > 0
		   && (strncasecmp(argv[argi], (char*) thisState->param, len)
		       == 0))
		 {
		   if (firstMatch == NULL)
		     firstMatch = thisState;
		   else if (firstMatch->type == CL_KEYWORD
			    || firstMatch->type == CL_INDEXED_KEYWORD)
		     isAmbiguous = TRUE;
		 }
		     
	       break;

	    case CL_PATH:
	       if (len > 0)
	       {
		  if (firstMatch == NULL)
		  {
		     args[nargs].u.string_val = argv[argi];
		     firstMatch = thisState;
		  }
	       }

	       break;

	    case CL_NUMBER:
	       if (len > 0)
	       {
		  val = strtoul(argv[argi], &next_char, 10);
		  if (*next_char == '\0')
		  {
		     if (firstMatch == NULL)
		     {
		        firstMatch = thisState;
			args[nargs].u.int_val = val;
		     }
		  }
	       }

	       break;

	    case CL_STRING:
	       if (firstMatch == NULL)
	       {
				/* If there is ? after parameter that expects a string,
               output help message and don't accept ? as the string. */
				if (!strcmp(argv[argi],"?"))
            	break;

				args[nargs].u.string_val = argv[argi];
				firstMatch = thisState;
	       }
	       break;

	    case CL_INDEXED_KEYWORD:
	       next_char = argv[argi] + len - 1;

	       for (;;)
	       {
		  if (next_char <= argv[argi] + 1)
		     break;

		  if (! isdigit(*next_char))
		     break;

		  if (strncasecmp(argv[argi], (char*) thisState->param,
				  next_char - argv[argi])
		      == 0)
		  {
		     if (firstMatch == NULL)
		     {
		        firstMatch = thisState;
			index = strtoul(next_char, NULL, 10);
		     }
		     else if (firstMatch->type == CL_KEYWORD
			      || firstMatch->type == CL_INDEXED_KEYWORD)
		        isAmbiguous = TRUE;

		     break;
		  }

		  --next_char;
	       }

	       break;

	    case CL_HOSTNAME:
	       if (len > 0)
	       {
                  if (len < MAXHOSTNAMELEN)
                  {
                     for (i = 0; i < len; ++i)
                        if (strchr("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-.",
                                   argv[argi][i])
                           == NULL)
                           break;

                     if (i >= len)
		     {
			if (firstMatch == NULL)
			{
			   firstMatch = thisState;
			   args[nargs].u.string_val = argv[argi];
			}
		     }
                  }
	       }

	       break;
	    }
	 }
      }

      if (firstMatch == NULL)
      {
	 fprintf(stderr, "** Unexpected command \"%s\"; expected words are:\n",
		 argv[argi]);
	 PrintHelp(currentState, "", historyDepth, commandHistory);
	 return;
      }

      if (isAmbiguous)
      {
	 fprintf(stderr,
		 "** Ambiguous command \"%s\"; possible matches are:\n",
		 argv[argi]);
	 PrintHelp(currentState, argv[argi], historyDepth, commandHistory);
	 return;
      }

      lastState = firstMatch;
      currentState = (CL_ELEMENT*) firstMatch->next;

      if (firstMatch->type != CL_KEYWORD
	  && firstMatch->type != CL_INDEXED_KEYWORD)
      {
	 args[nargs].userData = firstMatch->param;
	 args[nargs].index    = index;
	 index                = 0;

	 ++nargs;
	 if (nargs >= NUM_ELEMENTS(args))
	 {
	    fprintf(stderr, "** Command is too long\n");
	    return;
	 }
      }
      else if (firstMatch->flags & CL_SAVEARG)
      {
	 args[nargs].u.string_val = (char*) firstMatch->param;
	 args[nargs].userData     = 0;

	 if (firstMatch->type == CL_INDEXED_KEYWORD)
	    args[nargs].index = index;

	 index = 0;

	 ++nargs;
	 if (nargs >= NUM_ELEMENTS(args))
	 {
	    fprintf(stderr, "** Command is too long\n");
	    return;
	 }
      }
      else if (firstMatch->type == CL_KEYWORD)
	 index = 0;

      if ((firstMatch->flags & CL_NOREPEAT)
	  && historyDepth < NUM_ELEMENTS(commandHistory))
	commandHistory[historyDepth++] = firstMatch;
   }

   if (action == ACTION_HELP)
   {
      if (isEndInWord)
         PrintHelp(currentState, argv[argc - 1], historyDepth, commandHistory);
      else if (lastState != NULL)
         PrintHelp(lastState->next, "", historyDepth, commandHistory);
      else
         PrintHelp(begin_state, "", historyDepth, commandHistory);
   }
   else if (action == ACTION_COMPLETE)
   {
      if (isEndInWord)
         isIncomplete = AutoComplete(currentState, argv[argc - 1],
				     historyDepth, commandHistory,
				     &completeString, &completeLength);
      else if (lastState != NULL)
         isIncomplete = AutoComplete(lastState->next, "", historyDepth,
				     commandHistory, &completeString,
				     &completeLength);
      else
         isIncomplete = AutoComplete(begin_state, "", historyDepth,
				     commandHistory, &completeString,
				     &completeLength);
   }
   else
   {
      for (thisState = currentState; thisState->type != CL_END; ++thisState)
      {
	 while (thisState->type == CL_GOTO)
	    thisState = thisState->next;

	 if (thisState->type == CL_EOL)
	    break;
      }

      if (thisState->type == CL_END)
      {
	 fprintf(stderr, "** Incomplete command - type \"");
	 for (argi = 0; argi < argc; ++argi)
	    fprintf(stderr, "%s ", argv[argi]);
	 fprintf(stderr, "?\" for help\n");
      }
      else
	 (*(CL_CALLBACK*) thisState->param)(nargs, args);
   }
}

/*
 * argparse
 *
 * Parse and execute an arg array.
 */
void argparse(
   int      argc,
   char*    argv[],
   CL_STATE begin_state,
   int      print_help)
{
   ParseArgs(argc, argv, begin_state,
	     print_help ? ACTION_HELP : ACTION_EXECUTE, FALSE);
}

/*
 * ParseIntoArgv
 *
 * Parse a command line into arguments.
 * Return TRUE if the command ends in a word, false otherwise.
 */
static int ParseIntoArgv(
   char* command,
   char* argv[],
   int   argv_size,
   int*  nargs)
{
   char* begin;
   char* end;
   char  quote_char;
   int   argc;
   int   isInWord;

   argc     = 0;
   isInWord = FALSE;

   for (end = command; *end; )
   {
      isInWord = FALSE;

      /* Find the start of the next token */
      for (begin = end;
	   *begin && isascii(*begin) && isspace(*begin);
	   ++begin)
      {}

      if (*begin == '\0')
	 break;

      /* Find the end of the next token */
      quote_char = '\0';

      end = begin;
      while (*end)
      {
	 if (quote_char)
	 {
	    if (*end == quote_char)
	    {
	       strcpy(end, end + 1);
	       quote_char = '\0';
	       continue;
	    }

            if (*end == '\\')
               strcpy(end, end + 1);
	 }
	 else
	 {
	    if (*end == '"' || *end == '\'')
	    {
	       quote_char = *end;
	       strcpy(end, end + 1);
	       continue;
	    }
            else if (*end == '\\')
               strcpy(end, end + 1);
	    else if (isascii(*end) && isspace(*end))
	       break;
	 }

	 ++end;
      }

      if (*end)
	 *end++ = '\0';
      else
	 isInWord = TRUE;

      if (argc >= argv_size)
      {
	 isInWord = FALSE;
	 break;
      }

      argv[argc++] = begin;
   }

   *nargs = argc;

   return isInWord;
}

/*
 * ParseCommandLine
 *
 * Parse and execute a command line.
 */
static void ParseCommandLine(
   char*    cmdline,
   CL_STATE begin_state)
{
   char* command;
   int   argc;
   char* argv[100];
   int   isEndInWord;

   command = (char*) malloc(strlen(cmdline) + 1);
   if (command == NULL)
      exit(9);

   strcpy(command, cmdline);
   isEndInWord = ParseIntoArgv(command, argv, NUM_ELEMENTS(argv), &argc);
   ParseArgs(argc, argv, begin_state, ACTION_EXECUTE, FALSE);
   free(command);
}

/*
 * HelpCommandLine
 *
 * Parse a command line and print help for what comes next.
 */
static void HelpCommandLine(
   char*    cmdline,
   CL_STATE begin_state)
{
   char* command;
   int   argc;
   char* argv[100];
   int   isEndInWord;

   command = (char*) malloc(strlen(cmdline) + 1);
   if (command == NULL)
      exit(9);

   strcpy(command, cmdline);
   isEndInWord = ParseIntoArgv(command, argv, NUM_ELEMENTS(argv), &argc);
   ParseArgs(argc, argv, begin_state, ACTION_HELP, isEndInWord);
   free(command);
}

/*
 * FinishCommandLine
 *
 * Parse a command line and automatically finish what comes next.
 */
static void FinishCommandLine(
   char*    cmdline,
   CL_STATE begin_state)
{
   char* command;
   int   argc;
   char* argv[100];
   int   isEndInWord;

   command = (char*) malloc(strlen(cmdline) + 1);
   if (command == NULL)
      exit(9);

   strcpy(command, cmdline);
   isEndInWord = ParseIntoArgv(command, argv, NUM_ELEMENTS(argv), &argc);
   ParseArgs(argc, argv, begin_state, ACTION_COMPLETE, isEndInWord);
   free(command);
}

/*
 * clparse
 *
 * Parse and execute a command line.
 */
void clparse(
   char*    cmdline,
   CL_STATE begin_state)
{
   ParseCommandLine(cmdline, begin_state);
}

/*
 * clread
 *
 * Read a command (possibly multi-line).
 * Return 0 if successful, -1 if not
 */
int clread(
   FILE* infp,			/* File to read the command from */
   FILE* outfp,			/* File to print the prompts to or NULL */
   char* buffer,		/* Buffer to read the command into */
   int   buffer_size,		/* Size of the buffer */
   char* prompt,		/* Command prompt */
   char* continuation_prompt)	/* Continuation line prompt */
{
   int   len;
   char* cstatus;

   len = 0;

   if (outfp != NULL && prompt != NULL && isatty(fileno(infp)))
      fputs(prompt, outfp);

   for (;;)
   {
      /* Read the event summary */
      cstatus = fgets(buffer + len, buffer_size - len - 1, infp);
      if (cstatus == NULL)
      {
	 if (len == 0)
	    return -1;

	 break;
      }

      len = strlen(buffer);
      if (len == 0 || buffer[len - 1] != '\n')
	 break;

      buffer[--len] = '\0';
      if (len == 0 || buffer[len - 1] != '\\') break;
      buffer[len - 1] = '\n';

      if (outfp == NULL && continuation_prompt != NULL && isatty(fileno(infp)))
	 fputs(continuation_prompt, outfp);
   }

   return 0;
}

/*
 * cliexec
 *
 * Read a command (possibly multi-line), providing command-line editing,
 * and execute it. Repeat until end-of-file on input or clexit() is called.
 */
static void cliexec(
   char* buffer,		/* Buffer to read the command into */
   int   buffer_size,		/* Size of the buffer */
   char* prompt,		/* Command prompt */
   char* continuation_prompt,	/* Continuation line prompt */
   CL_STATE begin_state)	/* Beginning parsing state */
{
   int   len;
   char* cstatus;
   int   bytes;
   int   i;
   int   isNeedsRepaint;
   char  eolChar;
   char  eraseChar;
   char  killChar;
   char  eofChar;
   int   quotePos;

   put_tty_in_raw_input_mode();
   get_tty_cntl_chars(&eolChar, &eraseChar, &killChar, &eofChar);

   for (done = FALSE; ! done; )
   {
      fputs(prompt, stdout);
      len            = 0;
      isNeedsRepaint = FALSE;
      quotePos       = -1;

      for (;;)
      {
	 fflush(stdout);

	 bytes = read_tty(buffer + len, 1);
	 if (bytes <= 0 || buffer[len] == eofChar)
	 {
	    if (len == 0)
	    {
	       fputs("\n", stdout);
	       put_tty_in_cooked_mode();
	       return;
	    }

	    buffer[len] = '\0';
	    fputs("\n", stdout);
	    break;
	 }

	 if (buffer[len] == eraseChar)
	 {
	    if (isNeedsRepaint)
	    {
	       fputs("\n", stdout);
	       fputs(prompt, stdout);
	       buffer[len] = '\0';
	       fputs(buffer, stdout);
	       isNeedsRepaint = FALSE;
	    }
	    else if (len == 0)
	       fputc('\a', stdout);
	    else
	    {
	       if (len == quotePos)
		  quotePos = -1;

	       fputs("\b \b", stdout);
	       --len;
	    }

	    continue;
	 }

	 if (buffer[len] == killChar)
	 {
	    if (isNeedsRepaint)
	    {
	       fputs("\n", stdout);
	       fputs(prompt, stdout);
	       isNeedsRepaint = FALSE;
	    }
	    else if (len == 0)
	       fputc('\a', stdout);
	    else
	    {
	       for (i = 0; i < len; ++i)
		  fputs("\b", stdout);
	       for (i = 0; i < len; ++i)
		  fputs(" ", stdout);
	       for (i = 0; i < len; ++i)
		  fputs("\b", stdout);
	    }

	    len = 0;
	    quotePos = -1;
	    continue;
	 }

	 if (buffer[len] == eolChar)
	 {
	    if (quotePos != -1 && buffer[quotePos] == '\\')
	    {
	       fputs("\\\n", stdout);
	       fputs(continuation_prompt, stdout);
	       isNeedsRepaint = TRUE;
	       continue;
	    }

	    buffer[len] = '\0';
	    fputs("\n", stdout);
	    break;
	 }

	 if (quotePos == -1)
	 {
	    if (buffer[len] == '\t')
	    {
	       buffer[len] = '\0';
	       fputs("\n", stdout);
	       completeString = "";
	       completeLength = 0;
	       FinishCommandLine(buffer, begin_state);
	       len = strlen(buffer);
	       memcpy(buffer + len, completeString, completeLength);
	       buffer[len + completeLength] = '\0';
	       len += completeLength;

	       if (! isIncomplete && len > 0 && buffer[len - 1] != ' ')
	       {
		  buffer[len++] = ' ';
		  buffer[len] = '\0';
	       }

	       fputs(prompt, stdout);
	       fputs(buffer, stdout);

	       if (isIncomplete)
		  fputc('\a', stdout);

	       continue;
	    }

	    if (buffer[len] == '?')
	    {
	       buffer[len] = '\0';
	       fputs("?\n", stdout);
	       HelpCommandLine(buffer, begin_state);
	       fputs(prompt, stdout);
	       fputs(buffer, stdout);
	       continue;
	    }
	 }

	 if (isprint(buffer[len]))
	 {
            if (len >= buffer_size - 1)
	    {
	       buffer[len] = '\0';
	       fputs("\nCommand is too long\n", stdout);
               fputs(prompt, stdout);
	       fputs(buffer, stdout);
	    }
	    else
	    {
	       fputc(buffer[len], stdout);

	       if (quotePos == -1)
	       {
		  if (buffer[len] == '\\' || buffer[len] == '\''
		      || buffer[len] == '"')
		     quotePos = len;
	       }
	       else if (buffer[quotePos] == '\\'
			|| buffer[len] == buffer[quotePos])
		  quotePos = -1;

	       ++len;
	    }
	 }
	 else
	    fputc('\a', stdout);

      }

      fflush(stdout);
      ParseCommandLine(buffer, begin_state);
   }

   put_tty_in_cooked_mode();
}

/*
 * clexec
 *
 * Get command line input, parse, and execute.  Repeat until end-of-file
 * on the input or clexit() is called by a processing function.
 */
void clexec(
   int      max_size,			/* Maximum command size */
   char*    prompt,			/* Command prompt */
   char*    continuation_prompt,	/* Continuation line prompt */
   CL_STATE begin_state)		/* Beginning parsing state */
{
   char* buffer;	/* Buffer to read the command into */
   int   status;

   buffer = (char*) malloc(max_size + 1);
   if (buffer == NULL)
      exit(9);

   if (isatty(fileno(stdin)) && prompt != NULL)
   {
      isInteractive = TRUE;
      cliexec(buffer, max_size + 1, prompt, continuation_prompt, begin_state);
      isInteractive = FALSE;
   }
   else
   {
      for (done = FALSE; ! done; )
      {
	 status = clread(stdin, stdout, buffer, max_size + 1, prompt,
			 continuation_prompt);
	 if (status != 0)
	    break;

	 clparse(buffer, begin_state);
      }
   }

   free(buffer);
}

/*
 * clexit
 *
 * Signal clexec to exit.
 */
void clexit(void)
{
   done = TRUE;
}

/*
 * clscreenrows
 *
 * Return the number of rows on the terminal.
 */
int clscreenrows(void)
{
  return get_tty_rows();
}

/*
 * clpause
 *
 * Print a message and wait for the user to press a key.
 * Return the key that was pressed, or 0 if no message was printed.
 */
char clpause(
   char* message)
{
   char buffer;
   int  bytes;

   if (! isInteractive)
     return 0;

   fputs(message, stdout);
   fflush(stdout);
   bytes = read_tty(&buffer, 1);
   fputc('\n', stdout);

   return buffer;
}
