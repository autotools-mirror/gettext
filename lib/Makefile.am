## Makefile for the support lib subdirectory of the GNU NLS Utilities
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

AUTOMAKE_OPTIONS = 1.2 gnits

noinst_LIBRARIES = libnlsut.a

EXTRA_DIST = alloca.c config.charset error.c getline.c memset.c memmove.c \
mkdtemp.c ref-add.sin ref-del.sin stpcpy.c stpncpy.c strcasecmp.c strcspn.c \
strncasecmp.c strstr.c strtol.c strtoul.c vasprintf.c \
stdbool.h.in \
gen-lbrkprop.c 3level.h

libnlsut_a_SOURCES = basename.c c-ctype.c concatpath.c execute.c findprog.c \
fstrcmp.c full-write.c gcd.c getopt.c getopt1.c hash.c linebreak.c \
localcharset.c mbswidth.c obstack.c pipe-bidi.c pipe-in.c pipe-out.c \
progname.c safe-read.c tmpdir.c wait-process.c xerror.c xgetcwd.c xmalloc.c \
xstrdup.c

libnlsut_a_LIBADD = @ALLOCA@ @LIBOBJS@

noinst_HEADERS = c-ctype.h error.h execute.h findprog.h fstrcmp.h \
full-write.h gcd.h getline.h getopt.h hash.h lbrkprop.h linebreak.h mbswidth.h \
mkdtemp.h obstack.h pathmax.h pipe.h progname.h safe-read.h system.h tmpdir.h \
utf8-ucs4.h utf16-ucs4.h wait-process.h xerror.h

DEFS = -DLIBDIR=\"$(libdir)\" @DEFS@
INCLUDES = -I. -I$(srcdir) -I.. -I../intl


all-local: @STDBOOL_H@
stdbool.h: stdbool.h.in
	cp $(srcdir)/stdbool.h.in stdbool.h
MOSTLYCLEANFILES = @STDBOOL_H@


# The following is needed in order to install a simple file in $(libdir)
# which is shared with other installed packages. We use a list of referencing
# packages so that "make uninstall" will remove the file if and only if it
# is not used by another installed package.
# On systems with glibc-2.1 or newer, the file is redundant, therefore we
# avoid installing it.

all-local: charset.alias ref-add.sed ref-del.sed

charset_alias = $(DESTDIR)$(libdir)/charset.alias
charset_tmp = $(DESTDIR)$(libdir)/charset.tmp
install-exec-local: all-local
	$(mkinstalldirs) $(DESTDIR)$(libdir)
	if test -f $(charset_alias); then \
	  sed -f ref-add.sed $(charset_alias) > $(charset_tmp) ; \
	  $(INSTALL_DATA) $(charset_tmp) $(charset_alias) ; \
	  rm -f $(charset_tmp) ; \
	else \
	  if test @GLIBC21@ = no; then \
	    sed -f ref-add.sed charset.alias > $(charset_tmp) ; \
	    $(INSTALL_DATA) $(charset_tmp) $(charset_alias) ; \
	    rm -f $(charset_tmp) ; \
	  fi ; \
	fi

uninstall-local: all-local
	if test -f $(charset_alias); then \
	  sed -f ref-del.sed $(charset_alias) > $(charset_tmp); \
	  if grep '^# Packages using this file: $$' $(charset_tmp) \
	      > /dev/null; then \
	    rm -f $(charset_alias); \
	  else \
	    $(INSTALL_DATA) $(charset_tmp) $(charset_alias); \
	  fi; \
	  rm -f $(charset_tmp); \
	fi

charset.alias: config.charset
	$(SHELL) $(srcdir)/config.charset '@host@' > t-$@
	mv t-$@ $@

SUFFIXES = .sed .sin
.sin.sed:
	sed -e '/^#/d' -e 's/@''PACKAGE''@/@PACKAGE@/g' $< > t-$@
	mv t-$@ $@

CLEANFILES = charset.alias ref-add.sed ref-del.sed
