## Makefile for libgrep directory in GNU gettext package.
## Copyright (C) 2005-2007, 2009 Free Software Foundation, Inc.
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 3 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <http://www.gnu.org/licenses/>.

## Process this file with automake to produce Makefile.in.

AUTOMAKE_OPTIONS = 1.5 gnits no-dependencies
EXTRA_DIST =


noinst_LIBRARIES = libgrep.a

# Sources that are compiled on all platforms.

libgrep_a_SOURCES = \
  libgrep.h \
  kwset.h kwset.c \
  dfa.h dfa.c \
  m-common.h m-common.c m-fgrep.c m-regex.c \
  hard-locale.h hard-locale.c

# Sources that are compiled only on platforms that lack the functions.

LIBADD_SOURCE = \
  memchr.c \
  regex.h regex.c

# How to build libgrep.a.
libgrep_a_LIBADD = @LIBGREPOBJS@
libgrep_a_DEPENDENCIES = @LIBGREPOBJS@

# List of files to be distributed.

EXTRA_DIST += $(LIBADD_SOURCE)

AM_CPPFLAGS = -I.. -I../gnulib-lib -I$(top_srcdir)/gnulib-lib -I../intl