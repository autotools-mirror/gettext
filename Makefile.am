## Makefile for the toplevel directory of the GNU NLS Utilities
## Copyright (C) 1995-1999, 2000, 2001 Free Software Foundation, Inc.
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

AUTOMAKE_OPTIONS = 1.3 gnits
MAINT_CHARSET = latin1
ACLOCAL = ./aclocal.sh @ACLOCAL@
ACLOCAL_AMFLAGS = -I m4

gettextsrcdir = $(datadir)/gettext
gettextsrc_DATA = ABOUT-NLS

SUBDIRS = doc intl lib src po man m4 misc tests

EXTRA_DIST = BUGS DISCLAIM README.gemtext aclocal.sh \
             djgpp/Makefile.maint djgpp/README.in djgpp/README \
             djgpp/config.bat djgpp/config.in djgpp/config.sed \
             djgpp/config.site djgpp/edtests.bat djgpp/fnchange.in \
             djgpp/fnchange.lst djgpp/tscript.sed \
             os2/README.os2 os2/os2.diff

ABOUT-NLS: $(srcdir)/doc/nls.texi $(srcdir)/doc/matrix.texi
	rm -f $(srcdir)/ABOUT-NLS \
	  && $(MAKEINFO) --output=$(srcdir)/ABOUT-NLS --no-header \
	    -I $(srcdir)/doc --no-validate nls.texi


# Documentation tags not known to automake.

ps:
	cd doc && $(MAKE) ps

pdf:
	cd doc && $(MAKE) pdf

html:
	cd doc && $(MAKE) html
