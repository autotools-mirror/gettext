## Makefile for the man subdirectory of the GNU NLS Utilities
## Copyright (C) 2001 Free Software Foundation, Inc.
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

VERSION = @VERSION@
mandir = @mandir@
docdir = $(prefix)/doc/@PACKAGE@
htmldir = $(docdir)

AUTOMAKE_OPTIONS = 1.2 gnits

man_MAN3 = gettext.3 ngettext.3 \
textdomain.3 bindtextdomain.3 bind_textdomain_codeset.3
man_MAN3IN = gettext.3.in ngettext.3.in \
textdomain.3.in bindtextdomain.3.in bind_textdomain_codeset.3.in
man_MANS = $(man_MAN3) \
dgettext.3 dcgettext.3 dngettext.3 dcngettext.3

man_HTML = gettext.3.html ngettext.3.html \
textdomain.3.html bindtextdomain.3.html bind_textdomain_codeset.3.html

EXTRA_DIST = $(man_MANS) $(man_MAN3IN) $(man_HTML)
MAINTAINERCLEANFILES = $(man_MAN1) $(man_MAN3) $(man_HTML)

RM = rm -f

# groff 1.17 or newer.
MAN2HTML = groff -mandoc -Thtml


# We distribute both the man pages and their HTML equivalent.
# The user can generate the parts, via
#   make man
#   make html

all-local: html
install-data-local: install-html
installdirs-local: installdirs-html
uninstall-local: uninstall-html


# Man pages.

gettext.3: gettext.3.in $(top_srcdir)/configure.in
	sed -e 's/@''VERSION''@/@VERSION@/g' < $(srcdir)/gettext.3.in > t-$@
	mv t-$@ $@
ngettext.3: ngettext.3.in $(top_srcdir)/configure.in
	sed -e 's/@''VERSION''@/@VERSION@/g' < $(srcdir)/ngettext.3.in > t-$@
	mv t-$@ $@
textdomain.3: textdomain.3.in $(top_srcdir)/configure.in
	sed -e 's/@''VERSION''@/@VERSION@/g' < $(srcdir)/textdomain.3.in > t-$@
	mv t-$@ $@
bindtextdomain.3: bindtextdomain.3.in $(top_srcdir)/configure.in
	sed -e 's/@''VERSION''@/@VERSION@/g' < $(srcdir)/bindtextdomain.3.in > t-$@
	mv t-$@ $@
bind_textdomain_codeset.3: bind_textdomain_codeset.3.in $(top_srcdir)/configure.in
	sed -e 's/@''VERSION''@/@VERSION@/g' < $(srcdir)/bind_textdomain_codeset.3.in > t-$@
	mv t-$@ $@


# Man pages in HTML format.

html: $(man_HTML)

gettext.3.html: gettext.3.in
	$(MAN2HTML) $(srcdir)/gettext.3.in > t-$@
	mv t-$@ $@
ngettext.3.html: ngettext.3.in
	$(MAN2HTML) $(srcdir)/ngettext.3.in > t-$@
	mv t-$@ $@
textdomain.3.html: textdomain.3.in
	$(MAN2HTML) $(srcdir)/textdomain.3.in > t-$@
	mv t-$@ $@
bindtextdomain.3.html: bindtextdomain.3.in
	$(MAN2HTML) $(srcdir)/bindtextdomain.3.in > t-$@
	mv t-$@ $@
bind_textdomain_codeset.3.html: bind_textdomain_codeset.3.in
	$(MAN2HTML) $(srcdir)/bind_textdomain_codeset.3.in > t-$@
	mv t-$@ $@

install-html:
	$(mkinstalldirs) $(DESTDIR)$(htmldir)
	for file in $(man_HTML); do \
	  if test -f $$file; then dir=.; else dir=$(srcdir); fi; \
	  $(INSTALL_DATA) $$dir/$$file $(DESTDIR)$(htmldir)/$$file; \
	done

installdirs-html:
	$(mkinstalldirs) $(DESTDIR)$(htmldir)

uninstall-html:
	for file in $(man_HTML); do \
	  $(RM) $(DESTDIR)$(htmldir)/$$file; \
	done


# One more automake bug.
installdirs: installdirs-local
