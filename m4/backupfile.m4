#serial 1

# Prerequisites of lib/backupfile.h

AC_DEFUN([gt_PREREQ_BACKUPFILE],
[
  dnl For backupfile.c.
  AC_REQUIRE([AC_HEADER_DIRENT])
  AC_FUNC_CLOSEDIR_VOID
  AC_CHECK_HEADERS(limits.h string.h)
  dnl For addext.c.
  AC_SYS_LONG_FILE_NAMES
  AC_CHECK_FUNCS(pathconf)
  AC_CHECK_HEADERS(limits.h string.h unistd.h)
])
