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

/* POSIX does not specify clearly what happens when a character in the
   source encoding is valid but cannot be represented in the destination
   encoding.
   GNU libc and libiconv stop the conversion in this case, with errno = EINVAL.
   Irix iconv() inserts a NUL byte in this case.  NetBSD iconv() inserts
   a '?' byte.  For other implementations, we don't know.  Normally the
   number of failed conversions is available as the iconv() result.
   The problem with these implementations is that when iconv() fails, for
   example with errno = E2BIG or = EINVAL, the number of failed conversions
   gets lost.  As a workaround, we need to process the input string slowly,
   byte after byte.  */
# if !(defined __GLIBC__ || defined _LIBICONV_VERSION)
#  define UNSAFE_ICONV
# endif

/* Converts an entire string from one encoding to another, using iconv.
   Return value: 0 if successful, otherwise -1 and errno set.  */
int
iconv_string (iconv_t cd, const char *start, const char *end,
	      char **resultp, size_t *lengthp)
{
#define tmpbufsize 4096
  size_t length;
  char *result;
# ifdef UNSAFE_ICONV
  int expect_einval = 0;
# endif

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
	      {
# ifdef UNSAFE_ICONV
		expect_einval = 1;
# endif
		break;
	      }
	    else
	      return -1;
	  }
# ifdef UNSAFE_ICONV
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
    char *outptr = result;
    size_t outsize = length;

# ifdef UNSAFE_ICONV
    if (expect_einval)
      {
	/* Process the characters one by one, so as to not lose the
	   number of conversion failures.  */
	const char *inptr_end = end;

	while (inptr < inptr_end)
	  {
	    size_t insize_max = inptr_end - inptr;
	    size_t insize_avail;
	    size_t res;

	    for (insize_avail = 1; ; insize_avail++)
	      {
		/* Here 1 <= insize_avail <= insize_max.  */
		size_t insize = insize_avail;

		res = iconv (cd,
			     (ICONV_CONST char **) &inptr, &insize,
			     &outptr, &outsize);
		if (res == (size_t)(-1))
		  {
		    if (errno == EINVAL)
		      {
			if (insize_avail < insize_max)
			  continue;
			else
			  break;
		      }
		    else
		      /* E2BIG and other errors shouldn't happen in this
			 round any more.  */
		      return -1;
		  }
		else
		  break;
	      }
	    if (res == (size_t)(-1))
	      /* errno = EINVAL.  Ignore the trailing incomplete character.  */
	      break;
	    else if (res > 0)
	      return -1;
	  }
      }
    else
# endif
      {
	size_t insize = end - start;

	while (insize > 0)
	  {
	    size_t res = iconv (cd,
				(ICONV_CONST char **) &inptr, &insize,
				&outptr, &outsize);

	    if (res == (size_t)(-1))
	      {
		if (errno == EINVAL)
		  {
# ifdef UNSAFE_ICONV
		    /* EINVAL should already have occurred in the first
		       round.  */
		    abort ();
# endif
		    /* Ignore the trailing incomplete character.  */
		    break;
		  }
		else
		  /* E2BIG and other errors shouldn't happen in this round
		     any more.  */
		  return -1;
	      }
# ifdef UNSAFE_ICONV
	    else if (res > 0)
	      return -1;
# endif
	  }
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
