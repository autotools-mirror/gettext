## Makefile for the doc subdirectory of the GNU NLS Utilities
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

docdir = $(prefix)/doc/@PACKAGE@
dvidir = $(docdir)
psdir = $(docdir)
pdfdir = $(docdir)
htmldir = $(docdir)

AUTOMAKE_OPTIONS = 1.2 gnits

SED = sed
RM = rm -f

MAKEINFO = env LANG= LANGUAGE= @MAKEINFO@

info_TEXINFOS = gettext.texi
gettext_TEXINFOS = iso-639.texi iso-3166.texi

EXTRA_DIST = iso-639.sed iso-3166.sed ISO_639 ISO_3166 nls.texi matrix.texi $(EXTRA_DIST_html)

iso-639.texi: ISO_639 iso-639.sed
	$(SED) -f $(srcdir)/iso-639.sed $(srcdir)/ISO_639 > iso-639.tmp
	rm -f $(srcdir)/iso-639.texi
	mv iso-639.tmp $(srcdir)/iso-639.texi

iso-3166.texi: ISO_3166 iso-3166.sed
	$(SED) -f $(srcdir)/iso-3166.sed $(srcdir)/ISO_3166 > iso-3166.tmp
	rm -f $(srcdir)/iso-3166.texi
	mv iso-3166.tmp $(srcdir)/iso-3166.texi


# We distribute only the split HTML documentation.
# The user can generate the others, via
#   make gettext.ps
#   make gettext.pdf
#   make gettext.html

all-local: html
install-data-local: install-html
installdirs-local: installdirs-html
uninstall-local: uninstall-html

html: html-split
install-html: install-html-split
uninstall-html: uninstall-html-split

EXTRA_DIST_html = $(wildcard gettext_*.html)
CLEANFILES = gettext.pdf gettext.html
# gettext.dvi and gettext.ps are already known to automake.
MAINTAINERCLEANFILES = gettext_*.html


# Documentation in DVI format.

install-dvi: gettext.dvi
	$(mkinstalldirs) $(DESTDIR)$(dvidir)
	$(INSTALL_DATA) `if test -f gettext.dvi; then echo .; else echo $(srcdir); fi`/gettext.dvi $(DESTDIR)$(dvidir)/gettext.dvi

installdirs-dvi:
	$(mkinstalldirs) $(DESTDIR)$(dvidir)

uninstall-dvi:
	$(RM) $(DESTDIR)$(dvidir)/gettext.dvi


# Documentation in Postscript format.

DVIPS = @DVIPS@ -D600

ps: gettext.ps

gettext.ps: gettext.dvi
	$(DVIPS) -o $@ `if test -f gettext.dvi; then echo gettext.dvi; else echo $(srcdir)/gettext.dvi; fi`

install-ps: gettext.ps
	$(mkinstalldirs) $(DESTDIR)$(psdir)
	$(INSTALL_DATA) `if test -f gettext.ps; then echo .; else echo $(srcdir); fi`/gettext.ps $(DESTDIR)$(psdir)/gettext.ps

installdirs-ps:
	$(mkinstalldirs) $(DESTDIR)$(psdir)

uninstall-ps:
	$(RM) $(DESTDIR)$(psdir)/gettext.ps


# Documentation in Portable Document Format.

TEXI2PDF = @TEXI2PDF@
SUFFIXES = .pdf

pdf: gettext.pdf

.texi.pdf:
	TEXINPUTS=.:$$TEXINPUTS \
	  MAKEINFO='$(MAKEINFO) -I $(srcdir)' $(TEXI2PDF) $<

install-pdf: gettext.pdf
	$(mkinstalldirs) $(DESTDIR)$(pdfdir)
	$(INSTALL_DATA) `if test -f gettext.pdf; then echo .; else echo $(srcdir); fi`/gettext.pdf $(DESTDIR)$(pdfdir)/gettext.pdf

installdirs-pdf:
	$(mkinstalldirs) $(DESTDIR)$(pdfdir)

uninstall-pdf:
	$(RM) $(DESTDIR)$(pdfdir)/gettext.pdf


# Documentation in HTML format.

TEXI2HTML = @TEXI2HTML@

html-monolithic: gettext.html
html-split: gettext_toc.html

gettext.html: gettext.texi version.texi $(gettext_TEXINFOS)
	$(TEXI2HTML) -expandinfo -number -monolithic `if test -f gettext.texi; then echo gettext.texi; else echo $(srcdir)/gettext.texi; fi`

gettext_toc.html: gettext.texi version.texi $(gettext_TEXINFOS)
	case "$(TEXI2HTML)" in \
	  *"/missing texi2html") \
	     $(TEXI2HTML) -expandinfo -number -split_chapter `if test -f gettext.texi; then echo gettext.texi; else echo $(srcdir)/gettext.texi; fi` || exit 0 ;; \
	  *) $(RM) gettext_*.html ; \
	     $(TEXI2HTML) -expandinfo -number -split_chapter `if test -f gettext.texi; then echo gettext.texi; else echo $(srcdir)/gettext.texi; fi` ;; \
	esac

install-html-monolithic: gettext.html
	$(mkinstalldirs) $(DESTDIR)$(htmldir)
	$(INSTALL_DATA) `if test -f gettext.html; then echo .; else echo $(srcdir); fi`/gettext.html $(DESTDIR)$(htmldir)/gettext.html

install-html-split: gettext_toc.html
	$(mkinstalldirs) $(DESTDIR)$(htmldir)
	for file in `if test -f gettext_toc.html; then echo .; else echo $(srcdir); fi`/gettext_*.html; do \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(htmldir)/`basename $$file`; \
	done

installdirs-html:
	$(mkinstalldirs) $(DESTDIR)$(htmldir)

uninstall-html-monolithic:
	$(RM) $(DESTDIR)$(htmldir)/gettext.html

uninstall-html-split:
	$(RM) $(DESTDIR)$(htmldir)/gettext_*.html


# One more automake bug.
installdirs: installdirs-local
