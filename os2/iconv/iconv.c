/* OS/2 iconv() implementation through OS/2 Unicode API
   Copyright (C) 2001 Free Software Foundation, Inc.

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

/*
   This file implements an iconv wrapper based on OS/2 Unicode API.
*/

#include <uconv.h>

typedef struct _iconv_t
{
  UconvObject from;
  UconvObject to;
} *iconv_t;

/* Tell "iconv.h" to not define iconv_t by itself.  */
#define _ICONV_T
#include "iconv.h"

#include <alloca.h>

/* Convert an encoding name to te form understood by UniCreateUconvObject.  */
static inline void
cp_convert (const char *cp, UniChar *ucp)
{
  size_t sl = 0;

  /* Transform CPXXX naming style to IBM-XXX style */
  if ((cp[0] == 'C' || cp[0] == 'c') && (cp[1] == 'P' || cp[1] == 'p'))
    {
      ucp[sl++] = 'I';
      ucp[sl++] = 'B';
      ucp[sl++] = 'M';
      ucp[sl++] = '-';
      cp += 2;
    }

  while (*cp != '\0')
    ucp[sl++] = *cp++;
  ucp[sl] = 0;
}

iconv_t
iconv_open (const char *cp_to, const char *cp_from)
{
  UniChar *ucp;
  iconv_t conv;

  conv = (iconv_t) malloc (sizeof (struct _iconv_t));
  if (conv == NULL)
    {
      errno = ENOMEM;
      return (iconv_t)(-1);
    }

  ucp = (UniChar *) alloca ((strlen (cp_from) + 2 + 1) * sizeof (UniChar));
  cp_convert (cp_from, ucp);
  if (UniCreateUconvObject (ucp, &conv->from))
    {
      free (conv);
      errno = EINVAL;
      return (iconv_t)(-1);
    }

  ucp = (UniChar *) alloca ((strlen (cp_to) + 2 + 1) * sizeof (UniChar));
  cp_convert (cp_to, ucp);
  if (UniCreateUconvObject (ucp, &conv->to))
    {
      UniFreeUconvObject (conv->from);
      free (conv);
      errno = EINVAL;
      return (iconv_t)(-1);
    }

  return conv;
}

size_t
iconv (iconv_t conv,
       const char **in, size_t *in_left,
       char **out, size_t *out_left)
{
  size_t bytes_converted = 0;
  int rc;
  size_t sl = *in_left, nonid;
  UniChar *ucs = (UniChar *) alloca (sl * sizeof (UniChar));
  UniChar *orig_ucs = ucs;

  rc = UniUconvToUcs (conv->from, (void **)in, in_left, &ucs, &sl, &nonid);
  if (rc)
    goto error;
  sl = ucs - orig_ucs;
  ucs = orig_ucs;
  /* Uh-oh, seems like a bug in UniUconvFromUcs, at least when
     translating from KOI8-R to KOI8-R (null translation) */
#if 0
  rc = UniUconvFromUcs (conv->to, &ucs, &sl, (void **)out, out_left, &nonid);
  if (rc)
    goto error;
#else
  while (sl)
    {
      size_t usl = 0;
      while (sl && (ucs[usl] != 0))
        usl++, sl--;
      rc = UniUconvFromUcs (conv->to, &ucs, &usl, (void **)out, out_left, &nonid);
      if (rc)
        goto error;
      if (sl && *out_left)
        {
          *(*out)++ = 0;
          (*out_left)--;
          ucs++; sl--;
        }
    }
#endif
  return 0;
 error:
  errno = EILSEQ;
  return (size_t)(-1);
}

int
iconv_close (iconv_t conv)
{
  if (conv != (iconv_t)(-1))
    {
      UniFreeUconvObject (conv->to);
      UniFreeUconvObject (conv->from);
      free (conv);
    }
  return 0;
}
