/* Return the name-within-directory of a file name.
   Copyright (C) 1996-1999, 2000, 2001 Free Software Foundation, Inc.

   NOTE: The canonical source of this file is maintained with the GNU C Library.
   Bugs can be reported to bug-glibc@gnu.org.

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <assert.h>

#ifndef FILESYSTEM_PREFIX_LEN
# define FILESYSTEM_PREFIX_LEN(Filename) 0
#endif

#ifndef ISSLASH
# define ISSLASH(C) ((C) == '/')
#endif

#ifndef _LIBC
/* We cannot generally use the name `basename' since XPG defines an unusable
   variant of the function but we cannot use it.  */
# define basename gnu_basename
#endif

/* In general, we can't use the builtin `basename' function if available,
   since it has different meanings in different environments.
   In some environments the builtin `basename' modifies its argument.
   If NAME is all slashes, be sure to return `/'.  */

char *
basename (name)
     char const *name;
{
  char const *base = name += FILESYSTEM_PREFIX_LEN (name);
  int all_slashes = 1;
  char const *p;

  for (p = name; *p; p++)
    {
      if (ISSLASH (*p))
	base = p + 1;
      else
	all_slashes = 0;
    }

  /* If NAME is all slashes, arrange to return `/'.  */
  if (*base == '\0' && ISSLASH (*name) && all_slashes)
    --base;

  /* Make sure the last byte is not a slash.  */
  assert (all_slashes || !ISSLASH (*(p - 1)));

  return (char *) base;
}
