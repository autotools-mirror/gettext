typedef union
{
  char *string;
  long number;
  lex_pos_ty pos;
  struct msgstr_def rhs;
} YYSTYPE;
#define	COMMENT	257
#define	DOMAIN	258
#define	JUNK	259
#define	MSGID	260
#define	MSGID_PLURAL	261
#define	MSGSTR	262
#define	NAME	263
#define	NUMBER	264
#define	STRING	265


extern YYSTYPE yylval;
