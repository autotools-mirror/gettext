typedef union
{
  char *string;
  long number;
  lex_pos_ty pos;
} YYSTYPE;
#define	COMMENT	257
#define	DOMAIN	258
#define	JUNK	259
#define	MSGID	260
#define	MSGSTR	261
#define	NAME	262
#define	NUMBER	263
#define	STRING	264


extern YYSTYPE yylval;
