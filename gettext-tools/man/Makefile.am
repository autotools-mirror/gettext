## Makefile for the man subdirectory of the GNU NLS Utilities
## Copyright (C) 2001-2002 Free Software Foundation, Inc.
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

localedir = $(datadir)/locale

AUTOMAKE_OPTIONS = 1.2 gnits

# A manual page for each of the bin_PROGRAMS in src/Makefile.am
# and for each of the bin_SCRIPTS in misc/Makefile.am.

man_aux  = gettext.x ngettext.x \
msgcmp.x msgfmt.x msgmerge.x msgunfmt.x xgettext.x \
msgattrib.x msgcat.x msgcomm.x msgconv.x msgen.x msgexec.x msgfilter.x msggrep.x msginit.x msguniq.x \
gettextize.x autopoint.x

# Likewise, plus additional manual pages for the libintl functions.

man_MAN1GEN = gettext.1 ngettext.1
man_MAN1IN = gettext.1.in ngettext.1.in
man_MAN1OTHER = \
msgcmp.1 msgfmt.1 msgmerge.1 msgunfmt.1 xgettext.1 \
msgattrib.1 msgcat.1 msgcomm.1 msgconv.1 msgen.1 msgexec.1 msgfilter.1 msggrep.1 msginit.1 msguniq.1 \
gettextize.1 autopoint.1
man_MAN1 = $(man_MAN1GEN) $(man_MAN1OTHER)
man_MAN3 = gettext.3 ngettext.3 \
textdomain.3 bindtextdomain.3 bind_textdomain_codeset.3
man_MAN3IN = gettext.3.in ngettext.3.in \
textdomain.3.in bindtextdomain.3.in bind_textdomain_codeset.3.in
man_MAN3LINK = dgettext.3 dcgettext.3 dngettext.3 dcngettext.3
man_MANS = $(man_MAN1) $(man_MAN3) $(man_MAN3LINK)

man_HTMLGEN = gettext.1.html ngettext.1.html
man_HTMLIN = gettext.1.html.in ngettext.1.html.in
man_HTMLOTHER = \
msgcmp.1.html msgfmt.1.html msgmerge.1.html msgunfmt.1.html xgettext.1.html \
msgattrib.1.html msgcat.1.html msgcomm.1.html msgconv.1.html msgen.1.html msgexec.1.html msgfilter.1.html msggrep.1.html msginit.1.html msguniq.1.html \
gettextize.1.html autopoint.1.html \
gettext.3.html ngettext.3.html \
textdomain.3.html bindtextdomain.3.html bind_textdomain_codeset.3.html
man_HTML = $(man_HTMLGEN) $(man_HTMLOTHER)

EXTRA_DIST = help2man $(man_aux) $(man_MAN1IN) $(man_MAN1OTHER) $(man_MAN3) $(man_MAN3IN) $(man_MAN3LINK) $(man_HTMLIN) $(man_HTMLOTHER)
CLEANFILES = $(man_MAN1GEN) $(man_HTMLGEN)
MAINTAINERCLEANFILES = $(man_MAN1IN) $(man_MAN1OTHER) $(man_MAN3) $(man_HTMLIN) $(man_HTMLOTHER)

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

msgcmp.1: msgcmp.x
	$(SHELL) x-to-1 "$(PERL)" "$(HELP2MAN)" ../src/msgcmp$(EXEEXT) $(srcdir)/msgcmp.x msgcmp.1
msgfmt.1: msgfmt.x
	$(SHELL) x-to-1 "$(PERL)" "$(HELP2MAN)" ../src/msgfmt$(EXEEXT) $(srcdir)/msgfmt.x msgfmt.1
msgmerge.1: msgmerge.x
	$(SHELL) x-to-1 "$(PERL)" "$(HELP2MAN)" ../src/msgmerge$(EXEEXT) $(srcdir)/msgmerge.x msgmerge.1
msgunfmt.1: msgunfmt.x
	$(SHELL) x-to-1 "$(PERL)" "$(HELP2MAN)" ../src/msgunfmt$(EXEEXT) $(srcdir)/msgunfmt.x msgunfmt.1
xgettext.1: xgettext.x
	$(SHELL) x-to-1 "$(PERL)" "$(HELP2MAN)" ../src/xgettext$(EXEEXT) $(srcdir)/xgettext.x xgettext.1
msgattrib.1: msgattrib.x
	$(SHELL) x-to-1 "$(PERL)" "$(HELP2MAN)" ../src/msgattrib$(EXEEXT) $(srcdir)/msgattrib.x msgattrib.1
msgcat.1: msgcat.x
	$(SHELL) x-to-1 "$(PERL)" "$(HELP2MAN)" ../src/msgcat$(EXEEXT) $(srcdir)/msgcat.x msgcat.1
msgcomm.1: msgcomm.x
	$(SHELL) x-to-1 "$(PERL)" "$(HELP2MAN)" ../src/msgcomm$(EXEEXT) $(srcdir)/msgcomm.x msgcomm.1
msgconv.1: msgconv.x
	$(SHELL) x-to-1 "$(PERL)" "$(HELP2MAN)" ../src/msgconv$(EXEEXT) $(srcdir)/msgconv.x msgconv.1
msgen.1: msgen.x
	$(SHELL) x-to-1 "$(PERL)" "$(HELP2MAN)" ../src/msgen$(EXEEXT) $(srcdir)/msgen.x msgen.1
msgexec.1: msgexec.x
	$(SHELL) x-to-1 "$(PERL)" "$(HELP2MAN)" ../src/msgexec$(EXEEXT) $(srcdir)/msgexec.x msgexec.1
msgfilter.1: msgfilter.x
	$(SHELL) x-to-1 "$(PERL)" "$(HELP2MAN)" ../src/msgfilter$(EXEEXT) $(srcdir)/msgfilter.x msgfilter.1
msggrep.1: msggrep.x
	$(SHELL) x-to-1 "$(PERL)" "$(HELP2MAN)" ../src/msggrep$(EXEEXT) $(srcdir)/msggrep.x msggrep.1
msginit.1: msginit.x
	$(SHELL) x-to-1 "$(PERL)" "$(HELP2MAN)" ../src/msginit$(EXEEXT) $(srcdir)/msginit.x msginit.1
msguniq.1: msguniq.x
	$(SHELL) x-to-1 "$(PERL)" "$(HELP2MAN)" ../src/msguniq$(EXEEXT) $(srcdir)/msguniq.x msguniq.1

gettextize.1: gettextize.x
	$(SHELL) x-to-1 "$(PERL)" "$(HELP2MAN)" ../misc/gettextize $(srcdir)/gettextize.x gettextize.1
autopoint.1: autopoint.x
	$(SHELL) x-to-1 "$(PERL)" "$(HELP2MAN)" ../misc/autopoint $(srcdir)/autopoint.x autopoint.1

# Depend on configure.in to get version number changes.
$(man_MAN1): help2man $(top_srcdir)/configure.in

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

msgcmp.1.html: msgcmp.1
	$(MAN2HTML) `if test -f msgcmp.1; then echo .; else echo $(srcdir); fi`/msgcmp.1 | sed -e '/CreationDate:/d' > t-$@
	mv t-$@ $@
msgfmt.1.html: msgfmt.1
	$(MAN2HTML) `if test -f msgfmt.1; then echo .; else echo $(srcdir); fi`/msgfmt.1 | sed -e '/CreationDate:/d' > t-$@
	mv t-$@ $@
msgmerge.1.html: msgmerge.1
	$(MAN2HTML) `if test -f msgmerge.1; then echo .; else echo $(srcdir); fi`/msgmerge.1 | sed -e '/CreationDate:/d' > t-$@
	mv t-$@ $@
msgunfmt.1.html: msgunfmt.1
	$(MAN2HTML) `if test -f msgunfmt.1; then echo .; else echo $(srcdir); fi`/msgunfmt.1 | sed -e '/CreationDate:/d' > t-$@
	mv t-$@ $@
xgettext.1.html: xgettext.1
	$(MAN2HTML) `if test -f xgettext.1; then echo .; else echo $(srcdir); fi`/xgettext.1 | sed -e '/CreationDate:/d' > t-$@
	mv t-$@ $@
msgattrib.1.html: msgattrib.1
	$(MAN2HTML) `if test -f msgattrib.1; then echo .; else echo $(srcdir); fi`/msgattrib.1 | sed -e '/CreationDate:/d' > t-$@
	mv t-$@ $@
msgcat.1.html: msgcat.1
	$(MAN2HTML) `if test -f msgcat.1; then echo .; else echo $(srcdir); fi`/msgcat.1 | sed -e '/CreationDate:/d' > t-$@
	mv t-$@ $@
msgcomm.1.html: msgcomm.1
	$(MAN2HTML) `if test -f msgcomm.1; then echo .; else echo $(srcdir); fi`/msgcomm.1 | sed -e '/CreationDate:/d' > t-$@
	mv t-$@ $@
msgconv.1.html: msgconv.1
	$(MAN2HTML) `if test -f msgconv.1; then echo .; else echo $(srcdir); fi`/msgconv.1 | sed -e '/CreationDate:/d' > t-$@
	mv t-$@ $@
msgen.1.html: msgen.1
	$(MAN2HTML) `if test -f msgen.1; then echo .; else echo $(srcdir); fi`/msgen.1 | sed -e '/CreationDate:/d' > t-$@
	mv t-$@ $@
msgexec.1.html: msgexec.1
	$(MAN2HTML) `if test -f msgexec.1; then echo .; else echo $(srcdir); fi`/msgexec.1 | sed -e '/CreationDate:/d' > t-$@
	mv t-$@ $@
msgfilter.1.html: msgfilter.1
	$(MAN2HTML) `if test -f msgfilter.1; then echo .; else echo $(srcdir); fi`/msgfilter.1 | sed -e '/CreationDate:/d' > t-$@
	mv t-$@ $@
msggrep.1.html: msggrep.1
	$(MAN2HTML) `if test -f msggrep.1; then echo .; else echo $(srcdir); fi`/msggrep.1 | sed -e '/CreationDate:/d' > t-$@
	mv t-$@ $@
msginit.1.html: msginit.1
	$(MAN2HTML) `if test -f msginit.1; then echo .; else echo $(srcdir); fi`/msginit.1 | sed -e '/CreationDate:/d' > t-$@
	mv t-$@ $@
msguniq.1.html: msguniq.1
	$(MAN2HTML) `if test -f msguniq.1; then echo .; else echo $(srcdir); fi`/msguniq.1 | sed -e '/CreationDate:/d' > t-$@
	mv t-$@ $@
gettextize.1.html: gettextize.1
	$(MAN2HTML) `if test -f gettextize.1; then echo .; else echo $(srcdir); fi`/gettextize.1 | sed -e '/CreationDate:/d' > t-$@
	mv t-$@ $@
autopoint.1.html: autopoint.1
	$(MAN2HTML) `if test -f autopoint.1; then echo .; else echo $(srcdir); fi`/autopoint.1 | sed -e '/CreationDate:/d' > t-$@
	mv t-$@ $@
gettext.3.html: gettext.3.in
	$(MAN2HTML) $(srcdir)/gettext.3.in | sed -e '/CreationDate:/d' > t-$@
	mv t-$@ $@
ngettext.3.html: ngettext.3.in
	$(MAN2HTML) $(srcdir)/ngettext.3.in | sed -e '/CreationDate:/d' > t-$@
	mv t-$@ $@
textdomain.3.html: textdomain.3.in
	$(MAN2HTML) $(srcdir)/textdomain.3.in | sed -e '/CreationDate:/d' > t-$@
	mv t-$@ $@
bindtextdomain.3.html: bindtextdomain.3.in
	$(MAN2HTML) $(srcdir)/bindtextdomain.3.in | sed -e '/CreationDate:/d' > t-$@
	mv t-$@ $@
bind_textdomain_codeset.3.html: bind_textdomain_codeset.3.in
	$(MAN2HTML) $(srcdir)/bind_textdomain_codeset.3.in | sed -e '/CreationDate:/d' > t-$@
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
