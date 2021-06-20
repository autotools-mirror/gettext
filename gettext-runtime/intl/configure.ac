dnl Configuration for the gettext-runtime directory of GNU gettext
dnl Copyright (C) 1995-2010, 2012-2015, 2018-2021 Free Software Foundation, Inc.
dnl
dnl This program is free software: you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 3 of the License, or
dnl (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program.  If not, see <https://www.gnu.org/licenses/>.

dnl Process this file with autoconf to produce a configure script.

AC_PREREQ([2.64])
AC_INIT([libintl],
	m4_esyscmd([../../build-aux/git-version-gen ../../.tarball-version]),
	[bug-gettext@gnu.org])
AC_CONFIG_SRCDIR([dcigettext.c])
AC_CONFIG_AUX_DIR([../../build-aux])
AM_INIT_AUTOMAKE([silent-rules])
AC_CONFIG_HEADERS([config.h])

dnl Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

dnl Check for host type.
AC_CANONICAL_HOST

dnl Checks for compiler output filename suffixes.
AC_OBJEXT
AC_EXEEXT

dnl For a 64-bit build on AIX, AC_PROG_RANLIB is not sufficient.
gl_PROG_AR_RANLIB

dnl Set preferences for the gnulib module 'threadlib'.
gl_AVOID_WINPTHREAD

dnl Make sure we see all GNU and Solaris extensions.
gl_USE_SYSTEM_EXTENSIONS
AM_PROG_CC_C_O

dnl Check for build configuration.

gl_WOE32_DLL

LT_INIT([win32-dll])

dnl Prepares the libtool configuration for handling of Windows resources, and
dnl sets the RC variable to a program that compiles Windows resource files.
LT_LANG([Windows Resource])

dnl On mingw and Cygwin, we can activate special Makefile rules which add
dnl version information to the shared libraries and executables.
case "$host_os" in
  mingw* | cygwin*) is_woe32=yes ;;
  *) is_woe32=no ;;
esac
AM_CONDITIONAL([WOE32], [test $is_woe32 = yes])

dnl Some code is only meant to be compiled on native Windows.
case "$host_os" in
  mingw*) is_windows_native=yes ;;
  *) is_windows_native=no ;;
esac
AM_CONDITIONAL([WINDOWS_NATIVE], [test $is_windows_native = yes])

dnl Checks for libraries.

dnl These are the only lines required to internationalize the package.
dnl (OK, not quite, the AC_CONFIG_FILES has also some parts.)
AM_GNU_GETTEXT([here], [need-ngettext])
dnl Conditionals for the intl/ directory.
AM_CONDITIONAL([USE_INCLUDED_LIBINTL], [test $USE_INCLUDED_LIBINTL = yes])
AM_CONDITIONAL([BUILD_INCLUDED_LIBINTL], [test $BUILD_INCLUDED_LIBINTL = yes])
AM_CONDITIONAL([PRELOADABLE_LIBINTL],
  [test $USE_INCLUDED_LIBINTL = no && test $GLIBC2 = yes && test "$enable_shared" = yes])

dnl Get common Gnulib macros.
gl_COMMON
dnl Get definitions for relocatable.c.
gl_RELOCATABLE_LIBRARY

dnl Compilation on mingw and Cygwin needs special Makefile rules, because
dnl 1. when we install a shared library, we must arrange to export
dnl    auxiliary pointer variables for every exported variable,
dnl 2. when we install a shared library and a static library simultaneously,
dnl    the include file specifies __declspec(dllimport) and therefore we
dnl    must arrange to define the auxiliary pointer variables for the
dnl    exported variables _also_ in the static library.
if test "$enable_shared" = yes; then
  case "$host_os" in
    mingw* | cygwin*) is_woe32dll=yes ;;
    *) is_woe32dll=no ;;
  esac
else
  is_woe32dll=no
fi
AM_CONDITIONAL([WOE32DLL], [test $is_woe32dll = yes])
if test $is_woe32dll = yes; then
  AC_DEFINE([WOE32DLL], [1],
    [Define when --enable-shared is used on mingw or Cygwin.])
fi

dnl Put some default definitions into config.h.
AH_BOTTOM([
/* On Windows, variables that may be in a DLL must be marked specially.  */
#if (defined _MSC_VER && defined _DLL) && !defined IN_RELOCWRAPPER
# define DLL_VARIABLE __declspec (dllimport)
#else
# define DLL_VARIABLE
#endif

/* Extra OS/2 (emx+gcc) defines.  */
#if defined __EMX__ && !defined __KLIBC__
# include "os2compat.h"
#endif
])

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
