/* String copying.
   Copyright (C) 1995, 2001-2002 Free Software Foundation, Inc.

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

#ifndef _STPNCPY_H
#define _STPNCPY_H

#ifndef PARAMS
# if __STDC__ || defined __GNUC__ || defined __SUNPRO_C || defined __cplusplus || __PROTOTYPES
#  define PARAMS(args) args
# else
#  define PARAMS(args) ()
# endif
#endif

#include <string.h>

#ifndef __GNU_LIBRARY__

/* When not using the GNU libc we use the stpncpy implementation we
   provide here.  */
extern char *gnu_stpncpy PARAMS ((char *__dst, const char *__src, size_t __n));
#define stpncpy(Dst, Src, N) gnu_stpncpy (Dst, Src, N)

#endif

#endif /* _STPNCPY_H */
