#serial 1

# Prerequisites of javaexec.sh.
# Sets HAVE_JAVAEXEC to nonempty if javaexec.sh will work.

AC_DEFUN([gt_JAVAEXEC],
[
  AC_MSG_CHECKING([for Java virtual machine])
  AC_EGREP_CPP(yes, [
#if defined _WIN32 || defined __WIN32__ || defined __EMX__ || defined __DJGPP__
  yes
#endif
], CLASSPATH_SEPARATOR=';', CLASSPATH_SEPARATOR=':')
  HAVE_JAVAEXEC=1
  if test -n "$JAVA"; then
    ac_result="$JAVA"
  else
    if gij --version >/dev/null 2>/dev/null; then
      HAVE_GIJ=1
      ac_result="gij"
    else
      if java -version >/dev/null 2>/dev/null; then
        HAVE_JAVA_JVM=1
        ac_result="java"
      else
        if (jre >/dev/null 2>/dev/null || test $? = 1); then
          HAVE_JRE=1
          ac_result="jre"
        else
          if (jview -? >/dev/null 2>/dev/null || test $? = 1); then
            HAVE_JVIEW=1
            ac_result="jview"
          else
            HAVE_JAVAEXEC=
            ac_result="no"
          fi
        fi
      fi
    fi
  fi
  AC_MSG_RESULT([$ac_result])
  AC_SUBST(JAVA)
  AC_SUBST(CLASSPATH)
  AC_SUBST(CLASSPATH_SEPARATOR)
  AC_SUBST(HAVE_GIJ)
  AC_SUBST(HAVE_JAVA_JVM)
  AC_SUBST(HAVE_JRE)
  AC_SUBST(HAVE_JVIEW)
])
