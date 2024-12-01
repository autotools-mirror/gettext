dnl Example for use of GNU gettext.
dnl This file is in the public domain.
dnl
dnl Configuration file - processed by autoconf.

AC_INIT([hello-objc], [0])
AC_CONFIG_SRCDIR([hello.m])
AM_INIT_AUTOMAKE([1.11])

dnl Test for gobjc before gcc, since that's what the EPEL 9 package 'gcc-objc'
dnl installs (on systems where gcc does not support Objective C).
AC_PROG_OBJC([gobjc gcc objcc objc cc CC clang])
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.23])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([m4/Makefile])
AC_CONFIG_FILES([po/Makefile.in])
AC_OUTPUT
