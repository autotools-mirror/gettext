
/*  A Bison parser, made from po-gram-gen.y
    by GNU Bison version 1.28  */

#define YYBISON 1  /* Identify Bison output.  */

#define	COMMENT	257
#define	DOMAIN	258
#define	JUNK	259
#define	MSGID	260
#define	MSGID_PLURAL	261
#define	MSGSTR	262
#define	NAME	263
#define	NUMBER	264
#define	STRING	265

#line 20 "po-gram-gen.y"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/* Specification.  */
#include "po-gram.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "str-list.h"
#include "po-lex.h"
#include "error.h"
#include "xmalloc.h"
#include "gettext.h"
#include "po.h"

#define _(str) gettext (str)

/* Remap normal yacc parser interface names (yyparse, yylex, yyerror, etc),
   as well as gratuitiously global symbol names, so we can have multiple
   yacc generated parsers in the same program.  Note that these are only
   the variables produced by yacc.  If other parser generators (bison,
   byacc, etc) produce additional global names that conflict at link time,
   then those parser generators need to be fixed instead of adding those
   names to this list. */

#define yymaxdepth po_gram_maxdepth
#define yyparse po_gram_parse
#define yylex   po_gram_lex
#define yyerror po_gram_error
#define yylval  po_gram_lval
#define yychar  po_gram_char
#define yydebug po_gram_debug
#define yypact  po_gram_pact
#define yyr1    po_gram_r1
#define yyr2    po_gram_r2
#define yydef   po_gram_def
#define yychk   po_gram_chk
#define yypgo   po_gram_pgo
#define yyact   po_gram_act
#define yyexca  po_gram_exca
#define yyerrflag po_gram_errflag
#define yynerrs po_gram_nerrs
#define yyps    po_gram_ps
#define yypv    po_gram_pv
#define yys     po_gram_s
#define yy_yys  po_gram_yys
#define yystate po_gram_state
#define yytmp   po_gram_tmp
#define yyv     po_gram_v
#define yy_yyv  po_gram_yyv
#define yyval   po_gram_val
#define yylloc  po_gram_lloc
#define yyreds  po_gram_reds          /* With YYDEBUG defined */
#define yytoks  po_gram_toks          /* With YYDEBUG defined */
#define yylhs   po_gram_yylhs
#define yylen   po_gram_yylen
#define yydefred po_gram_yydefred
#define yydgoto po_gram_yydgoto
#define yysindex po_gram_yysindex
#define yyrindex po_gram_yyrindex
#define yygindex po_gram_yygindex
#define yytable  po_gram_yytable
#define yycheck  po_gram_yycheck

static long plural_counter;

#define check_obsolete(value1,value2) \
  if ((value1).obsolete != (value2).obsolete) \
    po_gram_error_at_line (&(value2).pos, _("inconsistent use of #~"));


#line 107 "po-gram-gen.y"
typedef union
{
  struct { char *string; lex_pos_ty pos; bool obsolete; } string;
  struct { string_list_ty stringlist; lex_pos_ty pos; bool obsolete; } stringlist;
  struct { long number; lex_pos_ty pos; bool obsolete; } number;
  struct { lex_pos_ty pos; bool obsolete; } pos;
  struct { struct msgstr_def rhs; lex_pos_ty pos; bool obsolete; } rhs;
} YYSTYPE;
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		28
#define	YYFLAG		-32768
#define	YYNTBASE	14

#define YYTRANSLATE(x) ((unsigned)(x) <= 265 ? yytranslate[x] : 22)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    10,     2,    11,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     3,     4,     5,     6,
     7,     8,     9,    12,    13
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     1,     4,     7,    10,    13,    16,    21,    26,    30,
    34,    37,    40,    42,    45,    51,    53,    56
};

static const short yyrhs[] = {    -1,
    14,    21,     0,    14,    15,     0,    14,    16,     0,    14,
     1,     0,     4,    13,     0,     6,    20,     8,    20,     0,
     6,    20,    17,    18,     0,     6,    20,    17,     0,     6,
    20,    18,     0,     6,    20,     0,     7,    20,     0,    19,
     0,    18,    19,     0,     8,    10,    12,    11,    20,     0,
    13,     0,    20,    13,     0,     3,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   127,   128,   129,   130,   131,   135,   142,   160,   178,   186,
   194,   203,   214,   218,   233,   255,   262,   273
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","COMMENT",
"DOMAIN","JUNK","MSGID","MSGID_PLURAL","MSGSTR","NAME","'['","']'","NUMBER",
"STRING","msgfmt","domain","message","msgid_pluralform","pluralform_list","pluralform",
"string_list","comment", NULL
};
#endif

static const short yyr1[] = {     0,
    14,    14,    14,    14,    14,    15,    16,    16,    16,    16,
    16,    17,    18,    18,    19,    20,    20,    21
};

static const short yyr2[] = {     0,
     0,     2,     2,     2,     2,     2,     4,     4,     3,     3,
     2,     2,     1,     2,     5,     1,     2,     1
};

static const short yydefact[] = {     1,
     0,     5,    18,     0,     0,     3,     4,     2,     6,    16,
    11,     0,     0,    17,     9,    10,    13,    12,     0,     7,
     0,     8,    14,     0,     0,    15,     0,     0
};

static const short yydefgoto[] = {     1,
     6,     7,    15,    16,    17,    11,     8
};

static const short yypact[] = {-32768,
     2,-32768,-32768,    -1,     1,-32768,-32768,-32768,-32768,-32768,
     3,     1,    -6,-32768,     9,     9,-32768,     5,     7,     5,
    10,     9,-32768,    11,     1,     5,    21,-32768
};

static const short yypgoto[] = {-32768,
-32768,-32768,-32768,     8,    -7,   -12,-32768
};


#define	YYLAST		23


static const short yytable[] = {    18,
    20,    27,     2,    19,     3,     4,    10,     5,    23,    12,
    13,     9,    26,    10,    23,    14,    21,    14,    24,    19,
    28,    25,    22
};

static const short yycheck[] = {    12,
    13,     0,     1,    10,     3,     4,    13,     6,    16,     7,
     8,    13,    25,    13,    22,    13,     8,    13,    12,    10,
     0,    11,    15
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/local/share/bison.simple"
/* This file comes from bison-1.28.  */

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

#ifndef YYSTACK_USE_ALLOCA
#ifdef alloca
#define YYSTACK_USE_ALLOCA
#else /* alloca not defined */
#ifdef __GNUC__
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi) || (defined (__sun) && defined (__i386))
#define YYSTACK_USE_ALLOCA
#include <alloca.h>
#else /* not sparc */
/* We think this test detects Watcom and Microsoft C.  */
/* This used to test MSDOS, but that is a bad idea
   since that symbol is in the user namespace.  */
#if (defined (_MSDOS) || defined (_MSDOS_)) && !defined (__TURBOC__)
#if 0 /* No need for malloc.h, which pollutes the namespace;
	 instead, just don't use alloca.  */
#include <malloc.h>
#endif
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
/* I don't know what this was needed for, but it pollutes the namespace.
   So I turned it off.   rms, 2 May 1997.  */
/* #include <malloc.h>  */
 #pragma alloca
#define YYSTACK_USE_ALLOCA
#else /* not MSDOS, or __TURBOC__, or _AIX */
#if 0
#ifdef __hpux /* haible@ilog.fr says this works for HPUX 9.05 and up,
		 and on HPUX 10.  Eventually we can turn this on.  */
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#endif /* __hpux */
#endif
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc */
#endif /* not GNU C */
#endif /* alloca not defined */
#endif /* YYSTACK_USE_ALLOCA not defined */

#ifdef YYSTACK_USE_ALLOCA
#define YYSTACK_ALLOC alloca
#else
#define YYSTACK_ALLOC malloc
#endif

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Define __yy_memcpy.  Note that the size argument
   should be passed with type unsigned int, because that is what the non-GCC
   definitions require.  With GCC, __builtin_memcpy takes an arg
   of type size_t, but it can handle unsigned int.  */

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(TO,FROM,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (to, from, count)
     char *to;
     char *from;
     unsigned int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *to, char *from, unsigned int count)
{
  register char *t = to;
  register char *f = from;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 217 "/usr/local/share/bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#ifdef __cplusplus
#define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else /* not __cplusplus */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#endif /* not __cplusplus */
#else /* not YYPARSE_PARAM */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif /* not YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
#ifdef YYPARSE_PARAM
int yyparse (void *);
#else
int yyparse (void);
#endif
#endif

int
yyparse(YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;
  int yyfree_stacks = 0;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  if (yyfree_stacks)
	    {
	      free (yyss);
	      free (yyvs);
#ifdef YYLSP_NEEDED
	      free (yyls);
#endif
	    }
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
#ifndef YYSTACK_USE_ALLOCA
      yyfree_stacks = 1;
#endif
      yyss = (short *) YYSTACK_ALLOC (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1,
		   size * (unsigned int) sizeof (*yyssp));
      yyvs = (YYSTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1,
		   size * (unsigned int) sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1,
		   size * (unsigned int) sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 6:
#line 136 "po-gram-gen.y"
{
		   po_callback_domain (yyvsp[0].string.string);
		;
    break;}
case 7:
#line 143 "po-gram-gen.y"
{
		  char *string2 = string_list_concat_destroy (&yyvsp[-2].stringlist.stringlist);
		  char *string4 = string_list_concat_destroy (&yyvsp[0].stringlist.stringlist);

		  check_obsolete (yyvsp[-3].pos, yyvsp[-2].stringlist);
		  check_obsolete (yyvsp[-3].pos, yyvsp[-1].pos);
		  check_obsolete (yyvsp[-3].pos, yyvsp[0].stringlist);
		  if (!yyvsp[-3].pos.obsolete || pass_obsolete_entries)
		    po_callback_message (string2, &yyvsp[-3].pos.pos, NULL,
					 string4, strlen (string4) + 1, &yyvsp[-1].pos.pos,
					 yyvsp[-3].pos.obsolete);
		  else
		    {
		      free (string2);
		      free (string4);
		    }
		;
    break;}
case 8:
#line 161 "po-gram-gen.y"
{
		  char *string2 = string_list_concat_destroy (&yyvsp[-2].stringlist.stringlist);

		  check_obsolete (yyvsp[-3].pos, yyvsp[-2].stringlist);
		  check_obsolete (yyvsp[-3].pos, yyvsp[-1].string);
		  check_obsolete (yyvsp[-3].pos, yyvsp[0].rhs);
		  if (!yyvsp[-3].pos.obsolete || pass_obsolete_entries)
		    po_callback_message (string2, &yyvsp[-3].pos.pos, yyvsp[-1].string.string,
					 yyvsp[0].rhs.rhs.msgstr, yyvsp[0].rhs.rhs.msgstr_len, &yyvsp[0].rhs.pos,
					 yyvsp[-3].pos.obsolete);
		  else
		    {
		      free (string2);
		      free (yyvsp[-1].string.string);
		      free (yyvsp[0].rhs.rhs.msgstr);
		    }
		;
    break;}
case 9:
#line 179 "po-gram-gen.y"
{
		  check_obsolete (yyvsp[-2].pos, yyvsp[-1].stringlist);
		  check_obsolete (yyvsp[-2].pos, yyvsp[0].string);
		  po_gram_error_at_line (&yyvsp[-2].pos.pos, _("missing `msgstr[]' section"));
		  string_list_destroy (&yyvsp[-1].stringlist.stringlist);
		  free (yyvsp[0].string.string);
		;
    break;}
case 10:
#line 187 "po-gram-gen.y"
{
		  check_obsolete (yyvsp[-2].pos, yyvsp[-1].stringlist);
		  check_obsolete (yyvsp[-2].pos, yyvsp[0].rhs);
		  po_gram_error_at_line (&yyvsp[-2].pos.pos, _("missing `msgid_plural' section"));
		  string_list_destroy (&yyvsp[-1].stringlist.stringlist);
		  free (yyvsp[0].rhs.rhs.msgstr);
		;
    break;}
case 11:
#line 195 "po-gram-gen.y"
{
		  check_obsolete (yyvsp[-1].pos, yyvsp[0].stringlist);
		  po_gram_error_at_line (&yyvsp[-1].pos.pos, _("missing `msgstr' section"));
		  string_list_destroy (&yyvsp[0].stringlist.stringlist);
		;
    break;}
case 12:
#line 204 "po-gram-gen.y"
{
		  check_obsolete (yyvsp[-1].pos, yyvsp[0].stringlist);
		  plural_counter = 0;
		  yyval.string.string = string_list_concat_destroy (&yyvsp[0].stringlist.stringlist);
		  yyval.string.pos = yyvsp[-1].pos.pos;
		  yyval.string.obsolete = yyvsp[-1].pos.obsolete;
		;
    break;}
case 13:
#line 215 "po-gram-gen.y"
{
		  yyval.rhs = yyvsp[0].rhs;
		;
    break;}
case 14:
#line 219 "po-gram-gen.y"
{
		  check_obsolete (yyvsp[-1].rhs, yyvsp[0].rhs);
		  yyval.rhs.rhs.msgstr = (char *) xmalloc (yyvsp[-1].rhs.rhs.msgstr_len + yyvsp[0].rhs.rhs.msgstr_len);
		  memcpy (yyval.rhs.rhs.msgstr, yyvsp[-1].rhs.rhs.msgstr, yyvsp[-1].rhs.rhs.msgstr_len);
		  memcpy (yyval.rhs.rhs.msgstr + yyvsp[-1].rhs.rhs.msgstr_len, yyvsp[0].rhs.rhs.msgstr, yyvsp[0].rhs.rhs.msgstr_len);
		  yyval.rhs.rhs.msgstr_len = yyvsp[-1].rhs.rhs.msgstr_len + yyvsp[0].rhs.rhs.msgstr_len;
		  free (yyvsp[-1].rhs.rhs.msgstr);
		  free (yyvsp[0].rhs.rhs.msgstr);
		  yyval.rhs.pos = yyvsp[-1].rhs.pos;
		  yyval.rhs.obsolete = yyvsp[-1].rhs.obsolete;
		;
    break;}
case 15:
#line 234 "po-gram-gen.y"
{
		  check_obsolete (yyvsp[-4].pos, yyvsp[-3].pos);
		  check_obsolete (yyvsp[-4].pos, yyvsp[-2].number);
		  check_obsolete (yyvsp[-4].pos, yyvsp[-1].pos);
		  check_obsolete (yyvsp[-4].pos, yyvsp[0].stringlist);
		  if (yyvsp[-2].number.number != plural_counter)
		    {
		      if (plural_counter == 0)
			po_gram_error_at_line (&yyvsp[-4].pos.pos, _("first plural form has nonzero index"));
		      else
			po_gram_error_at_line (&yyvsp[-4].pos.pos, _("plural form has wrong index"));
		    }
		  plural_counter++;
		  yyval.rhs.rhs.msgstr = string_list_concat_destroy (&yyvsp[0].stringlist.stringlist);
		  yyval.rhs.rhs.msgstr_len = strlen (yyval.rhs.rhs.msgstr) + 1;
		  yyval.rhs.pos = yyvsp[-4].pos.pos;
		  yyval.rhs.obsolete = yyvsp[-4].pos.obsolete;
		;
    break;}
case 16:
#line 256 "po-gram-gen.y"
{
		  string_list_init (&yyval.stringlist.stringlist);
		  string_list_append (&yyval.stringlist.stringlist, yyvsp[0].string.string);
		  yyval.stringlist.pos = yyvsp[0].string.pos;
		  yyval.stringlist.obsolete = yyvsp[0].string.obsolete;
		;
    break;}
case 17:
#line 263 "po-gram-gen.y"
{
		  check_obsolete (yyvsp[-1].stringlist, yyvsp[0].string);
		  yyval.stringlist.stringlist = yyvsp[-1].stringlist.stringlist;
		  string_list_append (&yyval.stringlist.stringlist, yyvsp[0].string.string);
		  yyval.stringlist.pos = yyvsp[-1].stringlist.pos;
		  yyval.stringlist.obsolete = yyvsp[-1].stringlist.obsolete;
		;
    break;}
case 18:
#line 274 "po-gram-gen.y"
{
		  po_callback_comment (yyvsp[0].string.string);
		;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 543 "/usr/local/share/bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;

 yyacceptlab:
  /* YYACCEPT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 0;

 yyabortlab:
  /* YYABORT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 1;
}
#line 278 "po-gram-gen.y"
