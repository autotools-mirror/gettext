#serial 2

# Check for setlocale declaration.

AC_DEFUN([gt_SETLOCALE],[
AC_CHECK_HEADERS([locale.h])
AC_MSG_CHECKING([for setlocale declaration])
AC_CACHE_VAL(gt_cv_proto_setlocale, [
AC_TRY_COMPILE([
#include <stdlib.h>
#include <locale.h>
extern
#ifdef __cplusplus
"C"
#endif
#if defined(__STDC__) || defined(__cplusplus)
char *setlocale (int category, char *locale);
#else
char *setlocale();
#endif
], [], gt_cv_proto_setlocale_arg1="", gt_cv_proto_setlocale_arg1="const")
gt_cv_proto_setlocale="extern char *setlocale (int category, $gt_cv_proto_setlocale_arg1 char *locale);"])
gt_cv_proto_setlocale=`echo "[$]gt_cv_proto_setlocale" | tr -s ' ' | sed -e 's/( /(/'`
AC_MSG_RESULT([$]{ac_t:-
         }[$]gt_cv_proto_setlocale)
AC_DEFINE_UNQUOTED(SETLOCALE_CONST,$gt_cv_proto_setlocale_arg1,
  [Define as const if the declaration of setlocale() needs const.])
])
