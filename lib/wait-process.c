/* Waiting for a subprocess to finish.
   Copyright (C) 2001-2002 Free Software Foundation, Inc.
   Written by Bruno Haible <haible@clisp.cons.org>, 2001.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/* Specification.  */
#include "wait-process.h"

#include <errno.h>
#include <stdlib.h>

#include <sys/types.h>

#include <sys/wait.h>
/* On Linux, WEXITSTATUS are bits 15..8 and WTERMSIG are bits 7..0, while
   BeOS uses the contrary.  Therefore we use the abstract macros.  */
#if HAVE_UNION_WAIT
# define WAIT_T union wait
# ifndef WTERMSIG
#  define WTERMSIG(x) ((x).w_termsig)
# endif
# ifndef WCOREDUMP
#  define WCOREDUMP(x) ((x).w_coredump)
# endif
# ifndef WEXITSTATUS
#  define WEXITSTATUS(x) ((x).w_retcode)
# endif
#else
# define WAIT_T int
# ifndef WTERMSIG
#  define WTERMSIG(x) ((x) & 0x7f)
# endif
# ifndef WCOREDUMP
#  define WCOREDUMP(x) ((x) & 0x80)
# endif
# ifndef WEXITSTATUS
#  define WEXITSTATUS(x) (((x) >> 8) & 0xff)
# endif
#endif
/* For valid x, exactly one of WIFSIGNALED(x), WIFEXITED(x), WIFSTOPPED(x)
   is true.  */
#ifndef WIFSIGNALED
# define WIFSIGNALED(x) (WTERMSIG (x) != 0 && WTERMSIG(x) != 0x7f)
#endif
#ifndef WIFEXITED
# define WIFEXITED(x) (WTERMSIG (x) == 0)
#endif
#ifndef WIFSTOPPED
# define WIFSTOPPED(x) (WTERMSIG (x) == 0x7f)
#endif
/* Note that portable applications may access
   WTERMSIG(x) only if WIFSIGNALED(x) is true, and
   WEXITSTATUS(x) only if WIFEXITED(x) is true.  */

#include "error.h"
#include "exit.h"
#include "gettext.h"

#define _(str) gettext (str)


int
wait_subprocess (child, progname, exit_on_error)
     pid_t child;
     const char *progname;
     bool exit_on_error;
{
  /* waitpid() is just as portable as wait() nowadays.  */
  WAIT_T status;

  *(int *) &status = 0;
  for (;;)
    {
      int result = waitpid (child, &status, 0);

      if (result != child)
	{
#ifdef EINTR
	  if (errno == EINTR)
	    continue;
#endif
#if 0 /* defined ECHILD */
	  if (errno == ECHILD)
	    {
	      /* Child process nonexistent?! Assume it terminated
		 successfully.  */
	      *(int *) &status = 0;
	      break;
	    }
#endif
	  if (exit_on_error)
	    error (EXIT_FAILURE, errno, _("%s subprocess"), progname);
	  else
	    return 127;
	}

      /* One of WIFSIGNALED (status), WIFEXITED (status), WIFSTOPPED (status)
	 must always be true.  Loop until the program terminates.  */
      if (!WIFSTOPPED (status))
	break;
    }

  if (WIFSIGNALED (status))
    {
      if (exit_on_error)
	error (EXIT_FAILURE, 0, _("%s subprocess got fatal signal %d"),
	       progname, (int) WTERMSIG (status));
      else
	return 127;
    }
  if (WEXITSTATUS (status) == 127)
    {
      if (exit_on_error)
	error (EXIT_FAILURE, 0, _("%s subprocess failed"), progname);
      else
	return 127;
    }
  return WEXITSTATUS (status);
}
