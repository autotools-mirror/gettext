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


# Documentation tag not known to automake.

html:
	cd doc && $(MAKE) html


all-local: ABOUT-NLS

ABOUT-NLS: $(srcdir)/doc/nls.texi $(srcdir)/doc/matrix.texi
	rm -f $(srcdir)/ABOUT-NLS \
	  && $(MAKEINFO) --output=$(srcdir)/ABOUT-NLS --no-header \
	    -I $(srcdir)/doc --no-validate nls.texi


# Woe32 support.

config.h.msvc: config.h.in
	sed -e 's/#undef ENABLE_NLS$$/#define ENABLE_NLS 1/' \
	    -e 's/#undef ENABLE_RELOCATABLE$$/#define ENABLE_RELOCATABLE 1/' \
	    -e 's/#undef HAVE_ALLOCA$$/#define HAVE_ALLOCA 1/' \
	    -e 's/#undef HAVE_ENVIRON_DECL$$/#define HAVE_ENVIRON_DECL 1/' \
	    -e 's/#undef HAVE_ERRNO_DECL$$/#define HAVE_ERRNO_DECL 1/' \
	    -e 's/#undef HAVE_GETCWD$$/#define HAVE_GETCWD 1/' \
	    -e 's/#undef HAVE_ICONV$$/#define HAVE_ICONV 1/' \
	    -e 's/#undef HAVE_LIMITS_H$$/#define HAVE_LIMITS_H 1/' \
	    -e 's/#undef HAVE_LOCALE_H$$/#define HAVE_LOCALE_H 1/' \
	    -e 's/#undef HAVE_MEMMOVE$$/#define HAVE_MEMMOVE 1/' \
	    -e 's/#undef HAVE_PUTENV$$/#define HAVE_PUTENV 1/' \
	    -e 's/#undef HAVE_SETLOCALE$$/#define HAVE_SETLOCALE 1/' \
	    -e 's/#undef HAVE_STDDEF_H$$/#define HAVE_STDDEF_H 1/' \
	    -e 's/#undef HAVE_STDLIB_H$$/#define HAVE_STDLIB_H 1/' \
	    -e 's/#undef HAVE_STRDUP$$/#define HAVE_STRDUP 1/' \
	    -e 's/#undef HAVE_STRERROR$$/#define HAVE_STRERROR 1/' \
	    -e 's/#undef HAVE_STRING_H$$/#define HAVE_STRING_H 1/' \
	    -e 's/#undef HAVE_STRTOUL$$/#define HAVE_STRTOUL 1/' \
	    -e 's/#undef HAVE_SYS_STAT_H$$/#define HAVE_SYS_STAT_H 1/' \
	    -e 's/#undef HAVE_SYS_TYPES_H$$/#define HAVE_SYS_TYPES_H 1/' \
	    -e 's/#undef HAVE_VPRINTF$$/#define HAVE_VPRINTF 1/' \
	    -e 's/#undef ICONV_CONST$$/#define ICONV_CONST const/' \
	    -e 's/#undef STACK_DIRECTION$$/#define STACK_DIRECTION -1/' \
	    -e 's/#undef STDC_HEADERS$$/#define STDC_HEADERS 1/' \
	    -e 's/#undef uintmax_t$$/#define uintmax_t unsigned long/' \
	    -e '/#undef INSTALLPREFIX$$/d' \
	  < $< > $@

intl/libgnuintl.h.msvc-shared: intl/libgnuintl.h.in windows/dllexport.h
	sed -e 's/extern \([^"]\)/extern LIBINTL_DLL_EXPORTED \1/' -e '/#define _LIBINTL_H/r windows/dllexport.h' < $< > $@

EXTRA_DIST += README.woe32 Makefile.msvc config.h.msvc windows/dllexport.h windows/intl.rc
