# woe32-dll.m4 serial 1
dnl Copyright (C) 2005 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl From Bruno Haible.

# Add --disable-auto-import to the LDFLAGS if the linker supports it.
# GNU ld has an --enable-auto-import option, and it is the default on Cygwin
# since July 2005. But it two fatal drawbacks:
#   - It produces executables and shared libraries with relocations in the
#     .text segment, defeating the principles of virtual memory.
#   - For some constructs such as
#         extern int var;
#         int * const b = &var;
#     it creates an executable that will give an error at runtime, rather than
#     either a compile-time or link-time error or a working executable.
#     (This is with both gcc and g++.) Whereas this code, not relying on auto-
#     import:
#         extern __declspec (dllimport) int var;
#         int * const b = &var;
#     gives a compile-time error with gcc and works with g++.
AC_DEFUN([gl_WOE32_DLL],
[
  AC_REQUIRE([AC_CANONICAL_HOST])
  case "$host_os" in
    mingw* | cygwin*)
      AC_MSG_CHECKING([for auto-import of symbols])
      AC_CACHE_VAL([gl_cv_ld_autoimport], [
        gl_save_LDFLAGS="$LDFLAGS"
        LDFLAGS="$LDFLAGS -Wl,--disable-auto-import"
        AC_TRY_LINK([], [], [gl_cv_ld_autoimport=yes], [gl_cv_ld_autoimport=no])
        LDFLAGS="$gl_save_LDFLAGS"])
      AC_MSG_RESULT([$gl_cv_ld_autoimport])
      if test $gl_cv_ld_autoimport = yes; then
        LDFLAGS="$LDFLAGS -Wl,--disable-auto-import"
      fi
      ;;
  esac
])
