#!/bin/sh
# Convenience script for regenerating all autogeneratable files that are
# omitted from the version control repository. In particular, this script
# also regenerates all aclocal.m4, config.h.in, Makefile.in, configure files
# with new versions of autoconf or automake.
#
# This script requires autoconf-2.64..2.72 and automake-1.13..1.17 in the PATH.

# Copyright (C) 2003-2025 Free Software Foundation, Inc.
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

# Prerequisite (if not used from a released tarball): either
#   - the GNULIB_SRCDIR environment variable pointing to a gnulib checkout, or
#   - a preceding invocation of './autopull.sh'.
#
# Usage: ./autogen.sh [--skip-gnulib]
#
# Options:
#   --skip-gnulib       Avoid fetching files from Gnulib.
#                       This option is useful
#                       - when you are working from a released tarball (possibly
#                         with modifications), or
#                       - as a speedup, if the set of gnulib modules did not
#                         change since the last time you ran this script.

# Nuisances.
(unset CDPATH) >/dev/null 2>&1 && unset CDPATH

skip_gnulib=false
skip_gnulib_option=

while :; do
  case "$1" in
    --skip-gnulib) skip_gnulib=true; skip_gnulib_option='--skip-gnulib'; shift;;
    *) break ;;
  esac
done

# The tests in gettext-tools/tests are not meant to be executable, because
# they have a TESTS_ENVIRONMENT that specifies the shell explicitly.

if ! $skip_gnulib; then
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
  # Sync files that might have been modified in gnulib or in gettext.
  sed_extract_serial='s/^#.* serial \([^ ]*\).*/\1/p
1q'
  for file in gettext.m4 nls.m4 po.m4 progtest.m4; do
    if cmp "$GNULIB_SRCDIR/m4/$file" "gettext-runtime/m4/$file" >/dev/null; then
      :
    else
      gnulib_serial=`sed -n -e "$sed_extract_serial" < "$GNULIB_SRCDIR/m4/$file"`
      gettext_serial=`sed -n -e "$sed_extract_serial" < "gettext-runtime/m4/$file"`
      if test -n "$gnulib_serial" && test -n "$gettext_serial"; then
        if test "$gnulib_serial" -ge "$gettext_serial" 2> /dev/null; then
          # The gnulib copy is newer, or it has an update copyright line.
          mv "gettext-runtime/m4/$file" "gettext-runtime/m4/$file"'~'
          cp "$GNULIB_SRCDIR/m4/$file" "gettext-runtime/m4/$file"
        else
          # The gettext copy is newer.
          mv "$GNULIB_SRCDIR/m4/$file" "$GNULIB_SRCDIR/m4/$file"'~'
          cp "gettext-runtime/m4/$file" "$GNULIB_SRCDIR/m4/$file"
        fi
      fi
    fi
  done
  # In gettext-runtime:
  GNULIB_MODULES_RUNTIME_FOR_SRC='
    atexit
    attribute
    basename-lgpl
    binary-io
    bool
    closeout
    error
    getopt-gnu
    gettext-h
    havelib
    memmove
    noreturn
    progname
    propername
    relocatable-prog
    setlocale
    sigpipe
    stdio-h
    stdlib-h
    strtoul
    unistd-h
    unlocked-io
    xalloc
    xstring-buffer
  '
  GNULIB_MODULES_RUNTIME_OTHER='
    gettext-runtime-misc
    ansi-c++-opt
    csharpcomp-script
    java
    javacomp-script
    manywarnings
  '
  $GNULIB_TOOL --dir=gettext-runtime --lib=libgrt --source-base=gnulib-lib --m4-base=gnulib-m4 --no-libtool --local-dir=gnulib-local --local-symlink \
    --import $GNULIB_MODULES_RUNTIME_FOR_SRC $GNULIB_MODULES_RUNTIME_OTHER || exit $?
  $GNULIB_TOOL --copy-file m4/build-to-host.m4 gettext-runtime/m4/build-to-host.m4 || exit $?
  # In gettext-runtime/intl:
  GNULIB_MODULES_LIBINTL='
    gettext-runtime-intl-misc
    attribute
    bison
    filename
    flexmember
    getcwd-lgpl
    havelib
    iconv
    lib-symbol-visibility
    localcharset
    localename
    lock
    manywarnings
    tsearch
    vasnprintf-posix
    vasnwprintf-posix
    wgetcwd-lgpl
  '
  GNULIB_SETLOCALE_DEPENDENCIES=`$GNULIB_TOOL --extract-dependencies setlocale | sed -e 's/ .*//'`
  $GNULIB_TOOL --dir=gettext-runtime/intl --source-base=gnulib-lib --m4-base=gnulib-m4 --lgpl=2 --libtool --local-dir=gnulib-local --local-symlink \
    --import $GNULIB_MODULES_LIBINTL $GNULIB_SETLOCALE_DEPENDENCIES || exit $?
  # In gettext-runtime/libasprintf:
  GNULIB_MODULES_LIBASPRINTF='
    alloca
    manywarnings
    vasnprintf
  '
  $GNULIB_TOOL --dir=gettext-runtime/libasprintf --source-base=gnulib-lib --m4-base=gnulib-m4 --lgpl=2 --libtool --local-dir=gnulib-local --local-symlink \
    --import $GNULIB_MODULES_LIBASPRINTF || exit $?
  # In gettext-tools:
  GNULIB_MODULES_TOOLS_FOR_SRC='
    alloca-opt
    atexit
    attribute
    backupfile
    basename-lgpl
    bcp47
    binary-io
    bison
    bison-i18n
    bool
    byteswap
    c-ctype
    c-strcase
    c-strcasestr
    c-strstr
    clean-temp
    closedir
    closeout
    copy-file
    csharpcomp
    csharpexec
    cygpath
    error
    error-progname
    execute
    filename
    findprog
    flexmember
    fnmatch
    fopen
    free-posix
    fstrcmp
    full-write
    fwriteerror
    gcd
    getaddrinfo
    getline
    getopt-gnu
    gettext-h
    iconv
    javacomp
    javaexec
    libunistring-optional
    libxml
    localcharset
    locale-h
    localename
    localtime
    lock
    mem-hash-map
    memchr
    memmove
    memset
    minmax
    mkdir
    noreturn
    obstack
    open
    opendir
    openmp-init
    pipe-filter-ii
    progname
    propername
    read-file
    readdir
    relocatable-prog
    relocatable-script
    setlocale
    sf-istream
    sh-filename
    sh-quote
    sigpipe
    sigprocmask
    spawn-pipe
    stdio-h
    stdlib-h
    stpcpy
    stpncpy
    str_startswith
    strchrnul
    strcspn
    strerror
    string-desc
    strnlen
    strpbrk
    strtol
    strtoul
    supersede
    sys_select-h
    sys_stat-h
    sys_time-h
    trim
    unicase/u8-casefold
    unictype/ctype-space
    unictype/property-white-space
    unictype/syntax-java-whitespace
    unictype/property-xid-start
    unictype/property-xid-continue
    unilbrk/ulc-width-linebreaks
    uniname/uniname
    uninorm/nfc
    unistd-h
    unistr/u8-check
    unistr/u8-mbtouc
    unistr/u8-mbtoucr
    unistr/u8-uctomb
    unistr/u16-mbtouc
    uniwidth/width
    unlocked-io
    unsetenv
    vasprintf
    vc-mtime
    wait-process
    write
    xalloc
    xconcat-filename
    xerror
    xmalloca
    xmemdup0
    xsetenv
    xstrerror
    xstriconv
    xstriconveh
    xstring-buffer
    xstring-desc
    xvasprintf
  '
  # Common dependencies of GNULIB_MODULES_TOOLS_FOR_SRC and GNULIB_MODULES_TOOLS_FOR_LIBGREP.
  GNULIB_MODULES_TOOLS_FOR_SRC_COMMON_DEPENDENCIES='
    alloca-opt
    bool
    extensions
    gettext-h
    include_next
    locale-h
    localcharset
    malloc-posix
    mbrtowc
    mbsinit
    multiarch
    setlocale-null
    snippet/arg-nonnull
    snippet/c++defs
    snippet/warn-on-use
    ssize_t
    stddef-h
    stdint-h
    stdlib-h
    streq
    unistd-h
    verify
    wchar-h
    wctype-h
    windows-mutex
    windows-once
    windows-recmutex
    windows-rwlock
  '
  GNULIB_MODULES_TOOLS_OTHER='
    gettext-tools-misc
    ansi-c++-opt
    csharpcomp-script
    csharpexec-script
    java
    javacomp-script
    javaexec-script
    manywarnings
    stdint-h
    test-xfail
  '
  GNULIB_MODULES_TOOLS_LIBUNISTRING_TESTS='
    unilbrk/u8-possible-linebreaks-tests
    unilbrk/ulc-width-linebreaks-tests
    unistr/u8-mbtouc-tests
    unistr/u8-mbtouc-unsafe-tests
    uniwidth/width-tests
  '
  GNULIB_MODULES_LIBGETTEXTLIB="$GNULIB_MODULES_TOOLS_FOR_SRC $GNULIB_MODULES_TOOLS_FOR_SRC_COMMON_DEPENDENCIES $GNULIB_MODULES_TOOLS_OTHER"
  $GNULIB_TOOL --dir=gettext-tools --lib=libgettextlib --source-base=gnulib-lib --m4-base=gnulib-m4 --tests-base=gnulib-tests --makefile-name=Makefile.gnulib --libtool --with-tests --local-dir=gnulib-local --local-symlink \
    --import \
    --avoid=fdutimensat-tests --avoid=futimens-tests --avoid=utime-tests --avoid=utimens-tests --avoid=utimensat-tests \
    --avoid=array-list-tests --avoid=linked-list-tests --avoid=linkedhash-list-tests \
    `for m in $GNULIB_MODULES_TOOLS_LIBUNISTRING_TESTS; do echo --avoid=$m; done` \
    $GNULIB_MODULES_LIBGETTEXTLIB || exit $?
  $GNULIB_TOOL --copy-file m4/libtextstyle.m4 gettext-tools/gnulib-m4/libtextstyle.m4 || exit $?
  # In gettext-tools/libgrep:
  GNULIB_MODULES_TOOLS_FOR_LIBGREP='
    mbrlen
    regex
  '
  $GNULIB_TOOL --dir=gettext-tools --macro-prefix=grgl --lib=libgrep --source-base=libgrep --m4-base=libgrep/gnulib-m4 --witness-c-macro=IN_GETTEXT_TOOLS_LIBGREP --makefile-name=Makefile.gnulib --local-dir=gnulib-local --local-symlink \
    --import \
    `for m in $GNULIB_MODULES_TOOLS_FOR_SRC_COMMON_DEPENDENCIES; do \
       if test \`$GNULIB_TOOL --extract-applicability $m\` != all; then \
         case $m in \
           bool | locale-h | stddef-h | stdint-h | stdlib-h | unistd-h | wchar-h | wctype-h) ;; \
           *) echo --avoid=$m ;; \
         esac; \
       fi; \
     done` \
    $GNULIB_MODULES_TOOLS_FOR_LIBGREP || exit $?
  # In gettext-tools/libgettextpo:
  # This is a subset of the GNULIB_MODULES_TOOLS_FOR_SRC.
  GNULIB_MODULES_LIBGETTEXTPO='
    attribute
    basename-lgpl
    bool
    close
    c-ctype
    c-strcase
    c-strstr
    error
    filename
    fopen
    free-posix
    fstrcmp
    fwriteerror
    gcd
    getline
    gettext-h
    iconv
    libtextstyle-dummy
    libunistring-optional
    markup
    mem-hash-map
    minmax
    once
    open
    relocatable-lib
    sigpipe
    stdio-h
    stdlib-h
    stpcpy
    stpncpy
    str_startswith
    strchrnul
    strerror
    string-desc
    strnlen
    unictype/ctype-space
    unictype/property-white-space
    unictype/property-xid-start
    unictype/property-xid-continue
    unilbrk/ulc-width-linebreaks
    unistr/u8-mbtouc
    unistr/u8-mbtoucr
    unistr/u8-uctomb
    unistr/u16-mbtouc
    uniwidth/width
    unlocked-io
    vasprintf
    xalloc
    xconcat-filename
    xmalloca
    xstrerror
    xstriconv
    xvasprintf
  '
  # Module 'fdopen' is enabled in gettext-tools/config.status,
  # because it occurs as dependency of some module ('supersede') in
  # GNULIB_MODULES_TOOLS_FOR_SRC. Therefore on mingw, libgettextpo/stdio.h
  # contains '#define fdopen rpl_fdopen'. Therefore we need to include
  # fdopen.lo in libgettextpo.la.
  # Module 'realloc-posix' is enabled in gettext-tools/config.status,
  # because it occurs as dependency of some module ('read-file') in
  # GNULIB_MODULES_TOOLS_FOR_SRC. Therefore on mingw, libgettextpo/stdlib.h
  # contains '#define realloc rpl_realloc'. Therefore we need to include
  # realloc.lo in libgettextpo.la.
  # Module 'strerror_r-posix' is enabled in gettext-tools/config.status,
  # because it occurs as dependency of some module ('xstrerror') in
  # GNULIB_MODULES_TOOLS_FOR_SRC. Therefore gettext-tools/config.h contains
  # '#define GNULIB_STRERROR_R_POSIX 1'. Therefore on mingw,
  # libgettextpo/error.o references strerror_r. Therefore we need to include
  # strerror_r.lo in libgettextpo.la.
  # Module 'mixin/printf-posix' is enabled in gettext-tools/config.status,
  # because it occurs as dependency of some module ('xstring-buffer' ->
  # 'string-buffer' -> 'vsnzprintf-posix') in GNULIB_MODULES_TOOLS_FOR_SRC.
  # Therefore gettext-tools/config.h defines many NEED_* macros, from
  # gl_PREREQ_VASNPRINTF_WITH_POSIX_EXTRAS. Therefore on mingw,
  # libgettextpo/vasnprintf.c references isnand-nolibm.h.
  GNULIB_MODULES_LIBGETTEXTPO_OTHER='
    fdopen
    realloc-posix
    strerror_r-posix
    mixin/printf-posix
  '
  $GNULIB_TOOL --dir=gettext-tools --source-base=libgettextpo --m4-base=libgettextpo/gnulib-m4 --macro-prefix=gtpo --makefile-name=Makefile.gnulib --libtool --local-dir=gnulib-local --local-symlink \
    --import --avoid=progname $GNULIB_MODULES_LIBGETTEXTPO $GNULIB_MODULES_LIBGETTEXTPO_OTHER || exit $?
  # In gettext-tools/tests:
  GNULIB_MODULES_TOOLS_TESTS='
    test-xfail
    thread
  '
  $GNULIB_TOOL --dir=gettext-tools --macro-prefix=gttgl --lib=libtestsgnu --source-base=tests/gnulib-lib --m4-base=tests/gnulib-m4 --makefile-name=Makefile.gnulib --local-dir=gnulib-local --local-symlink \
    --import \
    `for m in $GNULIB_MODULES_LIBGETTEXTLIB; do \
       if test \`$GNULIB_TOOL --local-dir=gnulib-local --extract-applicability $m\` != all; then \
         echo --avoid=$m; \
       fi; \
     done` \
    $GNULIB_MODULES_TOOLS_TESTS || exit $?
  # Overwrite older versions of .m4 files with the up-to-date version.
  cp gettext-runtime/m4/gettext.m4 gettext-tools/gnulib-m4/gettext.m4
  # Import build tools.  We use --copy-file to avoid directory creation.
  $GNULIB_TOOL --copy-file tests/init.sh gettext-tools || exit $?
  $GNULIB_TOOL --copy-file build-aux/x-to-1.in gettext-runtime/man/x-to-1.in || exit $?
  $GNULIB_TOOL --copy-file build-aux/x-to-1.in gettext-tools/man/x-to-1.in || exit $?
  $GNULIB_TOOL --copy-file build-aux/git-version-gen || exit $?
  $GNULIB_TOOL --copy-file build-aux/gitlog-to-changelog || exit $?
  $GNULIB_TOOL --copy-file build-aux/update-copyright || exit $?
  $GNULIB_TOOL --copy-file build-aux/useless-if-before-free || exit $?
  $GNULIB_TOOL --copy-file build-aux/vc-list-files || exit $?
  $GNULIB_TOOL --copy-file m4/init-package-version.m4 || exit $?
  $GNULIB_TOOL --copy-file m4/version-stamp.m4 || exit $?
  $GNULIB_TOOL --copy-file top/GNUmakefile . || exit $?
  $GNULIB_TOOL --copy-file top/maint.mk . || exit $?

  # Fetch config.guess, config.sub.
  for file in config.guess config.sub; do
    $GNULIB_TOOL --copy-file build-aux/$file && chmod a+x build-aux/$file || exit $?
  done

  # Fetch INSTALL.generic.
  $GNULIB_TOOL --copy-file doc/INSTALL.UTF-8 INSTALL.generic
fi

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
(if ! $skip_gnulib; then export GNULIB_SRCDIR; fi
 ./autogen.sh $skip_gnulib_option
) || exit $?
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
cp -p gettext-runtime/po/insert-header.sed gettext-tools/po/insert-header.sed
cp -p gettext-runtime/po/remove-potcdate.sed gettext-tools/po/remove-potcdate.sed

echo "$0: generating configure in gettext-tools..."
cd gettext-tools
aclocal -I m4 -I ../gettext-runtime/m4 -I ../m4 -I gnulib-m4 -I libgrep/gnulib-m4 -I libgettextpo/gnulib-m4 -I tests/gnulib-m4 \
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
