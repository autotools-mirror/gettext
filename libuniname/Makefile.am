## Makefile for libuniname directory in GNU gettext package.
## Copyright (C) 2002 Free Software Foundation, Inc.
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

noinst_LIBRARIES = libuniname.a

libuniname_a_SOURCES = uniname.h uniname.c uninames.h

INCLUDES = -I.. -I../lib -I$(top_srcdir)/lib

TESTS = test-names.sh
TESTS_ENVIRONMENT = top_srcdir=$(top_srcdir) $(SHELL)

noinst_PROGRAMS = test-names
test_names_LDADD = libuniname.a ../lib/libgettextlib.la

EXTRA_DIST = $(TESTS) gen-uninames UnicodeDataNames.txt
