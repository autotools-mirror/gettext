dnl serial 1

dnl From Bruno Haible.
dnl Test whether <stdbool.h> is supported or must be substituted.

AC_DEFUN([gt_STDBOOL_H],
[dnl gcc 2.95.2 has an <stdbool.h> for which both 'true' and 'false' evaluate
dnl to 0 in preprocessor expressions.
AC_MSG_CHECKING([for stdbool.h])
AC_CACHE_VAL(gt_cv_header_stdbool_h, [
  AC_TRY_COMPILE([#include <stdbool.h>
#if false
int A[-1];
#endif
#define b -1
#if true
#undef b
#define b 1
#endif
int B[b];
], [], gt_cv_header_stdbool_h=yes, gt_cv_header_stdbool_h=no)])
AC_MSG_RESULT([$gt_cv_header_stdbool_h])
if test $gt_cv_header_stdbool_h = yes; then
  AC_DEFINE(HAVE_STDBOOL_H, 1,
            [Define if you have a working <stdbool.h> header file.])
  STDBOOL_H=''
else
  STDBOOL_H='stdbool.h'
fi
AC_SUBST(STDBOOL_H)
])
