## Makefile for the gettext-tools/misc subdirectory of GNU gettext
## Copyright (C) 1995-1997, 2000-2003, 2005-2007 Free Software Foundation, Inc.
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

AUTOMAKE_OPTIONS = 1.2 gnits
EXTRA_DIST =
DISTCLEANFILES =


# Scripts for maintainers.

bin_SCRIPTS = gettextize autopoint

DISTCLEANFILES += gettextize autopoint

EXTRA_DIST += add-to-archive cvsuser.c archive.tar.gz

gettextsrcdir = $(datadir)/gettext
gettextsrc_DATA = archive.tar.gz

# The archive.tar.gz is not in the CVS. Alpha-testers must use the one from an
# already installed release.
archive.tar.gz:
	if test -f $(gettextsrcdir)/archive.tar.gz; then \
	  cp -p $(gettextsrcdir)/archive.tar.gz $@; \
	else \
	  echo "archive.tar.gz is not in the CVS. Please copy the archive.tar.gz from an already installed release to gettext-tools/misc/." 1>&2; \
	  exit 1; \
	fi

# Emacs Lisp code.

lisp_LISP = start-po.el po-mode.el po-compat.el

EXTRA_DIST += $(lisp_LISP)
