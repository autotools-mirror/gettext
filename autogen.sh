#!/bin/sh
# Convenience script for regenerating all autogeneratable files that are
# omitted from the version control repository. In particular, this script
# also regenerates all aclocal.m4, config.h.in, Makefile.in, configure files
# with new versions of autoconf or automake.
#
# This script requires autoconf-2.60..2.61 and automake-1.10 in the PATH.
# It also requires either
#   - the GNULIB_TOOL environment variable pointing to the gnulib-tool script
#     in a gnulib checkout, or
#   - the cvs program in the PATH and an internet connection.

# Copyright (C) 2003-2007 Free Software Foundation, Inc.
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
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Usage: ./autogen.sh [--quick] [--skip-gnulib]
#
# Usage after a first-time CVS checkout:     ./autogen.sh
# Usage after a CVS update:                  ./autogen.sh --quick
# This uses an up-to-date gnulib checkout.
# (The gettext-0.17 release was prepared using gnulib commit
# bf5ebbca03613397b66521e98dd4ce28741c4689 from 2007-11-07.)
#
# Usage from a released tarball:             ./autogen.sh --quick --skip-gnulib
# This does not use a gnulib checkout.

quick=false
skip_gnulib=false
while :; do
  case "$1" in
    --quick) quick=true; shift;;
    --skip-gnulib) skip_gnulib=true; shift;;
    *) break ;;
  esac
done

if test $skip_gnulib = false; then
  if test -z "$GNULIB_TOOL"; then
    # Check out gnulib in a subdirectory 'gnulib'.
    GNULIB_CVS_ROOT=':pserver:anonymous@pserver.git.sv.gnu.org:/gnulib.git'
    GNULIB_CVS_REPOSITORY='gnulib'
    if test -d gnulib; then
      (cd gnulib && cvs update -d -P)
    else
      cvs -d "$GNULIB_CVS_ROOT" checkout -d $GNULIB_CVS_REPOSITORY HEAD
    fi
    # Now it should contain a gnulib-tool.
    if test -f gnulib/gnulib-tool; then
      GNULIB_TOOL=`pwd`/gnulib/gnulib-tool
    else
      echo "** warning: gnulib-tool not found" 1>&2
    fi
  fi
  # Skip the gnulib-tool step if gnulib-tool was not found.
  if test -n "$GNULIB_TOOL"; then
    # In gettext-runtime:
    if test -f gettext-runtime/gnulib-m4/gnulib-cache.m4; then
      mv -f gettext-runtime/gnulib-m4/gnulib-cache.m4 gettext-runtime/gnulib-m4/gnulib-cache.m4~
    fi
    GNULIB_MODULES_RUNTIME_FOR_SRC='
      atexit
      basename
      closeout
      error
      exit
      getopt
      gettext-h
      memmove
      progname
      propername
      relocatable-prog
      stdbool
      strtoul
      unlocked-io
      xalloc
    '
    GNULIB_MODULES_RUNTIME_OTHER='
      gettext-runtime-misc
      csharpcomp-script
      java
      javacomp-script
    '
    $GNULIB_TOOL --dir=gettext-runtime --lib=libgrt --source-base=gnulib-lib --m4-base=gnulib-m4 --no-libtool --local-dir=gnulib-local --local-symlink \
      --import $GNULIB_MODULES_RUNTIME_FOR_SRC $GNULIB_MODULES_RUNTIME_OTHER
    # In gettext-tools:
    if test -f gettext-tools/gnulib-m4/gnulib-cache.m4; then
      mv -f gettext-tools/gnulib-m4/gnulib-cache.m4 gettext-tools/gnulib-m4/gnulib-cache.m4~
    fi
    GNULIB_MODULES_TOOLS_FOR_SRC='
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
      fd-ostream
      file-ostream
      filename
      findprog
      fnmatch-posix
      fopen
      fstrcmp
      full-write
      fwriteerror
      gcd
      getline
      getopt
      gettext-h
      hash
      html-styled-ostream
      iconv
      javacomp
      javaexec
      linebreak
      localcharset
      locale
      localename
      lock
      memmove
      memset
      minmax
      obstack
      open
      openmp
      ostream
      pipe
      progname
      propername
      relocatable-prog
      relocatable-script
      sh-quote
      stdbool
      stpcpy
      stpncpy
      strcspn
      strpbrk
      strtol
      strtoul
      styled-ostream
      sys_stat
      sys_time
      term-styled-ostream
      ucs4-utf8
      uniname/uniname
      unistd
      uniwidth/width
      unlocked-io
      utf8-ucs4
      utf16-ucs4
      vasprintf
      wait-process
      xalloc
      xmalloca
      xerror
      xsetenv
      xstriconv
      xvasprintf
    '
    # Not yet used. Add some files to gettext-tools-misc instead.
    GNULIB_MODULES_TOOLS_FOR_LIBGREP='
      error
      exitfail
      gettext-h
      hard-locale
      obstack
      regex
      stdbool
      xalloc
    '
    GNULIB_MODULES_TOOLS_OTHER='
      gettext-tools-misc
      gcj
      java
      stdint
    '
    $GNULIB_TOOL --dir=gettext-tools --lib=libgettextlib --source-base=gnulib-lib --m4-base=gnulib-m4 --tests-base=gnulib-tests --makefile-name=Makefile.gnulib --libtool --with-tests --local-dir=gnulib-local --local-symlink \
      --import $GNULIB_MODULES_TOOLS_FOR_SRC $GNULIB_MODULES_TOOLS_OTHER
    # In gettext-tools/libgettextpo:
    if test -f gettext-tools/libgettextpo/gnulib-m4/gnulib-cache.m4; then
      mv -f gettext-tools/libgettextpo/gnulib-m4/gnulib-cache.m4 gettext-tools/libgettextpo/gnulib-m4/gnulib-cache.m4~
    fi
    # This is a subset of the GNULIB_MODULES_FOR_SRC.
    GNULIB_MODULES_LIBGETTEXTPO='
      basename
      c-ctype
      c-strcase
      c-strstr
      error
      error-progname
      exit
      file-ostream
      filename
      fopen
      fstrcmp
      fwriteerror
      gcd
      getline
      gettext-h
      hash
      iconv
      linebreak
      minmax
      ostream
      progname
      relocatable-lib
      stdbool
      ucs4-utf8
      uniwidth/width
      unlocked-io
      utf8-ucs4
      utf16-ucs4
      vasprintf
      xalloc
      xmalloca
      xerror
      xstriconv
      xvasprintf
    '
    GNULIB_MODULES_LIBGETTEXTPO_OTHER='
    '
    $GNULIB_TOOL --dir=gettext-tools --source-base=libgettextpo --m4-base=libgettextpo/gnulib-m4 --macro-prefix=gtpo --makefile-name=Makefile.gnulib --libtool --local-dir=gnulib-local --local-symlink \
      --import $GNULIB_MODULES_LIBGETTEXTPO $GNULIB_MODULES_LIBGETTEXTPO_OTHER
  fi
fi

(cd autoconf-lib-link
 ../build-aux/fixaclocal aclocal -I m4 -I ../m4
 autoconf
 automake
)

(cd gettext-runtime
 ../build-aux/fixaclocal aclocal -I m4 -I ../autoconf-lib-link/m4 -I ../m4 -I gnulib-m4
 autoconf
 autoheader && touch config.h.in
 automake
)

(cd gettext-runtime/libasprintf
 ../../build-aux/fixaclocal aclocal -I ../../m4 -I ../m4
 autoconf
 autoheader && touch config.h.in
 automake
)

cp -p gettext-runtime/ABOUT-NLS gettext-tools/ABOUT-NLS

(cd gettext-tools
 ../build-aux/fixaclocal aclocal -I m4 -I ../gettext-runtime/m4 -I ../autoconf-lib-link/m4 -I ../m4 -I gnulib-m4 -I libgettextpo/gnulib-m4
 autoconf
 autoheader && touch config.h.in
 automake
)

(cd gettext-tools/examples
 ../../build-aux/fixaclocal aclocal -I ../../gettext-runtime/m4 -I ../../m4
 autoconf
 automake
 # Rebuilding the examples PO files is only rarely needed.
 if ! $quick; then
   ./configure && (cd po && make update-po) && make distclean
 fi
)

build-aux/fixaclocal aclocal
autoconf
automake

cp -p autoconf-lib-link/config.rpath build-aux/config.rpath
