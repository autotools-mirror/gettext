/* argmatch.c -- find a match for a string in an array
   Copyright (C) 1990, 1998, 1999, 2001 Free Software Foundation, Inc.

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

/* Written by David MacKenzie <djm@ai.mit.edu>
   Modified by Akim Demaille <demaille@inf.enst.fr> */

#include "argmatch.h"

#include <stdio.h>
#ifdef STDC_HEADERS
# include <string.h>
#endif

#include <locale.h>

#if ENABLE_NLS
# include <libintl.h>
# define _(Text) gettext (Text)
#else
# define _(Text) Text
#endif

#include "error.h"
#include "exit.h"

/* Non failing version of argmatch call this function after failing. */
#ifndef ARGMATCH_DIE
# define ARGMATCH_DIE exit (EXIT_FAILURE)
#endif

#ifdef ARGMATCH_DIE_DECL
ARGMATCH_DIE_DECL;
#endif


/* Prototypes for local functions.  Needed to ensure compiler checking of
   function argument counts despite of K&R C function definition syntax.  */
static void __argmatch_die PARAMS ((void));


static void
__argmatch_die ()
{
  ARGMATCH_DIE;
}

/* Used by XARGMATCH and XARGCASEMATCH.  See description in argmatch.h.
   Default to __argmatch_die, but allow caller to change this at run-time. */
argmatch_exit_fn argmatch_die = __argmatch_die;


/* If ARG is an unambiguous match for an element of the
   null-terminated array ARGLIST, return the index in ARGLIST
   of the matched element, else -1 if it does not match any element
   or -2 if it is ambiguous (is a prefix of more than one element).

   If VALLIST is none null, use it to resolve ambiguities limited to
   synonyms, i.e., for
     "yes", "yop" -> 0
     "no", "nope" -> 1
   "y" is a valid argument, for `0', and "n" for `1'.  */

int
argmatch (arg, arglist, vallist, valsize)
     const char *arg;
     const char *const *arglist;
     const char *vallist;
     size_t valsize;
{
  int i;			/* Temporary index in ARGLIST.  */
  size_t arglen;		/* Length of ARG.  */
  int matchind = -1;		/* Index of first nonexact match.  */
  int ambiguous = 0;		/* If nonzero, multiple nonexact match(es).  */

  arglen = strlen (arg);

  /* Test all elements for either exact match or abbreviated matches.  */
  for (i = 0; arglist[i]; i++)
    {
      if (!strncmp (arglist[i], arg, arglen))
	{
	  if (strlen (arglist[i]) == arglen)
	    /* Exact match found.  */
	    return i;
	  else if (matchind == -1)
	    /* First nonexact match found.  */
	    matchind = i;
	  else
	    {
	      /* Second nonexact match found.  */
	      if (vallist == NULL
		  || memcmp (vallist + valsize * matchind,
			     vallist + valsize * i, valsize))
		{
		  /* There is a real ambiguity, or we could not
		     disambiguate. */
		  ambiguous = 1;
		}
	    }
	}
    }
  if (ambiguous)
    return -2;
  else
    return matchind;
}

/* Error reporting for argmatch.
   CONTEXT is a description of the type of entity that was being matched.
   VALUE is the invalid value that was given.
   PROBLEM is the return value from argmatch.  */

void
argmatch_invalid (context, value, problem)
     const char *context;
     const char *value;
     int problem;
{
  char const *format = (problem == -1
			? _("invalid argument `%s' for `%s'")
			: _("ambiguous argument `%s' for `%s'"));

  error (0, 0, format, value, context);
}

/* List the valid arguments for argmatch.
   ARGLIST is the same as in argmatch.
   VALLIST is a pointer to an array of values.
   VALSIZE is the size of the elements of VALLIST */
void
argmatch_valid (arglist, vallist, valsize)
     const char *const *arglist;
     const char *vallist;
     size_t valsize;
{
  int i;
  const char *last_val = NULL;

  /* We try to put synonyms on the same line.  The assumption is that
     synonyms follow each other */
  fprintf (stderr, _("Valid arguments are:"));
  for (i = 0; arglist[i]; i++)
    if ((i == 0)
	|| memcmp (last_val, vallist + valsize * i, valsize))
      {
	fprintf (stderr, "\n  - `%s'", arglist[i]);
	last_val = vallist + valsize * i;
      }
    else
      {
	fprintf (stderr, ", `%s'", arglist[i]);
      }
  putc ('\n', stderr);
}

/* Never failing versions of the previous functions.

   CONTEXT is the context for which argmatch is called (e.g.,
   "--version-control", or "$VERSION_CONTROL" etc.).  Upon failure,
   calls the (supposed never to return) function EXIT_FN. */

int
__xargmatch_internal (context, arg, arglist, vallist, valsize, exit_fn)
     const char *context;
     const char *arg;
     const char *const *arglist;
     const char *vallist;
     size_t valsize;
     argmatch_exit_fn exit_fn;
{
  int res = argmatch (arg, arglist, vallist, valsize);
  if (res >= 0)
    /* Success. */
    return res;

  /* We failed.  Explain why. */
  argmatch_invalid (context, arg, res);
  argmatch_valid (arglist, vallist, valsize);
  (*exit_fn) ();

  return -1; /* To please the compilers. */
}

/* Look for VALUE in VALLIST, an array of objects of size VALSIZE and
   return the first corresponding argument in ARGLIST */
const char *
argmatch_to_argument (value, arglist, vallist, valsize)
     const char *value;
     const char *const *arglist;
     const char *vallist;
     size_t valsize;
{
  int i;

  for (i = 0; arglist[i]; i++)
    if (!memcmp (value, vallist + valsize * i, valsize))
      return arglist[i];
  return NULL;
}
