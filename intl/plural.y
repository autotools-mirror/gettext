%{
/* Expression parsing for plural form selection.
   Copyright (C) 2000, 2001 Free Software Foundation, Inc.
   Written by Ulrich Drepper <drepper@cygnus.com>, 2000.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include "gettext.h"
#include "gettextP.h"

#define YYLEX_PARAM	&((struct parse_args *) arg)->cp
#define YYPARSE_PARAM	arg
%}
%pure_parser
%expect 10

%union {
  unsigned long int num;
  struct expression *exp;
}

%{
/* Prototypes for local functions.  */
static struct expression *new_exp_0 PARAMS ((enum operator op));
static struct expression *new_exp_2 PARAMS ((enum operator op,
					     struct expression *left,
					     struct expression *right));
static struct expression *new_exp_3 PARAMS ((enum operator op,
					     struct expression *bexp,
					     struct expression *tbranch,
					     struct expression *fbranch));
static int yylex PARAMS ((YYSTYPE *lval, const char **pexp));
static void yyerror PARAMS ((const char *str));
%}

%left '?'
%left '|'
%left '&'
%left '=', '!'
%left '+', '-'
%left '*', '/', '%'
%token <num> NUMBER
%type <exp> exp

%%

start:	  exp
	  {
	    ((struct parse_args *) arg)->res = $1;
	  }
	;

exp:	  exp '?' exp ':' exp
	  {
	    if (($$ = new_exp_3 (qmop, $1, $3, $5)) == NULL)
	      YYABORT
	  }
	| exp '|' exp
	  {
	    if (($$ = new_exp_2 (lor, $1, $3)) == NULL)
	      YYABORT
	  }
	| exp '&' exp
	  {
	    if (($$ = new_exp_2 (land, $1, $3)) == NULL)
	      YYABORT
	  }
	| exp '=' exp
	  {
	    if (($$ = new_exp_2 (equal, $1, $3)) == NULL)
	      YYABORT
	  }
	| exp '!' exp
	  {
	    if (($$ = new_exp_2 (not_equal, $1, $3)) == NULL)
	      YYABORT
	  }
	| exp '+' exp
	  {
	    if (($$ = new_exp_2 (plus, $1, $3)) == NULL)
	      YYABORT
	  }
	| exp '-' exp
	  {
	    if (($$ = new_exp_2 (minus, $1, $3)) == NULL)
	      YYABORT
	  }
	| exp '*' exp
	  {
	    if (($$ = new_exp_2 (mult, $1, $3)) == NULL)
	      YYABORT
	  }
	| exp '/' exp
	  {
	    if (($$ = new_exp_2 (divide, $1, $3)) == NULL)
	      YYABORT
	  }
	| exp '%' exp
	  {
	    if (($$ = new_exp_2 (module, $1, $3)) == NULL)
	      YYABORT
	  }
	| 'n'
	  {
	    if (($$ = new_exp_0 (var)) == NULL)
	      YYABORT
	  }
	| NUMBER
	  {
	    if (($$ = new_exp_0 (num)) == NULL)
	      YYABORT;
	    $$->val.num = $1
	  }
	| '(' exp ')'
	  {
	    $$ = $2
	  }
	;

%%

static struct expression *
new_exp_0 (op)
     enum operator op;
{
  struct expression *newp = (struct expression *) malloc (sizeof (*newp));

  if (newp != NULL)
    newp->operation = op;

  return newp;
}

static struct expression *
new_exp_2 (op, left, right)
     enum operator op;
     struct expression *left;
     struct expression *right;
{
  struct expression *newp = NULL;

  if (left != NULL && right != NULL)
    newp = (struct expression *) malloc (sizeof (*newp));

  if (newp != NULL)
    {
      newp->operation = op;
      newp->val.args2.left = left;
      newp->val.args2.right = right;
    }
  else
    {
      __gettext_free_exp (left);
      __gettext_free_exp (right);
    }

  return newp;
}

static struct expression *
new_exp_3 (op, bexp, tbranch, fbranch)
     enum operator op;
     struct expression *bexp;
     struct expression *tbranch;
     struct expression *fbranch;
{
  struct expression *newp = NULL;

  if (bexp != NULL && tbranch != NULL && fbranch != NULL)
    newp = (struct expression *) malloc (sizeof (*newp));

  if (newp != NULL)
    {
      newp->operation = op;
      newp->val.args3.bexp = bexp;
      newp->val.args3.tbranch = tbranch;
      newp->val.args3.fbranch = fbranch;
    }
  else
    {
      __gettext_free_exp (bexp);
      __gettext_free_exp (tbranch);
      __gettext_free_exp (fbranch);
    }

  return newp;
}

void
internal_function
__gettext_free_exp (exp)
     struct expression *exp;
{
  if (exp == NULL)
    return;

  /* Handle the recursive case.  */
  switch (exp->operation)
    {
    case qmop:
      __gettext_free_exp (exp->val.args3.fbranch);
      /* FALLTHROUGH */

    case mult:
    case divide:
    case module:
    case plus:
    case minus:
    case equal:
    case not_equal:
    case land:
    case lor:
      __gettext_free_exp (exp->val.args2.right);
      __gettext_free_exp (exp->val.args2.left);
      break;

    default:
      break;
    }

  free (exp);
}


static int
yylex (lval, pexp)
     YYSTYPE *lval;
     const char **pexp;
{
  const char *exp = *pexp;
  int result;

  while (1)
    {
      if (exp[0] == '\\' && exp[1] == '\n')
	{
	  exp += 2;
	  continue;
	}

      if (exp[0] == '\0')
	{
	  *pexp = exp;
	  return YYEOF;
	}

      if (exp[0] != ' ' && exp[0] != '\t')
	break;

      ++exp;
    }

  result = *exp++;
  switch (result)
    {
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      {
	unsigned long int n = exp[-1] - '0';
	while (exp[0] >= '0' && exp[0] <= '9')
	  {
	    n *= 10;
	    n += exp[0] - '0';
	    ++exp;
	  }
	lval->num = n;
	result = NUMBER;
      }
      break;

    case '=':
    case '!':
      if (exp[0] == '=')
	++exp;
      else
	result = YYERRCODE;
      break;

    case '&':
    case '|':
      if (exp[0] == result)
	++exp;
      else
	result = YYERRCODE;
      break;

    case 'n':
    case '*':
    case '/':
    case '%':
    case '+':
    case '-':
    case '?':
    case ':':
    case '(':
    case ')':
      /* Nothing, just return the character.  */
      break;

    case ';':
    case '\n':
    case '\0':
      /* Be safe and let the user call this function again.  */
      --exp;
      result = YYEOF;
      break;

    default:
      result = YYERRCODE;
#if YYDEBUG != 0
      --exp;
#endif
      break;
    }

  *pexp = exp;

  return result;
}


static void
yyerror (str)
     const char *str;
{
  /* Do nothing.  We don't print error messages here.  */
}
