/* Memory allocation on the stack.
   Copyright (C) 1995, 1999, 2001-2002 Free Software Foundation, Inc.

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

/* When this file is included, it may be preceded only by preprocessor
   declarations.  Thanks to AIX.  Therefore we include it right after
   "config.h", not later.  */

#ifndef _LIBALLOCA_H
#define _LIBALLOCA_H

#ifdef __GNUC__
# ifndef alloca
#  define alloca __builtin_alloca
# endif
#else
# ifdef _MSC_VER
#  include <malloc.h>
#  define alloca _alloca
# else
#  if HAVE_ALLOCA_H
#   include <alloca.h>
#  else
#   ifdef _AIX
 #pragma alloca
#   else
#    ifdef __hpux /* This section must match that of bison generated files. */
#     ifdef __cplusplus
extern "C" void *alloca (unsigned int);
#     else /* not __cplusplus */
void *alloca ();
#     endif /* not __cplusplus */
#    else /* not __hpux */
#     ifndef alloca
char *alloca ();
#     endif
#    endif /* __hpux */
#   endif
#  endif
# endif
#endif

#endif /* _LIBALLOCA_H */
