#!/bin/sh
# Copyright (C) 2003-2022 Free Software Foundation, Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# This script populates the build infrastructure in the source tree
# checked-out from VCS.
#
# This script requires:
#   - Autoconf
#   - Automake >= 1.13

# Prerequisite (if not used from a released tarball): ./autopull.sh
# Usage: ./autogen.sh

# Nuisances.
(unset CDPATH) >/dev/null 2>&1 && unset CDPATH

# Make sure we get new versions of files brought in by automake.
(cd build-aux && rm -f ar-lib compile depcomp install-sh mdate-sh missing test-driver ylwrap)

# Generate configure script in each subdirectories.
# The aclocal and autoconf invocations need to be done bottom-up
# (subdirs first), so that 'configure --help' shows also the options
# that matter for the subdirs.
dir0=`pwd`

echo "$0: generating configure in gettext-runtime/intl..."
cd gettext-runtime/intl
aclocal -I ../../m4 -I ../m4 -I gnulib-m4 \
  && autoconf \
  && autoheader && touch config.h.in \
  && touch ChangeLog \
  && automake --add-missing --copy \
  && rm -rf autom4te.cache \
  || exit $?
cd "$dir0"

echo "$0: generating configure in gettext-runtime/libasprintf..."
cd gettext-runtime/libasprintf
aclocal -I ../../m4 -I ../m4 -I gnulib-m4 \
  && autoconf \
  && autoheader && touch config.h.in \
  && touch ChangeLog \
  && automake --add-missing --copy \
  && rm -rf autom4te.cache \
  || exit $?
cd "$dir0"

echo "$0: generating configure in gettext-runtime..."
cd gettext-runtime
aclocal -I m4 -I ../m4 -I gnulib-m4 \
  && autoconf \
  && autoheader && touch config.h.in \
  && touch ChangeLog \
  && automake --add-missing --copy \
  && rm -rf autom4te.cache \
  || exit $?
cd "$dir0"

echo "$0: generating files in libtextstyle..."
cd libtextstyle
./autogen.sh || exit $?
cd "$dir0"

echo "$0: generating configure in gettext-tools/examples..."
cd gettext-tools/examples
aclocal -I ../../gettext-runtime/m4 -I ../../m4 \
  && autoconf \
  && touch ChangeLog \
  && automake --add-missing --copy \
  && rm -rf autom4te.cache \
  || exit $?
cd "$dir0"

echo "$0: copying common files from gettext-runtime to gettext-tools..."
cp -p gettext-runtime/ABOUT-NLS gettext-tools/ABOUT-NLS
cp -p gettext-runtime/po/Makefile.in.in gettext-tools/po/Makefile.in.in
cp -p gettext-runtime/po/Rules-quot gettext-tools/po/Rules-quot
cp -p gettext-runtime/po/boldquot.sed gettext-tools/po/boldquot.sed
cp -p gettext-runtime/po/quot.sed gettext-tools/po/quot.sed
cp -p gettext-runtime/po/en@quot.header gettext-tools/po/en@quot.header
cp -p gettext-runtime/po/en@boldquot.header gettext-tools/po/en@boldquot.header
cp -p gettext-runtime/po/insert-header.sin gettext-tools/po/insert-header.sin
cp -p gettext-runtime/po/remove-potcdate.sin gettext-tools/po/remove-potcdate.sin
# This file might be newer than Gnulib's.
sed_extract_serial='s/^#.* serial \([^ ]*\).*/\1/p
1q'
for file in po.m4; do
  existing_serial=`sed -n -e "$sed_extract_serial" < "gettext-tools/gnulib-m4/$file"`
  gettext_serial=`sed -n -e "$sed_extract_serial" < "gettext-runtime/m4/$file"`
  if test -n "$existing_serial" && test -n "$gettext_serial" \
        && test "$existing_serial" -ge "$gettext_serial" 2> /dev/null; then
    :
  else
    cp -p "gettext-runtime/m4/$file" "gettext-tools/gnulib-m4/$file"
  fi
done

echo "$0: generating configure in gettext-tools..."
cd gettext-tools
aclocal -I m4 -I ../gettext-runtime/m4 -I ../m4 -I gnulib-m4 -I libgrep/gnulib-m4 -I libgettextpo/gnulib-m4 \
  && autoconf \
  && autoheader && touch config.h.in \
  && touch ChangeLog \
  && automake --add-missing --copy \
  && rm -rf autom4te.cache \
  || exit $?
cd "$dir0"

echo "$0: generating configure at the top-level..."
aclocal -I m4 \
  && autoconf \
  && touch ChangeLog \
  && automake --add-missing --copy \
  && rm -rf autom4te.cache \
            gettext-runtime/autom4te.cache \
            gettext-runtime/intl/autom4te.cache \
            gettext-runtime/libasprintf/autom4te.cache \
            libtextstyle/autom4te.cache \
            gettext-tools/autom4te.cache \
            gettext-tools/examples/autom4te.cache \
  || exit $?

echo "$0: done.  Now you can run './configure'."
