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

# A manual page for each of the bin_PROGRAMS in src/Makefile.am.

man_aux  = gettext.x ngettext.x \
msgcmp.x msgfmt.x msgmerge.x msgunfmt.x xgettext.x \
msgcat.x msgcomm.x msgconv.x msgen.x msgexec.x msggrep.x msguniq.x

# Likewise, plus additional manual pages for the libintl functions.

man_MAN1 = gettext.1 ngettext.1 \
msgcmp.1 msgfmt.1 msgmerge.1 msgunfmt.1 xgettext.1 \
msgcat.1 msgcomm.1 msgconv.1 msgen.1 msgexec.1 msggrep.1 msguniq.1
man_MAN3 = gettext.3 ngettext.3 \
textdomain.3 bindtextdomain.3 bind_textdomain_codeset.3
man_MAN3IN = gettext.3.in ngettext.3.in \
textdomain.3.in bindtextdomain.3.in bind_textdomain_codeset.3.in
man_MANS = $(man_MAN1) $(man_MAN3) \
dgettext.3 dcgettext.3 dngettext.3 dcngettext.3

man_HTML = gettext.1.html ngettext.1.html \
msgcmp.1.html msgfmt.1.html msgmerge.1.html msgunfmt.1.html xgettext.1.html \
msgcat.1.html msgcomm.1.html msgconv.1.html msgen.1.html msgexec.1.html msggrep.1.html msguniq.1.html \
gettext.3.html ngettext.3.html \
textdomain.3.html bindtextdomain.3.html bind_textdomain_codeset.3.html

EXTRA_DIST = help2man $(man_aux) $(man_MANS) $(man_MAN3IN) $(man_HTML)
MAINTAINERCLEANFILES = $(man_MAN1) $(man_MAN3) $(man_HTML)
SUFFIXES = .1 .x

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
.x.1:
	@progname=`basename $@ .1`; \
	executable=../src/$$progname; \
	case "$(PERL)" in *"/missing perl") perlok=no;; *) perlok=yes;; esac; \
	if test @CROSS_COMPILING@ = no && test -f $$executable && test $$perlok = yes; then \
	  echo "Updating man page $@"; \
	  echo "$(HELP2MAN) --include=$$progname.x $$executable > $$progname.1"; \
	  rm -f t-$$progname.1; \
	  $(HELP2MAN) --include=$$progname.x $$executable > t-$$progname.1 || exit 1; \
	  mv t-$$progname.1 $$progname.1; \
	else \
	  echo "WARNING: The man page $@ cannot be updated yet."; \
	fi

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

gettext.1.html: gettext.1
	$(MAN2HTML) `if test -f gettext.1; then echo .; else echo $(srcdir); fi`/gettext.1 > t-$@
	mv t-$@ $@
ngettext.1.html: ngettext.1
	$(MAN2HTML) `if test -f ngettext.1; then echo .; else echo $(srcdir); fi`/ngettext.1 > t-$@
	mv t-$@ $@
msgcmp.1.html: msgcmp.1
	$(MAN2HTML) `if test -f msgcmp.1; then echo .; else echo $(srcdir); fi`/msgcmp.1 > t-$@
	mv t-$@ $@
msgfmt.1.html: msgfmt.1
	$(MAN2HTML) `if test -f msgfmt.1; then echo .; else echo $(srcdir); fi`/msgfmt.1 > t-$@
	mv t-$@ $@
msgmerge.1.html: msgmerge.1
	$(MAN2HTML) `if test -f msgmerge.1; then echo .; else echo $(srcdir); fi`/msgmerge.1 > t-$@
	mv t-$@ $@
msgunfmt.1.html: msgunfmt.1
	$(MAN2HTML) `if test -f msgunfmt.1; then echo .; else echo $(srcdir); fi`/msgunfmt.1 > t-$@
	mv t-$@ $@
xgettext.1.html: xgettext.1
	$(MAN2HTML) `if test -f xgettext.1; then echo .; else echo $(srcdir); fi`/xgettext.1 > t-$@
	mv t-$@ $@
msgcat.1.html: msgcat.1
	$(MAN2HTML) `if test -f msgcat.1; then echo .; else echo $(srcdir); fi`/msgcat.1 > t-$@
	mv t-$@ $@
msgcomm.1.html: msgcomm.1
	$(MAN2HTML) `if test -f msgcomm.1; then echo .; else echo $(srcdir); fi`/msgcomm.1 > t-$@
	mv t-$@ $@
msgconv.1.html: msgconv.1
	$(MAN2HTML) `if test -f msgconv.1; then echo .; else echo $(srcdir); fi`/msgconv.1 > t-$@
	mv t-$@ $@
msgen.1.html: msgen.1
	$(MAN2HTML) `if test -f msgen.1; then echo .; else echo $(srcdir); fi`/msgen.1 > t-$@
	mv t-$@ $@
msgexec.1.html: msgexec.1
	$(MAN2HTML) `if test -f msgexec.1; then echo .; else echo $(srcdir); fi`/msgexec.1 > t-$@
	mv t-$@ $@
msggrep.1.html: msggrep.1
	$(MAN2HTML) `if test -f msggrep.1; then echo .; else echo $(srcdir); fi`/msggrep.1 > t-$@
	mv t-$@ $@
msguniq.1.html: msguniq.1
	$(MAN2HTML) `if test -f msguniq.1; then echo .; else echo $(srcdir); fi`/msguniq.1 > t-$@
	mv t-$@ $@
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
