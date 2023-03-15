/* GNU gettext - internationalization aids
   Copyright (C) 2023 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Bruno Haible <bruno@clisp.org>, 2023.  */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/* Specification.  */
#include "str-desc.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "xalloc.h"


/* ==== Side-effect-free operations on string descriptors ==== */

size_t
string_desc_length (string_desc_ty s)
{
  return s.nbytes;
}

char
string_desc_char_at (string_desc_ty s, size_t i)
{
  if (!(i < s.nbytes))
    /* Invalid argument.  */
    abort ();
  return s.data[i];
}

const char *
string_desc_data (string_desc_ty s)
{
  return s.data;
}

bool
string_desc_is_empty (string_desc_ty s)
{
  return s.nbytes == 0;
}

bool
string_desc_startswith (string_desc_ty s, string_desc_ty prefix)
{
  return (s.nbytes >= prefix.nbytes
          && (prefix.nbytes == 0
              || memcmp (s.data, prefix.data, prefix.nbytes) == 0));
}

bool
string_desc_endswith (string_desc_ty s, string_desc_ty suffix)
{
  return (s.nbytes >= suffix.nbytes
          && (suffix.nbytes == 0
              || memcmp (s.data + (s.nbytes - suffix.nbytes), suffix.data,
                         suffix.nbytes) == 0));
}

int
string_desc_cmp (string_desc_ty a, string_desc_ty b)
{
  if (a.nbytes > b.nbytes)
    {
      if (b.nbytes == 0)
        return 1;
      return (memcmp (a.data, b.data, b.nbytes) < 0 ? -1 : 1);
    }
  else if (a.nbytes < b.nbytes)
    {
      if (a.nbytes == 0)
        return -1;
      return (memcmp (a.data, b.data, a.nbytes) > 0 ? 1 : -1);
    }
  else /* a.nbytes == b.nbytes */
    {
      if (a.nbytes == 0)
        return 0;
      return memcmp (a.data, b.data, a.nbytes);
    }
}

ptrdiff_t
string_desc_index (string_desc_ty s, char c)
{
  if (s.nbytes > 0)
    {
      void *found = memchr (s.data, (unsigned char) c, s.nbytes);
      if (found != NULL)
        return (char *) found - s.data;
    }
  return -1;
}

ptrdiff_t
string_desc_last_index (string_desc_ty s, char c)
{
  if (s.nbytes > 0)
    {
      void *found = memrchr (s.data, (unsigned char) c, s.nbytes);
      if (found != NULL)
        return (char *) found - s.data;
    }
  return -1;
}

ptrdiff_t
string_desc_contains (string_desc_ty haystack, string_desc_ty needle)
{
  if (needle.nbytes == 0)
    return 0;
  void *found =
    memmem (haystack.data, haystack.nbytes, needle.data, needle.nbytes);
  if (found != NULL)
    return (char *) found - haystack.data;
  else
    return -1;
}

string_desc_ty
string_desc_from_c (const char *s)
{
  string_desc_ty result;

  result.nbytes = strlen (s);
  result.data = (char *) s;

  return result;
}

string_desc_ty
string_desc_substring (string_desc_ty s, size_t start, size_t end)
{
  string_desc_ty result;

  if (!(start <= end))
    /* Invalid arguments.  */
    abort ();

  result.nbytes = end - start;
  result.data = s.data + start;

  return result;
}


/* ==== Memory-allocating operations on string descriptors ==== */

string_desc_ty
string_desc_new (size_t n)
{
  string_desc_ty result;

  result.nbytes = n;
  if (n == 0)
    result.data = NULL;
  else
    result.data = (char *) xmalloc (n);

  return result;
}

string_desc_ty
string_desc_new_addr (size_t n, char *addr)
{
  string_desc_ty result;

  result.nbytes = n;
  if (n == 0)
    result.data = NULL;
  else
    result.data = addr;

  return result;
}

string_desc_ty
string_desc_new_filled (size_t n, char c)
{
  string_desc_ty result;

  result.nbytes = n;
  if (n == 0)
    result.data = NULL;
  else
    {
      result.data = (char *) xmalloc (n);
      memset (result.data, (unsigned char) c, n);
    }

  return result;
}

string_desc_ty
string_desc_copy (string_desc_ty s)
{
  string_desc_ty result;
  size_t n = s.nbytes;

  result.nbytes = n;
  if (n == 0)
    result.data = NULL;
  else
    {
      result.data = (char *) xmalloc (n);
      memcpy (result.data, s.data, n);
    }

  return result;
}

string_desc_ty
string_desc_concat (size_t n, string_desc_ty string1, ...)
{
  if (n == 0)
    /* Invalid argument.  */
    abort ();

  size_t total = 0;
  total += string1.nbytes;
  if (n > 1)
    {
      va_list other_strings;
      size_t i;

      va_start (other_strings, string1);
      for (i = --n; i > 0; i--)
        {
          string_desc_ty arg = va_arg (other_strings, string_desc_ty);
          total += arg.nbytes;
        }
      va_end (other_strings);
    }

  char *combined = (char *) xmalloc (total);
  size_t pos = 0;
  memcpy (combined, string1.data, string1.nbytes);
  pos += string1.nbytes;
  if (n > 1)
    {
      va_list other_strings;
      size_t i;

      va_start (other_strings, string1);
      for (i = --n; i > 0; i--)
        {
          string_desc_ty arg = va_arg (other_strings, string_desc_ty);
          if (arg.nbytes > 0)
            memcpy (combined + pos, arg.data, arg.nbytes);
          pos += arg.nbytes;
        }
      va_end (other_strings);
    }

  string_desc_ty result;
  result.nbytes = total;
  result.data = combined;

  return result;
}

char *
string_desc_c (string_desc_ty s)
{
  size_t n = s.nbytes;
  char *result = (char *) xmalloc (n + 1);
  if (n > 0)
    memcpy (result, s.data, n);
  result[n] = '\0';

  return result;
}


/* ==== Operations with side effects on string descriptors ==== */

void
string_desc_set_char_at (string_desc_ty s, size_t i, char c)
{
  if (!(i < s.nbytes))
    /* Invalid argument.  */
    abort ();
  s.data[i] = c;
}

void
string_desc_fill (string_desc_ty s, size_t start, size_t end, char c)
{
  if (!(start <= end))
    /* Invalid arguments.  */
    abort ();

  if (start < end)
    memset (s.data + start, (unsigned char) c, end - start);
}

void
string_desc_overwrite (string_desc_ty s, size_t start, string_desc_ty t)
{
  if (!(start + t.nbytes <= s.nbytes))
    /* Invalid arguments.  */
    abort ();

  if (t.nbytes > 0)
    memcpy (s.data + start, t.data, t.nbytes);
}

void
string_desc_free (string_desc_ty s)
{
  free (s.data);
}
