## Makefile for the check subdirectory of the GNU NLS Utilities
## Copyright (C) 1995-1997, 2001 Free Software Foundation, Inc.
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

TESTS = gettext-1 gettext-2 msgcmp-1 msgcmp-2 msgfmt-1 msgfmt-2 msgfmt-3 \
	msgfmt-4 msgmerge-1 msgmerge-2 msgmerge-3 msgmerge-4 msgmerge-5 \
	msgunfmt-1 xgettext-1 xgettext-2 xgettext-3 xgettext-4 xgettext-5 \
	xgettext-6 xgettext-7 xgettext-8 xgettext-9 plural-1 plural-2

EXTRA_DIST = $(TESTS) test.mo xg-test1.ok.po

XGETTEXT = ../src/xgettext

TESTS_ENVIRONMENT = top_srcdir=$(top_srcdir) PATH=.:../src:$$PATH \
		    GETTEXT=tstgettext \
		    NGETTEXT=tstngettext \
		    XGETTEXT=`echo xgettext|sed '$(transform)'` \
		    MSGFMT=`echo msgfmt|sed '$(transform)'` \
		    MSGCMP=`echo msgcmp|sed '$(transform)'` \
		    MSGMERGE=`echo msgmerge|sed '$(transform)'` \
		    MSGUNFMT=`echo msgunfmt|sed '$(transform)'` \
		    $(SHELL)

xg-test1.ok.po: $(top_srcdir)/src/xgettext.c $(top_srcdir)/src/msgfmt.c \
		$(top_srcdir)/src/gettext.c
	$(XGETTEXT) -d xg-test1.ok -p $(srcdir) -k_ --omit-header \
	  $(top_srcdir)/src/xgettext.c $(top_srcdir)/src/msgfmt.c \
	  $(top_srcdir)/src/gettext.c

# Two auxiliary programs used by the tests.
INCLUDES = -I.. -I$(top_srcdir)/lib -I$(top_srcdir)/intl
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@
LDADD = $(LDADD_@USE_INCLUDED_LIBINTL@)
LDADD_yes = ../intl/libintl.la
LDADD_no = ../intl/libgnuintl.la @INTLLIBS@
EXTRA_PROGRAMS = tstgettext tstngettext cake
tstgettext_SOURCES = tstgettext.c setlocale.c
tstgettext_LDADD = ../lib/libnlsut.a $(LDADD)
tstngettext_SOURCES = tstngettext.c setlocale.c
tstngettext_LDADD = ../lib/libnlsut.a $(LDADD)
cake_SOURCES = plural-1-prg.c setlocale.c
all-local: $(EXTRA_PROGRAMS)
CLEANFILES = $(EXTRA_PROGRAMS)
