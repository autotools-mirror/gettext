/* xgettext Java backend.					-*- C -*-
   Copyright (C) 2001-2002 Free Software Foundation, Inc.
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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "message.h"
#include "x-java.h"
#include "xgettext.h"
#include "xmalloc.h"
#include "strstr.h"

typedef enum
{
  JAVA_WORD,
  JAVA_STRING,
  JAVA_OPERATOR,
  JAVA_FLOW,
  JAVA_COMMENT
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
  STATE_KEYWORD
} PARSER_STATE;

typedef struct
{
  char *data;
  int len;
  int maxlen;
} char_buf;


typedef struct _object_list
{
  int num_obj;
  int max_num_obj;
  void **objects;
} object_list;

#define INITIAL_OBJECT_LIST_SIZE 10
#define OBJECT_LIST_GROWTH 10

typedef struct _java_keyword
{
  char *keyword;
  int msgid_arg;
  int msgid_plural_arg;
} java_keyword;


/* Prototypes for local functions.  Needed to ensure compiler checking of
   function argument counts despite of K&R C function definition syntax.  */
static char_buf *create_char_buf PARAMS ((void));
static void append_char_buf PARAMS ((char_buf *b, int c));
static char *get_string PARAMS ((char_buf *b));
static void destroy_charbuf PARAMS ((char_buf *b));
static void update_line_no PARAMS ((int c));
static void strip_ending_spaces PARAMS ((char *str));
static char *append_strings PARAMS ((char *a, char *b));
static inline bool isplus PARAMS ((char *s));
static inline bool isdot PARAMS ((char *s));
static char *translate_esc PARAMS ((char *s));
static object_list * object_list_alloc PARAMS ((void));
static void object_list_destroy PARAMS ((object_list *list));
static int get_num_objects PARAMS ((const object_list *list));
static void * get_object PARAMS ((const object_list *list, int i));
static void add_object PARAMS ((object_list *list, void *object));
static java_keyword * alloc_keyword PARAMS ((const char *keyword,
					     int arg1, int arg2));
static bool tailcmp PARAMS ((const char *s1, const char *s2));
static bool do_compare PARAMS ((const char *s1, const char *s2));
static java_keyword *is_keyword PARAMS ((const char *s));
static void free_global PARAMS ((void));


#define INITIAL_CHARBUF_SIZE 500
#define CHARBUF_GROWTH 100
static char_buf *
create_char_buf ()
{
  char_buf *b = (char_buf *) xmalloc (sizeof (char_buf));
  b->data = (char *) xmalloc (INITIAL_CHARBUF_SIZE);
  b->data[0] = '\0';
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

static void
strip_ending_spaces (str)
     char *str;
{
  int len = strlen (str);

  while (len > 0 && isspace ((unsigned char) str[len - 1]))
    len--;
  str[len] = '\0';
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
  for (;;)
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
  strip_ending_spaces (str);
  parser_global->comment = str;
  return JAVA_COMMENT;
}

{NUM}| {NUM}+"."{NUM}*
\" {
  int c;
  char *str;
  char_buf *charbuf = create_char_buf ();
  while ((c = input ()) != EOF && c != '"')
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
  parser_global->comment = xstrdup (yytext + 2);
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
  size_t i;
  size_t j = 0;

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

static object_list *
object_list_alloc ()
{
  object_list *list = xmalloc (sizeof (object_list));
  list->max_num_obj = INITIAL_OBJECT_LIST_SIZE;
  list->num_obj = 0;
  list->objects = xmalloc (sizeof (void *) * INITIAL_OBJECT_LIST_SIZE);
  return list;
}

static void
object_list_destroy (list)
     object_list *list;
{
  free (list->objects);
  free (list);
}

static int
get_num_objects (list)
     const object_list *list;
{
  return list->num_obj;
}

static void *
get_object (list, i)
     const object_list *list;
     int i;
{
  return list->objects[i];
}

static void
add_object (list, object)
     object_list *list;
     void *object;
{
  if (list->num_obj + 1 >= list->max_num_obj)
    {
      list->max_num_obj += OBJECT_LIST_GROWTH;
      list->objects =
	xrealloc (list->objects, list->max_num_obj * sizeof (void *));
    }
  list->objects[list->num_obj ++] = object;
}


/* options */
static bool extract_all_strings = false;

void
x_java_extract_all ()
{
  extract_all_strings = true;
}


static java_keyword *
alloc_keyword (keyword, arg1, arg2)
     const char *keyword;
     int arg1;
     int arg2;
{
  java_keyword *jk = xmalloc (sizeof (java_keyword));
  jk->keyword = xstrdup (keyword);
  jk->msgid_arg = arg1;
  jk->msgid_plural_arg = arg2;
  return jk;
}

static object_list *java_keywords = NULL;


/**
 * Backwards substring match.
 */
static bool
tailcmp (s1, s2)
     const char *s1;
     const char *s2;
{
  int len1 = strlen (s1);
  int len2 = strlen (s2);
  int start = len1 - len2;
  if (start < 0)
    return false;
  return (start == 0 || s1[start-1] == '.') && (strcmp (s1 + start, s2) == 0);
}

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
    return tailcmp (s1, s2);
}

/**
 * Check if a string is a keyword or not.
 */
static java_keyword *
is_keyword (s)
     const char *s;
{
  int i;
  int num_keywords = get_num_objects (java_keywords);
  java_keyword *kw;

  for (i = 0; i < num_keywords; i++)
    {
      kw = (java_keyword *) get_object (java_keywords, i);

      if (do_compare (s, kw->keyword))
	return kw;
    }
  return NULL;
}

/**
 * Add a keyword to the list of possible keywords.
 */
void
x_java_keyword (keyword)
     const char *keyword;
{
  const char *keyword_end;
  int arg1;
  int arg2;
  size_t len;
  char *kw;

  if (keyword == NULL)
    {
      if (java_keywords != NULL)
	{
	  object_list_destroy (java_keywords);
	  java_keywords = NULL;
	}
      return;
    }

  if (java_keywords == NULL)
    java_keywords = object_list_alloc ();

  split_keywordspec (keyword, &keyword_end, &arg1, &arg2);
  len = keyword_end - keyword;
  kw = (char *) xmalloc (len + 1);
  memcpy (kw, keyword, len);
  kw[len] = '\0';

  /* kw should be a valid Java identifier sequence with dots.
     A colon means an invalid parse in split_keywordspec().  */
  if (strchr (kw, ':') == NULL)
    {
      if (arg1 == 0)
	arg1 = 1;
      add_object (java_keywords, alloc_keyword (kw, arg1, arg2));
    }
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
  char *str = NULL;		/* used only if state == STATE_STRING
						|| state == STATE_APPEND */
  char *key = NULL;		/* used only if state == STATE_WORD
						|| state == STATE_INVOCATION */
  message_ty *plural = NULL;	/* used only after state was STATE_KEYWORD */
  message_list_ty *mlp = mdlp->item[0]->messages;
  java_keyword *current_keyword = NULL;
  java_keyword *keyword;
  int argument_counter = 0;

  if (java_keywords == NULL)
    {
      /* ops, no standard keywords */
      x_java_keyword ("GettextResource.gettext:2");	/* static method */
      x_java_keyword ("GettextResource.ngettext:2,3");	/* static method */
      x_java_keyword ("gettext");
      x_java_keyword ("ngettext:1,2");
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
	  if (state == STATE_KEYWORD)
	    {
	      last_state = STATE_KEYWORD;
	      argument_counter ++;
	    }
	  if (state == STATE_INVOCATION)
	    {
	      char *k2;
	      k2 = append_strings (key, ".");
	      free (key);
	      key = append_strings (k2, parser_global->word);
	      free (k2);
	    }
	  else
	    {
	      if (str != NULL)
		{
		  free (str);
		  str = NULL;
		}
	      state = STATE_WORD;
	      key = xstrdup (parser_global->word);
	    }
	  /* For java we try to match both things like object.methodCall()
	     and methodCall(). */
	  if ((keyword = is_keyword (key)) != NULL
	      || (keyword = is_keyword (parser_global->word)) != NULL)
	    {
	      current_keyword = keyword;
	      free (key);
	      state = STATE_KEYWORD;
	      argument_counter = 1;
	      plural = NULL;
	    }
	  break;

	case JAVA_STRING:
	  if (state == STATE_KEYWORD)
	    last_state = STATE_KEYWORD;
	  if (state == STATE_APPEND)
	    {
	      char *s2;
	      s2 = append_strings (str, translate_esc (parser_global->string));
	      free (str);
	      str = s2;
	    }
	  else
	    str = translate_esc (parser_global->string);
	  state = STATE_STRING;
	  break;

	case JAVA_OPERATOR:
	  if (state == STATE_STRING && isplus (parser_global->operator))
	    state = STATE_APPEND;
	  else
	    {
	      if (str != NULL)
		{
		  free (str);
		  str = NULL;
		}
	      state = STATE_NONE;
	    }
	  break;

	case JAVA_FLOW:
	  /* Did we get something? */
	  if (state == STATE_STRING
	      && (last_state == STATE_KEYWORD || extract_all_strings))
	    {
	      lex_pos_ty pos;
	      pos.file_name = logical_file_name;
	      pos.line_number = parser_global->line_no;
	      if (extract_all_strings)
		{
		  remember_a_message (mlp, str, &pos);
		}
	      else if (argument_counter == current_keyword->msgid_arg)
		{
		  plural = remember_a_message (mlp, str, &pos);
		  if (current_keyword->msgid_plural_arg == 0)
		    {
		      /**
		       * we don't expect any plural arg, reset state
		       */
		      state = STATE_NONE;
		      last_state = STATE_NONE;
		      argument_counter = 0;
		    }
		  else
		    {
		      argument_counter ++;
		    }

		}
	      else if (argument_counter == current_keyword->msgid_plural_arg
		       && str != NULL)
		{
		  remember_a_message_plural (plural, str, &pos);
		  state = STATE_NONE;
		  last_state = STATE_NONE;
		  argument_counter = 0;
		}
	      else
		{
		  if (str != NULL)
		    free (str);
		}
	      str = NULL;
	    }

	  if (extract_all_strings)
	    {
	      if (str != NULL)
		{
		  free (str);
		  str = NULL;
		}
	      state = STATE_NONE;
	      last_state = STATE_NONE;
	    }

	  if (state == STATE_WORD && isdot (parser_global->flow))
	    {
	      state = STATE_INVOCATION;
	    }

	  break;

	case JAVA_COMMENT:
	  if (str != NULL)
	    {
	      free (str);
	      str = NULL;
	    }
	  state = STATE_NONE;
	  last_state = STATE_NONE;
	  xgettext_comment_add (parser_global->comment);
	  break;

	default:
	  if (str != NULL)
	    {
	      free (str);
	      str = NULL;
	    }
	  state = STATE_NONE;
	}
      free_global ();
    }
  while (token != -1);

  if (str != NULL)
    free (str);
}
