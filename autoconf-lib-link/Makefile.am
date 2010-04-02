## Makefile for the autoconf-lib-link directory of GNU gettext
## Copyright (C) 1995-1999, 2000-2003, 2005-2006, 2010 Free Software Foundation, Inc.
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

AUTOMAKE_OPTIONS = 1.5 gnu no-dependencies
ACLOCAL = ../build-aux/fixaclocal @ACLOCAL@
ACLOCAL_AMFLAGS = -I m4 -I ../m4
EXTRA_DIST =
DISTCLEANFILES =

SUBDIRS = m4

EXTRA_DIST += config.rpath


# We want to maintain only one copy of config.rpath. The one in this directory
# is the source.

all-local: $(srcdir)/../build-aux/config.rpath

$(srcdir)/../build-aux/config.rpath: $(srcdir)/config.rpath
	cp $(srcdir)/config.rpath $(srcdir)/../build-aux/config.rpath
