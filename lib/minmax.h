/* MIN, MAX macros.
   Copyright (C) 1995, 1998, 2001 Free Software Foundation, Inc.

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

#ifndef _MINMAX_H
#define _MINMAX_H

/* Before we define the following symbols we get the <limits.h> file
   since otherwise we get redefinitions on some systems.  */
#include <limits.h>

#ifndef MAX
# if __STDC__ && defined __GNUC__ && __GNUC__ >= 2
#  define MAX(a,b) (__extension__					    \
		     ({__typeof__ (a) _a = (a);				    \
		       __typeof__ (b) _b = (b);				    \
		       _a > _b ? _a : _b;				    \
		      }))
# else
#  define MAX(a,b) ((a) > (b) ? (a) : (b))
# endif
#endif

#ifndef MIN
# if __STDC__ && defined __GNUC__ && __GNUC__ >= 2
#  define MIN(a,b) (__extension__					    \
		     ({__typeof__ (a) _a = (a);				    \
		       __typeof__ (b) _b = (b);				    \
		       _a < _b ? _a : _b;				    \
		      }))
# else
#  define MIN(a,b) ((a) < (b) ? (a) : (b))
# endif
#endif

#endif /* _MINMAX_H */
