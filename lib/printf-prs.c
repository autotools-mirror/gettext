/* Copyright (C) 1991, 1992, 1995, 1996 Free Software Foundation, Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <printf.h>
#if HAVE_STDLIB_H
# include <stdlib.h>
#endif

#include "printf-parse.h"


size_t
parse_printf_format (fmt, n, argtypes)
      const char *fmt;
      size_t n;
      int *argtypes;
{
  size_t nargs;			/* Number of arguments.  */
  size_t max_ref_arg;		/* Highest index used in a positional arg.  */
  struct printf_spec spec;

  nargs = 0;
  max_ref_arg = 0;

  /* Search for format specifications.  */
  for (fmt = find_spec (fmt); *fmt != '\0'; fmt = spec.next_fmt)
    {
      /* Parse this spec.  */
      nargs += parse_one_spec (fmt, nargs, &spec, &max_ref_arg);

      /* If the width is determined by an argument this is an int.  */
      if (spec.width_arg != -1 && (size_t) spec.width_arg < n)
	argtypes[spec.width_arg] = PA_INT;

      /* If the precision is determined by an argument this is an int.  */
      if (spec.prec_arg != -1 && (size_t) spec.prec_arg < n)
	argtypes[spec.prec_arg] = PA_INT;

      if ((size_t) spec.data_arg < n)
	switch (spec.ndata_args)
	  {
	  case 0:		/* No arguments.  */
	    break;
	  case 1:		/* One argument; we already have the type.  */
	    argtypes[spec.data_arg] = spec.data_arg_type;
	    break;
	  default:
	    /* We don't handle this here.  Beside GNU libc no other
	       libc provides printf function registration.  But while
	       having this feature it also provides this function, so
	       that using *this* file is not needed.  */
#if 0
	    /* We have more than one argument for this format spec.  We must
	       call the arginfo function again to determine all the types.  */
	    (void) (*__printf_arginfo_table[spec.info.spec])
	      (&spec.info, n - spec.data_arg, &argtypes[spec.data_arg]);
#endif
	    break;
	  }
    }

  return MAX (nargs, max_ref_arg);
}
