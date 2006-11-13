# termcap.m4 serial 1 (gettext-0.16.1)
dnl Copyright (C) 2000-2002, 2006 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl From Bruno Haible.

AC_DEFUN([gl_TERMCAP],
[
  AC_REQUIRE([gl_TERMCAP_BODY])
])

AC_DEFUN([gl_TERMCAP_BODY],
[
  dnl Some systems have tgetent(), tgetnum(), tgetstr(), tgetflag(), tparm(),
  dnl tputs(), tgoto() in libc, some have it in libtermcap, some have it in
  dnl libncurses.
  dnl When both libtermcap and libncurses exist, we prefer the latter,
  dnl because libtermcap is unsecure by design and obsolete since 1994.
  dnl libcurses is useless: all platforms which have libcurses also have
  dnl libtermcap, and libcurses is unusable on some old Unices.

  dnl Prerequisites of AC_LIB_LINKFLAGS_BODY.
  AC_REQUIRE([AC_LIB_PREPARE_PREFIX])
  AC_REQUIRE([AC_LIB_RPATH])

  dnl Search for libncurses and define LIBNCURSES, LTLIBNCURSES and INCNCURSES
  dnl accordingly.
  AC_LIB_LINKFLAGS_BODY([ncurses])

  dnl Search for libtermcap and define LIBTERMCAP, LTLIBTERMCAP and INCTERMCAP
  dnl accordingly.
  AC_LIB_LINKFLAGS_BODY([termcap])

  AC_CACHE_CHECK([where termcap library functions come from], [gl_cv_termcap], [
    gl_cv_termcap="not found, consider installing GNU ncurses"
    AC_TRY_LINK([extern
      #ifdef __cplusplus
      "C"
      #endif
      int tgetent (char *, const char *);
      ], [return tgetent ((char *) 0, "xterm");], [gl_cv_termcap=libc])
    if test "$gl_cv_termcap" != libc; then
      gl_save_LIBS="$LIBS"
      LIBS="$LIBS $LIBNCURSES"
      AC_TRY_LINK([extern
        #ifdef __cplusplus
        "C"
        #endif
        int tgetent (char *, const char *);
        ], [return tgetent ((char *) 0, "xterm");], [gl_cv_termcap=libncurses])
      LIBS="$gl_save_LIBS"
      if test "$gl_cv_termcap" != libncurses; then
        gl_save_LIBS="$LIBS"
        LIBS="$LIBS $LIBTERMCAP"
        AC_TRY_LINK([extern
          #ifdef __cplusplus
          "C"
          #endif
          int tgetent (char *, const char *);
          ], [return tgetent ((char *) 0, "xterm");], [gl_cv_termcap=libtermcap])
        LIBS="$gl_save_LIBS"
      fi
    fi
  ])
  case "$gl_cv_termcap" in
    libc)
      LIBTERMCAP=
      LTLIBTERMCAP=
      INCTERMCAP=
      ;;
    libncurses)
      LIBTERMCAP="$LIBNCURSES"
      LTLIBTERMCAP="$LTLIBNCURSES"
      INCTERMCAP="$INCNCURSES"
      ;;
    libtermcap)
      ;;
  esac
  AC_SUBST([LIBTERMCAP])
  AC_SUBST([LTLIBTERMCAP])
  AC_SUBST([INCTERMCAP])
])
