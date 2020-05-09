/* malloc with out of memory checking.
   Copyright (C) 2001-2004, 2006, 2019-2020 Free Software Foundation, Inc.
   Written by Bruno Haible <haible@clisp.cons.org>, 2001.

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

#ifndef _XALLOC_H
#define _XALLOC_H

#include <stddef.h>

#include "noreturn.h"
#include "xalloc-oversized.h"


#ifdef __cplusplus
extern "C" {
#endif


/* Defined in xmalloc.c.  */

/* Allocate SIZE bytes of memory dynamically, with error checking.  */
extern void *xmalloc (size_t size);

/* Allocate memory for NMEMB elements of SIZE bytes, with error checking.
   SIZE must be > 0.  */
extern void *xnmalloc (size_t nmemb, size_t size);

/* Allocate SIZE bytes of memory dynamically, with error checking,
   and zero it.  */
extern void *xzalloc (size_t size);

/* Allocate memory for NMEMB elements of SIZE bytes, with error checking,
   and zero it.  */
extern void *xcalloc (size_t nmemb, size_t size);

/* Change the size of an allocated block of memory PTR to SIZE bytes,
   with error checking.  If PTR is NULL, run xmalloc.  */
extern void *xrealloc (void *ptr, size_t size);
#ifdef __cplusplus
}
template <typename T>
  inline T * xrealloc (T * ptr, size_t size)
  {
    return (T *) xrealloc ((void *) ptr, size);
  }
extern "C" {
#endif

/* If P is null, allocate a block of at least *PN bytes; otherwise,
   reallocate P so that it contains more than *PN bytes.  *PN must be
   nonzero unless P is null.  Set *PN to the new block's size, and
   return the pointer to the new block.  *PN is never set to zero, and
   the returned pointer is never null.  */
extern void *x2realloc (void *ptr, size_t *pn);
#ifdef __cplusplus
}
template <typename T>
  inline T * x2realloc (T * ptr, size_t *pn)
  {
    return (T *) x2realloc ((void *) ptr, pn);
  }
extern "C" {
#endif

/* This function is always triggered when memory is exhausted.  It is
   in charge of honoring the three previous items.  This is the
   function to call when one wants the program to die because of a
   memory allocation failure.  */
_GL_NORETURN_FUNC extern void xalloc_die (void);

/* In the following macros, T must be an elementary or structure/union or
   typedef'ed type, or a pointer to such a type.  To apply one of the
   following macros to a function pointer or array type, you need to typedef
   it first and use the typedef name.  */

/* Allocate an object of type T dynamically, with error checking.  */
/* extern T *XMALLOC (typename T); */
#define XMALLOC(T) \
  ((T *) xmalloc (sizeof (T)))

/* Allocate memory for NMEMB elements of type T, with error checking.  */
/* extern T *XNMALLOC (size_t nmemb, typename T); */
#if HAVE_INLINE
/* xnmalloc performs a division and multiplication by sizeof (T).  Arrange to
   perform the division at compile-time and the multiplication with a factor
   known at compile-time.  */
# define XNMALLOC(N,T) \
   ((T *) (sizeof (T) == 1 \
           ? xmalloc (N) \
           : xnboundedmalloc(N, (size_t) (sizeof (ptrdiff_t) <= sizeof (size_t) ? -1 : -2) / sizeof (T), sizeof (T))))
static inline void *
xnboundedmalloc (size_t n, size_t bound, size_t s)
{
  if (n > bound)
    xalloc_die ();
  return xmalloc (n * s);
}
#else
# define XNMALLOC(N,T) \
   ((T *) (sizeof (T) == 1 ? xmalloc (N) : xnmalloc (N, sizeof (T))))
#endif

/* Allocate an object of type T dynamically, with error checking,
   and zero it.  */
/* extern T *XZALLOC (typename T); */
#define XZALLOC(T) \
  ((T *) xzalloc (sizeof (T)))

/* Allocate memory for NMEMB elements of type T, with error checking,
   and zero it.  */
/* extern T *XCALLOC (size_t nmemb, typename T); */
#define XCALLOC(N,T) \
  ((T *) xcalloc (N, sizeof (T)))

/* Return a pointer to a new buffer of N bytes.  This is like xmalloc,
   except it returns char *.  */
#define xcharalloc(N) \
  XNMALLOC (N, char)


/* Defined in xstrdup.c.  */

/* Return a newly allocated copy of the N bytes of memory starting at P.  */
extern void *xmemdup (const void *p, size_t n);
#ifdef __cplusplus
}
template <typename T>
  inline T * xmemdup (const T * p, size_t n)
  {
    return (T *) xmemdup ((const void *) p, n);
  }
extern "C" {
#endif

/* Return a newly allocated copy of STRING.  */
extern char *xstrdup (const char *string);


#ifdef __cplusplus
}
#endif


#endif /* _XALLOC_H */
