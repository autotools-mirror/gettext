## Makefile for the toplevel directory of the GNU NLS Utilities
## Copyright (C) 1995-1999, 2000-2002 Free Software Foundation, Inc.
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

AUTOMAKE_OPTIONS = 1.5 gnu no-dependencies
ACLOCAL_AMFLAGS = -I m4

MAKEINFO = env LANG= LANGUAGE= @MAKEINFO@

gettextsrcdir = $(datadir)/gettext
gettextsrc_DATA = ABOUT-NLS
gettextsrc_SCRIPTS = config.rpath mkinstalldirs

SUBDIRS = doc intl intl-java lib libuniname src po projects misc man m4 tests

EXTRA_DIST = config.rpath BUGS DISCLAIM PACKAGING README.gemtext \
             djgpp/COPYING.DJ djgpp/Makefile.maint djgpp/README.DJ \
             djgpp/README.in djgpp/README djgpp/config.bat djgpp/config.in \
             djgpp/config.sed djgpp/config.site djgpp/djpwd.h \
             djgpp/edtests.bat djgpp/fnchange.in djgpp/fnchange.lst \
             djgpp/getpwnam.c djgpp/msginit.sed djgpp/tscript.sed \
             djgpp/unsetenv.c \
             os2/README.OS2 os2/Makefile os2/backward.def os2/configure.awk

ABOUT-NLS: $(srcdir)/doc/nls.texi $(srcdir)/doc/matrix.texi
	rm -f $(srcdir)/ABOUT-NLS \
	  && $(MAKEINFO) --output=$(srcdir)/ABOUT-NLS --no-header \
	    -I $(srcdir)/doc --no-validate nls.texi


# Documentation tags not known to automake.

ps:
	cd doc && $(MAKE) ps

pdf:
	cd doc && $(MAKE) pdf

html:
	cd doc && $(MAKE) html


# Subdirectories with separate autoconfiguration.

CONFIGURES_RPATHX = $(srcdir)/tests/rpathx/Makefile.in $(srcdir)/tests/rpathx/aclocal.m4 $(srcdir)/tests/rpathx/configure
$(srcdir)/tests/rpathx/Makefile.in: $(srcdir)/tests/rpathx/Makefile.am $(srcdir)/tests/rpathx/configure.in
	cd $(srcdir)/tests/rpathx && automake Makefile
$(srcdir)/tests/rpathx/aclocal.m4: $(srcdir)/tests/rpathx/configure.in
	cd $(srcdir)/tests/rpathx && aclocal -I ../../m4
$(srcdir)/tests/rpathx/configure: $(srcdir)/tests/rpathx/configure.in $(srcdir)/tests/rpathx/aclocal.m4
	cd $(srcdir)/tests/rpathx && autoconf

CONFIGURES_RPATHY = $(srcdir)/tests/rpathy/Makefile.in $(srcdir)/tests/rpathy/aclocal.m4 $(srcdir)/tests/rpathy/configure
$(srcdir)/tests/rpathy/Makefile.in: $(srcdir)/tests/rpathy/Makefile.am $(srcdir)/tests/rpathy/configure.in
	cd $(srcdir)/tests/rpathy && automake Makefile
$(srcdir)/tests/rpathy/aclocal.m4: $(srcdir)/tests/rpathy/configure.in $(srcdir)/m4/lib-link.m4 $(srcdir)/m4/lib-prefix.m4 $(srcdir)/m4/lib-ld.m4
	cd $(srcdir)/tests/rpathy && aclocal -I ../../m4
$(srcdir)/tests/rpathy/configure: $(srcdir)/tests/rpathy/configure.in $(srcdir)/tests/rpathy/aclocal.m4
	cd $(srcdir)/tests/rpathy && autoconf

CONFIGURES_RPATHZ = $(srcdir)/tests/rpathz/Makefile.in $(srcdir)/tests/rpathz/aclocal.m4 $(srcdir)/tests/rpathz/configure
$(srcdir)/tests/rpathz/Makefile.in: $(srcdir)/tests/rpathz/Makefile.am $(srcdir)/tests/rpathz/configure.in
	cd $(srcdir)/tests/rpathz && automake Makefile
$(srcdir)/tests/rpathz/aclocal.m4: $(srcdir)/tests/rpathz/configure.in $(srcdir)/m4/lib-link.m4 $(srcdir)/m4/lib-prefix.m4 $(srcdir)/m4/lib-ld.m4
	cd $(srcdir)/tests/rpathz && aclocal -I ../../m4
$(srcdir)/tests/rpathz/configure: $(srcdir)/tests/rpathz/configure.in $(srcdir)/tests/rpathz/aclocal.m4
	cd $(srcdir)/tests/rpathz && autoconf

CONFIGURES_RPATHLX = $(srcdir)/tests/rpathlx/Makefile.in $(srcdir)/tests/rpathlx/aclocal.m4 $(srcdir)/tests/rpathlx/configure
$(srcdir)/tests/rpathlx/Makefile.in: $(srcdir)/tests/rpathlx/Makefile.am $(srcdir)/tests/rpathlx/configure.in
	cd $(srcdir)/tests/rpathlx && automake Makefile
$(srcdir)/tests/rpathlx/aclocal.m4: $(srcdir)/tests/rpathlx/configure.in $(srcdir)/m4/lib-link.m4 $(srcdir)/m4/lib-prefix.m4 $(srcdir)/m4/lib-ld.m4
	cd $(srcdir)/tests/rpathlx && aclocal -I ../../m4
$(srcdir)/tests/rpathlx/configure: $(srcdir)/tests/rpathlx/configure.in $(srcdir)/tests/rpathlx/aclocal.m4
	cd $(srcdir)/tests/rpathlx && autoconf

CONFIGURES_RPATHLY = $(srcdir)/tests/rpathly/Makefile.in $(srcdir)/tests/rpathly/aclocal.m4 $(srcdir)/tests/rpathly/configure
$(srcdir)/tests/rpathly/Makefile.in: $(srcdir)/tests/rpathly/Makefile.am $(srcdir)/tests/rpathly/configure.in
	cd $(srcdir)/tests/rpathly && automake Makefile
$(srcdir)/tests/rpathly/aclocal.m4: $(srcdir)/tests/rpathly/configure.in $(srcdir)/m4/lib-link.m4 $(srcdir)/m4/lib-prefix.m4 $(srcdir)/m4/lib-ld.m4
	cd $(srcdir)/tests/rpathly && aclocal -I ../../m4
$(srcdir)/tests/rpathly/configure: $(srcdir)/tests/rpathly/configure.in $(srcdir)/tests/rpathly/aclocal.m4
	cd $(srcdir)/tests/rpathly && autoconf

CONFIGURES_RPATHLYX = $(srcdir)/tests/rpathlyx/Makefile.in $(srcdir)/tests/rpathlyx/aclocal.m4 $(srcdir)/tests/rpathlyx/configure
$(srcdir)/tests/rpathlyx/Makefile.in: $(srcdir)/tests/rpathlyx/Makefile.am $(srcdir)/tests/rpathlyx/configure.in
	cd $(srcdir)/tests/rpathlyx && automake Makefile
$(srcdir)/tests/rpathlyx/aclocal.m4: $(srcdir)/tests/rpathlyx/configure.in $(srcdir)/m4/lib-link.m4 $(srcdir)/m4/lib-prefix.m4 $(srcdir)/m4/lib-ld.m4
	cd $(srcdir)/tests/rpathlyx && aclocal -I ../../m4
$(srcdir)/tests/rpathlyx/configure: $(srcdir)/tests/rpathlyx/configure.in $(srcdir)/tests/rpathlyx/aclocal.m4
	cd $(srcdir)/tests/rpathlyx && autoconf

CONFIGURES_RPATHLZ = $(srcdir)/tests/rpathlz/Makefile.in $(srcdir)/tests/rpathlz/aclocal.m4 $(srcdir)/tests/rpathlz/configure
$(srcdir)/tests/rpathlz/Makefile.in: $(srcdir)/tests/rpathlz/Makefile.am $(srcdir)/tests/rpathlz/configure.in
	cd $(srcdir)/tests/rpathlz && automake Makefile
$(srcdir)/tests/rpathlz/aclocal.m4: $(srcdir)/tests/rpathlz/configure.in $(srcdir)/m4/lib-link.m4 $(srcdir)/m4/lib-prefix.m4 $(srcdir)/m4/lib-ld.m4
	cd $(srcdir)/tests/rpathlz && aclocal -I ../../m4
$(srcdir)/tests/rpathlz/configure: $(srcdir)/tests/rpathlz/configure.in $(srcdir)/tests/rpathlz/aclocal.m4
	cd $(srcdir)/tests/rpathlz && autoconf

CONFIGURES_RPATHLZYX = $(srcdir)/tests/rpathlzyx/Makefile.in $(srcdir)/tests/rpathlzyx/aclocal.m4 $(srcdir)/tests/rpathlzyx/configure
$(srcdir)/tests/rpathlzyx/Makefile.in: $(srcdir)/tests/rpathlzyx/Makefile.am $(srcdir)/tests/rpathlzyx/configure.in
	cd $(srcdir)/tests/rpathlzyx && automake Makefile
$(srcdir)/tests/rpathlzyx/aclocal.m4: $(srcdir)/tests/rpathlzyx/configure.in $(srcdir)/m4/lib-link.m4 $(srcdir)/m4/lib-prefix.m4 $(srcdir)/m4/lib-ld.m4
	cd $(srcdir)/tests/rpathlzyx && aclocal -I ../../m4
$(srcdir)/tests/rpathlzyx/configure: $(srcdir)/tests/rpathlzyx/configure.in $(srcdir)/tests/rpathlzyx/aclocal.m4
	cd $(srcdir)/tests/rpathlzyx && autoconf

SUBCONFIGURES = $(CONFIGURES_RPATHX) $(CONFIGURES_RPATHY) $(CONFIGURES_RPATHZ) $(CONFIGURES_RPATHLX) $(CONFIGURES_RPATHLY) $(CONFIGURES_RPATHLYX) $(CONFIGURES_RPATHLZ) $(CONFIGURES_RPATHLZYX)

subconfigures: $(SUBCONFIGURES)

# Hidden from automake, but really activated. Works around an automake-1.5 bug.
#distdir: subconfigures

