/* Multiline error-reporting functions.
   Copyright (C) 2001 Free Software Foundation, Inc.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xerror.h"
#include "error.h"
#include "progname.h"
#include "mbswidth.h"
#include "libgettext.h"

#define _(str) gettext (str)

#if __STDC__
# include <stdarg.h>
# define VA_START(args, lastarg) va_start(args, lastarg)
#else
# include <varargs.h>
# define VA_START(args, lastarg) va_start(args)
# define NEW_VARARGS 1 /* or 0 if it doesn't work */
#endif

/* Format a message and return the freshly allocated resulting string.  */
char *
#if __STDC__
xasprintf (const char *format, ...)
#elif NEW_VARARGS
xasprintf (format, va_alist)
     const char *format;
     va_dcl
#else
xasprintf (va_alist)
     va_dcl
#endif
{
#if !__STDC__ && !NEW_VARARGS
  const char *format;
#endif
  va_list args;
  char *result;

  va_start (args, format);
#if !__STDC__ && !NEW_VARARGS
  format = va_arg (args, const char *);
#endif
  if (vasprintf (&result, format, args) < 0)
    error (EXIT_FAILURE, 0, _("memory exhausted"));
  va_end (args);
  return result;
}

/* Emit a multiline warning to stderr, consisting of MESSAGE, with the
   first line prefixed with PREFIX and the remaining lines prefixed with
   the same amount of spaces.  Reuse the spaces of the previous call if
   PREFIX is NULL.  Free the PREFIX and MESSAGE when done.  */
void
multiline_warning (prefix, message)
     char *prefix;
     char *message;
{
  static int width;
  const char *cp;
  int i;

  fflush (stdout);

  cp = message;

  if (prefix != NULL)
    {
      width = 0;
      if (error_with_progname)
	{
	  fprintf (stderr, "%s: ", program_name);
	  width += mbswidth (program_name, 0) + 2;
	}
      fputs (prefix, stderr);
      width += mbswidth (prefix, 0);
      free (prefix);
      goto after_indent;
    }

  while (1)
    {
      const char *np;

      for (i = width; i > 0; i--)
	putc (' ', stderr);

    after_indent:
      np = strchr (cp, '\n');

      if (np == NULL || np[1] == '\0')
	{
	  fputs (cp, stderr);
	  break;
	}

      np++;
      fwrite (cp, 1, np - cp, stderr);
      cp = np;
    }

  free (message);
}

/* Emit a multiline error to stderr, consisting of MESSAGE, with the
   first line prefixed with PREFIX and the remaining lines prefixed with
   the same amount of spaces.  Reuse the spaces of the previous call if
   PREFIX is NULL.  Free the PREFIX and MESSAGE when done.  */
void
multiline_error (prefix, message)
     char *prefix;
     char *message;
{
  if (prefix != NULL)
    ++error_message_count;
  multiline_warning (prefix, message);
}
