/* Charset conversion.
   Copyright (C) 2001-2003, 2006 Free Software Foundation, Inc.
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
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  */

#ifndef _ICONVSTRING_H
#define _ICONVSTRING_H

#include <stddef.h>
#if HAVE_ICONV
#include <iconv.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif


#if HAVE_ICONV

/* Convert an entire string from one encoding to another, using iconv.
   *RESULTP should initially contain NULL or malloced memory block.
   Return value: 0 if successful, otherwise -1 and errno set.
   If successful, the resulting string is stored in *RESULTP and its length
   in *LENGTHP.  */
extern int iconv_string (iconv_t cd, const char *start, const char *end,
			 char **resultp, size_t *lengthp);

#endif


#ifdef __cplusplus
}
#endif


#endif /* _ICONVSTRING_H */
