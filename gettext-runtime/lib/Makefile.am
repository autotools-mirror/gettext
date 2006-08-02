## Makefile for the gettext-runtime/lib subdirectory of GNU gettext
## Copyright (C) 1995-1998, 2000-2006 Free Software Foundation, Inc.
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software Foundation,
## Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

## Process this file with automake to produce Makefile.in.

AUTOMAKE_OPTIONS = 1.5 gnits no-dependencies
EXTRA_DIST =
BUILT_SOURCES =
MOSTLYCLEANFILES =

noinst_LIBRARIES = libgrt.a

# Sources that are compiled on all platforms.

libgrt_a_SOURCES = \
  ../../gettext-tools/gnulib-lib/allocsa.h ../../gettext-tools/gnulib-lib/allocsa.c \
  ../../gettext-tools/gnulib-lib/basename.h ../../gettext-tools/gnulib-lib/basename.c \
  ../../gettext-tools/gnulib-lib/c-ctype.h ../../gettext-tools/gnulib-lib/c-ctype.c \
  ../../gettext-tools/gnulib-lib/c-strcase.h ../../gettext-tools/gnulib-lib/c-strcasecmp.c \
  ../../gettext-tools/gnulib-lib/c-strstr.h ../../gettext-tools/gnulib-lib/c-strstr.c \
  ../../gettext-tools/gnulib-lib/closeout.h ../../gettext-tools/gnulib-lib/closeout.c \
  ../../gettext-tools/gnulib-lib/exit.h \
  ../../gettext-tools/gnulib-lib/fwriteerror.h ../../gettext-tools/gnulib-lib/fwriteerror.c \
  ../../gettext-tools/gnulib-lib/gettext.h \
  ../../gettext-tools/gnulib-lib/iconvstring.h ../../gettext-tools/gnulib-lib/iconvstring.c \
  ../../gettext-tools/gnulib-lib/localcharset.h ../../gettext-tools/gnulib-lib/localcharset.c \
  ../../gettext-tools/gnulib-lib/pathmax.h \
  ../../gettext-tools/gnulib-lib/progname.h ../../gettext-tools/gnulib-lib/progname.c ../../gettext-tools/gnulib-lib/progreloc.c \
  ../../gettext-tools/gnulib-lib/propername.h ../../gettext-tools/gnulib-lib/propername.c \
  ../../gettext-tools/gnulib-lib/strstr.h \
  ../../gettext-tools/gnulib-lib/unlocked-io.h \
  ../../gettext-tools/gnulib-lib/xalloc.h ../../gettext-tools/gnulib-lib/xmalloc.c ../../gettext-tools/gnulib-lib/xstrdup.c \
  ../../gettext-tools/gnulib-lib/xreadlink.h ../../gettext-tools/gnulib-lib/xreadlink.c

# Sources that are compiled only on platforms that lack the functions.

LIBADD_SOURCE = \
  atexit.c \
  ../../gettext-tools/gnulib-lib/canonicalize.h canonicalize.c \
  error.h error.c \
  getopt.c getopt1.c ../../gettext-tools/gnulib-lib/getopt_int.h \
  memmove.c \
  readlink.c \
  ../../gettext-tools/gnulib-lib/relocatable.h relocatable.c \
  ../../gettext-tools/gnulib-lib/setenv.h setenv.c unsetenv.c \
  strerror.c \
  strtoul.c

# How to build libgrt.a.

libgrt_a_LIBADD = @LIBOBJS@

AM_CPPFLAGS = \
  -I. -I$(srcdir) \
  -I.. \
  -I../intl -I$(srcdir)/../intl \
  -I../../gettext-tools/gnulib-lib -I$(srcdir)/../../gettext-tools/gnulib-lib

DEFS = -DEXEEXT=\"$(EXEEXT)\" -DDEPENDS_ON_LIBINTL=1 @DEFS@

# List of files to be distributed.

EXTRA_DIST += $(LIBADD_SOURCE)


# >>> gnulib module stdbool.
BUILT_SOURCES += $(STDBOOL_H)
EXTRA_DIST += ../../gettext-tools/gnulib-lib/stdbool_.h

# The following is needed in order to create an <stdbool.h> when the system
# doesn't have one that works.
stdbool.h: ../../gettext-tools/gnulib-lib/stdbool_.h
	sed -e 's/@''HAVE__BOOL''@/@HAVE__BOOL@/g' < $(srcdir)/../../gettext-tools/gnulib-lib/stdbool_.h > stdbool.h
MOSTLYCLEANFILES += stdbool.h
# <<< gnulib module stdbool.


# >>> gnulib module alloca.
BUILT_SOURCES += $(ALLOCA_H)
EXTRA_DIST += ../../gettext-tools/gnulib-lib/alloca_.h

# The following is needed in order to create an <alloca.h> when the system
# doesn't have one that works with the given compiler.
alloca.h: ../../gettext-tools/gnulib-lib/alloca_.h
	cp $(srcdir)/../../gettext-tools/gnulib-lib/alloca_.h alloca.h
MOSTLYCLEANFILES += alloca.h
# <<< gnulib module alloca.


# >>> gnulib module getopt.
BUILT_SOURCES += $(GETOPT_H)
EXTRA_DIST += ../../gettext-tools/gnulib-lib/getopt_.h ../../gettext-tools/gnulib-lib/getopt_int.h

# The following is needed in order to create a <getopt.h> when the system
# doesn't have one that works.
getopt.h: ../../gettext-tools/gnulib-lib/getopt_.h
	cp $(srcdir)/../../gettext-tools/gnulib-lib/getopt_.h getopt.h
MOSTLYCLEANFILES += getopt.h
# <<< gnulib module getopt.


# >>> gnulib module localcharset.
DEFS += -DLIBDIR=\"$(libdir)\"
# <<< gnulib module localcharset.
