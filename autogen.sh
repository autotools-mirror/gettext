#!/bin/sh
# Convenience script for regenerating all aclocal.m4, config.h.in, Makefile.in,
# configure files with new versions of autoconf or automake.
#
# This script requires autoconf-2.60 and automake-1.8.2..1.9 in the PATH.
# It also requires the GNULIB_TOOL environment variable pointing to the
# gnulib-tool script in a gnulib checkout.

# Copyright (C) 2003-2006 Free Software Foundation, Inc.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

# Usage: ./autogen.sh [--quick]

if test "x$1" = "x--quick"; then
  quick=true
else
  quick=false
fi

if test -n "$GNULIB_TOOL"; then
  # In gettext-runtime:
  # In gettext-tools:
  GNULIB_MODULES_FOR_SRC='
  alloca-opt
  atexit
  backupfile
  basename
  binary-io
  bison-i18n
  byteswap
  c-ctype
  c-strcase
  c-strcasestr
  c-strstr
  clean-temp
  closeout
  copy-file
  csharpcomp
  csharpexec
  error
  error-progname
  execute
  exit
  findprog
  fnmatch-posix
  fstrcmp
  full-write
  fwriteerror
  gcd
  getline
  getopt
  gettext-h
  hash
  iconv
  iconvstring
  javacomp
  javaexec
  linebreak
  localcharset
  lock
  memmove
  memset
  minmax
  obstack
  pathname
  pipe
  progname
  propername
  relocatable
  relocwrapper
  sh-quote
  stdbool
  stpcpy
  stpncpy
  strcspn
  strpbrk
  strtol
  strtoul
  ucs4-utf8
  unistd
  unlocked-io
  utf8-ucs4
  utf16-ucs4
  vasprintf
  wait-process
  xalloc
  xallocsa
  xerror
  xsetenv
  xvasprintf
  '
  # Not yet used. Add some files to gettext-tools-misc instead.
  GNULIB_MODULES_FOR_LIBGREP='
  error
  exitfail
  gettext-h
  hard-locale
  obstack
  regex
  stdbool
  xalloc
  '
  GNULIB_MODULES_OTHER='
  gettext-tools-misc
  gcj
  java
  '
  $GNULIB_TOOL --dir=gettext-tools --lib=libgettextlib --source-base=gnulib-lib --m4-base=gnulib-m4 --libtool --local-dir=gnulib-local \
    --import $GNULIB_MODULES_FOR_SRC $GNULIB_MODULES_OTHER
fi

aclocal
autoconf
automake

(cd autoconf-lib-link
 aclocal -I m4 -I ../m4
 autoconf
 automake
)

(cd gettext-runtime
 aclocal -I m4 -I ../gettext-tools/m4 -I ../gettext-tools/gnulib-m4 -I ../autoconf-lib-link/m4 -I ../m4
 autoconf
 autoheader && touch config.h.in
 automake
)

(cd gettext-runtime/libasprintf
 aclocal -I ../../m4 -I ../m4
 autoconf
 autoheader && touch config.h.in
 automake
)

cp -p gettext-runtime/ABOUT-NLS gettext-tools/ABOUT-NLS

(cd gettext-tools
 aclocal -I m4 -I gnulib-m4 -I ../gettext-runtime/m4 -I ../autoconf-lib-link/m4 -I ../m4
 autoconf
 autoheader && touch config.h.in
 automake
)

(cd gettext-tools/examples
 aclocal -I ../../gettext-runtime/m4 -I ../../m4
 autoconf
 automake
 # Rebuilding the examples PO files is only rarely needed.
 if ! $quick; then
   ./configure && (cd po && make update-po) && make distclean
 fi
)

cp -p autoconf-lib-link/config.rpath build-aux/config.rpath
