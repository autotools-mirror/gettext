# Configuration of a program that links with librpathy.
AC_INIT(configure.in)
AC_CONFIG_AUX_DIRS($global_top_srcdir ../..)
AM_INIT_AUTOMAKE(gettext_rpathlyx, 0)
AC_PROG_CC
AC_LIB_LINKFLAGS([rpathx])
AC_LIB_LINKFLAGS([rpathy],[rpathx])
LIBRPATHY="$LIBRPATHY $LIBRPATHX"
AC_SUBST(global_top_srcdir)
AC_OUTPUT([Makefile])
