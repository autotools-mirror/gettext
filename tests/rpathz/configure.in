# Configuration of librpathz that depends on librpathx and librpathy.
AC_INIT(configure.in)
AC_CONFIG_AUX_DIRS($global_top_srcdir ../..)
AM_INIT_AUTOMAKE(gettext_rpathz, 0)
AC_PROG_CC
AM_PROG_LIBTOOL
AC_LIB_LINKFLAGS([rpathx])
AC_LIB_LINKFLAGS([rpathy],[rpathx])
AC_SUBST(global_top_srcdir)
AC_OUTPUT([Makefile])
