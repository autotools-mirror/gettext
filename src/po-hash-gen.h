typedef union
{
  char *string;
  size_t number;
} YYSTYPE;
#define	STRING	257
#define	NUMBER	258
#define	COLON	259
#define	COMMA	260
#define	FILE_KEYWORD	261
#define	LINE_KEYWORD	262
#define	NUMBER_KEYWORD	263


extern YYSTYPE yylval;
