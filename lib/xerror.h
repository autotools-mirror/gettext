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

#ifndef _XERROR_H
#define _XERROR_H

/* Get fallback definition of __attribute__.  */
#include "error.h"

/* Format a message and return the freshly allocated resulting string.  */
#if defined (__STDC__) && __STDC__
extern char *xasprintf (const char *format, ...)
     __attribute__ ((__format__ (__printf__, 1, 2)));
#else
extern char *xasprintf ();
#endif

/* Emit a multiline warning to stderr, consisting of MESSAGE, with the
   first line prefixed with PREFIX and the remaining lines prefixed with
   the same amount of spaces.  Reuse the spaces of the previous call if
   PREFIX is NULL.  Free the PREFIX and MESSAGE when done.  */
extern void multiline_warning PARAMS ((char *prefix, char *message));

/* Emit a multiline error to stderr, consisting of MESSAGE, with the
   first line prefixed with PREFIX and the remaining lines prefixed with
   the same amount of spaces.  Reuse the spaces of the previous call if
   PREFIX is NULL.  Free the PREFIX and MESSAGE when done.  */
extern void multiline_error PARAMS ((char *prefix, char *message));

#endif /* _XERROR_H */
