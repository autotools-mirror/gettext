## Makefile for the misc subdirectory of the GNU NLS Utilities
## Copyright (C) 1995-1997, 2000-2002 Free Software Foundation, Inc.
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

AUTOMAKE_OPTIONS = 1.2 gnits

EXTRA_DIST = gettext.perl gettext-sh tcl_gettext.c README-Tcl add-to-archive cvsuser.c archive.tar.gz $(lisp_LISP)
DISTCLEANFILES = gettextize autopoint
lisp_LISP = po-mode.el po-compat.el

bin_SCRIPTS = gettextize autopoint

gettextsrcdir = $(datadir)/gettext
gettextsrc_DATA = archive.tar.gz

archive.tar.gz:
	(cd autopoint-files && tar cf - --owner=root --group=root `find archive -type f | grep -v /CVS/`) | gzip -c -9 > $@
