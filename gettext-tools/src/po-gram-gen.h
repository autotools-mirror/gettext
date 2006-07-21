#ifndef BISON_PO_GRAM_GEN_H
# define BISON_PO_GRAM_GEN_H

#ifndef YYSTYPE
typedef union
{
  struct { char *string; lex_pos_ty pos; bool obsolete; } string;
  struct { string_list_ty stringlist; lex_pos_ty pos; bool obsolete; } stringlist;
  struct { long number; lex_pos_ty pos; bool obsolete; } number;
  struct { lex_pos_ty pos; bool obsolete; } pos;
  struct { struct msgstr_def rhs; lex_pos_ty pos; bool obsolete; } rhs;
} yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif
# define	COMMENT	257
# define	DOMAIN	258
# define	JUNK	259
# define	MSGCTXT	260
# define	MSGID	261
# define	MSGID_PLURAL	262
# define	MSGSTR	263
# define	NAME	264
# define	NUMBER	265
# define	STRING	266


extern YYSTYPE yylval;

#endif /* not BISON_PO_GRAM_GEN_H */
