## Makefile for the gettext-runtime/lib subdirectory of GNU gettext
## Copyright (C) 1995-1998, 2000-2003 Free Software Foundation, Inc.
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
## along with this program; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

## Process this file with automake to produce Makefile.in.

AUTOMAKE_OPTIONS = 1.5 gnits no-dependencies

noinst_LIBRARIES = libgrt.a

# Sources that are compiled on all platforms.

libgrt_a_SOURCES = \
  basename.h basename.c \
  error.h error.c \
  exit.h \
  getopt.h getopt.c getopt1.c \
  xmalloc.h xmalloc.c

INCLUDES = \
  -I. -I$(srcdir) \
  -I.. \
  -I../intl -I$(srcdir)/../intl \
  -I../../gettext-tools/lib -I$(srcdir)/../../gettext-tools/lib
