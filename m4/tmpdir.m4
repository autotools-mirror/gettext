#serial 1

# Prerequisites for lib/tmpdir.c

AC_DEFUN([gt_TMPDIR],
[
  AC_STAT_MACROS_BROKEN
  AC_CHECK_FUNCS(__secure_getenv)
])
