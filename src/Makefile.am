## Makefile for program src directory in GNU NLS utilities package.
## Copyright (C) 1995-1998, 2000-2002 Free Software Foundation, Inc.
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
msgattrib msgcat msgcomm msgconv msgen msgexec msgfilter msggrep msginit msguniq

noinst_PROGRAMS = hostname urlget

lib_LTLIBRARIES = libgettextsrc.la

noinst_HEADERS = pos.h message.h po-gram.h po-hash.h po-charset.h po-lex.h \
po.h open-po.h read-po.h str-list.h write-po.h dir-list.h file-list.h \
po-gram-gen.h po-hash-gen.h msgl-charset.h msgl-equal.h msgl-iconv.h \
msgl-ascii.h msgl-cat.h msgl-english.h msgfmt.h msgunfmt.h read-mo.h \
write-mo.h read-java.h write-java.h po-time.h format.h xgettext.h x-c.h \
x-po.h x-lisp.h x-elisp.h x-librep.h x-java.h x-ycp.h x-rst.h

EXTRA_DIST = FILES project-id \
gnu/gettext/DumpResource.java gnu/gettext/GetURL.java

localedir = $(datadir)/locale
jardir = $(datadir)/gettext
projectsdir = $(pkgdatadir)/projects

INCLUDES = -I. -I$(srcdir) -I.. -I../lib -I$(top_srcdir)/lib -I../intl \
-I$(top_srcdir)/intl
DEFS = -DLOCALEDIR=\"$(localedir)\" -DGETTEXTJAR=\"$(jardir)/gettext.jar\" \
-DLIBDIR=\"$(libdir)\" -DPROJECTSDIR=\"$(projectsdir)\" @DEFS@
LDADD = ../lib/libgettextlib.la @LTLIBINTL@

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
format-c.c format-java.c format-lisp.c format-elisp.c format-librep.c \
format-python.c format-pascal.c format-ycp.c

# libgettextsrc contains all code that is needed by at least two programs.
libgettextsrc_la_SOURCES = \
$(COMMON_SOURCE) read-po.c write-po.c msgl-ascii.c msgl-iconv.c msgl-equal.c \
msgl-cat.c msgl-english.c file-list.c msgl-charset.c po-time.c plural.c \
$(FORMAT_SOURCE)

# Source dependencies.
gettext_SOURCES = gettext.c
ngettext_SOURCES = ngettext.c
msgcmp_SOURCES = msgcmp.c
msgfmt_SOURCES = msgfmt.c write-mo.c write-java.c plural-eval.c
msgmerge_SOURCES = msgmerge.c
msgunfmt_SOURCES = msgunfmt.c read-mo.c read-java.c
xgettext_SOURCES = xgettext.c \
  x-c.c x-po.c x-lisp.c x-elisp.c x-librep.c x-java.l x-ycp.c x-rst.c
msgattrib_SOURCES = msgattrib.c
msgcat_SOURCES = msgcat.c
msgcomm_SOURCES = msgcomm.c
msgconv_SOURCES = msgconv.c
msgen_SOURCES = msgen.c
msgexec_SOURCES = msgexec.c
msgfilter_SOURCES = msgfilter.c
msggrep_SOURCES = msggrep.c
msginit_SOURCES = msginit.c
msguniq_SOURCES = msguniq.c
hostname_SOURCES = hostname.c
urlget_SOURCES = urlget.c

# How to build libgettextsrc.la.
# Need ../lib/libgettextlib.la.
# Need @LTLIBINTL@ because many source files use gettext().
# Need @LTLIBICONV@ because po-charset.c, po-lex.c, msgl-iconv.c, write-po.c
# use iconv().
libgettextsrc_la_LDFLAGS = \
  -release @VERSION@ \
  ../lib/libgettextlib.la @LTLIBINTL@ @LTLIBICONV@ -lc -no-undefined

# Link dependencies.
gettext_LDADD = ../lib/basename.lo ../lib/error.lo ../lib/getopt.lo ../lib/getopt1.lo ../lib/strtoul.lo ../lib/xmalloc.lo @LTLIBINTL@
ngettext_LDADD = ../lib/basename.lo ../lib/error.lo ../lib/getopt.lo ../lib/getopt1.lo ../lib/strtoul.lo ../lib/xmalloc.lo @LTLIBINTL@
msgcmp_LDADD = libgettextsrc.la
msgfmt_LDADD = libgettextsrc.la
msgmerge_LDADD = libgettextsrc.la
msgunfmt_LDADD = libgettextsrc.la
xgettext_LDADD = libgettextsrc.la
msgattrib_LDADD = libgettextsrc.la
msgcat_LDADD = libgettextsrc.la
msgcomm_LDADD = libgettextsrc.la
msgconv_LDADD = libgettextsrc.la
msgen_LDADD = libgettextsrc.la
msgexec_LDADD = libgettextsrc.la
msgfilter_LDADD = libgettextsrc.la
msggrep_LDADD = libgettextsrc.la
msginit_LDADD = ../intl/localealias.$(OBJEXT) ../intl/localename.$(OBJEXT) \
                libgettextsrc.la
msguniq_LDADD = libgettextsrc.la


# Special rules for bison and flex generated files.

BUILT_SOURCES = po-gram-gen.c po-hash-gen.c po-gram-gen.h po-hash-gen.h

po-lex.o po-lex.lo: po-gram-gen2.h
po-gram-gen2.h: po-gram-gen.h
	$(SED) 's/[yY][yY]/po_gram_/g' $(srcdir)/po-gram-gen.h > $@-tmp
	mv $@-tmp $@

x-java.c: x-java.l
	$(LEX) -o$@-tmp -Px_java_yy $(srcdir)/x-java.l
	test "$(LEX)" = ":" || mv $@-tmp $@

DISTCLEANFILES = po-gram-gen2.h user-email


# Special rules for installation of auxiliary programs.

install-exec-local:
	$(mkinstalldirs) $(DESTDIR)$(libdir)/$(PACKAGE)
	$(INSTALL_PROGRAM_ENV) $(LIBTOOL) --mode=install $(INSTALL_PROGRAM) hostname$(EXEEXT) $(DESTDIR)$(libdir)/$(PACKAGE)/hostname$(EXEEXT)
	$(INSTALL_PROGRAM_ENV) $(LIBTOOL) --mode=install $(INSTALL_PROGRAM) urlget$(EXEEXT) $(DESTDIR)$(libdir)/$(PACKAGE)/urlget$(EXEEXT)
	$(INSTALL_SCRIPT) user-email $(DESTDIR)$(libdir)/$(PACKAGE)/user-email
	$(INSTALL_SCRIPT) $(srcdir)/project-id $(DESTDIR)$(libdir)/$(PACKAGE)/project-id

installdirs-local:
	$(mkinstalldirs) $(DESTDIR)$(libdir)/$(PACKAGE)

uninstall-local:
	$(RM) $(DESTDIR)$(libdir)/$(PACKAGE)/hostname$(EXEEXT)
	$(RM) $(DESTDIR)$(libdir)/$(PACKAGE)/urlget$(EXEEXT)
	$(RM) $(DESTDIR)$(libdir)/$(PACKAGE)/user-email
	$(RM) $(DESTDIR)$(libdir)/$(PACKAGE)/project-id


# Special rules for Java compilation.

all-local: all-java-@BUILDJAVA@
all-java-no:
all-java-yes: gettext.jar

gnu/gettext/DumpResource.class: $(srcdir)/gnu/gettext/DumpResource.java
	$(JAVACOMP) -d . $(srcdir)/gnu/gettext/DumpResource.java

gnu/gettext/GetURL.class: $(srcdir)/gnu/gettext/GetURL.java
	$(JAVACOMP) -d . $(srcdir)/gnu/gettext/GetURL.java

gettext.jar: gnu/gettext/DumpResource.class gnu/gettext/GetURL.class
	$(JAR) cf $@ gnu/gettext/DumpResource*.class gnu/gettext/GetURL*.class

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
