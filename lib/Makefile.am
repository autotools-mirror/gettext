## Makefile for the support lib subdirectory of the GNU NLS Utilities
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

lib_LTLIBRARIES = libgettextlib.la

# Sources that are compiled on all platforms.

libgettextlib_la_SOURCES = \
  argmatch.h argmatch.c \
  backupfile.h backupfile.c addext.c \
  basename.h basename.c \
  binary-io.h \
  c-ctype.h c-ctype.c \
  copy-file.h copy-file.c \
  error.h error.c \
  execute.h execute.c \
  exit.h \
  findprog.h findprog.c \
  fstrcmp.h fstrcmp.c \
  full-write.h full-write.c \
  gcd.h gcd.c \
  getopt.h getopt.c getopt1.c \
  hash.h hash.c \
  javacomp.h javacomp.c \
  javaexec.h javaexec.c \
  linebreak.h linebreak.c lbrkprop.h utf8-ucs4.h utf16-ucs4.h \
  localcharset.c \
  mbswidth.h mbswidth.c \
  minmax.h \
  obstack.h obstack.c \
  pathmax.h \
  pathname.h concatpath.c \
  pipe.h pipe-bidi.c pipe-in.c pipe-out.c \
  progname.h progname.c \
  safe-read.h safe-read.c \
  sh-quote.h sh-quote.c \
  stpncpy.h stpncpy.c \
  strtoul.c \
  tmpdir.h tmpdir.c \
  ucs4-utf8.h ucs4-utf16.h \
  wait-process.h wait-process.c \
  xerror.h xerror.c \
  xmalloc.h xmalloc.c xstrdup.c \
  xsetenv.h xsetenv.c

# Sources that are compiled only on platforms that lack the functions.

LIBADD_SOURCE = \
  liballoca.h alloca.c \
  getline.h getline.c \
  memset.c \
  mkdtemp.h mkdtemp.c \
  pfnmatch.h pfnmatch.c \
  setenv.h setenv.c unsetenv.c \
  libstdarg.h \
  stpcpy.h stpcpy.c \
  strcase.h strcasecmp.c strncasecmp.c \
  strcspn.c \
  strpbrk.h strpbrk.c \
  strstr.h strstr.c \
  strtol.c \
  vasprintf.h vasprintf.c

# Unused sources.

UNUSED_SOURCE = \
  memmove.c \
  xgetcwd.h xgetcwd.c

# How to build libgettextlib.la.

libgettextlib_la_LIBADD = @LTALLOCA@ @LTLIBOBJS@

# Need @LTLIBINTL@ because many source files use gettext().
# Need @LTLIBICONV@ because linebreak.c uses iconv().
libgettextlib_la_LDFLAGS = \
  -release @VERSION@ \
  @LTLIBINTL@ @LTLIBICONV@ -lc -no-undefined

# No need to install libgettextlib.a.
install-exec-am: install-exec-clean
install-exec-clean:
	$(RM) $(DESTDIR)$(libdir)/libgettextlib.a

# Extra files to be installed.

gettextsrcdir = $(datadir)/gettext
gettextsrc_DATA = gettext.h

# List of files to be distributed.

noinst_HEADERS = gettext.h

EXTRA_DIST = classpath.c config.charset ref-add.sin ref-del.sin \
$(LIBADD_SOURCE) $(UNUSED_SOURCE) \
stdbool.h.in \
gen-lbrkprop.c 3level.h Combining.txt

DEFS = -DLIBDIR=\"$(libdir)\" -DEXEEXT=\"$(EXEEXT)\" @DEFS@
INCLUDES = -I. -I$(srcdir) -I.. -I../intl -I$(top_srcdir)/intl


DISTCLEANFILES = fnmatch.h


all-local c-ctype.lo execute.lo javacomp.lo javaexec.lo pipe-bidi.lo pipe-in.lo pipe-out.lo progname.lo tmpdir.lo wait-process.lo xerror.lo: @STDBOOL_H@
stdbool.h: stdbool.h.in
	sed -e 's/@''HAVE__BOOL''@/@HAVE__BOOL@/g' < $(srcdir)/stdbool.h.in > stdbool.h
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
