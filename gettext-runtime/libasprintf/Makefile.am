## Makefile for the gettext-runtime/libasprintf subdirectory of GNU gettext
## Copyright (C) 2002-2003 Free Software Foundation, Inc.
##
## This program is free software; you can redistribute it and/or modify it
## under the terms of the GNU Library General Public License as published
## by the Free Software Foundation; either version 2, or (at your option)
## any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## Library General Public License for more details.
##
## You should have received a copy of the GNU Library General Public
## License along with this program; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
## USA.

## Process this file with automake to produce Makefile.in.

AUTOMAKE_OPTIONS = 1.5 gnu no-dependencies
ACLOCAL_AMFLAGS = -I ../../config/m4 -I ../m4
EXTRA_DIST =
MOSTLYCLEANFILES =


# Library include file.

include_HEADERS = autosprintf.h


# Library code.

lib_LTLIBRARIES = libasprintf.la

libasprintf_la_SOURCES = \
  lib-asprintf.h lib-asprintf.c \
  autosprintf.h autosprintf.cc

# Sources used only on platforms lacking vasprintf().
lib_asprintf_EXTRASOURCES = \
  printf-args.h printf-args.c \
  printf-parse.h printf-parse.c \
  vasnprintf.h vasnprintf.c asnprintf.c \
  vasprintf.h vasprintf.c asprintf.c
lib-asprintf.lo: $(lib_asprintf_EXTRASOURCES)
EXTRA_DIST += $(lib_asprintf_EXTRASOURCES)


# >>> gnulib module alloca.
EXTRA_DIST += alloca_.h

# The following is needed in order to create an <alloca.h> when the system
# doesn't have one that works with the given compiler.
all-local $(libasprintf_la_OBJECTS): @ALLOCA_H@
alloca.h: alloca_.h
	cp $(srcdir)/alloca_.h alloca.h
MOSTLYCLEANFILES += alloca.h
# <<< gnulib module alloca.


# Clean up after Solaris cc.
clean-am:
	rm -rf SunWS_cache


# Documentation.

docdir = $(prefix)/doc/@PACKAGE@
dvidir = $(docdir)
psdir = $(docdir)
pdfdir = $(docdir)
htmldir = $(docdir)

MAKEINFO = env LANG= LANGUAGE= @MAKEINFO@

info_TEXINFOS = autosprintf.texi

# We distribute only the HTML documentation.
# The user can generate the others, via
#   make autosprintf.ps
#   make autosprintf.pdf

all-local: html
install-data-local: install-html
installdirs-local: installdirs-html
uninstall-local: uninstall-html

CLEANFILES = autosprintf.pdf
# autosprintf.dvi and autosprintf.ps are already known to automake.


# Documentation in DVI format.

install-dvi: autosprintf.dvi
	$(mkinstalldirs) $(DESTDIR)$(dvidir)
	$(INSTALL_DATA) `if test -f autosprintf.dvi; then echo .; else echo $(srcdir); fi`/autosprintf.dvi $(DESTDIR)$(dvidir)/autosprintf.dvi

installdirs-dvi:
	$(mkinstalldirs) $(DESTDIR)$(dvidir)

uninstall-dvi:
	$(RM) $(DESTDIR)$(dvidir)/autosprintf.dvi

# Temporary index files. automake doesn't remove them reliably by itself.
MOSTLYCLEANFILES += \
  autosprintf.am autosprintf.ams \
  autosprintf.cp autosprintf.cps \
  autosprintf.ef autosprintf.efs \
  autosprintf.em autosprintf.ems \
  autosprintf.ev autosprintf.evs \
  autosprintf.fn autosprintf.fns \
  autosprintf.kw autosprintf.kws \
  autosprintf.ky autosprintf.kys \
  autosprintf.op autosprintf.ops \
  autosprintf.pg autosprintf.pgs \
  autosprintf.tp autosprintf.tps \
  autosprintf.vr autosprintf.vrs


# Documentation in Postscript format.

DVIPS = @DVIPS@ -D600

ps: autosprintf.ps

autosprintf.ps: autosprintf.dvi
	$(DVIPS) -o $@ `if test -f autosprintf.dvi; then echo autosprintf.dvi; else echo $(srcdir)/autosprintf.dvi; fi`

install-ps: autosprintf.ps
	$(mkinstalldirs) $(DESTDIR)$(psdir)
	$(INSTALL_DATA) `if test -f autosprintf.ps; then echo .; else echo $(srcdir); fi`/autosprintf.ps $(DESTDIR)$(psdir)/autosprintf.ps

installdirs-ps:
	$(mkinstalldirs) $(DESTDIR)$(psdir)

uninstall-ps:
	$(RM) $(DESTDIR)$(psdir)/autosprintf.ps


# Documentation in Portable Document Format.

TEXI2PDF = @TEXI2PDF@
SUFFIXES = .pdf

pdf: autosprintf.pdf

.texi.pdf:
	TEXINPUTS=.:$$TEXINPUTS \
	  MAKEINFO='$(MAKEINFO) -I $(srcdir)' $(TEXI2PDF) $<

install-pdf: autosprintf.pdf
	$(mkinstalldirs) $(DESTDIR)$(pdfdir)
	$(INSTALL_DATA) `if test -f autosprintf.pdf; then echo .; else echo $(srcdir); fi`/autosprintf.pdf $(DESTDIR)$(pdfdir)/autosprintf.pdf

installdirs-pdf:
	$(mkinstalldirs) $(DESTDIR)$(pdfdir)

uninstall-pdf:
	$(RM) $(DESTDIR)$(pdfdir)/autosprintf.pdf


# Documentation in HTML format.

TEXI2HTML = @PERL@ $(srcdir)/texi2html

html: autosprintf.html

autosprintf.html: autosprintf.texi
	$(TEXI2HTML) -expandinfo -number -monolithic `if test -f autosprintf.texi; then echo autosprintf.texi; else echo $(srcdir)/autosprintf.texi; fi`

install-html: autosprintf.html
	$(mkinstalldirs) $(DESTDIR)$(htmldir)
	$(INSTALL_DATA) `if test -f autosprintf.html; then echo .; else echo $(srcdir); fi`/autosprintf.html $(DESTDIR)$(htmldir)/autosprintf.html

installdirs-html:
	$(mkinstalldirs) $(DESTDIR)$(htmldir)

uninstall-html:
	$(RM) $(DESTDIR)$(htmldir)/autosprintf.html

EXTRA_DIST += texi2html autosprintf.html
