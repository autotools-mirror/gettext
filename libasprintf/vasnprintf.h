/* vsprintf with automatic memory allocation.
   Copyright (C) 2002 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published
   by the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA.  */

#ifndef _VASNPRINTF_H
#define _VASNPRINTF_H

#ifndef PARAMS
# if __STDC__ || defined __GNUC__ || defined __SUNPRO_C || defined __cplusplus || __PROTOTYPES
#  define PARAMS(Args) Args
# else
#  define PARAMS(Args) ()
# endif
#endif

/* Get va_list.  */
#if __STDC__ || defined __SUNPRO_C || defined __cplusplus
# include <stdarg.h>
#else
# include <varargs.h>
#endif

/* Get size_t.  */
#include <stddef.h>

#ifndef __attribute__
/* This feature is available in gcc versions 2.5 and later.  */
# if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 5) || __STRICT_ANSI__
#  define __attribute__(Spec) /* empty */
# endif
/* The __-protected variants of `format' and `printf' attributes
   are accepted by gcc versions 2.6.4 (effectively 2.7) and later.  */
# if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 7)
#  define __format__ format
#  define __printf__ printf
# endif
#endif

#ifdef	__cplusplus
extern "C" {
#endif

extern char * asnprintf PARAMS ((char *resultbuf, size_t *lengthp, const char *format, ...))
       __attribute__ ((__format__ (__printf__, 3, 4)));
extern char * vasnprintf PARAMS ((char *resultbuf, size_t *lengthp, const char *format, va_list args))
       __attribute__ ((__format__ (__printf__, 3, 0)));

#ifdef	__cplusplus
}
#endif

#endif /* _VASNPRINTF_H */
