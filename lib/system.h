/* Header for GNU gettext libiberty
   Copyright (C) 1995-1997, 2000, 2001 Free Software Foundation, Inc.

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
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#ifndef _SYSTEM_H
#define _SYSTEM_H 1

#ifndef PARAMS
# if defined (__GNUC__) || __STDC__
#  define PARAMS(args) args
# else
#  define PARAMS(args) ()
# endif
#endif

#include <stdio.h>
#include <sys/types.h>

#if defined STDC_HEADERS
# include <stdlib.h>
#else
# ifdef HAVE_MALLOC_H
#  include <malloc.h>
# endif
unsigned long strtoul ();
#endif

/* Wrapper functions with error checking for standard functions.  */
extern char *xgetcwd PARAMS ((void));
extern void *xmalloc PARAMS ((size_t __n));
extern void *xrealloc PARAMS ((void *__p, size_t __n));
extern char *xstrdup PARAMS ((const char *__string));
extern char *stpcpy PARAMS ((char *__dst, const char *__src));
extern char *stpncpy PARAMS ((char *__dst, const char *__src, size_t __n));
extern size_t parse_printf_format PARAMS ((const char *__fmt, size_t __n,
					   int *__argtypes));
extern int asprintf PARAMS ((char **, const char *, ...));
extern int strcasecmp PARAMS ((const char *__s1, const char *__s2));
extern int strncasecmp PARAMS ((const char *__s1, const char *__s2,
				size_t __n));
extern char *strstr PARAMS ((const char *__str, const char *__sub));

#include <string.h>
#if !STDC_HEADERS && HAVE_MEMORY_H
# include <memory.h>
#endif
#if !HAVE_MEMCPY
# ifndef memcpy
#  define memcpy(D, S, N) bcopy ((S), (D), (N))
# endif
#endif
#if !HAVE_STRCHR
# ifndef strchr
#  define strchr index
# endif
#endif

#ifdef __GNUC__
# ifndef alloca
#  define alloca __builtin_alloca
# endif
#else
# if HAVE_ALLOCA_H
#  include <alloca.h>
# else
#  ifdef _AIX
 #pragma alloca
#  else
#   ifdef __hpux /* This section must match that of bison generated files. */
#    ifdef __cplusplus
extern "C" void *alloca (unsigned int);
#    else /* not __cplusplus */
void *alloca ();
#    endif /* not __cplusplus */
#   else /* not __hpux */
#    ifndef alloca
char *alloca ();
#    endif
#   endif /* __hpux */
#  endif
# endif
#endif

/* Before we define the following symbols we get the <limits.h> file if
   available since otherwise we get redefinitions on some systems.  */
#if HAVE_LIMITS_H
# include <limits.h>
#endif

#ifndef MAX
# if __STDC__ && defined __GNUC__ && __GNUC__ >= 2
#  define MAX(a,b) (__extension__					    \
		     ({__typeof__ (a) _a = (a);				    \
		       __typeof__ (b) _b = (b);				    \
		       _a > _b ? _a : _b;				    \
		      }))
# else
#  define MAX(a,b) ((a) > (b) ? (a) : (b))
# endif
#endif

/* Some systems do not define EXIT_*, even with STDC_HEADERS.  */
#ifndef EXIT_SUCCESS
# define EXIT_SUCCESS 0
#endif
#ifndef EXIT_FAILURE
# define EXIT_FAILURE 1
#endif


/* Pathname support.
   ISSLASH(C)           tests whether C is a directory separator character.
   IS_ABSOLUTE_PATH(P)  tests whether P is an absolute path.  If it is not,
                        it may be concatenated to a directory pathname.
   IS_PATH_WITH_DIR(P)  tests whether P contains a directory specification.
 */
#if defined _WIN32 || defined __WIN32__ || defined __EMX__ || defined __DJGPP__
  /* Win32, OS/2, DOS */
# define ISSLASH(C) ((C) == '/' || (C) == '\\')
# define HAS_DEVICE(P) \
    ((((P)[0] >= 'A' && (P)[0] <= 'Z') || ((P)[0] >= 'a' && (P)[0] <= 'z')) \
     && (P)[1] == ':')
# define IS_ABSOLUTE_PATH(P) (ISSLASH ((P)[0]) || HAS_DEVICE (P))
# define IS_PATH_WITH_DIR(P) \
    (strchr (P, '/') != NULL || strchr (P, '\\') != NULL || HAS_DEVICE (P))
# define FILESYSTEM_PREFIX_LEN(P) (HAS_DEVICE (P) ? 2 : 0)
#else
  /* Unix */
# define ISSLASH(C) ((C) == '/')
# define IS_ABSOLUTE_PATH(P) ISSLASH ((P)[0])
# define IS_PATH_WITH_DIR(P) (strchr (P, '/') != NULL)
# define FILESYSTEM_PREFIX_LEN(P) 0
#endif

/* Concatenate a directory pathname, a relative pathname and an optional
   suffix.  Return a freshly allocated pathname.  */
extern char *concatenated_pathname PARAMS ((const char *directory,
					    const char *filename,
					    const char *suffix));

/* When not using the GNU libc we use the basename implementation we
   provide here.  */
#ifndef __GNU_LIBRARY__
extern char *gnu_basename PARAMS ((const char *));
# define basename(Arg) gnu_basename (Arg)
#endif


#include <fcntl.h>
/* For systems that distinguish between text and binary I/O.
   O_BINARY is usually declared in <fcntl.h>. */
#if !defined O_BINARY && defined _O_BINARY
  /* For MSC-compatible compilers.  */
# define O_BINARY _O_BINARY
# define O_TEXT _O_TEXT
#endif
#ifdef __BEOS__
  /* BeOS 5 has O_BINARY and O_TEXT, but they have no effect.  */
# undef O_BINARY
# undef O_TEXT
#endif
#if O_BINARY
# if !(defined(__EMX__) || defined(__DJGPP__))
#  define setmode _setmode
#  define fileno _fileno
# endif
# ifdef __DJGPP__
#  include <io.h> /* declares setmode() */
#  include <unistd.h> /* declares isatty() */
#  /* Avoid putting stdin/stdout in binary mode if it is connected to the
#     console, because that would make it impossible for the user to
#     interrupt the program through Ctrl-C or Ctrl-Break.  */
#  define SET_BINARY(fd) (!isatty(fd) ? (setmode(fd,O_BINARY), 0) : 0)
# else
#  define SET_BINARY(fd) setmode(fd,O_BINARY)
# endif
#else
# define SET_BINARY(fd) /* nothing */
#endif

#endif
