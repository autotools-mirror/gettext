# serial 1

dnl From Bruno Haible.

dnl AC_LIB_LINKFLAGS(name [, dependencies]) searches for libname and
dnl the libraries corresponding to explicit and implicit dependencies.
dnl Sets and AC_SUBSTs the LIB${NAME} variable and augments the CPPFLAGS
dnl variable.
AC_DEFUN([AC_LIB_LINKFLAGS],
[
  AC_REQUIRE([AC_LIB_PREPARE_PREFIX])
  AC_REQUIRE([AC_LIB_RPATH])
  define([name],[translit([$1],[./-], [___])])
  define([NAME],[translit([$1],[abcdefghijklmnopqrstuvwxyz./-],
                               [ABCDEFGHIJKLMNOPQRSTUVWXYZ___])])
  AC_CACHE_CHECK([how to link with lib[]$1], [ac_cv_lib[]name[]_libs], [
    AC_LIB_LINKFLAGS_BODY([$1], [$2])
    ac_cv_lib[]name[]_libs="$LIB[]NAME"
    ac_cv_lib[]name[]_cppflags="$INC[]NAME"
  ])
  LIB[]NAME="$ac_cv_lib[]name[]_libs"
  INC[]NAME="$ac_cv_lib[]name[]_cppflags"
  AC_LIB_APPENDTOVAR([CPPFLAGS], [$INC[]NAME])
  AC_SUBST([LIB[]NAME])
])

dnl Determine the platform dependent parameters needed to use rpath:
dnl libext, shlibext, hardcode_libdir_flag_spec, hardcode_libdir_separator,
dnl hardcode_direct, hardcode_minus_L,
dnl sys_lib_search_path_spec, sys_lib_dlsearch_path_spec.
AC_DEFUN([AC_LIB_RPATH],
[
  AC_REQUIRE([AC_PROG_CC])                dnl we use $CC, $GCC, $LDFLAGS
  AC_REQUIRE([AC_LIB_PROG_LD])            dnl we use $LD, $with_gnu_ld
  AC_REQUIRE([AC_CANONICAL_HOST])         dnl we use $host
  AC_REQUIRE([AC_CONFIG_AUX_DIR_DEFAULT]) dnl we use $ac_aux_dir
  AC_CACHE_CHECK([for shared library run path origin], acl_cv_rpath, [
    CC="$CC" GCC="$GCC" LDFLAGS="$LDFLAGS" LD="$LD" with_gnu_ld="$with_gnu_ld" \
    ${CONFIG_SHELL-/bin/sh} "$ac_aux_dir/config.rpath" "$host" > conftest.sh
    . ./conftest.sh
    rm -f ./conftest.sh
    acl_cv_rpath=done
  ])
  wl="$acl_cv_wl"
  libext="$acl_cv_libext"
  shlibext="$acl_cv_shlibext"
  hardcode_libdir_flag_spec="$acl_cv_hardcode_libdir_flag_spec"
  hardcode_libdir_separator="$acl_cv_hardcode_libdir_separator"
  hardcode_direct="$acl_cv_hardcode_direct"
  hardcode_minus_L="$acl_cv_hardcode_minus_L"
  sys_lib_search_path_spec="$acl_cv_sys_lib_search_path_spec"
  sys_lib_dlsearch_path_spec="$acl_cv_sys_lib_dlsearch_path_spec"
])

dnl AC_LIB_LINKFLAGS_BODY(name [, dependencies]) searches for libname and
dnl the libraries corresponding to explicit and implicit dependencies.
dnl Sets the INC${NAME} and LIB${NAME} variables.
AC_DEFUN([AC_LIB_LINKFLAGS_BODY],
[
  define([NAME],[translit([$1],[abcdefghijklmnopqrstuvwxyz./-],
                               [ABCDEFGHIJKLMNOPQRSTUVWXYZ___])])
  dnl By default, look in $includedir and $libdir.
  use_additional=yes
  prefix="$acl_final_prefix" exec_prefix="$acl_final_exec_prefix" eval additional_includedir=\"$includedir\"
  prefix="$acl_final_prefix" exec_prefix="$acl_final_exec_prefix" eval additional_libdir=\"$libdir\"
  AC_ARG_WITH([lib$1-prefix],
[  --with-lib$1-prefix[=DIR]  search for lib$1 in DIR/include and DIR/lib
  --without-lib$1-prefix     don't search for lib$1 in includedir and libdir],
[
    if test "X$withval" = "Xno"; then
      use_additional=no
    else
      if test "X$withval" = "X"; then
        prefix="$acl_final_prefix" exec_prefix="$acl_final_exec_prefix" eval additional_includedir=\"$includedir\"
        prefix="$acl_final_prefix" exec_prefix="$acl_final_exec_prefix" eval additional_libdir=\"$libdir\"
      else
        additional_includedir="$withval/include"
        additional_libdir="$withval/lib"
      fi
    fi
])
  dnl Search the library and its dependencies in $additional_libdir and
  dnl $LDFLAGS. Using breadth-first-seach.
  LIB[]NAME=
  INC[]NAME=
  rpathdirs=
  names_already_handled=
  names_next_round='$1 $2'
  while test -n "$names_next_round"; do
    names_this_round="$names_next_round"
    names_next_round=
    for name in $names_this_round; do
      already_handled=
      for n in $names_already_handled; do
        if test "$n" = "$name"; then
          already_handled=yes
          break
        fi
      done
      if test -z "$already_handled"; then
        names_already_handled="$names_already_handled $name"
        dnl Search the library lib$name in $additional_libdir and $LDFLAGS
        dnl and the already constructed $LIBNAME.
        found_dir=
        found_la=
        found_so=
        found_a=
        if test $use_additional = yes; then
          if test -n "$shlibext" && test -f "$additional_libdir/lib$name.$shlibext"; then
            found_dir="$additional_libdir"
            found_so="$additional_libdir/lib$name.$shlibext"
            if test -f "$additional_libdir/lib$name.la"; then
              found_la="$additional_libdir/lib$name.la"
            fi
          else
            if test -f "$additional_libdir/lib$name.$libext"; then
              found_dir="$additional_libdir"
              found_a="$additional_libdir/lib$name.$libext"
              if test -f "$additional_libdir/lib$name.la"; then
                found_la="$additional_libdir/lib$name.la"
              fi
            fi
          fi
        fi
        if test "X$found_dir" = "X"; then
          for x in $LDFLAGS $LIB[]NAME; do
            prefix="$acl_final_prefix" exec_prefix="$acl_final_exec_prefix" eval x=\"$x\"
            case "$x" in
              -L*)
                dir=`echo "X$x" | sed -e 's/^X-L//'`
                if test -n "$shlibext" && test -f "$dir/lib$name.$shlibext"; then
                  found_dir="$dir"
                  found_so="$dir/lib$name.$shlibext"
                  if test -f "$dir/lib$name.la"; then
                    found_la="$dir/lib$name.la"
                  fi
                else
                  if test -f "$dir/lib$name.$libext"; then
                    found_dir="$dir"
                    found_a="$dir/lib$name.$libext"
                    if test -f "$dir/lib$name.la"; then
                      found_la="$dir/lib$name.la"
                    fi
                  fi
                fi
                ;;
            esac
            if test "X$found_dir" != "X"; then
              break
            fi
          done
        fi
        if test "X$found_dir" != "X"; then
          dnl Found the library.
          dnl Most of the following complexities is not needed when libtool
          dnl is used.
          ifdef([AC_PROG_][LIBTOOL], [], [
            if test "X$found_so" != "X"; then
              dnl Linking with a shared library. We attempt to hardcode its
              dnl directory into the executable's runpath, unless it's the
              dnl standard /usr/lib.
              if test "X$found_dir" = "X/usr/include"; then
                dnl No hardcoding is needed.
                LIB[]NAME="${LIB[]NAME}${LIB[]NAME:+ }$found_so"
              else
                if test "$hardcode_direct" = yes; then
                  dnl Using DIR/libNAME.so during linking hardcodes DIR into the
                  dnl resulting binary.
                  LIB[]NAME="${LIB[]NAME}${LIB[]NAME:+ }$found_so"
                else
                  if test -n "$hardcode_libdir_flag_spec" && test "$hardcode_minus_L" = no; then
                    dnl Use an explicit option to hardcode DIR into the resulting
                    dnl binary.
                    LIB[]NAME="${LIB[]NAME}${LIB[]NAME:+ }$found_so"
                    dnl Potentially add DIR to rpathdirs.
                    dnl The rpathdirs will be appended to $LIBNAME at the end.
                    haveit=
                    for x in $rpathdirs; do
                      if test "X$x" = "X$found_dir"; then
                        haveit=yes
                        break
                      fi
                    done
                    if test -z "$haveit"; then
                      rpathdirs="$rpathdirs $found_dir"
                    fi
                  else
                    dnl Rely on "-L$found_dir".
                    dnl But don't add it if it's already contained in the LDFLAGS
                    dnl or the already constructed $LIBNAME
                    haveit=
                    for x in $LDFLAGS $LIB[]NAME; do
                      prefix="$acl_final_prefix" exec_prefix="$acl_final_exec_prefix" eval x=\"$x\"
                      if test "X$x" = "X-L$found_dir"; then
                        haveit=yes
                        break
                      fi
                    done
                    if test -z "$haveit"; then
                      LIB[]NAME="${LIB[]NAME}${LIB[]NAME:+ }-L$found_dir"
                    fi
                    if test "$hardcode_minus_L" != no; then
                      dnl FIXME: Not sure whether we should use
                      dnl "-L$found_dir -l$name" or "-L$found_dir $found_so"
                      dnl here.
                      LIB[]NAME="${LIB[]NAME}${LIB[]NAME:+ }$found_so"
                    else
                      dnl We cannot use $hardcode_runpath_var and LD_RUN_PATH
                      dnl here, because this doesn't fit in flags passed to the
                      dnl compiler. So give up. No hardcoding.
                      dnl FIXME: Not sure whether we should use
                      dnl "-L$found_dir -l$name" or "-L$found_dir $found_so"
                      dnl here.
                      dnl FIXME: Which systems does this affect?
                      LIB[]NAME="${LIB[]NAME}${LIB[]NAME:+ }-l$name"
                    fi
                  fi
                fi
              fi
            else
              if test "X$found_a" != "X"; then
                dnl Linking with a static library.
                LIB[]NAME="${LIB[]NAME}${LIB[]NAME:+ }$found_a"
              else
                dnl We shouldn't come here, but anyway it's good to have a
                dnl fallback.
          ])
                LIB[]NAME="${LIB[]NAME}${LIB[]NAME:+ }-L$found_dir -l$name"
          ifdef([AC_PROG_][LIBTOOL], [], [
              fi
            fi
          ])
          dnl Assume the include files are nearby.
          additional_includedir=
          case "$found_dir" in
            */lib | */lib/)
              basedir=`echo "X$found_dir" | sed -e 's,^X,,' -e 's,/lib/*$,,'`
              additional_includedir="$basedir/include"
              ;;
          esac
          if test "X$additional_includedir" != "X"; then
            dnl Potentially add $additional_includedir to $INCNAME.
            dnl But don't add it
            dnl   1. if it's the standard /usr/include,
            dnl   2. if it's already present in $CPPFLAGS or the already
            dnl      constructed $INCNAME,
            dnl   3. if it's /usr/local/include and we are using GCC on Linux,
            dnl   4. if it doesn't exist as a directory.
            if test "X$additional_includedir" != "X/usr/include"; then
              haveit=
              for x in $CPPFLAGS $INC[]NAME; do
                prefix="$acl_final_prefix" exec_prefix="$acl_final_exec_prefix" eval x=\"$x\"
                if test "X$x" = "X-I$additional_includedir"; then
                  haveit=yes
                  break
                fi
              done
              if test -z "$haveit"; then
                if test "X$additional_includedir" = "X/usr/local/include"; then
                  if test -n "$GCC"; then
                    case $host_os in
                      linux*) haveit=yes;;
                    esac
                  fi
                fi
                if test -z "$haveit"; then
                  if test -d "$additional_includedir"; then
                    dnl Really add $additional_includedir to $INCNAME.
                    INC[]NAME="${INC[]NAME}${INC[]NAME:+ }-I$additional_includedir"
                  fi
                fi
              fi
            fi
          fi
          dnl Look for dependencies.
          if test -n "$found_la"; then
            dnl Read the .la file. It defines the variables
            dnl dlname, library_names, old_library, dependency_libs, current,
            dnl age, revision, installed, dlopen, dlpreopen, libdir.
            save_libdir="$libdir"
            case "$found_la" in
              */* | *\\*) . "$found_la" ;;
              *) . "./$found_la" ;;
            esac
            libdir="$save_libdir"
            dnl We use only dependency_libs.
            for dep in $dependency_libs; do
              case "$dep" in
                -L*)
                  additional_libdir=`echo "X$dep" | sed -e 's/^X-L//'`
                  dnl Potentially add $additional_libdir to $LIBNAME.
                  dnl But don't add it
                  dnl   1. if it's the standard /usr/lib,
                  dnl   2. if it's already present in $LDFLAGS or the already
                  dnl      constructed $LIBNAME,
                  dnl   3. if it's /usr/local/lib and we are using GCC on Linux,
                  dnl   4. if it doesn't exist as a directory.
                  if test "X$additional_libdir" != "X/usr/lib"; then
                    haveit=
                    for x in $LDFLAGS $LIB[]NAME; do
                      prefix="$acl_final_prefix" exec_prefix="$acl_final_exec_prefix" eval x=\"$x\"
                      if test "X$x" = "X-L$additional_libdir"; then
                        haveit=yes
                        break
                      fi
                    done
                    if test -z "$haveit"; then
                      if test "X$additional_libdir" = "X/usr/local/lib"; then
                        if test -n "$GCC"; then
                          case $host_os in
                            linux*) haveit=yes;;
                          esac
                        fi
                      fi
                      if test -z "$haveit"; then
                        if test -d "$additional_libdir"; then
                          dnl Really add $additional_libdir to $LIBNAME.
                          LIB[]NAME="${LIB[]NAME}${LIB[]NAME:+ }-L$additional_libdir"
                        fi
                      fi
                    fi
                  fi
                  ;;
                -l*)
                  dnl Handle this in the next round.
                  names_next_round="$names_next_round "`echo "X$dep" | sed -e 's/^X-l//'`
                  ;;
                *.la)
                  dnl Handle this in the next round. Throw away the .la's
                  dnl directory; it is already contained in a preceding -L
                  dnl option.
                  names_next_round="$names_next_round "`echo "X$dep" | sed -e 's,^X.*/,,' -e 's,^lib,,' -e 's,\.la$,,'`
                  ;;
                *)
                  dnl Most likely an immediate library name.
                  LIB[]NAME="${LIB[]NAME}${LIB[]NAME:+ }$dep"
                  ;;
              esac
            done
          fi
        else
          dnl Didn't find the library; assume it is in the system directories
          dnl known to the linker and runtime loader. (All the system
          dnl directories known to the linker should also be known to the
          dnl runtime loader, otherwise the system is severely misconfigured.)
          LIB[]NAME="${LIB[]NAME}${LIB[]NAME:+ }-l$name"
        fi
      fi
    done
  done
  if test "X$rpathdirs" != "X"; then
    if test -n "$hardcode_libdir_separator"; then
      dnl Weird platform: only the last -rpath option counts, the user must
      dnl pass all path elements in one option. We can arrange that for a
      dnl single library, but not when more than one $LIBNAMEs are used.
      alldirs=
      for found_dir in $rpathdirs; do
        alldirs="${alldirs}${alldirs:+$hardcode_libdir_separator}$found_dir"
      done
      libdir="$alldirs" wl="$wl" eval flag=\"$hardcode_libdir_flag_spec\"
      LIB[]NAME="${LIB[]NAME}${LIB[]NAME:+ }$flag"
    else
      dnl The -rpath options are cumulative.
      for found_dir in $rpathdirs; do
        libdir="$found_dir" wl="$wl" eval flag=\"$hardcode_libdir_flag_spec\"
        LIB[]NAME="${LIB[]NAME}${LIB[]NAME:+ }$flag"
      done
    fi
  fi
])

dnl AC_LIB_APPENDTOVAR(VAR, CONTENTS) appends the elements of CONTENTS to VAR,
dnl unless already present in VAR.
AC_DEFUN([AC_LIB_APPENDTOVAR],
[
  for element in [$2]; do
    haveit=
    for x in $[$1]; do
      prefix="$acl_final_prefix" exec_prefix="$acl_final_exec_prefix" eval x=\"$x\"
      if test "X$x" = "X$element"; then
        haveit=yes
        break
      fi
    done
    if test -z "$haveit"; then
      [$1]="${[$1]}${[$1]:+ }$element"
    fi
  done
])
