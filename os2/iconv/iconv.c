/* OS/2 iconv() implementation through OS/2 Unicode API
   Copyright (C) 2001-2002, 2015-2016 Free Software Foundation, Inc.

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

/*
   This file implements an iconv wrapper based on OS/2 Unicode API.
*/

#include <uconv.h>

typedef struct _iconv_t
{
  UconvObject from;             /* "From" conversion handle */
  UconvObject to;               /* "To" conversion handle */
} *iconv_t;

/* Tell "iconv.h" to not define iconv_t by itself.  */
#define _ICONV_T
#include "iconv.h"

#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <alloca.h>

/* Convert an encoding name to te form understood by UniCreateUconvObject.  */
static inline void
cp_convert (const char *cp, UniChar *ucp)
{
  size_t sl = 0;

  if (!stricmp (cp, "EUC-JP"))
    memcpy (ucp, L"IBM-954", 8*2);
  else if (!stricmp (cp, "EUC-KR"))
    memcpy (ucp, L"IBM-970", 8*2);
  else if (!stricmp (cp, "EUC-TW"))
    memcpy (ucp, L"IBM-964", 8*2);
  else if (!stricmp (cp, "EUC-CN"))
    memcpy (ucp, L"IBM-1383", 9*2);
  else if (!stricmp (cp, "BIG5"))
    memcpy (ucp, L"IBM-950", 8*2);
  else
    {
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
}

iconv_t
iconv_open (const char *cp_to, const char *cp_from)
{
  UniChar *ucp;
  iconv_t conv;
  uconv_attribute_t attr;

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

  UniQueryUconvObject (conv->from, &attr, sizeof (attr), NULL, NULL, NULL);
  /* Do not treat 0x7f as a control character
     (don't understand what it exactly means but without it MBCS prefix
     character detection sometimes could fail (when 0x7f is a prefix)).
     And don't treat the string as a path (the docs also don't explain
     what it exactly means, but I'm pretty sure converted texts will
     mostly not be paths).  */
  attr.converttype &= ~(CVTTYPE_CTRL7F | CVTTYPE_PATH);
  UniSetUconvObject (conv->from, &attr);

  return conv;
}

size_t
iconv (iconv_t conv,
       const char **in, size_t *in_left,
       char **out, size_t *out_left)
{
  int rc;
  size_t sl = *in_left, nonid;
  UniChar *ucs = (UniChar *) alloca (sl * sizeof (UniChar));
  UniChar *orig_ucs = ucs;
  size_t retval = 0;

  rc = UniUconvToUcs (conv->from, (void **)in, in_left, &ucs, &sl, &retval);
  if (rc)
    goto error;
  sl = ucs - orig_ucs;
  ucs = orig_ucs;
  /* UniUconvFromUcs will stop at first nul byte (huh? indeed?)
     while we want ALL the bytes converted.  */
#if 1
  rc = UniUconvFromUcs (conv->to, &ucs, &sl, (void **)out, out_left, &nonid);
  if (rc)
    goto error;
  retval += nonid;
#else
  while (sl)
    {
      size_t usl = 0;
      while (sl && (ucs[usl] != 0))
        usl++, sl--;
      rc = UniUconvFromUcs (conv->to, &ucs, &usl, (void **)out, out_left, &nonid);
      if (rc)
        goto error;
      retval += nonid;
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
  /* Convert OS/2 error code to errno.  */
  switch (rc)
  {
    case ULS_ILLEGALSEQUENCE:
      errno = EILSEQ;
      break;
    case ULS_INVALID:
      errno = EINVAL;
      break;
    case ULS_BUFFERFULL:
      errno = E2BIG;
      break;
    default:
      errno = EBADF;
      break;
  }
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
