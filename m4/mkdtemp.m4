#serial 1

# Prerequisites of lib/mkdtemp.c

AC_DEFUN([gt_FUNC_MKDTEMP],
[
  AC_REPLACE_FUNCS(mkdtemp)
  AC_STAT_MACROS_BROKEN
  jm_AC_HEADER_INTTYPES_H
  jm_AC_HEADER_STDINT_H
  AC_CHECK_HEADERS(fcntl.h sys/time.h time.h unistd.h)
  AC_CHECK_FUNCS(gettimeofday)
])
