/* <stdarg.h> with fallback on <varargs.h> for old platforms.
   Copyright (C) 2001-2002 Free Software Foundation, Inc.

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

#ifndef _LIBSTDARG_H
#define _LIBSTDARG_H

#if __STDC__ || defined __cplusplus
# include <stdarg.h>
# define VA_START(args, lastarg) va_start (args, lastarg)
# define VA_PARAMS(stdc_params, oldc_params) stdc_params
#else
# include <varargs.h>
# define VA_START(args, lastarg) va_start (args)
# define VA_PARAMS(stdc_params, oldc_params) oldc_params
#endif

#endif /* _LIBSTDARG_H */
