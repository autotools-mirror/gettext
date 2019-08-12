/* xmalloc.c -- malloc with out of memory checking
   Copyright (C) 1990-1996, 2000-2003, 2005-2007, 2012 Free Software
   Foundation, Inc.

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

#include <config.h>

/* Specification.  */
#include "xalloc.h"

#include <stdlib.h>

#include "error.h"
#include "gettext.h"

#define _(str) gettext (str)


/* Exit value when the requested amount of memory is not available.
   The caller may set it to some other value.  */
int xmalloc_exit_failure = EXIT_FAILURE;

void
xalloc_die ()
{
  error (xmalloc_exit_failure, 0, _("memory exhausted"));
  /* _Noreturn cannot be given to error, since it may return if
     its first argument is 0.  To help compilers understand the
     xalloc_die does terminate, call exit. */
  exit (EXIT_FAILURE);
}

static void *
fixup_null_alloc (size_t n)
{
  void *p;

  p = NULL;
  if (n == 0)
    p = malloc ((size_t) 1);
  if (p == NULL)
    xalloc_die ();
  return p;
}

/* Allocate N bytes of memory dynamically, with error checking.  */

void *
xmalloc (size_t n)
{
  void *p;

  p = malloc (n);
  if (p == NULL)
    p = fixup_null_alloc (n);
  return p;
}

/* Allocate memory for NMEMB elements of SIZE bytes, with error checking.
   SIZE must be > 0.  */

void *
xnmalloc (size_t nmemb, size_t size)
{
  size_t n;
  void *p;

  if (xalloc_oversized (nmemb, size))
    xalloc_die ();
  n = nmemb * size;
  p = malloc (n);
  if (p == NULL)
    p = fixup_null_alloc (n);
  return p;
}

/* Allocate SIZE bytes of memory dynamically, with error checking,
   and zero it.  */

void *
xzalloc (size_t size)
{
  void *p;

  p = xmalloc (size);
  memset (p, 0, size);
  return p;
}

/* Allocate memory for N elements of S bytes, with error checking,
   and zero it.  */

void *
xcalloc (size_t n, size_t s)
{
  void *p;

  p = calloc (n, s);
  if (p == NULL)
    p = fixup_null_alloc (n);
  return p;
}

/* Change the size of an allocated block of memory P to N bytes,
   with error checking.
   If P is NULL, run xmalloc.  */

void *
xrealloc (void *p, size_t n)
{
  if (p == NULL)
    return xmalloc (n);
  p = realloc (p, n);
  if (p == NULL)
    p = fixup_null_alloc (n);
  return p;
}

/* If P is null, allocate a block of at least *PN such objects;
   otherwise, reallocate P so that it contains more than *PN objects
   each of S bytes.  S must be nonzero.  Set *PN to the new number of
   objects, and return the pointer to the new block.  *PN is never set
   to zero, and the returned pointer is never null.

   Repeated reallocations are guaranteed to make progress, either by
   allocating an initial block with a nonzero size, or by allocating a
   larger block.

   In the following implementation, nonzero sizes are increased by a
   factor of approximately 1.5 so that repeated reallocations have
   O(N) overall cost rather than O(N**2) cost, but the
   specification for this function does not guarantee that rate.

   Here is an example of use:

     int *p = NULL;
     size_t used = 0;
     size_t allocated = 0;

     void
     append_int (int value)
       {
         if (used == allocated)
           p = x2nrealloc (p, &allocated, sizeof *p);
         p[used++] = value;
       }

   This causes x2nrealloc to allocate a block of some nonzero size the
   first time it is called.

   To have finer-grained control over the initial size, set *PN to a
   nonzero value before calling this function with P == NULL.  For
   example:

     int *p = NULL;
     size_t used = 0;
     size_t allocated = 0;
     size_t allocated1 = 1000;

     void
     append_int (int value)
       {
         if (used == allocated)
           {
             p = x2nrealloc (p, &allocated1, sizeof *p);
             allocated = allocated1;
           }
         p[used++] = value;
       }

   */

static inline void *
x2nrealloc (void *p, size_t *pn, size_t s)
{
  size_t n = *pn;

  if (! p)
    {
      if (! n)
        {
          /* The approximate size to use for initial small allocation
             requests, when the invoking code specifies an old size of
             zero.  This is the largest "small" request for the GNU C
             library malloc.  */
          enum { DEFAULT_MXFAST = 64 * sizeof (size_t) / 4 };

          n = DEFAULT_MXFAST / s;
          n += !n;
        }
      if (xalloc_oversized (n, s))
        xalloc_die ();
    }
  else
    {
      /* Set N = floor (1.5 * N) + 1 so that progress is made even if N == 0.
         Check for overflow, so that N * S stays in both ptrdiff_t and
         size_t range.  The check may be slightly conservative, but an
         exact check isn't worth the trouble.  */
      if ((PTRDIFF_MAX < SIZE_MAX ? PTRDIFF_MAX : SIZE_MAX) / 3 * 2 / s
          <= n)
        xalloc_die ();
      n += n / 2 + 1;
    }

  *pn = n;
  return xrealloc (p, n * s);
}

/* If P is null, allocate a block of at least *PN bytes; otherwise,
   reallocate P so that it contains more than *PN bytes.  *PN must be
   nonzero unless P is null.  Set *PN to the new block's size, and
   return the pointer to the new block.  *PN is never set to zero, and
   the returned pointer is never null.  */

void *
x2realloc (void *p, size_t *pn)
{
  return x2nrealloc (p, pn, 1);
}
