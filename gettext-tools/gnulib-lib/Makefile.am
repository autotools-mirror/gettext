## Makefile for the gettext-tools/gnulib-lib subdirectory of GNU gettext
## Copyright (C) 1995-2024 Free Software Foundation, Inc.
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
## along with this program.  If not, see <https://www.gnu.org/licenses/>.

## Process this file with automake to produce Makefile.in.

AUTOMAKE_OPTIONS = 1.5 gnits subdir-objects
EXTRA_DIST =
BUILT_SOURCES =
MOSTLYCLEANFILES = core *.stackdump
MOSTLYCLEANDIRS =
CLEANFILES =
DISTCLEANFILES =
MAINTAINERCLEANFILES =
SUFFIXES =

lib_LTLIBRARIES =
noinst_LTLIBRARIES =

# Extra files to be installed.

# Where to install javaversion.class.
pkgdatadir = $(datadir)/gettext
pkgdata_DATA =

# Note that Automake's $(DEFAULT_INCLUDES) already contains
# -I. -I$(srcdir) -I$(top_builddir).
AM_CPPFLAGS =

# Needed for the gnulib source code that does '#include "gettext.h"'.
AM_CPPFLAGS += -I../../gettext-runtime/intl
# Needed so that woe32dll/export.h is found.
AM_CPPFLAGS += -I$(top_srcdir)

# Parametrization of the 'relocatable' module.
AM_CPPFLAGS += -DDEPENDS_ON_LIBICONV=1 -DDEPENDS_ON_LIBINTL=1

# Parametrization of the 'libxml' module:
# When building a shared library, don't export the variables
# xmlMalloc, xmlMallocAtomic, xmlRealloc, xmlFree, xmlMemStrdup.
if INCLUDED_LIBXML
AM_CPPFLAGS += -DLIBXML_STATIC
endif

# Enable more warning options in this directory.
AM_CFLAGS = @WARN_CFLAGS@

# Rules generated and collected by gnulib-tool.
include Makefile.gnulib

# OS/2 does not support a DLL name longer than 8 characters.
if OS2
libgettextlib_la_LDFLAGS += -os2dllname gtlib
endif

# Specify installation directory, for --enable-relocatable.
if RELOCATABLE_VIA_LD
# This is needed, because libgettextlib depends on libintl.
libgettextlib_la_LDFLAGS += `$(RELOCATABLE_LDFLAGS) $(libdir)`
endif

# Support for relocatability.
RELOCATABLE_LIBRARY_PATH = $(libdir)

# Linking with C++ libraries is needed _only_ on mingw and Cygwin.
if !WOE32DLL
libgettextlib_la_LINK = $(LIBTOOL) $(AM_V_lt) --tag=CC \
	$(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=link \
	$(CCLD) $(AM_CFLAGS) $(CFLAGS) $(libgettextlib_la_LDFLAGS) $(LDFLAGS) \
	-o $@
else
libgettextlib_la_LINK = $(LIBTOOL) $(AM_V_lt) --tag=CXX \
	$(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=link \
	$(CXXLD) $(AM_CXXFLAGS) $(CXXFLAGS) $(libgettextlib_la_LDFLAGS) $(LDFLAGS) \
	-o $@
# Use the same definition of CXXLD as Automake generates in other Makefiles.
CXXLD = $(CXX)
endif

