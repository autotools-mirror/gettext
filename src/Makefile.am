## Makefile for program src directory in GNU NLS utilities package.
## Copyright (C) 1995-1998, 2000, 2001 Free Software Foundation, Inc.
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

bin_PROGRAMS = gettext ngettext msgcmp msgfmt msgmerge msgunfmt xgettext msgcomm

noinst_HEADERS = pos.h message.h po-gram.h po-hash.h po-lex.h po.h open-po.h \
str-list.h write-po.h xget-lex.h dir-list.h po-gram-gen.h po-hash-gen.h

EXTRA_DIST = FILES

datadir = $(prefix)/share
localedir = $(datadir)/locale

INCLUDES = -I. -I$(srcdir) -I.. -I$(top_srcdir)/lib -I../intl \
-I$(top_srcdir)/intl
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@
LDADD = ../lib/libnlsut.a @INTLLIBS@

SED = sed
YACC = @YACC@ -d

# Source dependencies.
gettext_SOURCES = gettext.c
ngettext_SOURCES = ngettext.c
msgcmp_SOURCES = message.c msgcmp.c open-po.c po-gram-gen.y po-hash-gen.y \
po-lex.c po.c str-list.c dir-list.c
msgfmt_SOURCES = msgfmt.c open-po.c po-gram-gen.y po-hash-gen.y po-lex.c po.c \
str-list.c message.c dir-list.c
msgmerge_SOURCES = message.c msgmerge.c open-po.c po-gram-gen.y po-hash-gen.y \
po-lex.c po.c str-list.c dir-list.c write-po.c
msgunfmt_SOURCES = message.c msgunfmt.c str-list.c write-po.c
xgettext_SOURCES = message.c open-po.c po-gram-gen.y po-hash-gen.y po-lex.c \
po.c str-list.c xget-lex.c xgettext.c dir-list.c write-po.c
msgcomm_SOURCES = msgcomm.c message.c po-gram-gen.y po-hash-gen.y po-lex.c \
open-po.c po.c str-list.c dir-list.c write-po.c

# Link dependencies.
# po-lex.c and po.c may need -liconv.
# write-po.c pulls in linebreak.c which may need -liconv.
msgcmp_LDADD = ../lib/libnlsut.a @INTLLIBS@ @LIBICONV@
msgfmt_LDADD = ../lib/libnlsut.a @INTLLIBS@ @LIBICONV@
msgmerge_LDADD = ../lib/libnlsut.a @INTLLIBS@ @LIBICONV@
msgunfmt_LDADD = ../lib/libnlsut.a @INTLLIBS@ @LIBICONV@
xgettext_LDADD = ../lib/libnlsut.a @INTLLIBS@ @LIBICONV@
msgcomm_LDADD = ../lib/libnlsut.a @INTLLIBS@ @LIBICONV@

BUILT_SOURCES = po-gram-gen.c po-hash-gen.c po-gram-gen.h po-hash-gen.h

po-lex.o: po-gram-gen2.h
po-gram-gen2.h: po-gram-gen.h
	$(SED) 's/[yY][yY]/po_gram_/g' $(srcdir)/po-gram-gen.h > $@-tmp
	mv $@-tmp $@

DISTCLEANFILES = po-gram-gen2.h
