## Makefile for the gettext-tools directory of GNU gettext
## Copyright (C) 1995-1999, 2000-2003 Free Software Foundation, Inc.
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

AUTOMAKE_OPTIONS = 1.5 gnu no-dependencies
ACLOCAL_AMFLAGS = -I m4 -I ../gettext-runtime/m4 -I ../autoconf-lib-link/m4 -I ../config/m4

SUBDIRS = doc intl lib libuniname src po projects misc man m4 tests

EXTRA_DIST = misc/DISCLAIM


# Files installed for use by gettextize.

gettextsrcdir = $(datadir)/gettext
gettextsrc_SCRIPTS = ../autoconf-lib-link/config.rpath ../config/mkinstalldirs


# Files copied from other directories.

all-local: $(srcdir)/ABOUT-NLS
$(srcdir)/ABOUT-NLS: $(srcdir)/../gettext-runtime/ABOUT-NLS
	cp $(srcdir)/../gettext-runtime/ABOUT-NLS $(srcdir)/ABOUT-NLS


# Documentation tag not known to automake.

html:
	cd doc && $(MAKE) html
