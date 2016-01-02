/* OS/2 iconv() implementation through OS/2 Unicode API
   Copyright (C) 2001, 2015-2016 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation; either version 2.1 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef __ICONV_H__
#define __ICONV_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* FIXME: This belongs in <errno.h>.  */
#define EILSEQ 1729

#ifndef _ICONV_T
typedef void *iconv_t;
#endif

extern iconv_t iconv_open (const char *, const char *);
extern size_t iconv (iconv_t, const char **, size_t *, char **, size_t *);
extern int iconv_close (iconv_t);

#ifdef __cplusplus
}
#endif

#endif /* __ICONV_H__ */
