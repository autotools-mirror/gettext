/* xgettext Java backend.					-*- C -*-
   Copyright (C) 2001 Free Software Foundation, Inc.
   Written by Tommy Johansson <tommy.johansson@kanalen.org>, 2001.

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

%{
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "message.h"
#include "x-java.h"
#include "xgettext.h"
#include "system.h"

typedef enum
{
  JAVA_WORD,
  JAVA_STRING,
  JAVA_OPERATOR,
  JAVA_FLOW,
  JAVA_COMMENT,
} TOKEN_TYPE;

typedef struct
{
  char *word;
  char *string;
  char *operator;
  char *flow;
  char *comment;

  int line_no;
} PARSER_GLOBAL;

static PARSER_GLOBAL pg;
static PARSER_GLOBAL *parser_global = &pg;

typedef enum
{
  STATE_NONE,
  STATE_STRING,
  STATE_WORD,
  STATE_APPEND,
  STATE_INVOCATION,
  STATE_KEYWORD,
} PARSER_STATE;

typedef struct
{
  char *data;
  int len;
  int maxlen;
} char_buf;


/* Prototypes for local functions.  Needed to ensure compiler checking of
   function argument counts despite of K&R C function definition syntax.  */
static char_buf *create_char_buf PARAMS ((void));
static void append_char_buf PARAMS ((char_buf *b, int c));
static char *get_string PARAMS ((char_buf *b));
static void destroy_charbuf PARAMS ((char_buf *b));
static void update_line_no PARAMS ((int c));
static char *append_strings PARAMS ((char *a, char *b));
static inline bool isplus PARAMS ((char *s));
static inline bool isdot PARAMS ((char *s));
static char *translate_esc PARAMS ((char *s));
static bool do_compare PARAMS ((const char *s1, const char *s2));
static bool is_keyword PARAMS ((const char *s));
static void free_global PARAMS ((void));


#define INITIAL_CHARBUF_SIZE 500
#define CHARBUF_GROWTH 100
static char_buf *
create_char_buf ()
{
  char_buf *b = (char_buf *) xmalloc (sizeof (char_buf));
  b->data = (char *) xmalloc (INITIAL_CHARBUF_SIZE);
  b->len = 0;
  b->maxlen = INITIAL_CHARBUF_SIZE;
  return b;
}

static void
append_char_buf (b, c)
     char_buf *b;
     int c;
{
  if (b->len >= b->maxlen - 1)
    {
      b->data = (char *) xrealloc (b->data, b->maxlen + CHARBUF_GROWTH);
      b->maxlen += CHARBUF_GROWTH;
    }
  b->data[b->len++] = c;
  b->data[b->len] = '\0';
}

static char *
get_string (b)
     char_buf *b;
{
  return xstrdup (b->data);
}

static void
destroy_charbuf (b)
     char_buf *b;
{
  free (b->data);
  free (b);
}

static void
update_line_no (c)
     int c;
{
  if (c == '\n')
    parser_global->line_no++;
}

%}

%option noyywrap

NUM [0-9]
ID [a-zA-Z_][a-zA-Z0-9_]*

%%

"/*" {
  int c;
  int last;
  char *str;

  char_buf *charbuf = create_char_buf ();
  while (1)
    {
      c = input ();
      last = input ();
      update_line_no (c);
      if ((c == '*' && last == '/') || c == EOF)
	break;
      unput (last);
      append_char_buf (charbuf, c);
    }
  str = get_string (charbuf);
  destroy_charbuf (charbuf);
  parser_global->comment = str;
  return JAVA_COMMENT;
}

{NUM}| {NUM}+"."{NUM}*
\" {
  int c;
  char *str;
  char_buf *charbuf = create_char_buf ();
  while ((c = input ()) != '"')
    {
      update_line_no (c);
      append_char_buf (charbuf, c);
    }
  str = get_string (charbuf);
  destroy_charbuf (charbuf);
  parser_global->string = str;
  return JAVA_STRING;
}

{ID} {
  parser_global->word = yytext;
  return JAVA_WORD;
}

"."|"("|")"|";"|"{"|"}"|"["|"]"|","|":"|"\\"|"?"|"\'" {
  parser_global->flow = yytext;
  return JAVA_FLOW;
}

"="|"<"|">"|"+"|"-"|"*"|"/"|"!"|"&"|"|"|"%"|"^"|"~" {
  parser_global->operator = yytext;
  return JAVA_OPERATOR;
}

"#"|"@"|"\r"|"`" /* ignore whitespace */

"//"[^\n]* {
  parser_global->comment = xstrdup (yytext);
  return JAVA_COMMENT;
}
"\n"|"\r"|"\r\n" parser_global->line_no++;
[ \t]+
.
<<EOF>> return -1;
%%

static char *
append_strings (a, b)
     char *a;
     char *b;
{
  int total_size = strlen (a) + strlen (b) + 1;
  char *new_string = (char *) xmalloc (total_size);
  strcpy (new_string, a);
  strcat (new_string, b);
  return new_string;
}

static inline bool
isplus (s)
     char *s;
{
  return *s == '+';
}

static inline bool
isdot (s)
     char *s;
{
  return *s == '.';
}


static char *
translate_esc (s)
     char *s;
{
  char *n = (char *) xmalloc (strlen (s) + 1);
  int i;
  int j = 0;

  for (i = 0; i < strlen (s); i++)
    switch (s[i])
      {
      case '\\':
	if (s[i + 1] == 'n')
	  {
	    n[j++] = '\n';
	    i++;
	  }
	break;
      default:
	n[j++] = s[i];
      }
  n[j] = '\0';
  return n;
}

static string_list_ty *java_keywords = NULL;

/**
 * Try to match a string against the keyword. If substring_match is
 * true substring match is used.
 */
static bool
do_compare (s1, s2)
     const char *s1;
     const char *s2;
{
  if (substring_match)
    return strstr (s1, s2) != NULL;
  else
    return strcmp (s1, s2) == 0;
}

/**
 * Check if a string is a keyword or not.
 */
static bool
is_keyword (s)
     const char *s;
{
  int i;

  for (i = 0; i < java_keywords->nitems; i++)
    if (do_compare (java_keywords->item[i], s))
      return true;
  return false;
}

/**
 * Add a keyword to the list of possible keywords.
 */
void
x_java_keyword (keyword)
     const char *keyword;
{
  if (java_keywords == NULL)
    java_keywords = string_list_alloc ();

  string_list_append (java_keywords, keyword);
}


/**
 * Free any memory allocated by the tokenizer.
 */
static void
free_global ()
{
  /**
   * free memory used by strings and comments as they are strdup'ed
   * by the lexer.
   */
  if (parser_global->string != NULL)
    {
      free (parser_global->string);
      parser_global->string = NULL;
    }
  if (parser_global->comment != NULL)
    {
      free (parser_global->comment);
      parser_global->comment = NULL;
    }
}


/**
 * Main java keyword extract function.
 */
void
extract_java (f, real_filename, logical_filename, mdlp)
     FILE *f;
     const char *real_filename;
     const char *logical_filename;
     msgdomain_list_ty *mdlp;
{
  char *logical_file_name = xstrdup (logical_filename);
  int token;
  PARSER_STATE state = STATE_NONE;
  PARSER_STATE last_state = STATE_NONE;
  char *str;
  char *key;
  message_list_ty *mlp = mdlp->item[0]->messages;

  if (java_keywords == NULL)
    {
      /* ops, no standard keywords */
      x_java_keyword ("gettext");	/* GettextResource.gettext */
      x_java_keyword ("ngettext");	/* GettextResource.ngettext */
      x_java_keyword ("getString");	/* ResourceBundle.getString */
    }

  memset (parser_global, 0, sizeof (*parser_global));
  /* first line is 1 */
  parser_global->line_no = 1;

  yyin = f;
  do
    {
      token = yylex ();
      switch (token)
	{

	case JAVA_WORD:
	  if (state == STATE_INVOCATION)
	    {
	      char *k2;
	      k2 = append_strings (key, ".");
	      free (key);
	      key = append_strings (k2, parser_global->word);
	      state = STATE_NONE;
	    }
	  else
	    {
	      state = STATE_WORD;
	      key = xstrdup (parser_global->word);
	    }
	  /* For java we try to match both things like object.methodCall()
	     and methodCall(). */
	  if (is_keyword (key) || is_keyword (parser_global->word))
	    {
	      free (key);
	      state = STATE_KEYWORD;
	    }
	  break;

	case JAVA_STRING:
	  if (state == STATE_KEYWORD)
	    {
	      last_state = STATE_KEYWORD;
	    }
	  if (state == STATE_APPEND)
	    {
	      char *s2;
	      s2 = append_strings (str, translate_esc (parser_global->string));
	      free (str);
	      str = s2;
	      state = STATE_STRING;
	    }
	  else
	    {
	      state = STATE_STRING;
	      str = translate_esc (parser_global->string);
	    }
	  break;

	case JAVA_OPERATOR:
	  if (state == STATE_STRING && isplus (parser_global->operator))
	    {
	      state = STATE_APPEND;
	    }
	  else
	    {
	      state = STATE_NONE;
	    }
	  break;

	case JAVA_FLOW:
	  /* Did we get something? */
	  if (state == STATE_STRING && last_state == STATE_KEYWORD)
	    {
	      lex_pos_ty pos;
	      pos.file_name = logical_file_name;
	      pos.line_number = parser_global->line_no;
	      state = STATE_NONE;
	      last_state = STATE_NONE;

	      remember_a_message (mlp, str, &pos);
	    }

	  if (state == STATE_WORD && isdot (parser_global->flow))
	    {
	      state = STATE_INVOCATION;
	    }

	  break;

	case JAVA_COMMENT:
	  state = STATE_NONE;
	  xgettext_comment_add (parser_global->comment);
	  break;

	default:
	  state = STATE_NONE;
	}
      free_global ();
    }
  while (token != -1);
}
