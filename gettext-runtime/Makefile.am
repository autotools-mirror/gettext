## Makefile for the gettext-runtime directory of GNU gettext
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
ACLOCAL_AMFLAGS = -I m4 -I ../gettext-tools/m4 -I ../autoconf-lib-link/m4 -I ../config/m4

MAKEINFO = env LANG= LANGUAGE= @MAKEINFO@

SUBDIRS =      doc intl intl-java lib @SUBDIR_libasprintf@ src po man m4
DIST_SUBDIRS = doc intl intl-java lib libasprintf          src po man m4

EXTRA_DIST = BUGS


# Files installed for the user and for use by gettextize.

gettextsrcdir = $(datadir)/gettext
gettextsrc_DATA = ABOUT-NLS


# Documentation tags not known to automake.

ps:
	cd doc && $(MAKE) ps

pdf:
	cd doc && $(MAKE) pdf

html:
	cd doc && $(MAKE) html


all-local: ABOUT-NLS

ABOUT-NLS: $(srcdir)/doc/nls.texi $(srcdir)/doc/matrix.texi
	rm -f $(srcdir)/ABOUT-NLS \
	  && $(MAKEINFO) --output=$(srcdir)/ABOUT-NLS --no-header \
	    -I $(srcdir)/doc --no-validate nls.texi
