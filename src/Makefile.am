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

AUTOMAKE_OPTIONS = 1.5 gnits no-dependencies

bin_PROGRAMS = gettext ngettext \
msgcmp msgfmt msgmerge msgunfmt xgettext \
msgattrib msgcat msgcomm msgconv msgen msgexec msggrep msguniq

noinst_PROGRAMS = hostname

noinst_HEADERS = pos.h message.h po-gram.h po-hash.h po-charset.h po-lex.h \
po.h open-po.h read-po.h str-list.h write-po.h dir-list.h file-list.h \
po-gram-gen.h po-hash-gen.h msgl-charset.h msgl-equal.h msgl-iconv.h \
msgl-ascii.h msgl-cat.h msgfmt.h read-mo.h write-mo.h xgettext.h x-c.h x-po.h \
x-java.h x-ycp.h x-rst.h

EXTRA_DIST = FILES

localedir = $(datadir)/locale
jardir = $(datadir)/gettext

INCLUDES = -I. -I$(srcdir) -I.. -I$(top_srcdir)/lib -I../intl \
-I$(top_srcdir)/intl
DEFS = -DLOCALEDIR=\"$(localedir)\" -DGETTEXTJAR=\"$(jardir)/gettext.jar\" \
@DEFS@
LDADD = ../lib/libnlsut.a @INTLLIBS@

SED = sed
YACC = @YACC@ -d
JAR = @JAR@
JAVACOMP = $(SHELL) ../lib/javacomp.sh


# All programs deal with message lists.
# All programs must read PO files. (msgunfmt also, for read-java.c.)
# message.c -> str-list.c.
# (po.c <--> po-hash-gen.y <--> po-gram-gen.y <--> po-lex.c) -> str-list.c.
# (po.c <--> po-hash-gen.y <--> po-gram-gen.y <--> po-lex.c) -> open-po.c -> dir-list.c -> str-list.c.
# (po.c <--> po-hash-gen.y <--> po-gram-gen.y <--> po-lex.c) -> po-charset.c.
# (po.c <--> po-hash-gen.y <--> po-gram-gen.y <--> po-lex.c) -> message.c -> str-list.c.
COMMON_SOURCE = message.c \
po.c po-lex.c po-gram-gen.y po-hash-gen.y po-charset.c \
open-po.c dir-list.c str-list.c

# xgettext and msgfmt deal with format strings.
FORMAT_SOURCE = format.c \
format-c.c format-java.c format-lisp.c format-python.c format-pascal.c \
format-ycp.c

# Source dependencies.
gettext_SOURCES = gettext.c
ngettext_SOURCES = ngettext.c
msgcmp_SOURCES    = msgcmp.c    $(COMMON_SOURCE)
msgfmt_SOURCES    = msgfmt.c    $(COMMON_SOURCE) msgl-ascii.c msgl-iconv.c write-mo.c write-java.c plural.c plural-eval.c $(FORMAT_SOURCE)
msgmerge_SOURCES  = msgmerge.c  $(COMMON_SOURCE) msgl-ascii.c write-po.c read-po.c msgl-equal.c
msgunfmt_SOURCES  = msgunfmt.c  $(COMMON_SOURCE) msgl-ascii.c write-po.c read-po.c read-mo.c read-java.c
xgettext_SOURCES  = xgettext.c  $(COMMON_SOURCE) msgl-ascii.c write-po.c file-list.c x-c.c x-po.c x-java.l x-ycp.c x-rst.c $(FORMAT_SOURCE)
msgattrib_SOURCES = msgattrib.c $(COMMON_SOURCE) msgl-ascii.c write-po.c read-po.c
msgcat_SOURCES    = msgcat.c    $(COMMON_SOURCE) msgl-ascii.c write-po.c read-po.c msgl-iconv.c msgl-cat.c file-list.c
msgcomm_SOURCES   = msgcomm.c   $(COMMON_SOURCE) msgl-ascii.c write-po.c read-po.c msgl-iconv.c msgl-cat.c file-list.c
msgconv_SOURCES   = msgconv.c   $(COMMON_SOURCE) msgl-ascii.c write-po.c read-po.c msgl-iconv.c
msgen_SOURCES     = msgen.c     $(COMMON_SOURCE) msgl-ascii.c write-po.c read-po.c
msgexec_SOURCES   = msgexec.c   $(COMMON_SOURCE) msgl-ascii.c write-po.c read-po.c msgl-charset.c
msggrep_SOURCES   = msggrep.c   $(COMMON_SOURCE) msgl-ascii.c write-po.c read-po.c msgl-charset.c
msguniq_SOURCES   = msguniq.c   $(COMMON_SOURCE) msgl-ascii.c write-po.c read-po.c msgl-iconv.c msgl-cat.c
hostname_SOURCES  = hostname.c

# Link dependencies.
# po-lex.c and po.c may need -liconv.
# write-po.c pulls in linebreak.c which may need -liconv.
msgcmp_LDADD = ../lib/libnlsut.a @INTLLIBS@ @LIBICONV@
msgfmt_LDADD = ../lib/libnlsut.a @INTLLIBS@ @LIBICONV@
msgmerge_LDADD = ../lib/libnlsut.a @INTLLIBS@ @LIBICONV@
msgunfmt_LDADD = ../lib/libnlsut.a @INTLLIBS@ @LIBICONV@
xgettext_LDADD = ../lib/libnlsut.a @INTLLIBS@ @LIBICONV@
msgattrib_LDADD = ../lib/libnlsut.a @INTLLIBS@ @LIBICONV@
msgcat_LDADD = ../lib/libnlsut.a @INTLLIBS@ @LIBICONV@
msgcomm_LDADD = ../lib/libnlsut.a @INTLLIBS@ @LIBICONV@
msgconv_LDADD = ../lib/libnlsut.a @INTLLIBS@ @LIBICONV@
msgen_LDADD = ../lib/libnlsut.a @INTLLIBS@ @LIBICONV@
msgexec_LDADD = ../lib/libnlsut.a @INTLLIBS@ @LIBICONV@
msggrep_LDADD = ../lib/libnlsut.a @INTLLIBS@ @LIBICONV@
msguniq_LDADD = ../lib/libnlsut.a @INTLLIBS@ @LIBICONV@


# Special rules for bison and flex generated files.

BUILT_SOURCES = po-gram-gen.c po-hash-gen.c po-gram-gen.h po-hash-gen.h

po-lex.o: po-gram-gen2.h
po-gram-gen2.h: po-gram-gen.h
	$(SED) 's/[yY][yY]/po_gram_/g' $(srcdir)/po-gram-gen.h > $@-tmp
	mv $@-tmp $@

x-java.c: x-java.l
	$(LEX) -o$@-tmp -Px_java_yy $(srcdir)/x-java.l
	test "$(LEX)" = ":" || mv $@-tmp $@

DISTCLEANFILES = po-gram-gen2.h


# Special rules for installation of auxiliary programs.

install-exec-local:
	$(mkinstalldirs) $(DESTDIR)$(libdir)/$(PACKAGE)
	$(INSTALL_PROGRAM_ENV) $(LIBTOOL) --mode=install $(INSTALL_PROGRAM) hostname$(EXEEXT) $(DESTDIR)$(libdir)/$(PACKAGE)/hostname$(EXEEXT)

installdirs-local:
	$(mkinstalldirs) $(DESTDIR)$(libdir)/$(PACKAGE)

uninstall-local:
	$(RM) $(DESTDIR)$(libdir)/$(PACKAGE)/hostname$(EXEEXT)


# Special rules for Java compilation.

all-local: all-java-@BUILDJAVA@
all-java-no:
all-java-yes: gettext.jar

gnu/gettext/DumpResource.class: $(srcdir)/gnu/gettext/DumpResource.java
	$(JAVACOMP) -d . $(srcdir)/gnu/gettext/DumpResource.java

gettext.jar: gnu/gettext/DumpResource.class
	$(JAR) cf $@ gnu/gettext/DumpResource*.class

CLEANFILES = gettext.jar gnu/gettext/*.class

install-data-local: install-java-@BUILDJAVA@
install-java-no:
install-java-yes: all-java-yes
	$(INSTALL_DATA) gettext.jar $(DESTDIR)$(jardir)/gettext.jar

installdirs-local: installdirs-java-@BUILDJAVA@
installdirs-java-no:
installdirs-java-yes:
	$(mkinstalldirs) $(DESTDIR)$(jardir)

uninstall-local: uninstall-java-@BUILDJAVA@
uninstall-java-no:
uninstall-java-yes:
	$(RM) $(DESTDIR)$(jardir)/gettext.jar


# One more automake bug.
installdirs: installdirs-local
