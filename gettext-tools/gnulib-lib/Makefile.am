## Makefile for the gettext-tools/gnulib-lib subdirectory of GNU gettext
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

AUTOMAKE_OPTIONS = 1.5 gnits subdir-objects no-dependencies
EXTRA_DIST =
BUILT_SOURCES =
MOSTLYCLEANFILES = core *.stackdump
MOSTLYCLEANDIRS =
CLEANFILES =
DISTCLEANFILES =
MAINTAINERCLEANFILES =
SUFFIXES =

noinst_LTLIBRARIES =

# Extra files to be installed.

gettextsrcdir = $(datadir)/gettext
gettextsrc_DATA = gettext.h

# Where to install javaversion.class.
pkgdatadir = $(datadir)/gettext
pkgdata_DATA =

AM_CPPFLAGS =

# Hmm... is this still necessary?
AM_CPPFLAGS += -I$(top_builddir)/intl -I$(top_srcdir)/intl
# Needed so that config.h and woe32dll/export.h are found.
AM_CPPFLAGS += -I$(top_builddir) -I$(top_srcdir)

# Parametrization of the 'relocatable' module.
AM_CPPFLAGS += -DDEPENDS_ON_LIBICONV=1 -DDEPENDS_ON_LIBINTL=1

# Rules generated and collected by gnulib-tool.
include Makefile.gnulib

# Which classes to export from the shared library.
MOOPPFLAGS += --dllexport=styled_ostream
