dnl Configuration for the gettext-runtime directory of GNU gettext
dnl Copyright (C) 1995-2016, 2018-2021 Free Software Foundation, Inc.
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

dnl Set preferences for the gnulib module 'relocatable-lib-lgpl':
dnl Enable relocatability unconditionally.
enable_relocatable=yes

dnl Early checks, assembled by gnulib.
gl_EARLY

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
if test $is_woe32 = yes; then
  dnl Check for a program that compiles Windows resource files.
  AC_CHECK_TOOL([WINDRES], [windres])
fi

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

dnl Checks for header files, functions and declarations.
gl_INIT

gt_INTL_SUBDIR_CORE

gt_PRINTF_POSIX
case $gt_cv_func_printf_posix in
  *yes) HAVE_POSIX_PRINTF=1 ;;
  *)    HAVE_POSIX_PRINTF=0 ;;
esac
AC_SUBST([HAVE_POSIX_PRINTF])

AC_CHECK_FUNCS_ONCE([asprintf])
if test "$ac_cv_func_asprintf" = yes; then
  HAVE_ASPRINTF=1
else
  HAVE_ASPRINTF=0
fi
AC_SUBST([HAVE_ASPRINTF])

AC_CHECK_FUNCS_ONCE([snprintf])
if test "$ac_cv_func_snprintf" = yes; then
  HAVE_SNPRINTF=1
else
  HAVE_SNPRINTF=0
fi
AC_SUBST([HAVE_SNPRINTF])

AC_CHECK_FUNCS_ONCE([wprintf])
if test "$ac_cv_func_wprintf" = yes; then
  HAVE_WPRINTF=1
else
  HAVE_WPRINTF=0
fi
AC_SUBST([HAVE_WPRINTF])

AC_CHECK_FUNCS_ONCE([newlocale])
if test "$ac_cv_func_newlocale" = yes; then
  HAVE_NEWLOCALE=1
else
  HAVE_NEWLOCALE=0
fi
AC_SUBST([HAVE_NEWLOCALE])

AC_CHECK_HEADERS([features.h])

dnl Use the _snprintf function only if it is declared (because on NetBSD it
dnl is defined as a weak alias of snprintf; we prefer to use the latter).
AC_CHECK_DECLS([_snprintf, _snwprintf], , , [#include <stdio.h>])

if test $gt_localename_enhances_locale_funcs = yes; then
  ENHANCE_LOCALE_FUNCS=1
else
  ENHANCE_LOCALE_FUNCS=0
fi
AC_SUBST([ENHANCE_LOCALE_FUNCS])

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
WOE32DLL=$is_woe32dll
AC_SUBST([WOE32DLL])
AM_CONDITIONAL([WOE32DLL], [test $is_woe32dll = yes])
if test $is_woe32dll = yes; then
  AC_DEFINE([WOE32DLL], [1],
    [Define when --enable-shared is used on mingw or Cygwin.])
fi

dnl Put some default definitions into config.h.
AH_BOTTOM([
/* Tweak gnulib code according to the needs of this library.  */
#define IN_LIBINTL 1

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

dnl Rename some macros and functions used for locking.
AH_VERBATIM([0locking], [
#define __libc_lock_t                   gl_lock_t
#define __libc_lock_define              gl_lock_define
#define __libc_lock_define_initialized  gl_lock_define_initialized
#define __libc_lock_init                gl_lock_init
#define __libc_lock_lock                gl_lock_lock
#define __libc_lock_unlock              gl_lock_unlock
#define __libc_lock_recursive_t                   gl_recursive_lock_t
#define __libc_lock_define_recursive              gl_recursive_lock_define
#define __libc_lock_define_initialized_recursive  gl_recursive_lock_define_initialized
#define __libc_lock_init_recursive                gl_recursive_lock_init
#define __libc_lock_lock_recursive                gl_recursive_lock_lock
#define __libc_lock_unlock_recursive              gl_recursive_lock_unlock
])

dnl Hide undesired symbols from the global namespace, by prefixing them with
dnl "_libintl_" or "_nl_".
AH_VERBATIM([0noexport], [
#define asnprintf                  _libintl_asnprintf
#define rpl_asnprintf              _libintl_asnprintf
/* Symbols defined by gnulib module 'float'.  */
#define gl_LDBL_MAX                _libintl_LDBL_MAX
/* Symbols defined by gnulib module 'localename'.  */
#define gl_locale_name_canonicalize      _nl_locale_name_canonicalize
#define gl_locale_name_from_win32_LANGID _nl_locale_name_from_win32_LANGID
#define gl_locale_name_from_win32_LCID   _nl_locale_name_from_win32_LCID
#define gl_locale_name_thread_unsafe     _nl_locale_name_thread_unsafe
#define gl_locale_name_thread            _nl_locale_name_thread
#define gl_locale_name_posix             _nl_locale_name_posix
#define gl_locale_name_environ           _nl_locale_name_environ
#define gl_locale_name_default           _nl_locale_name_default
#define gl_locale_name                   _nl_locale_name
/* Symbols defined by gnulib module 'lock'.  */
#define glthread_lock_init                            _libintl_lock_init
#define glthread_lock_lock                            _libintl_lock_lock
#define glthread_lock_unlock                          _libintl_lock_unlock
#define glthread_lock_destroy                         _libintl_lock_destroy
#define glthread_rwlock_init                          _libintl_rwlock_init
#define glthread_rwlock_rdlock                        _libintl_rwlock_rdlock
#define glthread_rwlock_wrlock                        _libintl_rwlock_wrlock
#define glthread_rwlock_unlock                        _libintl_rwlock_unlock
#define glthread_rwlock_destroy                       _libintl_rwlock_destroy
#define glthread_recursive_lock_init                  _libintl_recursive_lock_init
#define glthread_recursive_lock_lock                  _libintl_recursive_lock_lock
#define glthread_recursive_lock_unlock                _libintl_recursive_lock_unlock
#define glthread_recursive_lock_destroy               _libintl_recursive_lock_destroy
#define glthread_rwlock_init_for_glibc                _libintl_rwlock_init_for_glibc
#define glthread_rwlock_init_multithreaded            _libintl_rwlock_init_multithreaded
#define glthread_rwlock_rdlock_multithreaded          _libintl_rwlock_rdlock_multithreaded
#define glthread_rwlock_wrlock_multithreaded          _libintl_rwlock_wrlock_multithreaded
#define glthread_rwlock_unlock_multithreaded          _libintl_rwlock_unlock_multithreaded
#define glthread_rwlock_destroy_multithreaded         _libintl_rwlock_destroy_multithreaded
#define glthread_recursive_lock_init_multithreaded    _libintl_recursive_lock_init_multithreaded
#define glthread_recursive_lock_lock_multithreaded    _libintl_recursive_lock_lock_multithreaded
#define glthread_recursive_lock_unlock_multithreaded  _libintl_recursive_lock_unlock_multithreaded
#define glthread_recursive_lock_destroy_multithreaded _libintl_recursive_lock_destroy_multithreaded
#define glthread_once_singlethreaded                  _libintl_once_singlethreaded
#define glthread_once_multithreaded                   _libintl_once_multithreaded
/* Symbols defined by gnulib module 'relocatable-lib-lgpl'.  */
#define relocate                   _libintl_relocate
#define relocate2                  _libintl_relocate2
/* Symbols defined by gnulib module 'setlocale-null'.  */
#define setlocale_null             _libintl_setlocale_null
#define setlocale_null_r           _libintl_setlocale_null_r
/* Symbols defined by gnulib module 'threadlib'.  */
#define glthread_in_use            _libintl_glthread_in_use
/* Symbols defined by gnulib module 'vasnprintf'.  */
#define printf_fetchargs           _libintl_printf_fetchargs
#define printf_parse               _libintl_printf_parse
#define vasnprintf                 _libintl_vasnprintf
/* Symbols defined by gnulib module 'windows-mutex'.  */
#define glwthread_mutex_init       _libintl_glwthread_mutex_init
#define glwthread_mutex_lock       _libintl_glwthread_mutex_lock
#define glwthread_mutex_trylock    _libintl_glwthread_mutex_trylock
#define glwthread_mutex_unlock     _libintl_glwthread_mutex_unlock
#define glwthread_mutex_destroy    _libintl_glwthread_mutex_destroy
/* Symbols defined by gnulib module 'windows-once'.  */
#define glwthread_once             _libintl_glwthread_once
/* Symbols defined by gnulib module 'windows-recmutex'.  */
#define glwthread_recmutex_init    _libintl_glwthread_recmutex_init
#define glwthread_recmutex_lock    _libintl_glwthread_recmutex_lock
#define glwthread_recmutex_trylock _libintl_glwthread_recmutex_trylock
#define glwthread_recmutex_unlock  _libintl_glwthread_recmutex_unlock
#define glwthread_recmutex_destroy _libintl_glwthread_recmutex_destroy
/* Symbols defined by gnulib module 'windows-rwlock'.  */
#define glwthread_rwlock_init      _libintl_glwthread_rwlock_init
#define glwthread_rwlock_rdlock    _libintl_glwthread_rwlock_rdlock
#define glwthread_rwlock_wrlock    _libintl_glwthread_rwlock_wrlock
#define glwthread_rwlock_tryrdlock _libintl_glwthread_rwlock_tryrdlock
#define glwthread_rwlock_trywrlock _libintl_glwthread_rwlock_trywrlock
#define glwthread_rwlock_unlock    _libintl_glwthread_rwlock_unlock
#define glwthread_rwlock_destroy   _libintl_glwthread_rwlock_destroy
/* Symbols defined by gnulib module 'xsize'.  */
#define xmax                       _libintl_xmax
#define xsum                       _libintl_xsum
#define xsum3                      _libintl_xsum3
#define xsum4                      _libintl_xsum4
])
dnl Symbols defined by gnulib module 'memchr'.
if test $REPLACE_MEMCHR = 1; then
  AC_DEFINE([memchr],           [_libintl_memchr],    [Hidden symbol.])
  AC_DEFINE([rpl_memchr],       [_libintl_memchr],    [Hidden symbol.])
fi
dnl Symbols defined by gnulib module 'tsearch'.
if test $HAVE_TSEARCH = 0 || test $HAVE_TWALK = 0 || test $REPLACE_TSEARCH = 1; then
  AC_DEFINE([tsearch],          [_libintl_tsearch],   [Hidden symbol.])
  AC_DEFINE([rpl_tsearch],      [_libintl_tsearch],   [Hidden symbol.])
  AC_DEFINE([tfind],            [_libintl_tfind],     [Hidden symbol.])
  AC_DEFINE([rpl_tfind],        [_libintl_tfind],     [Hidden symbol.])
  AC_DEFINE([tdelete],          [_libintl_tdelete],   [Hidden symbol.])
  AC_DEFINE([rpl_tdelete],      [_libintl_tdelete],   [Hidden symbol.])
  AC_DEFINE([twalk],            [_libintl_twalk],     [Hidden symbol.])
  AC_DEFINE([rpl_twalk],        [_libintl_twalk],     [Hidden symbol.])
fi

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([gnulib-lib/Makefile])

AC_OUTPUT
