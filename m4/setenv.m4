#serial 1

# Check if a variable is properly declared.
# gt_CHECK_VAR_DECL(includes,variable)
AC_DEFUN([gt_CHECK_VAR_DECL],
[
  define(gt_cv_var, [gt_cv_var_]$2[_declaration])
  AC_MSG_CHECKING([if $2 is properly declared])
  AC_CACHE_VAL(gt_cv_var, [
    AC_TRY_COMPILE([$1
      extern struct { int foo; } $2;],
      [$2.foo = 1;],
      gt_cv_var=no,
      gt_cv_var=yes)])
  AC_MSG_RESULT($gt_cv_var)
  if test $gt_cv_var = yes; then
    AC_DEFINE([HAVE_]translit($2, [a-z], [A-Z])[_DECL], 1,
              [Define if you have the declaration of $2.])
  fi
])

# Prerequisites of lib/setenv.c

AC_DEFUN([gt_FUNC_SETENV],
[
  AC_REPLACE_FUNCS(setenv)
  AC_CHECK_HEADERS(search.h stdlib.h string.h unistd.h)
  AC_CHECK_FUNCS(tsearch)
  gt_CHECK_VAR_DECL([#include <errno.h>], errno)
  gt_CHECK_VAR_DECL([#include <unistd.h>], environ)
])
