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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/* Specification.  */
#include "iconvstring.h"

#include <errno.h>
#include <stdlib.h>

#if HAVE_ICONV
# include <iconv.h>
#endif

#include "xalloc.h"


#if HAVE_ICONV

/* Converts an entire string from one encoding to another, using iconv.
   Return value: 0 if successful, otherwise -1 and errno set.  */
int
iconv_string (iconv_t cd, const char *start, const char *end,
	      char **resultp, size_t *lengthp)
{
#define tmpbufsize 4096
  size_t length;
  char *result;

  /* Avoid glibc-2.1 bug and Solaris 2.7-2.9 bug.  */
# if defined _LIBICONV_VERSION \
    || !((__GLIBC__ - 0 == 2 && __GLIBC_MINOR__ - 0 <= 1) || defined __sun)
  /* Set to the initial state.  */
  iconv (cd, NULL, NULL, NULL, NULL);
# endif

  /* Determine the length we need.  */
  {
    size_t count = 0;
    char tmpbuf[tmpbufsize];
    const char *inptr = start;
    size_t insize = end - start;

    while (insize > 0)
      {
	char *outptr = tmpbuf;
	size_t outsize = tmpbufsize;
	size_t res = iconv (cd,
			    (ICONV_CONST char **) &inptr, &insize,
			    &outptr, &outsize);

	if (res == (size_t)(-1))
	  {
	    if (errno == E2BIG)
	      ;
	    else if (errno == EINVAL)
	      break;
	    else
	      return -1;
	  }
# if !defined _LIBICONV_VERSION && (defined sgi || defined __sgi)
	/* Irix iconv() inserts a NUL byte if it cannot convert.  */
	else if (res > 0)
	  return -1;
# endif
	count += outptr - tmpbuf;
      }
    /* Avoid glibc-2.1 bug and Solaris 2.7 bug.  */
# if defined _LIBICONV_VERSION \
    || !((__GLIBC__ - 0 == 2 && __GLIBC_MINOR__ - 0 <= 1) || defined __sun)
    {
      char *outptr = tmpbuf;
      size_t outsize = tmpbufsize;
      size_t res = iconv (cd, NULL, NULL, &outptr, &outsize);

      if (res == (size_t)(-1))
	return -1;
      count += outptr - tmpbuf;
    }
# endif
    length = count;
  }

  *lengthp = length;
  *resultp = result = xrealloc (*resultp, length);
  if (length == 0)
    return 0;

  /* Avoid glibc-2.1 bug and Solaris 2.7-2.9 bug.  */
# if defined _LIBICONV_VERSION \
    || !((__GLIBC__ - 0 == 2 && __GLIBC_MINOR__ - 0 <= 1) || defined __sun)
  /* Return to the initial state.  */
  iconv (cd, NULL, NULL, NULL, NULL);
# endif

  /* Do the conversion for real.  */
  {
    const char *inptr = start;
    size_t insize = end - start;
    char *outptr = result;
    size_t outsize = length;

    while (insize > 0)
      {
	size_t res = iconv (cd,
			    (ICONV_CONST char **) &inptr, &insize,
			    &outptr, &outsize);

	if (res == (size_t)(-1))
	  {
	    if (errno == EINVAL)
	      break;
	    else
	      return -1;
	  }
# if !defined _LIBICONV_VERSION && (defined sgi || defined __sgi)
	/* Irix iconv() inserts a NUL byte if it cannot convert.  */
	else if (res > 0)
	  return -1;
# endif
      }
    /* Avoid glibc-2.1 bug and Solaris 2.7 bug.  */
# if defined _LIBICONV_VERSION \
    || !((__GLIBC__ - 0 == 2 && __GLIBC_MINOR__ - 0 <= 1) || defined __sun)
    {
      size_t res = iconv (cd, NULL, NULL, &outptr, &outsize);

      if (res == (size_t)(-1))
	return -1;
    }
# endif
    if (outsize != 0)
      abort ();
  }

  return 0;
#undef tmpbufsize
}

#endif
