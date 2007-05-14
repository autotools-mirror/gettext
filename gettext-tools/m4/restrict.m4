# restrict.m4 serial 1 (gettext-0.16.2)
dnl Copyright (C) 2007 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl This file overrides the AC_C_RESTRICT macro from autoconf 2.60..2.61,
dnl so that mixed use of GNU C and GNU C++ works.
dnl This file can be removed once autoconf >= 2.62 can be assumed.

AC_DEFUN([AC_C_RESTRICT],
[AC_CACHE_CHECK([for C/C++ restrict keyword], ac_cv_c_restrict,
  [ac_cv_c_restrict=no
   # Try the official restrict keyword, then gcc's __restrict, and
   # the less common variants.
   for ac_kw in __restrict __restrict__ _Restrict restrict; do
     AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
      [[typedef int * int_ptr;
        int foo (int_ptr $ac_kw ip) {
        return ip[0];
       }]],
      [[int s[1];
        int * $ac_kw t = s;
        t[0] = 0;
        return foo(t)]])],
      [ac_cv_c_restrict=$ac_kw])
     test "$ac_cv_c_restrict" != no && break
   done
  ])
 case $ac_cv_c_restrict in
   restrict) ;;
   no) AC_DEFINE(restrict,,
         [Define to equivalent of C99 restrict keyword, or to nothing if this
          is not supported.  Do not define if restrict is supported directly.]) ;;
   *)  AC_DEFINE_UNQUOTED(restrict, $ac_cv_c_restrict) ;;
 esac
])# AC_C_RESTRICT
