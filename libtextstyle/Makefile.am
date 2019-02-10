## Makefile for the toplevel directory of GNU libtextstyle
## Copyright (C) 1995-2019 Free Software Foundation, Inc.
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

AUTOMAKE_OPTIONS = 1.13 gnu no-dependencies
ACLOCAL_AMFLAGS = -I m4 -I gnulib-m4

# The list of subdirectories containing Makefiles.
SUBDIRS = doc lib tests

EXTRA_DIST = \
  version.sh \
  DEPENDENCIES \
  HACKING \
  autogen.sh \
  build-aux/texi2html \
  build-aux/run-test \
  build-aux/test-driver.diff \
  examples/color-filter/README \
  examples/color-filter/INSTALL \
  examples/color-filter/autogen.sh \
  examples/color-filter/autoclean.sh \
  examples/color-filter/configure.ac \
  examples/color-filter/Makefile.am \
  examples/color-filter/filter.c \
  examples/color-filter/filter-default.css \
  examples/color-hello/README \
  examples/color-hello/INSTALL \
  examples/color-hello/autogen.sh \
  examples/color-hello/autoclean.sh \
  examples/color-hello/configure.ac \
  examples/color-hello/Makefile.am \
  examples/color-hello/hello.c \
  examples/color-hello/names.c \
  examples/color-hello/hello-default.css
#TEMPORARY
#  m4/woe32-dll.m4


# Allow users to use "gnulib-tool --update".
EXTRA_DIST += gnulib-m4/gnulib-cache.m4


# Cygwin support.

#TEMPORARY
#EXTRA_DIST += woe32dll/export.h


# Windows support.

EXTRA_DIST += INSTALL.windows
#TEMPORARY
#EXTRA_DIST += INSTALL.windows windows/windres-options
