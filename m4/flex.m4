#serial 1

# Check for flex.

AC_DEFUN([gt_PROG_LEX],
[
  dnl Don't use AC_PROG_LEX or AM_PROG_LEX; we insist on flex.
  dnl Thus we don't need LEXLIB.
  AC_CHECK_PROG(LEX, flex, flex, :)

  dnl The next line is a workaround against an automake warning.
  undefine([AC_DECL_YYTEXT])
  dnl Replacement for AC_DECL_YYTEXT.
  LEX_OUTPUT_ROOT=lex.yy
  AC_SUBST(LEX_OUTPUT_ROOT)
])
