# serial 1

dnl From Bruno Haible.
dnl Test whether ssize_t is defined.
dnl Prerequisite: AC_CHECK_HEADERS(unistd.h)

AC_DEFUN([gt_TYPE_SSIZE_T],
[
  AC_CACHE_CHECK([for ssize_t], gt_cv_ssize_t,
    [AC_TRY_COMPILE([
#include <sys/types.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif], [int x = sizeof (ssize_t *) + sizeof (ssize_t);],
       gt_cv_ssize_t=yes, gt_cv_ssize_t=no)])
  if test $gt_cv_ssize_t = no; then
    AC_DEFINE(ssize_t, int,
              [Define as a signed type of the same size as size_t.])
  fi
])
