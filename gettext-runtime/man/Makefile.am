## Makefile for the gettext-runtime/man subdirectory of GNU gettext
## Copyright (C) 2001-2003 Free Software Foundation, Inc.
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
docdir = $(prefix)/doc/gettext
htmldir = $(docdir)

localedir = $(datadir)/locale

AUTOMAKE_OPTIONS = 1.2 gnits

# A manual page for each of the bin_PROGRAMS in src/Makefile.am.

man_aux = gettext.x ngettext.x

man_MAN1GEN = gettext.1 ngettext.1
man_MAN1IN = gettext.1.in ngettext.1.in
man_MAN1 = $(man_MAN1GEN)
man_MANS = $(man_MAN1)

man_HTMLGEN = gettext.1.html ngettext.1.html
man_HTMLIN = gettext.1.html.in ngettext.1.html.in
man_HTML = $(man_HTMLGEN)

EXTRA_DIST = help2man $(man_aux) $(man_MAN1IN) $(man_HTMLIN)
CLEANFILES = $(man_MAN1GEN) $(man_HTMLGEN)
MAINTAINERCLEANFILES = $(man_MAN1IN) $(man_HTMLIN)

PERL = @PERL@
RM = rm -f

# help2man 1.24 or newer.
HELP2MAN = $(PERL) -w -- $(srcdir)/help2man

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

# The progname.x files contain some extra information not found in the
# "progname --help" output.

gettext.1: gettext.1.in Makefile
	sed -e 's|@''localedir''@|$(localedir)|g' < `if test -f gettext.1.in; then echo .; else echo $(srcdir); fi`/gettext.1.in > t-$@
	mv t-$@ $@
ngettext.1: ngettext.1.in Makefile
	sed -e 's|@''localedir''@|$(localedir)|g' < `if test -f ngettext.1.in; then echo .; else echo $(srcdir); fi`/ngettext.1.in > t-$@
	mv t-$@ $@

gettext.1.in: gettext.x
	IN_HELP2MAN=1 $(SHELL) x-to-1 "$(PERL)" "$(HELP2MAN)" ../src/gettext$(EXEEXT) $(srcdir)/gettext.x gettext.1.in
ngettext.1.in: ngettext.x
	IN_HELP2MAN=1 $(SHELL) x-to-1 "$(PERL)" "$(HELP2MAN)" ../src/ngettext$(EXEEXT) $(srcdir)/ngettext.x ngettext.1.in

# Depend on version.sh to get version number changes.
$(man_MAN1): help2man $(top_srcdir)/../version.sh


# Man pages in HTML format.

html: $(man_HTML)

gettext.1.html: gettext.1.html.in Makefile
	sed -e 's|@''localedir''@|$(localedir)|g' < `if test -f gettext.1.html.in; then echo .; else echo $(srcdir); fi`/gettext.1.html.in > t-$@
	mv t-$@ $@
ngettext.1.html: ngettext.1.html.in Makefile
	sed -e 's|@''localedir''@|$(localedir)|g' < `if test -f ngettext.1.html.in; then echo .; else echo $(srcdir); fi`/ngettext.1.html.in > t-$@
	mv t-$@ $@

gettext.1.html.in: gettext.1.in
	$(MAN2HTML) `if test -f gettext.1.in; then echo .; else echo $(srcdir); fi`/gettext.1.in | sed -e '/CreationDate:/d' > t-$@
	mv t-$@ $@
ngettext.1.html.in: ngettext.1.in
	$(MAN2HTML) `if test -f ngettext.1.in; then echo .; else echo $(srcdir); fi`/ngettext.1.in | sed -e '/CreationDate:/d' > t-$@
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
