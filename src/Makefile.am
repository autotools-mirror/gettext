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

RM = rm -f

bin_PROGRAMS = gettext ngettext \
msgcmp msgfmt msgmerge msgunfmt xgettext \
msgattrib msgcat msgcomm msgconv msgen msgexec msgfilter msggrep msginit msguniq

noinst_PROGRAMS = hostname urlget

lib_LTLIBRARIES = libgettextsrc.la

noinst_HEADERS = pos.h message.h po-gram.h po-hash.h po-charset.h po-lex.h \
po.h open-po.h read-po.h str-list.h write-po.h dir-list.h file-list.h \
po-gram-gen.h po-hash-gen.h msgl-charset.h msgl-equal.h msgl-iconv.h \
msgl-ascii.h msgl-cat.h msgl-english.h msgfmt.h msgunfmt.h read-mo.h \
write-mo.h read-java.h write-java.h read-tcl.h write-tcl.h po-time.h \
plural-table.h format.h xgettext.h x-c.h x-po.h x-python.h x-lisp.h \
x-elisp.h x-librep.h x-java.h x-awk.h x-ycp.h x-tcl.h x-rst.h x-glade.h

EXTRA_DIST = FILES project-id msgunfmt.tcl \
gnu/gettext/DumpResource.java gnu/gettext/GetURL.java

localedir = $(datadir)/locale
jardir = $(datadir)/gettext
projectsdir = $(pkgdatadir)/projects

INCLUDES = -I. -I$(srcdir) -I.. -I$(top_srcdir)/libuniname \
-I../lib -I$(top_srcdir)/lib -I../intl -I$(top_srcdir)/intl
DEFS = -DLOCALEDIR=\"$(localedir)\" -DUSEJEXE=$(USEJEXE) \
-DGETTEXTJEXEDIR=\"$(pkglibdir)\" -DGETTEXTJAR=\"$(jardir)/gettext.jar\" \
-DLIBDIR=\"$(libdir)\" -DGETTEXTDATADIR=\"$(pkgdatadir)\" \
-DPROJECTSDIR=\"$(projectsdir)\" @DEFS@
LDADD = ../lib/libgettextlib.la @LTLIBINTL@

SED = sed
YACC = @YACC@ -d
GCJ = @GCJ@
GCJFLAGS = @GCJFLAGS@
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
format-c.c format-python.c format-lisp.c format-elisp.c format-librep.c \
format-java.c format-awk.c format-pascal.c format-ycp.c format-tcl.c

# libgettextsrc contains all code that is needed by at least two programs.
libgettextsrc_la_SOURCES = \
$(COMMON_SOURCE) read-po.c write-po.c msgl-ascii.c msgl-iconv.c msgl-equal.c \
msgl-cat.c msgl-english.c file-list.c msgl-charset.c po-time.c plural.c \
plural-table.c $(FORMAT_SOURCE)

# x-python needs table of Unicode character names.
LIBUNINAME = ../libuniname/libuniname.a

# Source dependencies.
gettext_SOURCES = gettext.c
ngettext_SOURCES = ngettext.c
msgcmp_SOURCES = msgcmp.c
msgfmt_SOURCES = msgfmt.c write-mo.c write-java.c write-tcl.c plural-eval.c
msgmerge_SOURCES = msgmerge.c
msgunfmt_SOURCES = msgunfmt.c read-mo.c read-java.c read-tcl.c
xgettext_SOURCES = xgettext.c \
  x-c.c x-po.c x-python.c x-lisp.c x-elisp.c x-librep.c x-java.l x-awk.c \
  x-ycp.c x-tcl.c x-rst.c x-glade.c
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

# No need to install libgettextsrc.a.
install-exec-am: install-exec-clean
install-exec-clean:
	$(RM) $(DESTDIR)$(libdir)/libgettextsrc.a

# Link dependencies.
gettext_LDADD = ../lib/basename.lo ../lib/error.lo ../lib/getopt.lo ../lib/getopt1.lo ../lib/strtoul.lo ../lib/xmalloc.lo @LTLIBINTL@
ngettext_LDADD = ../lib/basename.lo ../lib/error.lo ../lib/getopt.lo ../lib/getopt1.lo ../lib/strtoul.lo ../lib/xmalloc.lo @LTLIBINTL@
msgcmp_LDADD = libgettextsrc.la
msgfmt_LDADD = libgettextsrc.la
msgmerge_LDADD = libgettextsrc.la
msgunfmt_LDADD = libgettextsrc.la
xgettext_LDADD = libgettextsrc.la $(LIBUNINAME) @LTLIBEXPAT@
msgattrib_LDADD = libgettextsrc.la
msgcat_LDADD = libgettextsrc.la
msgcomm_LDADD = libgettextsrc.la
msgconv_LDADD = libgettextsrc.la
msgen_LDADD = libgettextsrc.la
msgexec_LDADD = libgettextsrc.la
msgfilter_LDADD = libgettextsrc.la
msggrep_LDADD = libgettextsrc.la
msginit_LDADD = ../intl/localealias.@INTL_LIBTOOL_SUFFIX_PREFIX@o \
                ../intl/localename.@INTL_LIBTOOL_SUFFIX_PREFIX@o \
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

USEJEXE = $(USEJEXE_@BUILDJAVAEXE@)
USEJEXE_yes = 1
USEJEXE_no = 0

all-local: all-java-@BUILDJAVAEXE@
all-java-yes: gnu.gettext.DumpResource$(EXEEXT) gnu.gettext.GetURL$(EXEEXT)
all-java-no: all-java-no-@BUILDJAVA@
all-java-no-yes: gettext.jar
all-java-no-no:

gnu.gettext.DumpResource$(EXEEXT): $(srcdir)/gnu/gettext/DumpResource.java
	$(GCJ) $(GCJFLAGS) $(srcdir)/gnu/gettext/DumpResource.java --main=gnu.gettext.DumpResource -o $@

gnu.gettext.GetURL$(EXEEXT): $(srcdir)/gnu/gettext/GetURL.java
	$(GCJ) $(GCJFLAGS) $(srcdir)/gnu/gettext/GetURL.java --main=gnu.gettext.GetURL -o $@

gnu/gettext/DumpResource.class: $(srcdir)/gnu/gettext/DumpResource.java
	$(JAVACOMP) -d . $(srcdir)/gnu/gettext/DumpResource.java

gnu/gettext/GetURL.class: $(srcdir)/gnu/gettext/GetURL.java
	$(JAVACOMP) -d . $(srcdir)/gnu/gettext/GetURL.java

gettext.jar: gnu/gettext/DumpResource.class gnu/gettext/GetURL.class
	$(JAR) cf $@ gnu/gettext/DumpResource*.class gnu/gettext/GetURL*.class

CLEANFILES = gnu.gettext.DumpResource$(EXEEXT) gnu.gettext.GetURL$(EXEEXT) \
             gettext.jar gnu/gettext/*.class

install-exec-local: install-exec-java-@BUILDJAVAEXE@
install-exec-java-yes: all-java-yes
	$(mkinstalldirs) $(DESTDIR)$(libdir)/$(PACKAGE)
	$(INSTALL_PROGRAM) gnu.gettext.DumpResource$(EXEEXT) $(DESTDIR)$(libdir)/$(PACKAGE)/gnu.gettext.DumpResource$(EXEEXT)
	$(INSTALL_PROGRAM) gnu.gettext.GetURL$(EXEEXT) $(DESTDIR)$(libdir)/$(PACKAGE)/gnu.gettext.GetURL$(EXEEXT)
install-exec-java-no:

install-data-local: install-data-java-@BUILDJAVAEXE@
install-data-java-yes:
install-data-java-no: install-data-java-no-@BUILDJAVA@
install-data-java-no-yes: all-java-no-yes
	$(INSTALL_DATA) gettext.jar $(DESTDIR)$(jardir)/gettext.jar
install-data-java-no-no:

installdirs-local: installdirs-java-@BUILDJAVAEXE@
installdirs-java-yes:
	$(mkinstalldirs) $(DESTDIR)$(libdir)/$(PACKAGE)
installdirs-java-no: installdirs-java-no-@BUILDJAVA@
installdirs-java-no-yes:
	$(mkinstalldirs) $(DESTDIR)$(jardir)
installdirs-java-no-no:

uninstall-local: uninstall-java-@BUILDJAVAEXE@
uninstall-java-yes:
	$(RM) $(DESTDIR)$(libdir)/$(PACKAGE)/gnu.gettext.DumpResource$(EXEEXT)
	$(RM) $(DESTDIR)$(libdir)/$(PACKAGE)/gnu.gettext.GetURL$(EXEEXT)
uninstall-java-no: uninstall-java-no-@BUILDJAVA@
uninstall-java-no-yes:
	$(RM) $(DESTDIR)$(jardir)/gettext.jar
uninstall-java-no-no:


# Special rules for Tcl auxiliary program.

install-data-local: install-tcl
install-tcl:
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)
	$(INSTALL_DATA) $(srcdir)/msgunfmt.tcl $(DESTDIR)$(pkgdatadir)/msgunfmt.tcl

installdirs-local: installdirs-tcl
installdirs-tcl:
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)

uninstall-local: uninstall-tcl
uninstall-tcl:
	$(RM) $(DESTDIR)$(pkgdatadir)/msgunfmt.tcl


# One more automake bug.
installdirs: installdirs-local
