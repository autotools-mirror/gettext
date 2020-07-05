#!/bin/sh
# Convenience script for regenerating all autogeneratable files that are
# omitted from the version control repository. In particular, this script
# also regenerates all aclocal.m4, config.h.in, Makefile.in, configure files
# with new versions of autoconf or automake.
#
# This script requires autoconf-2.63..2.69 and automake-1.11..1.16 in the PATH.
# If not used from a released tarball, it also requires
#   - the GNULIB_SRCDIR environment variable pointing to a gnulib checkout.

# Copyright (C) 2003-2020 Free Software Foundation, Inc.
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

# Usage: ./autogen.sh [--skip-gnulib]
#
# Usage from a git checkout:                 ./autogen.sh
# This uses an up-to-date gnulib checkout.
#
# Usage from a released tarball:             ./autogen.sh --skip-gnulib
# This does not use a gnulib checkout.

skip_gnulib=false
while :; do
  case "$1" in
    --skip-gnulib) skip_gnulib=true; shift;;
    *) break ;;
  esac
done

TEXINFO_VERSION=6.5

if test $skip_gnulib = false; then
  mkdir -p build-aux
  # texinfo.tex
  # The most recent snapshot of it is available in the gnulib repository.
  # But this is a snapshot, with all possible dangers.
  # A stable release of it is available through "automake --add-missing --copy",
  # but that depends on the Automake version. So take the version which matches
  # the latest stable texinfo release.
  if test ! -f build-aux/texinfo.tex; then
    { wget -q --timeout=5 -O build-aux/texinfo.tex.tmp 'https://git.savannah.gnu.org/gitweb/?p=texinfo.git;a=blob_plain;f=doc/texinfo.tex;hb=refs/tags/texinfo-'"$TEXINFO_VERSION" \
        && mv build-aux/texinfo.tex.tmp build-aux/texinfo.tex; \
    } || rm -f build-aux/texinfo.tex.tmp
  fi

  if test -n "$GNULIB_SRCDIR"; then
    test -d "$GNULIB_SRCDIR" || {
      echo "*** GNULIB_SRCDIR is set but does not point to an existing directory." 1>&2
      exit 1
    }
  else
    GNULIB_SRCDIR=`pwd`/gnulib
    test -d "$GNULIB_SRCDIR" || {
      echo "*** Subdirectory 'gnulib' does not yet exist. Use './gitsub.sh pull' to create it, or set the environment variable GNULIB_SRCDIR." 1>&2
      exit 1
    }
  fi
  # Now it should contain a gnulib-tool.
  GNULIB_TOOL="$GNULIB_SRCDIR/gnulib-tool"
  test -f "$GNULIB_TOOL" || {
    echo "*** gnulib-tool not found." 1>&2
    exit 1
  }
  GNULIB_MODULES='
    ostream
      fd-ostream
      file-ostream
      html-ostream
      iconv-ostream
      memory-ostream
      term-ostream
    styled-ostream
      html-styled-ostream
      noop-styled-ostream
      term-styled-ostream
    filename
    isatty
    largefile
    vasprintf-posix
    xalloc
    xconcat-filename

    memory-ostream-tests
    term-ostream-tests
  '
  $GNULIB_TOOL --lib=libtextstyle --source-base=lib --m4-base=gnulib-m4 --tests-base=tests \
    --macro-prefix=lts \
    --makefile-name=Makefile.gnulib --libtool \
    --local-dir=gnulib-local --local-dir=../gnulib-local \
    --import $GNULIB_MODULES
  $GNULIB_TOOL --copy-file build-aux/config.guess; chmod a+x build-aux/config.guess
  $GNULIB_TOOL --copy-file build-aux/config.sub;   chmod a+x build-aux/config.sub
  $GNULIB_TOOL --copy-file build-aux/declared.sh lib/declared.sh; chmod a+x lib/declared.sh
  $GNULIB_TOOL --copy-file build-aux/run-test; chmod a+x build-aux/run-test
  $GNULIB_TOOL --copy-file build-aux/test-driver.diff
  # If we got no texinfo.tex so far, take the snapshot from gnulib.
  if test ! -f build-aux/texinfo.tex; then
    $GNULIB_TOOL --copy-file build-aux/texinfo.tex
  fi
  # For use by the example programs.
  $GNULIB_TOOL --copy-file m4/libtextstyle.m4
fi

# Copy some files from gettext.
cp -p ../INSTALL.windows .
mkdir -p m4
cp -p ../m4/libtool.m4 m4/
cp -p ../m4/lt*.m4 m4/
cp -p ../m4/woe32-dll.m4 m4/
cp -p ../build-aux/ltmain.sh build-aux/
cp -p ../build-aux/texi2html build-aux/
cp -p ../gettext-tools/m4/exported.m4 m4/
cp -p ../gettext-tools/libgettextpo/exported.sh.in lib/

aclocal -I m4 -I gnulib-m4
autoconf
autoheader && touch config.h.in
touch ChangeLog
# Make sure we get new versions of files brought in by automake.
(cd build-aux && rm -f ar-lib compile depcomp install-sh mdate-sh missing test-driver)
automake --add-missing --copy
# Support environments where sh exists but not /bin/sh (Android).
patch build-aux/test-driver < build-aux/test-driver.diff
# Get rid of autom4te.cache directory.
rm -rf autom4te.cache
