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

bin_PROGRAMS = gettext ngettext \
msgcmp msgfmt msgmerge msgunfmt xgettext \
msgcat msgcomm msgconv msgen msgexec msggrep msguniq

noinst_HEADERS = pos.h message.h po-gram.h po-hash.h po-charset.h po-lex.h \
po.h open-po.h read-po.h str-list.h write-po.h dir-list.h file-list.h \
po-gram-gen.h po-hash-gen.h msgl-charset.h msgl-iconv.h msgl-ascii.h \
msgl-cat.h msgfmt.h read-mo.h write-mo.h xgettext.h x-c.h x-po.h

EXTRA_DIST = FILES

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
po-charset.c po-lex.c po.c str-list.c dir-list.c
msgfmt_SOURCES = msgfmt.c open-po.c po-gram-gen.y po-hash-gen.y po-charset.c \
po-lex.c po.c str-list.c message.c dir-list.c write-mo.c \
format.c format-c.c format-java.c format-lisp.c format-python.c format-ycp.c
msgmerge_SOURCES = message.c msgmerge.c open-po.c po-gram-gen.y po-hash-gen.y \
po-charset.c po-lex.c po.c read-po.c str-list.c dir-list.c write-po.c \
msgl-ascii.c
msgunfmt_SOURCES = message.c msgunfmt.c str-list.c write-po.c msgl-ascii.c \
read-mo.c
xgettext_SOURCES = message.c open-po.c po-gram-gen.y po-hash-gen.y \
po-charset.c po-lex.c po.c str-list.c xgettext.c dir-list.c write-po.c \
msgl-ascii.c file-list.c x-c.c x-po.c x-java.l \
format.c format-c.c format-java.c format-lisp.c format-python.c format-ycp.c
msgcat_SOURCES = msgcat.c message.c open-po.c po-gram-gen.y po-hash-gen.y \
po-charset.c po-lex.c po.c read-po.c str-list.c dir-list.c write-po.c \
msgl-ascii.c msgl-iconv.c msgl-cat.c file-list.c
msgcomm_SOURCES = msgcomm.c message.c open-po.c po-gram-gen.y po-hash-gen.y \
po-charset.c po-lex.c po.c read-po.c str-list.c dir-list.c write-po.c \
msgl-ascii.c msgl-iconv.c msgl-cat.c file-list.c
msgconv_SOURCES = msgconv.c message.c open-po.c po-gram-gen.y po-hash-gen.y \
po-charset.c po-lex.c po.c read-po.c str-list.c dir-list.c write-po.c \
msgl-ascii.c msgl-iconv.c
msgen_SOURCES = msgen.c message.c open-po.c po-gram-gen.y po-hash-gen.y \
po-charset.c po-lex.c po.c read-po.c str-list.c dir-list.c write-po.c \
msgl-ascii.c
msgexec_SOURCES = msgexec.c message.c open-po.c po-gram-gen.y po-hash-gen.y \
po-charset.c po-lex.c po.c read-po.c str-list.c dir-list.c write-po.c \
msgl-ascii.c msgl-charset.c
msggrep_SOURCES = msggrep.c message.c open-po.c po-gram-gen.y po-hash-gen.y \
po-charset.c po-lex.c po.c read-po.c str-list.c dir-list.c write-po.c \
msgl-ascii.c msgl-charset.c
msguniq_SOURCES = msguniq.c message.c open-po.c po-gram-gen.y po-hash-gen.y \
po-charset.c po-lex.c po.c read-po.c str-list.c dir-list.c write-po.c \
msgl-ascii.c msgl-iconv.c msgl-cat.c

# Link dependencies.
# po-lex.c and po.c may need -liconv.
# write-po.c pulls in linebreak.c which may need -liconv.
msgcmp_LDADD = ../lib/libnlsut.a @INTLLIBS@ @LIBICONV@
msgfmt_LDADD = ../lib/libnlsut.a @INTLLIBS@ @LIBICONV@
msgmerge_LDADD = ../lib/libnlsut.a @INTLLIBS@ @LIBICONV@
msgunfmt_LDADD = ../lib/libnlsut.a @INTLLIBS@ @LIBICONV@
xgettext_LDADD = ../lib/libnlsut.a @INTLLIBS@ @LIBICONV@
msgcat_LDADD = ../lib/libnlsut.a @INTLLIBS@ @LIBICONV@
msgcomm_LDADD = ../lib/libnlsut.a @INTLLIBS@ @LIBICONV@
msgconv_LDADD = ../lib/libnlsut.a @INTLLIBS@ @LIBICONV@
msgen_LDADD = ../lib/libnlsut.a @INTLLIBS@ @LIBICONV@
msgexec_LDADD = ../lib/libnlsut.a @INTLLIBS@ @LIBICONV@
msggrep_LDADD = ../lib/libnlsut.a @INTLLIBS@ @LIBICONV@
msguniq_LDADD = ../lib/libnlsut.a @INTLLIBS@ @LIBICONV@

BUILT_SOURCES = po-gram-gen.c po-hash-gen.c po-gram-gen.h po-hash-gen.h

po-lex.o: po-gram-gen2.h
po-gram-gen2.h: po-gram-gen.h
	$(SED) 's/[yY][yY]/po_gram_/g' $(srcdir)/po-gram-gen.h > $@-tmp
	mv $@-tmp $@

x-java.c: x-java.l
	$(LEX) -o$@-tmp -Px_java_yy $(srcdir)/x-java.l
	test "$(LEX)" = ":" || mv $@-tmp $@

DISTCLEANFILES = po-gram-gen2.h
