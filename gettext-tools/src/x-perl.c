/* xgettext Perl backend.
   Copyright (C) 2002-2003 Free Software Foundation, Inc.

   This file was written by Guido Flohr <guido@imperia.net>, 2002-2003.

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
# include "config.h"
#endif

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "message.h"
#include "x-perl.h"
#include "xgettext.h"
#include "error.h"
#include "progname.h"
#include "xmalloc.h"
#include "exit.h"
#include "ucs4-utf8.h"
#include "uniname.h"
#include "gettext.h"

#define _(s) gettext(s)

/* The Perl syntax is defined in perlsyn.pod.  Try the command
   "perldoc perlsyn".  */

#define DEBUG_PERL 0
#define DEBUG_MEMORY 0

/* FIXME: All known Perl operators should be listed here.  It does not
   cost that much and it may improve the stability of the parser.  */
enum token_type_ty
{
  token_type_eof,
  token_type_lparen,		/* ( */
  token_type_rparen,		/* ) */
  token_type_comma,		/* , */
  token_type_fat_comma,		/* => */
  token_type_dereference,	/* , */
  token_type_semicolon,         /* ; */
  token_type_lbrace,            /* { */
  token_type_rbrace,            /* } */
  token_type_lbracket,          /* [ */
  token_type_rbracket,          /* ] */
  token_type_string,		/* quote-like */
  token_type_named_op,          /* if, unless, while, ... */
  token_type_variable,          /* $... */
  token_type_symbol,		/* symbol, number */
  token_type_regex_op,		/* s, tr, y, m.  */
  token_type_keyword_symbol,    /* keyword symbol (used by parser) */
  token_type_dot,               /* . */
  token_type_other		/* regexp, misc. operator */
};
typedef enum token_type_ty token_type_ty;

/* Subtypes for strings, important for interpolation.  */
enum string_type_ty
{
  string_type_verbatim,     /* "<<'EOF'", "m'...'", "s'...''...'",
			       "tr/.../.../", "y/.../.../".  */
  string_type_q,            /* "'..'", "q/.../".  */
  string_type_qq,           /* '"..."', "`...`", "qq/.../", "qx/.../",
			       "<file*glob>".  */
  string_type_qr,           /* Not supported.  */
};
typedef enum string_type_ty string_type_ty;

typedef struct token_ty token_ty;
struct token_ty
{
  token_type_ty type;
  string_type_ty string_type;
  char *string;		/* for token_type_{symbol,string} */
  int line_number;
};

#if DEBUG_PERL
static const char *
token2string (token_ty *token)
{
  switch (token->type)
    {
    case token_type_eof:
      return "token_type_eof";
    case token_type_lparen:
      return "token_type_lparen";
    case token_type_rparen:
      return "token_type_rparen";
    case token_type_comma:
      return "token_type_comma";
    case token_type_fat_comma:
      return "token_type_fat_comma";
    case token_type_dereference:
      return "token_type_dereference";
    case token_type_semicolon:
      return "token_type_semicolon";
    case token_type_lbrace:
      return "token_type_lbrace";
    case token_type_rbrace:
      return "token_type_rbrace";
    case token_type_lbracket:
      return "token_type_lbracket";
    case token_type_rbracket:
      return "token_type_rbracket";
    case token_type_string:
      return "token_type_string";
    case token_type_named_op:
      return "token_type_named_op";
    case token_type_variable:
      return "token_type_variable";
    case token_type_symbol:
      return "token_type_symbol";
    case token_type_regex_op:
      return "token_type_regex_op";
    case token_type_keyword_symbol:
      return "token_type_keyword_symbol";
    case token_type_dot:
      return "token_type_dot";
    case token_type_other:
      return "token_type_other";
    default:
      return "unknown";
    }
}
#endif

struct stack_entry
{
  struct stack_entry *next;
  struct stack_entry *prev;
  void *data;
  void (*destroy) (token_ty *data);
};

struct stack
{
  struct stack_entry *first;
  struct stack_entry *last;
};

struct stack *token_stack;

/* Prototypes for local functions.  Needed to ensure compiler checking of
   function argument counts despite of K&R C function definition syntax.  */
static void interpolate_keywords (message_list_ty *mlp, const char *string);
static char *extract_quotelike_pass1 (int delim);
static token_ty *x_perl_lex (message_list_ty *mlp);
static void x_perl_unlex (token_ty *tp);
static bool extract_balanced (message_list_ty *mlp, int arg_sg, int arg_pl, int state, token_type_ty delim);

#if DEBUG_MEMORY

static message_ty *
remember_a_message_debug (message_list_ty *mlp, char *string, lex_pos_ty *pos)
{
  void *retval;

  fprintf (stderr, "*** remember_a_message (%p): ", string); fflush (stderr);
  retval = remember_a_message (mlp, string, pos);
  fprintf (stderr, "%p\n", retval); fflush (stderr);
  return retval;
}

static void
remember_a_message_plural_debug (message_ty *mp, char *string, lex_pos_ty *pos)
{
  fprintf (stderr, "*** remember_a_message_plural (%p, %p): ", mp, string);
  fflush (stderr);
  remember_a_message_plural (mp, string, pos);
  fprintf (stderr, "done\n"); fflush (stderr);
}

static void *
xmalloc_debug (size_t bytes)
{
  void *retval;

  fprintf (stderr, "*** xmalloc (%u): ", bytes); fflush (stderr);
  retval = xmalloc (bytes);
  fprintf (stderr, "%p\n", retval); fflush (stderr);
  return retval;
}

static void *
xrealloc_debug (void *buf, size_t bytes)
{
  void *retval;

  fprintf (stderr, "*** xrealloc (%p, %u): ", buf, bytes); fflush (stderr);
  retval = xrealloc (buf, bytes);
  fprintf (stderr, "%p\n", retval); fflush (stderr);
  return retval;
}

static void *
xrealloc_static_debug (void *buf, size_t bytes)
{
  void *retval;

  fprintf (stderr, "*** xrealloc_static (%p, %u): ", buf, bytes);
  fflush (stderr);
  retval = xrealloc (buf, bytes);
  fprintf (stderr, "%p\n", retval); fflush (stderr);
  return retval;
}

static char *
xstrdup_debug (const char *string)
{
  char *retval;

  fprintf (stderr, "*** xstrdup (%p, %d): ", string, strlen (string));
  fflush (stderr);
  retval = xstrdup (string);
  fprintf (stderr, "%p\n", retval); fflush (stderr);
  return retval;
}

static void
free_debug (void *buf)
{
  fprintf (stderr, "*** free (%p): ", buf); fflush (stderr);
  free (buf);
  fprintf (stderr, "done\n"); fflush (stderr);
}

# define xmalloc(b) xmalloc_debug (b)
# define xrealloc(b, s) xrealloc_debug (b, s)
# define xstrdup(s) xstrdup_debug (s)
# define free(b) free_debug (b)

# define xrealloc_static(b, s) xrealloc_static_debug (b, s)

#define remember_a_message(m, s, p) remember_a_message_debug (m, s, p)
#define remember_a_message_plural(m, s, p) \
    remember_a_message_plural_debug (m, s, p)

#else
# define xrealloc_static(b, s) xrealloc (b, s)
#endif

#if DEBUG_PERL
/* Dumps all resources allocated by stack STACK.  */
static int
stack_dump (struct stack *stack)
{
  struct stack_entry *last = stack->last;

  fprintf (stderr, "BEGIN STACK DUMP\n");
  while (last)
    {
      struct stack_entry *next = last->prev;

      if (last->data)
	{
	  token_ty *token = (token_ty *) last->data;
	  fprintf (stderr, "  [%s]\n", token2string (token));
	  fflush (stderr);
	  switch (token->type)
	    {
	    case token_type_named_op:
	    case token_type_string:
	    case token_type_symbol:
	    case token_type_keyword_symbol:
	    case token_type_variable:
	      fprintf (stderr, "    string: %s\n", token->string);
	      fflush (stderr);
	      break;
	    }
	}
      last = next;
    }
  fprintf (stderr, "END STACK DUMP\n");
  return 1;
}
#endif

/* Unshifts the pointer DATA onto the stack STACK.  The argument DESTROY
 * is a pointer to a function that frees the resources associated with
 * DATA or NULL (no destructor).
 */
static void
stack_unshift (struct stack *stack, void *data, void (*destroy) (token_ty *data))
{
  struct stack_entry *entry =
    (struct stack_entry *) xmalloc (sizeof (struct stack_entry));

  if (stack->first == NULL)
    stack->last = entry;
  else
    stack->first->prev = entry;

  entry->next = stack->first;
  entry->prev = NULL;
  entry->data = data;
  entry->destroy = destroy;
  stack->first = entry;
}

/* Shifts the first element from the stack STACK and returns its contents or
 * NULL if the stack is empty.
 */
static void *
stack_shift (struct stack *stack)
{
  struct stack_entry *entry = stack->first;
  void *data;

  if (!entry)
    return NULL;

  stack->first = entry->next;
  if (!stack->first)
    stack->last = NULL;
  else
    stack->first->prev = NULL;

  data = entry->data;
  free (entry);

  return data;
}

/* Return the bottom of the stack without removing it from the stack or
 * NULL if the stack is empty.
 */
static void *
stack_head (struct stack *stack)
{
  struct stack_entry *entry = stack->first;
  void *data;

  if (!entry)
    return NULL;

  data = entry->data;

  return data;
}

/* Frees all resources allocated by stack STACK.  */
static void
stack_free (struct stack *stack)
{
  struct stack_entry *last = stack->last;

  while (last)
    {
      struct stack_entry *next = last->prev;
      if (last->data && last->destroy)
	last->destroy (last->data);
      free (last);
      last = next;
    }
}

/* ====================== Keyword set customization.  ====================== */

/* If true extract all strings.  */
static bool extract_all = false;

static hash_table keywords;
static bool default_keywords = true;


void
x_perl_extract_all ()
{
  extract_all = true;
}


void
x_perl_keyword (const char *name)
{
  if (name == NULL)
    default_keywords = false;
  else
    {
      const char *end;
      int argnum1;
      int argnum2;
      const char *colon;

      if (keywords.table == NULL)
	init_hash (&keywords, 100);

      split_keywordspec (name, &end, &argnum1, &argnum2);

      /* The characters between name and end should form a valid C identifier.
	 A colon means an invalid parse in split_keywordspec().  */
      colon = strchr (name, ':');
      if (colon == NULL || colon >= end)
	{
	  if (argnum1 == 0)
	    argnum1 = 1;
	  insert_entry (&keywords, name, end - name,
			(void *) (long) (argnum1 + (argnum2 << 10)));
	}
    }
}

/* Finish initializing the keywords hash table.
   Called after argument processing, before each file is processed.  */
static void
init_keywords ()
{
  if (default_keywords)
    {
      x_perl_keyword ("gettext");
      x_perl_keyword ("%gettext");
      x_perl_keyword ("$gettext");
      x_perl_keyword ("dgettext:2");
      x_perl_keyword ("dcgettext:2");
      x_perl_keyword ("ngettext:1,2");
      x_perl_keyword ("dngettext:2,3");
      x_perl_keyword ("dcngettext:2,3");
      x_perl_keyword ("gettext_noop");
      default_keywords = false;
    }
}


/* ================== Reading of characters and tokens.  =================== */

/* Real filename, used in error messages about the input file.  */
static const char *real_file_name;

/* Logical filename and line number, used to label the extracted messages.  */
static char *logical_file_name;
static int line_number;

/* The input file stream.  */
static FILE *fp;

/* These are for tracking whether comments count as immediately before
   keyword.  */
static int last_comment_line;
static int last_non_comment_line;

/* The current line buffer.  */
char *linebuf;

/* The size of the current line.  */
int linesize;

/* The position in the current line.  */
int linepos;

/* The size of the input buffer.  */
size_t linebuf_size;

/* The last token seen in the token stream.  This is important for the
   interpretation of '?' and '/'.  */
token_type_ty last_token;

/* The last string token waiting for a dot operator or finishing.  */
token_ty last_string;

/* True if LAST_STRING is finished.  */
bool last_string_finished;

/* Number of lines eaten for here documents.  */
int here_eaten;

/* Paranoia: EOF marker for __END__ or __DATA__.  */
bool end_of_file;

/* 1. line_number handling.  */
/* Returns the next character from the input stream or EOF.  */
static int
phase1_getc ()
{
  line_number += here_eaten;
  here_eaten = 0;

  if (end_of_file)
    return EOF;

  if (linepos >= linesize)
    {
      linesize = getline (&linebuf, &linebuf_size, fp);

      if (linesize == EOF)
	{
	  if (ferror (fp))
	    error (EXIT_FAILURE, errno, _("error while reading \"%s\""),
		   real_file_name);
	  end_of_file = true;
	  return EOF;
	}

      linepos = 0;
      ++line_number;

      /* Undosify.  This is important for catching the end of <<EOF and
	 <<'EOF'.  We could rely on stdio doing this for us but you
	 it is not uncommon to to come across Perl scripts with CRLF
	 newline conventions on systems that do not follow this
	 convention.  */
      if (linesize >= 2 && linebuf[linesize - 1] == '\n'
	  && linebuf[linesize - 2] == '\r')
	{
	  linebuf[linesize - 2] = '\n';
	  linebuf[linesize - 1] = '\0';
	  --linesize;
	}
    }

  return linebuf[linepos++];
}

static void
phase1_ungetc (int c)
{
  if (c != EOF)
    {
      if (linepos == 0)
	error (EXIT_FAILURE, 0, _("\
%s:%d: internal error: attempt to ungetc across line boundary"),
	       real_file_name, line_number);

      --linepos;
    }
}

static char *
get_here_document (const char *delimiter)
{
  static char *buffer;
  static size_t bufmax = 0;
  size_t bufpos = 0;
  static char *my_linebuf = NULL;
  static size_t my_linebuf_size = 0;
  bool chomp = false;

  if (bufpos >= bufmax)
    {
      buffer = xrealloc_static (NULL, 1);
      buffer[0] = '\0';
      bufmax = 1;
    }

  for (;;)
    {
      int read_bytes = getline (&my_linebuf, &my_linebuf_size, fp);

      if (read_bytes == EOF)
	{
	  if (ferror (fp))
	    {
	      error (EXIT_FAILURE, errno, _("error while reading \"%s\""),
		     real_file_name);
	    }
	  else
	    {
	      error_with_progname = false;
	      error (EXIT_SUCCESS, 0, _("\
%s:%d: can\'t find string terminator \"%s\" anywhere before EOF"),
		     real_file_name, line_number, delimiter);
	      error_with_progname = true;
	      fflush (stderr);

	      return xstrdup (buffer);
	    }
	}

      ++here_eaten;

      /* Undosify.  This is important for catching the end of <<EOF and
	 <<'EOF'.  We could rely on stdio doing this for us but you
	 it is not uncommon to to come across Perl scripts with CRLF
	 newline conventions on systems that do not follow this
	 convention.  */
      if (read_bytes >= 2 && my_linebuf[read_bytes - 1] == '\n'
	  && my_linebuf[read_bytes - 2] == '\r')
	{
	  my_linebuf[read_bytes - 2] = '\n';
	  my_linebuf[read_bytes - 1] = '\0';
	  --read_bytes;
	}

      if (read_bytes && my_linebuf[read_bytes - 1] == '\n')
	{
	  chomp = true;
	  my_linebuf[read_bytes - 1] = '\0';
	}
      if (strcmp (my_linebuf, delimiter) == 0)
	{
	  return xstrdup (buffer);
	}
      if (chomp)
	{
	  my_linebuf[read_bytes - 1] = '\n';
	}

      if (bufpos + read_bytes + 1 >= bufmax)
	{
	  bufmax += read_bytes + 1;
	  buffer = xrealloc_static (buffer, bufmax);
	}
      strcpy (buffer + bufpos, my_linebuf);
      bufpos += read_bytes;
    }
}

/* Skips pod sections.  */
static void
skip_pod ()
{
  line_number += here_eaten;
  here_eaten = 0;
  linepos = 0;

  for (;;)
    {
      linesize = getline (&linebuf, &linebuf_size, fp);

      if (linesize == EOF)
	{
	  if (ferror (fp))
	    error (EXIT_FAILURE, errno, _("error while reading \"%s\""),
		   real_file_name);
	  return;
	}

      ++line_number;

      if (strncmp ("=cut", linebuf, 4) == 0)
	{
	  /* Force reading of a new line on next call to phase1_getc().  */
	  linepos = linesize;
	  return;
	}
    }
}

/* 2. Replace each comment that is not inside a string literal or regular
   expression with a newline character.  We need to remember the comment
   for later, because it may be attached to a keyword string.  */

static int
phase2_getc ()
{
  static char *buffer;
  static size_t bufmax;
  size_t buflen;
  int lineno;
  int c;

  c = phase1_getc ();
  if (c == '#')
    {
      buflen = 0;
      lineno = line_number;
      /* Skip leading whitespace.  */
      for (;;)
	{
	  c = phase1_getc ();
	  if (c == EOF)
	    break;
	  if (c != ' ' && c != '\t' && c != '\r' && c != '\f')
	    {
	      phase1_ungetc (c);
	      break;
	    }
	}
      for (;;)
	{
	  c = phase1_getc ();
	  if (c == '\n' || c == EOF)
	    break;
	  if (buflen >= bufmax)
	    {
	      bufmax += 100;
	      buffer = xrealloc_static (buffer, bufmax);
	    }
	  buffer[buflen++] = c;
	}
      if (buflen >= bufmax)
	{
	  bufmax += 100;
	  buffer = xrealloc_static (buffer, bufmax);
	}
      buffer[buflen] = '\0';
      xgettext_comment_add (buffer);
      last_comment_line = lineno;
    }
  return c;
}

static void
phase2_ungetc (int c)
{
  if (c != EOF)
    phase1_ungetc (c);
}

/* There is an ambiguity about '/': It can start a division operator ('/' or
   '/=') or it can start a regular expression.  The distinction is important
   because inside regular expressions, '#' and '"' lose its special meanings.
   If you look at the awk grammar, you see that the operator is only allowed
   right after a 'variable' or 'simp_exp' nonterminal, and these nonterminals
   can only end in the NAME, LENGTH, YSTRING, YNUMBER, ')', ']' terminals.
   So we prefer the division operator interpretation only right after
   symbol, string, number, ')', ']', with whitespace but no newline allowed
   in between.  */
static bool prefer_division_over_regexp;

/* Free the memory pointed to by a 'struct token_ty'.  */
static inline void
free_token (token_ty *tp)
{
  switch (tp->type)
    {
    case token_type_named_op:
    case token_type_string:
    case token_type_symbol:
    case token_type_keyword_symbol:
    case token_type_variable:
      free (tp->string);
      break;
    default:
      break;
    }
  free (tp);
}

/* Extract an unsigned hexadecimal number from STRING, considering at
   most LEN bytes and place the result in RESULT.  Returns a pointer
   to the first character past the hexadecimal number.  */
static char *
extract_hex (char *string, size_t len, unsigned int *result)
{
  size_t i;

  *result = 0;

  for (i = 0; i < len; i++)
    {
      int number;

      if (string[i] >= 'A' && string[i] <= 'F')
	number = 10 + string[i] - 'A';
      else if (string[i] >= 'a' && string[i] <= 'f')
	number = 10 + string[i] - 'a';
      else if (string[i] >= '0' && string[i] <= '9')
	number = string[i] - '0';
      else
	break;

      *result <<= 4;
      *result |= number;
    }

  return string + i;
}

/* Extract an unsigned octal number from STRING, considering at
   most LEN bytes and place the result in RESULT.  Returns a pointer
   to the first character past the hexadecimal number.  */
static char *
extract_oct (char *string, size_t len, unsigned int *result)
{
  size_t i;

  *result = 0;

  for (i = 0; i < len; i++)
    {
      int number;

      if (string[i] >= '0' && string[i] <= '7')
	number = string[i] - '0';
      else
	break;

      *result <<= 3;
      *result |= number;
    }

  return string + i;
}

/* Extract the various quotelike constructs except for <<EOF.  See the
   section "Gory details of parsing quoted constructs" in perlop.pod.  */
static void
extract_quotelike (token_ty *tp, int delim)
{
  char *string = extract_quotelike_pass1 (delim);
  tp->type = token_type_string;

  string[strlen (string) - 1] = '\0';
  tp->string = xstrdup (string + 1);
  free (string);
  return;
}

/* Extract the quotelike constructs with double delimiters, like
   s/[SEARCH]/[REPLACE]/.  This function does not eat up trailing
   modifiers (left to the caller).  */
static void
extract_triple_quotelike (message_list_ty *mlp, token_ty *tp, int delim,
			  bool interpolate)
{
  char *string = extract_quotelike_pass1 (delim);

  tp->type = token_type_regex_op;
  if (interpolate && !extract_all && delim != '\'')
    interpolate_keywords (mlp, string);

  free (string);

  if (delim == '(' || delim == '<' || delim == '{' || delim == '[')
    {
      /* Things can change.  */
      delim = phase1_getc ();
      while (delim == ' ' || delim == '\t' || delim == '\r'
	     || delim == '\n' || delim  == '\f')
	{
	  /* The hash-sign is not a valid delimiter after whitespace, ergo
	     use phase2_getc() and not phase1_getc() now.  */
	  delim = phase2_getc ();
	}
    }
  string = extract_quotelike_pass1 (delim);
  if (interpolate && !extract_all && delim != '\'')
    interpolate_keywords (mlp, string);
  free (string);

  return;
}

/* Pass 1 of extracting quotes: Find the end of the string, regardless
   of the semantics of the construct.  */
static char *
extract_quotelike_pass1 (int delim)
{
  /* This function is called recursively.  No way to allocate stuff
     statically.  Consider using alloca() instead.  */
  char *buffer = (char *) xmalloc (100);
  int bufmax = 100;
  int bufpos = 0;
  bool nested = true;
  int counter_delim;

  buffer[bufpos++] = delim;

  /* Find the closing delimiter.  */
  switch (delim)
    {
    case '(':
      counter_delim = ')';
      break;
    case '{':
      counter_delim = '}';
      break;
    case '[':
      counter_delim = ']';
      break;
    case '<':
      counter_delim = '>';
      break;
    default:
      nested = false;
      counter_delim = delim;
      break;
    }

  for (;;)
    {
      int c = phase1_getc ();

      if (bufpos >= bufmax - 1)
	{
	  bufmax += 100;
	  buffer = xrealloc (buffer, bufmax);
	}

      if (c == counter_delim || c == EOF)
	{
	  /* Copying the EOF (actually 255) is not an error.  It will
	     be stripped off later.  */
	  buffer[bufpos++] = c;
	  buffer[bufpos++] = '\0';
#if DEBUG_PERL
	  fprintf (stderr, "PASS1: %s\n", buffer);
#endif
	  return buffer;
	}

      if (nested && c == delim)
	{
	  char *inner = extract_quotelike_pass1 (delim);
	  size_t len = strlen (inner);

	  if (bufpos + len >= bufmax)
	    {
	      bufmax += len;
	      buffer = xrealloc (buffer, bufmax);
	    }
	  strcpy (buffer + bufpos, inner);
	  free (inner);
	  bufpos += len;
	  continue;
	}

      if (c == '\\')
	{
	  c = phase1_getc ();
	  if (c == '\\')
	    {
	      buffer[bufpos++] = '\\';
	      buffer[bufpos++] = '\\';
	    }
	  else if (c == delim || c == counter_delim)
	    {
	      /* This is pass2 in Perl.  */
	      buffer[bufpos++] = c;
	    }
	  else
	    {
	      buffer[bufpos++] = '\\';
	      phase1_ungetc (c);
	    }
	}
      else
	{
	  buffer[bufpos++] = c;
	}
    }
}

/* Perform pass 3 of quotelike extraction (interpolation).  */
/* FIXME: Currently may writes null-bytes into the string.  */
static void
extract_quotelike_pass3 (token_ty *tp, int error_level)
{
  static char *buffer;
  static int bufmax = 0;
  int bufpos = 0;
  char *string = tp->string;
  unsigned char *crs = string;

  bool uppercase = false;
  bool lowercase = false;
  bool quotemeta = false;

#if DEBUG_PERL
  switch (tp->string_type)
    {
    case string_type_verbatim:
      fprintf (stderr, "Interpolating string_type_verbatim:\n");
      break;
    case string_type_q:
      fprintf (stderr, "Interpolating string_type_q:\n");
      break;
    case string_type_qq:
      fprintf (stderr, "Interpolating string_type_qq:\n");
      break;
    case string_type_qr:
      fprintf (stderr, "Interpolating string_type_qr:\n");
      break;
    }
  fprintf (stderr, "%s\n", tp->string);
  if (tp->string_type == string_type_verbatim)
    fprintf (stderr, "---> %s\n", tp->string);
#endif

  if (tp->string_type == string_type_verbatim)
    return;

  while (*crs)
    {
      if (bufpos >= bufmax - 6)
	{
	  bufmax += 100;
	  buffer = xrealloc_static (buffer, bufmax);
	}

      if (tp->string_type == string_type_q)
	{
	  switch (*crs)
	    {
	    case '\\':
	      if (crs[1] == '\\')
		{
		  ++crs;
		  buffer[bufpos++] = '\\';
		  continue;
		}
	      /* FALLTHROUGH */
	    default:
	      buffer[bufpos++] = *crs++;
	      break;
	    }
	  continue;
	}

      /* We only get here for double-quoted strings or regular expressions.
	 Unescape escape sequences.  */
      if (*crs == '\\')
	{
	  switch (crs[1])
	    {
	    case 't':
	      crs += 2;
	      buffer[bufpos++] = '\t';
	      continue;
	    case 'n':
	      crs += 2;
	      buffer[bufpos++] = '\n';
	      continue;
	    case 'r':
	      crs += 2;
	      buffer[bufpos++] = '\r';
	      continue;
	    case 'f':
	      crs += 2;
	      buffer[bufpos++] = '\f';
	      continue;
	    case 'b':
	      crs += 2;
	      buffer[bufpos++] = '\b';
	      continue;
	    case 'a':
	      crs += 2;
	      buffer[bufpos++] = '\a';
	      continue;
	    case 'e':
	      crs += 2;
	      buffer[bufpos++] = 0x1b;
	      continue;
	    case '0': case '1': case '2': case '3':
	    case '4': case '5': case '6': case '7':
	      {
		unsigned int oct_number;
		int length;

		crs = extract_oct (crs + 1, 3, &oct_number);
		length = u8_uctomb (buffer + bufpos, oct_number, 3);
		if (length > 0)
		  bufpos += length;
	      }
	      continue;
	    case 'x':
	      {
		unsigned int hex_number = 0;
		int length;

		++crs;

		if (*crs == '{')
		  {
		    char *end = strchr (crs, '}');
		    if (end == NULL)
		      {
			error_with_progname = false;
			error (error_level, 0, _("\
%s:%d: missing right brace on \\x{HEXNUMBER}"), real_file_name, line_number);
			error_with_progname = true;
			++crs;
			continue;
		      }
		    else
		      {
			++crs;
			(void) extract_hex (crs, 4, &hex_number);
		      }
		  }
		else
		  {
		    crs = extract_hex (crs, 2, &hex_number);
		  }

		length = u8_uctomb (buffer + bufpos, hex_number, 6);
		if (length > 0)
		  bufpos += length;
	      }
	      continue;
	    case 'c':
	      /* Perl's notion of control characters.  */
	      crs += 2;
	      if (*crs)
		{
		  int the_char = *crs;
		  if (the_char >= 'a' || the_char <= 'z')
		    the_char -= 0x20;
		  buffer[bufpos++] = the_char + (the_char & 0x40 ? -64 : 64);
		}
	      continue;
	    case 'N':
	      crs += 2;
	      if (*crs == '{')
		{
		  char *name = xstrdup (crs + 1);
		  char *end = strchr (name, '}');
		  if (end != NULL)
		    {
		      unsigned int unicode;
		      int length;

		      *end = '\0';

		      crs += 2 + strlen (name);
		      unicode = unicode_name_character (name);
		      if (unicode != UNINAME_INVALID)
			{
			  length = u8_uctomb (buffer + bufpos, unicode, 6);
			  if (length > 0)
			    bufpos += length;
			}
		    }
		  free (name);
		}
	      continue;
	    }
	}

      /* No escape sequence, go on.  */
      if (*crs == '\\')
	{
	  ++crs;
	  switch (*crs)
	    {
	    case 'E':
	      quotemeta = uppercase = lowercase = false;
	      ++crs;
	      continue;
	    case 'L':
	      quotemeta = uppercase = false;
	      lowercase = true;
	      ++crs;
	      continue;
	    case 'U':
	      quotemeta = lowercase = false;
	      uppercase = true;
	      ++crs;
	      continue;
	    case 'Q':
	      uppercase = lowercase = false;
	      quotemeta = true;
	      ++crs;
	      continue;
	    case 'l':
	      ++crs;
	      if (crs[1] >= 'A' && crs[1] <= 'Z')
		{
		  buffer[bufpos++] = crs[1] + 0x20;
		  ++crs;
		}
	      else if (crs[1] >= 0x80)
		{
		  error_with_progname = false;
		  error (error_level, 0, _("\
%s:%d: invalid interpolation (\"\\l\") of 8bit character \"%c\""),
			 real_file_name, line_number, *crs);
		  error_with_progname = true;
		  ++crs;
		}
	      else
		++crs;
	      continue;
	    case 'u':
	      ++crs;
	      if (crs[1] >= 'a' && crs[1] <= 'z')
		{
		  buffer[bufpos++] = crs[1] - 0x20;
		  ++crs;
		}
	      else if (crs[1] >= 0x80)
		{
		  error_with_progname = false;
		  error (error_level, 0, _("\
%s:%d: invalid interpolation (\"\\u\") of 8bit character \"%c\""),
			 real_file_name, line_number, *crs);
		  error_with_progname = true;
		  ++crs;
		}
	      else
		++crs;
	      continue;
	    case '\\':
	      if (crs[1])
		buffer[bufpos++] = crs[1];
	      crs++;
	      continue;
	    }
	}


      if (*crs == '$' || *crs == '@')
	{
	  ++crs;
	  error_with_progname = false;
	  error (error_level, 0, _("\
%s:%d: invalid variable interpolation at \"%c\""),
		 real_file_name, line_number, *crs);
	  error_with_progname = true;
	}
      else if (lowercase)
	{
	  if (*crs >= 'A' && *crs <= 'Z')
	    buffer[bufpos++] = 0x20 + *crs++;
	  else if (*crs >= 0x80)
	    {
	      error_with_progname = false;
	      error (error_level, 0, _("\
%s:%d: invalid interpolation (\"\\L\") of 8bit character \"%c\""),
		     real_file_name, line_number, *crs);
	      error_with_progname = true;
	      buffer[bufpos++] = *crs++;
	    }
	  else
	    buffer[bufpos++] = *crs++;
	}
      else if (uppercase)
	{
	  if (*crs >= 'a' && *crs <= 'z')
	    buffer[bufpos++] = *crs++ - 0x20;
	  else if (*crs >= 0x80)
	    {
	      error_with_progname = false;
	      error (error_level, 0, _("\
%s:%d: invalid interpolation (\"\\U\") of 8bit character \"%c\""),
		     real_file_name, line_number, *crs);
	      error_with_progname = true;
	      buffer[bufpos++] = *crs++;
	    }
	  else
	    buffer[bufpos++] = *crs++;
	}
      else if (quotemeta)
	{
	  buffer[bufpos++] = *crs++;
	}
      else
	{
	  buffer[bufpos++] = *crs++;
	}
    }

  if (bufpos >= bufmax - 1)
  {
      bufmax += 100;
      buffer = xrealloc_static (buffer, bufmax);
  }

  buffer[bufpos++] = '\0';

#if DEBUG_PERL
  fprintf (stderr, "---> %s\n", buffer);
#endif

  free (tp->string);
  tp->string = xstrdup (buffer);
}

/* Parse a variable.  This is done in several steps:
 *
 * 1) Consume all leading occurcencies of '$', '@', '%', and '*'.
 * 2) Determine the name of the variable from the following input
 * 3) Parse possible following hash keys or array indexes.
 */
static void
extract_variable (message_list_ty *mlp, token_ty *tp, int first)
{
  static char *buffer;
  static int bufmax = 0;
  int bufpos = 0;
  int c = first;
  size_t varbody_length = 0;
  bool maybe_hash_deref = false;
  bool maybe_hash_value = false;

  tp->type = token_type_variable;

#if DEBUG_PERL
  fprintf (stderr, "%s:%d: extracting variable type '%c'\n",
	   real_file_name, line_number, first);
#endif

  /*
   * 1) Consume dollars and so on (not euros ...).  Unconditionally
   *    accepting the hash sign (#) will maybe lead to inaccurate
   *    results.  FIXME!
   */
  while (c == '$' || c == '*' || c == '#' || c == '@' || c == '%')
    {
      if (bufpos >= bufmax)
	{
	  bufmax += 100;
	  buffer = xrealloc_static (buffer, bufmax);
	}
      buffer[bufpos++] = c;
      c = phase1_getc ();
    }

  if (c == EOF)
    {
      tp->type = token_type_eof;
      return;
    }

  /* Hash references are treated in a special way, when looking for
     our keywords.  */
  if (buffer[0] == '$')
    {
      if (bufpos == 1)
	maybe_hash_value = true;
      else if (bufpos == 2 && buffer[1] == '$')
	{
	  if (c != '{' && c != '_' && (!((c >= '0' && c <= '9')
					 || (c >= 'A' && c <= 'Z')
					 || (c >= 'a' && c <= 'z')
					 || c == ':' || c == '\''
					 || c >= 0x80)))
	    {
	      /* Special variable $$ for pid.  */
	      if (bufpos >= bufmax)
		{
		  bufmax += 100;
		  buffer = xrealloc_static (buffer, bufmax);
		}
	      buffer[bufpos++] = '\0';
	      tp->string = xstrdup (buffer);
#if DEBUG_PERL
	      fprintf (stderr, "%s:%d: is PID ($$)\n",
		       real_file_name, line_number);
#endif

	      phase1_ungetc (c);
	      return;
	    }

	  maybe_hash_deref = true;
	  bufpos = 1;
	}
    }

  /*
   * 2) Get the name of the variable.  The first character is practically
   *    arbitrary.  Punctuation and numbers automagically put a variable
   *    in the global namespace but that subtle difference is not interesting
   *    for us.
   */
  if (bufpos >= bufmax)
    {
      bufmax += 100;
      buffer = xrealloc_static (buffer, bufmax);
    }
  if (c == '{')
    {
      /* Yuck, we cannot accept ${gettext} as a keyword...  Except for
       * debugging purposes it is also harmless, that we suppress the
       * real name of the variable.
       */
#if DEBUG_PERL
      fprintf (stderr, "%s:%d: braced {variable_name}\n",
	       real_file_name, line_number);
#endif

      if (extract_balanced (mlp, -1, -1, 0, token_type_rbrace))
	return;
      buffer[bufpos++] = c;
    }
  else
    {
      while ((c >= 'A' && c <= 'Z') ||
	     (c >= 'a' && c <= 'z') ||
	     (c >= '0' && c <= '9') ||
	     c == '_' || c == ':' || c == '\'' || c >= 0x80)
	{
	  ++varbody_length;
	  if (bufpos >= bufmax)
	    {
	      bufmax += 100;
	      buffer = xrealloc_static (buffer, bufmax);
	    }
	  buffer[bufpos++] = c;
	  c = phase1_getc ();
	}
      phase1_ungetc (c);
    }

  if (bufpos >= bufmax - 1)
    {
      bufmax += 100;
      buffer = xrealloc_static (buffer, bufmax);
    }
  buffer[bufpos++] = '\0';

  /* Probably some strange Perl variable like $`.  */
  if (varbody_length == 0)
    {
      c = phase1_getc ();
      if (c == EOF || c == ' ' || c == '\n' || c == '\r'
	  || c == '\f' || c == '\t')
	phase1_ungetc (c);  /* Loser.  */
      else
	{
	  buffer[bufpos - 1] = c;
	  buffer[++bufpos] = '\0';
	}
    }
  tp->string = xstrdup (buffer);

#if DEBUG_PERL
  fprintf (stderr, "%s:%d: complete variable name: %s\n",
	   real_file_name, line_number, tp->string);
#endif
  prefer_division_over_regexp = true;

  /*
   * 3) If the following looks strange to you, this is valid Perl syntax:
   *
   *      $var = $$hashref    # We can place a
   *                          # comment here and then ...
   *             {key_into_hashref};
   *
   *    POD sections are not allowed but we leave complaints about
   *    that to the compiler/interpreter.
   */
  /* We only extract strings from the first hash key (if present).  */

  if (maybe_hash_deref || maybe_hash_value)
    {
      bool is_dereference = false;
      int c = phase2_getc ();

      while (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f')
	  c = phase2_getc ();

      if (c == '-')
	{
	  int c2 = phase1_getc ();

	  if (c2 == '>')
	    {
	      is_dereference = true;
	      c = phase2_getc ();
	      while (c == ' ' || c == '\t' || c == '\r'
		     || c == '\n' || c == '\f')
		  c = phase2_getc ();
	    }
	  else if (c2 != '\n')
	    {
	      /* Discarding the newline is harmless here.  The only
		 special character recognized after a minus is greater-than
		 for dereference.  However, the sequence "-\n>" that we
	         treat incorrectly here, is a syntax error.  */
		phase1_ungetc (c2);
	    }
	}

      if (maybe_hash_value && is_dereference)
	{
#if DEBUG_PERL
	  fprintf (stderr, "%s:%d: first keys preceded by \"->\"\n",
		   real_file_name, line_number);
#endif
	}
      else if (maybe_hash_value)
	{
	  /* Fake it into a hash.  */
	  tp->string[0] = '%';
	}

      /* Do NOT change that into else if (see above).  */
      if ((maybe_hash_value || maybe_hash_deref) && c == '{')
	{
	  void *keyword_value;

#if DEBUG_PERL
	  fprintf (stderr, "%s:%d: first keys preceded by '{'\n",
		   real_file_name, line_number);
#endif

	  if (find_entry (&keywords, tp->string, strlen (tp->string),
			  &keyword_value) == 0)
	    {
	      /* Extract a possible string from the key.  Before proceeding
		 we check whether the open curly is followed by a symbol and
		 then by a right curly.  */
	      token_ty *t1 = x_perl_lex (mlp);

#if DEBUG_PERL
	      fprintf (stderr, "%s:%d: extracting string key\n",
		       real_file_name, line_number);
#endif

	      if (t1->type == token_type_symbol
		  || t1->type == token_type_named_op)
		{
		  token_ty *t2 = x_perl_lex (mlp);
		  if (t2->type == token_type_rbrace)
		    {
		      lex_pos_ty pos;
		      pos.line_number = line_number;
		      pos.file_name = logical_file_name;
		      remember_a_message (mlp, xstrdup (t1->string), &pos);
		      free_token (t2);
		      free_token (t1);
		    }
		  else
		    {
		      x_perl_unlex (t2);
		    }
		}
	      else
		{
		  x_perl_unlex (t1);
		  if (extract_balanced (mlp, 1, -1, 1, token_type_rbrace))
		    return;
		}
	    }
	  else
	    {
	      phase2_ungetc (c);
	    }
	}
      else
	{
	  phase2_ungetc (c);
	}
    }

  /* Now consume "->", "[...]", and "{...}".  */
  for (;;)
    {
      int c = phase2_getc ();
      int c2;

      switch (c)
	{
	case '{':
#if DEBUG_PERL
	  fprintf (stderr, "%s:%d: extracting balanced '{' after varname\n",
		   real_file_name, line_number);
#endif
	  extract_balanced (mlp, -1, -1, 0, token_type_rbrace);
	  break;

	case '[':
#if DEBUG_PERL
	  fprintf (stderr, "%s:%d: extracting balanced '[' after varname\n",
		   real_file_name, line_number);
#endif
	  extract_balanced (mlp, -1, -1, 0, token_type_rbracket);
	  break;

	case '-':
	  c2 = phase1_getc ();
	  if (c2 == '>')
	    {
#if DEBUG_PERL
	      fprintf (stderr, "%s:%d: another \"->\" after varname\n",
		       real_file_name, line_number);
#endif
	      break;
	    }
	  else if (c2 != '\n')
	    {
	      /* Discarding the newline is harmless here.  The only
		 special character recognized after a minus is greater-than
		 for dereference.  However, the sequence "-\n>" that we
	         treat incorrectly here, is a syntax error.  */
	      phase1_ungetc (c2);
	    }
	  /* FALLTHROUGH */

	default:
#if DEBUG_PERL
	  fprintf (stderr, "%s:%d: variable finished\n",
		   real_file_name, line_number);
#endif
	  phase2_ungetc (c);
	  return;
	}
    }
}

/* Actually a simplified version of extract_variable().  It searches for
 * variables inside a double-quoted string that may interpolate to
 * some keyword hash (reference).
 */
static void
interpolate_keywords (message_list_ty *mlp, const char *string)
{
  static char *buffer;
  static int bufmax = 0;
  int bufpos = 0;
  int c = string[0];
  bool maybe_hash_deref = false;
  enum parser_state
    {
      initial,
      one_dollar,
      two_dollars,
      identifier,
      minus,
      wait_lbrace,
      wait_quote,
      dquote,
      squote,
      barekey,
      wait_rbrace,
    } state;
  token_ty token;

  lex_pos_ty pos;

  /* States are:
   *
   * initial:      initial
   * one_dollar:   dollar sign seen in state INITIAL
   * two_dollars:  another dollar-sign has been seen in state ONE_DOLLAR
   * identifier:   a valid identifier character has been seen in state
   *               ONE_DOLLAR or TWO_DOLLARS
   * minus:        a minus-sign has been seen in state IDENTIFIER
   * wait_lbrace:  a greater-than has been seen in state MINUS
   * wait_quote:   a left brace has been seen in state IDENTIFIER or in
   *               state WAIT_LBRACE
   * dquote:       a double-quote has been seen in state WAIT_QUOTE
   * squote:       a single-quote has been seen in state WAIT_QUOTE
   * barekey:      an bareword character has been seen in state WAIT_QUOTE
   * wait_rbrace:  closing quote has been seen in state DQUOTE or SQUOTE
   */
  state = initial;

  token.type = token_type_string;
  token.string_type = string_type_qq;
  token.line_number = line_number;
  pos.file_name = logical_file_name;
  pos.line_number = line_number;

  while ((c = *string++) != '\0')
    {
      void *keyword_value;

      if (state == initial)
	bufpos = 0;

      if (bufpos >= bufmax - 1)
	{
	  bufmax += 100;
	  buffer = xrealloc_static (buffer, bufmax);
	}

      switch (state)
	{
	case initial:
	  switch (c)
	    {
	    case '\\':
	      c = *string++;
	      if (!c) return;
	      break;
	    case '$':
	      buffer[bufpos++] = '$';
	      maybe_hash_deref = false;
	      state = one_dollar;
	      break;
	    default:
	      break;
	    }
	  break;
	case one_dollar:
	  switch (c)
	    {
	    case '$':
	      /*
	       * This is enough to make us believe later that we dereference
	       * a hash reference.
	       */
	      maybe_hash_deref = true;
	      state = two_dollars;
	      break;
	    default:
	      if (c == '_' || c == ':' || c == '\'' || c >= 0x80
		  || (c >= 'A' && c <= 'Z')
		  || (c >= 'a' && c <= 'z')
		  || (c >= '0' && c <= '9'))
		{
		  buffer[bufpos++] = c;
		  state = identifier;
		}
	      else
		state = initial;
	      break;
	    }
	  break;
	case two_dollars:
	  if (c == '_' || c == ':' || c == '\'' || c >= 0x80
	      || (c >= 'A' && c <= 'Z')
	      || (c >= 'a' && c <= 'z')
	      || (c >= '0' && c <= '9'))
	    {
	      buffer[bufpos++] = c;
	      state = identifier;
	      break;
	    }
	  else
	    {
	      state = initial;
	    }
	  break;
	case identifier:
	  switch (c)
	    {
	    case '-':
	      if (find_entry (&keywords, buffer, bufpos, &keyword_value) == 0)
		{
		  state = minus;
		}
	      else
		state = initial;
	      break;
	    case '{':
	      if (!maybe_hash_deref)
		{
		  buffer[0] = '%';
		}
	      if (find_entry (&keywords, buffer, bufpos, &keyword_value) == 0)
		{
		  state = wait_quote;
		}
	      else
		state = initial;
	      break;
	    default:
	      if (c == '_' || c == ':' || c == '\'' || c >= 0x80
		  || (c >= 'A' && c <= 'Z')
		  || (c >= 'a' && c <= 'z')
		  || (c >= '0' && c <= '9'))
		{
		  buffer[bufpos++] = c;
		}
	      else
		{
		  state = initial;
		}
	      break;
	    }
	  break;
	case minus:
	  switch (c)
	    {
	    case '>':
	      state = wait_lbrace;
	      break;
	    default:
	      state = initial;
	      break;
	    }
	  break;
	case wait_lbrace:
	  switch (c)
	    {
	    case '{':
	      state = wait_quote;
	      break;
	    default:
	      state = initial;
	      break;
	    }
	  break;
	case wait_quote:
	  switch (c)
	    {
	    case ' ':
	    case '\n':
	    case '\t':
	    case '\r':
	    case '\f':
	      break;
	    case '\'':
	      bufpos = 0;
	      state = squote;
	      break;
	    case '"':
	      bufpos = 0;
	      state = dquote;
	      break;
	    default:
	      if (c == '_' || (c >= '0' && c <= '9') || c >= 0x80
		  || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
		{
		  state = barekey;
		  bufpos = 0;
		  buffer[bufpos++] = c;
		}
	      else
		state = initial;
	      break;
	    }
	  break;
	case dquote:
	  switch (c)
	    {
	    case '"':
	      /* The resulting string has te be interpolated twice.  */
	      buffer[bufpos] = '\0';
	      token.string = xstrdup (buffer);
	      extract_quotelike_pass3 (&token, EXIT_FAILURE);
	      /* The string can only shrink with interpolation (because
		 we ignore \Q).  */
	      strcpy (buffer, token.string);
	      free (token.string);
	      state = wait_rbrace;
	      break;
	    case '\\':
	      if (string[0] == '\"')
		{
		  buffer[bufpos++] = string++[0];
		}
	      else if (string[0])
		{
		  buffer[bufpos++] = '\\';
		  buffer[bufpos++] = string++[0];
		}
	      else
		{
		  state = initial;
		}
	      break;
	    default:
	      buffer[bufpos++] = c;
	      break;
	    }
	  break;
	case squote:
	  switch (c)
	    {
	    case '\'':
	      state = wait_rbrace;
	      break;
	    case '\\':
	      if (string[0] == '\'')
		{
		  buffer[bufpos++] = string++[0];
		}
	      else if (string[0])
		{
		  buffer[bufpos++] = '\\';
		  buffer[bufpos++] = string++[0];
		}
	      else
		{
		  state = initial;
		}
	      break;
	    default:
	      buffer[bufpos++] = c;
	      break;
	    }
	  break;
	case barekey:
	  {
	    if (c == '_' || (c >= '0' && c <= '9') || c >= 0x80
		|| (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
	      {
		buffer[bufpos++] = c;
		break;
	      }
	    else if (c == ' ' || c == '\n' || c == '\t'
		     || c == '\r' || c == '\f')
	      {
		state = wait_rbrace;
		break;
	      }
	    else if (c != '}')
	      {
		state = initial;
		break;
	      }
	    /* Must be right brace.  */
	  }
	  /* FALLTHROUGH */
	case wait_rbrace:
	  switch (c)
	    {
	    case ' ':
	    case '\n':
	    case '\t':
	    case '\r':
	    case '\f':
	      break;
	    case '}':
	      buffer[bufpos] = '\0';
	      token.string = xstrdup (buffer);
	      extract_quotelike_pass3 (&token, EXIT_FAILURE);

	      remember_a_message (mlp, token.string, &pos);
	      /* FALLTHROUGH */
	    default:
	      state = initial;
	      break;
	    }
	  break;
	}
    }
}

/* Combine characters into tokens.  Discard whitespace.  */

static void
x_perl_prelex (message_list_ty *mlp, token_ty *tp)
{
  static char *buffer;
  static int bufmax;
  int bufpos;
  int c;

  for (;;)
    {
      c = phase2_getc ();
      tp->line_number = line_number;

      switch (c)
	{
	case EOF:
	  tp->type = token_type_eof;
	  return;

	case '\n':
	  if (last_non_comment_line > last_comment_line)
	    xgettext_comment_reset ();
	  /* FALLTHROUGH */
	case '\t':
	case ' ':
	  /* Ignore whitespace.  */
	  continue;

	case '%':
	case '@':
	case '*':
	case '$':
	  extract_variable (mlp, tp, c);
	  prefer_division_over_regexp = true;
	  return;
	}

      last_non_comment_line = tp->line_number;

      switch (c)
	{
	case '.':
	  {
	    int c2 = phase1_getc ();
	    phase1_ungetc (c2);
	    if (c2 == '.')
	      {
		tp->type = token_type_other;
		prefer_division_over_regexp = false;
		return;
	      }
	    else if (c2 >= '0' && c2 <= '9')
	      {
		prefer_division_over_regexp = false;
	      }
	    else
	      {
		tp->type = token_type_dot;
		prefer_division_over_regexp = true;
		return;
	      }
	  }
	  /* FALLTHROUGH */
	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
	case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
	case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
	case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
	case 'Y': case 'Z':
	case '_':
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
	case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
	case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
	case 's': case 't': case 'u': case 'v': case 'w': case 'x':
	case 'y': case 'z':
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	  /* Symbol, or part of a number.  */
	  prefer_division_over_regexp = true;
	  bufpos = 0;
	  for (;;)
	    {
	      if (bufpos >= bufmax)
		{
		  bufmax += 100;
		  buffer = xrealloc_static (buffer, bufmax);
		}
	      buffer[bufpos++] = c;
	      c = phase1_getc ();
	      switch (c)
		{
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
		case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
		case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
		case 'Y': case 'Z':
		case '_':
		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
		case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
		case 's': case 't': case 'u': case 'v': case 'w': case 'x':
		case 'y': case 'z':
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
		  continue;

		default:
		  phase1_ungetc (c);
		  break;
		}
	      break;
	    }
	  if (bufpos >= bufmax)
	    {
	      bufmax += 100;
	      buffer = xrealloc_static (buffer, bufmax);
	    }
	  buffer[bufpos] = '\0';

	  if (strcmp (buffer, "__END__") == 0
	      || strcmp (buffer, "__DATA__") == 0)
	    {
	      end_of_file = true;
	      tp->type = token_type_eof;
	      return;
	    }
	  else if (strcmp (buffer, "and") == 0
		   || strcmp (buffer, "cmp") == 0
		   || strcmp (buffer, "eq") == 0
		   || strcmp (buffer, "if") == 0
		   || strcmp (buffer, "ge") == 0
		   || strcmp (buffer, "gt") == 0
		   || strcmp (buffer, "le") == 0
		   || strcmp (buffer, "lt") == 0
		   || strcmp (buffer, "ne") == 0
		   || strcmp (buffer, "not") == 0
		   || strcmp (buffer, "or") == 0
		   || strcmp (buffer, "unless") == 0
		   || strcmp (buffer, "while") == 0
		   || strcmp (buffer, "xor") == 0)
	    {
	      tp->type = token_type_named_op;
	      tp->string = xstrdup (buffer);
	      prefer_division_over_regexp = false;
	      return;
	    }
	  else if (strcmp (buffer, "s") == 0
		 || strcmp (buffer, "y") == 0
		 || strcmp (buffer, "tr") == 0)
	    {
	      int delim = phase1_getc ();
	      while (delim == ' ' || delim == '\t' || delim == '\r'
		     || delim == '\n' || delim == '\f')
		{
		  delim = phase2_getc ();
		}
	      if (delim == EOF)
		{
		  tp->type = token_type_eof;
		  return;
		}
	      if ((delim >= '0' && delim <= '9')
		  || (delim >= 'A' && delim <= 'Z')
		  || (delim >= 'a' && delim <= 'z'))
		{
		  /* False positive.  */
		  tp->type = token_type_symbol;
		  phase2_ungetc (delim);
		  tp->string = xstrdup (buffer);
		  prefer_division_over_regexp = true;
		  return;
		}
	      extract_triple_quotelike (mlp, tp, delim, buffer[0] == 's');

	      /* Eat the following modifiers.  */
	      c = phase1_getc ();
	      while (c >= 'a' && c <= 'z')
		c = phase1_getc ();
	      phase1_ungetc (c);
	      return;
	    }
	  else if (strcmp (buffer, "m") == 0)
	    {
	      int delim = phase1_getc ();

	      while (delim == ' ' || delim == '\t' || delim == '\r'
		     || delim == '\n' || delim == '\f')
		{
		  delim = phase2_getc ();
		}
	      if (delim == EOF)
		{
		  tp->type = token_type_eof;
		  return;
		}
	      if ((delim >= '0' && delim <= '9')
		  || (delim >= 'A' && delim <= 'Z')
		  || (delim >= 'a' && delim <= 'z'))
		{
		  /* False positive.  */
		  tp->type = token_type_symbol;
		  phase2_ungetc (delim);
		  tp->string = xstrdup (buffer);
		  prefer_division_over_regexp = true;
		  return;
		}
	      extract_quotelike (tp, delim);
	      if (!extract_all && delim != '\'')
		  interpolate_keywords (mlp, tp->string);

	      free (tp->string);
	      tp->type = token_type_regex_op;
	      prefer_division_over_regexp = true;

	      /* Eat the following modifiers.  */
	      c = phase1_getc ();
	      while (c >= 'a' && c <= 'z')
		c = phase1_getc ();
	      phase1_ungetc (c);
	      return;
	    }
	  else if (strcmp (buffer, "qq") == 0
		   || strcmp (buffer, "q") == 0
		   || strcmp (buffer, "qx") == 0
		   || strcmp (buffer, "qw") == 0
		   || strcmp (buffer, "qr") == 0)
	    {
	      /* The qw (...) construct is not really a string but we
		 can treat in the same manner and then pretend it is
		 a symbol.  Rationale: Saying "qw (foo bar)" is the
		 same as "my @list = ('foo', 'bar'); @list;".  */

	      int delim = phase1_getc ();

	      while (delim == ' ' || delim == '\t' || delim == '\r'
		     || delim == '\n' || delim == '\f')
		{
		  delim = phase2_getc ();
		}
	      if (delim == EOF)
		{
		  tp->type = token_type_eof;
		  return;
		}
	      prefer_division_over_regexp = true;

	      if ((delim >= '0' && delim <= '9')
		  || (delim >= 'A' && delim <= 'Z')
		  || (delim >= 'a' && delim <= 'z'))
		{
		  /* False positive.  */
		  tp->type = token_type_symbol;
		  phase2_ungetc (delim);
		  tp->string = xstrdup (buffer);
		  prefer_division_over_regexp = true;
		  return;
		}

	      extract_quotelike (tp, delim);

	      switch (buffer[1])
		{
		case 'q':
		case 'x':
		  tp->string_type = string_type_qq;
		  tp->type = token_type_string;
		  if (!extract_all)
		    interpolate_keywords (mlp, tp->string);
		  break;
		case 'r':
		  tp->type = token_type_regex_op;
		  break;
		case 'w':
		  tp->type = token_type_symbol;
		  break;
		default: /* q\000 */
		  tp->type = token_type_string;
		  tp->string_type = string_type_q;
		  break;
		}
	      return;
	    }
	  else if (strcmp (buffer, "grep") == 0
		   || strcmp (buffer, "split") == 0)
	    {
	      prefer_division_over_regexp = false;
	    }
	  tp->string = xstrdup (buffer);

	  tp->type = token_type_symbol;
	  return;

	case '"':
	  prefer_division_over_regexp = true;
	  extract_quotelike (tp, c);
	  tp->string_type = string_type_qq;
	  if (!extract_all)
	    interpolate_keywords (mlp, tp->string);
	  return;

	case '`':
	  prefer_division_over_regexp = true;
	  extract_quotelike (tp, c);
	  tp->string_type = string_type_qq;
	  if (!extract_all)
	    interpolate_keywords (mlp, tp->string);
	  return;

	case '\'':
	  prefer_division_over_regexp = true;
	  extract_quotelike (tp, c);
	  tp->string_type = string_type_q;
	  return;

	case '(':
	  c = phase2_getc ();
	  if (c == ')')
	    {
	      continue;  /* Ignore empty list.  */
	    }
	  else
	    phase2_ungetc (c);
	  tp->type = token_type_lparen;
	  prefer_division_over_regexp = false;
	  return;

	case ')':
	  tp->type = token_type_rparen;
	  prefer_division_over_regexp = true;
	  return;

	case '{':
	  tp->type = token_type_lbrace;
	  prefer_division_over_regexp = false;
	  return;

	case '}':
	  tp->type = token_type_rbrace;
	  prefer_division_over_regexp = false;
	  return;

	case '[':
	  tp->type = token_type_lbracket;
	  prefer_division_over_regexp = false;
	  return;

	case ']':
	  tp->type = token_type_rbracket;
	  prefer_division_over_regexp = false;
	  return;

	case ';':
	  tp->type = token_type_semicolon;
	  prefer_division_over_regexp = false;
	  return;

	case ',':
	  tp->type = token_type_comma;
	  prefer_division_over_regexp = false;
	  return;

	case '=':
	  /* Check for fat comma.  */
	  c = phase1_getc ();
	  if (c == '>')
	    {
	      tp->type = token_type_fat_comma;
	      return;
	    }
	  else if (linepos == 2
		   && (last_token == token_type_semicolon
		       || last_token == token_type_rbrace)
		   && ((c >= 'A' && c <='Z')
		       || (c >= 'a' && c <= 'z')))
	    {
#if DEBUG_PERL
	      fprintf (stderr, "%s:%d: start pod section\n",
		       real_file_name, line_number);
#endif
	      skip_pod ();
#if DEBUG_PERL
	      fprintf (stderr, "%s:%d: end pod section\n",
		       real_file_name, line_number);
#endif
	      continue;
	    }
	  phase1_ungetc (c);
	  tp->type = token_type_other;
	  prefer_division_over_regexp = false;
	  return;

	case '<':
	  /* Check for <<EOF and friends.  */
	  prefer_division_over_regexp = false;
	  c = phase1_getc ();
	  if (c == '<')
	    {
	      c = phase1_getc ();
	      if (c == '\'')
		{
		  char *string;
		  extract_quotelike (tp, c);
		  string = get_here_document (tp->string);
		  free (tp->string);
		  tp->string = string;
		  tp->type = token_type_string;
		  tp->string_type = string_type_verbatim;
		  return;
		}
	      else if (c == '"')
		{
		  char *string;
		  extract_quotelike (tp, c);
		  string = get_here_document (tp->string);
		  free (tp->string);
		  tp->string = string;
		  tp->type = token_type_string;
		  tp->string_type = string_type_qq;
		  if (!extract_all)
		    interpolate_keywords (mlp, tp->string);
		  return;
		}
	      else if ((c >= 'A' && c <= 'Z')
		       || (c >= 'a' && c <= 'z')
		       || c == '_')
		{
		  bufpos = 0;
		  while ((c >= 'A' && c <= 'Z')
			 || (c >= 'a' && c <= 'z')
			 || (c >= '0' && c <= '9')
			 || c == '_' || c >= 0x80)
		    {
		      if (bufpos >= bufmax)
			{
			  bufmax += 100;
			  buffer = xrealloc_static (buffer, bufmax);
			}
		      buffer[bufpos++] = c;
		      c = phase1_getc ();
		    }
		  if (c == EOF)
		    {
		      tp->type = token_type_eof;
		      return;
		    }
		  else
		    {
		      char *string;

		      phase1_ungetc (c);
		      buffer[bufpos++] = '\0';
		      string = get_here_document (buffer);
		      tp->string = string;
		      tp->type = token_type_string;
		      tp->string_type = string_type_qq;
		      if (!extract_all)
			interpolate_keywords (mlp, tp->string);
		      return;
		    }
		}
	      else
		{
		  tp->type = token_type_other;
		  return;
		}
	    }
	  else
	    {
	      phase1_ungetc (c);
	      tp->type = token_type_other;
	    }
	  return;  /* End of case '>'.  */

	case '-':
	  /* Check for dereferencing operator.  */
	  c = phase1_getc ();
	  if (c == '>')
	    {
	      tp->type = token_type_dereference;
	      return;
	    }
	  phase1_ungetc (c);
	  tp->type = token_type_other;
	  prefer_division_over_regexp = false;
	  return;

	case '/':
	case '?':
	  if (!prefer_division_over_regexp)
	    {
	      extract_quotelike (tp, c);
	      if (!extract_all)
		interpolate_keywords (mlp, tp->string);
	      free (tp->string);
	      tp->type = token_type_other;
	      prefer_division_over_regexp = true;
	      /* Eat the following modifiers.  */
	      c = phase1_getc ();
	      while (c >= 'a' && c <= 'z')
		c = phase1_getc ();
	      phase1_ungetc (c);
	      return;
	    }
	  /* FALLTHROUGH */

	default:
	  /* We could carefully recognize each of the 2 and 3 character
	     operators, but it is not necessary, as we only need to recognize
	     gettext invocations.  Don't bother.  */
	  tp->type = token_type_other;
	  prefer_division_over_regexp = false;
	  return;
	}
    }
}

static token_ty *
x_perl_lex (message_list_ty *mlp)
{
#if DEBUG_PERL
  int dummy = stack_dump (token_stack);
#endif
  token_ty *tp = stack_shift (token_stack);

  if (!tp)
    {
      tp = (token_ty *) xmalloc (sizeof (token_ty));
      x_perl_prelex (mlp, tp);
#if DEBUG_PERL
      fprintf (stderr, "%s:%d: x_perl_prelex returned %s\n",
	       real_file_name, line_number, token2string (tp));
#endif
    }
#if DEBUG_PERL
  else
    {
      fprintf (stderr, "%s:%d: %s recycled from stack\n",
	       real_file_name, line_number, token2string (tp));
    }
#endif

  /* A symbol followed by a fat comma is really a single-quoted string.  */
  if (tp->type == token_type_symbol || tp->type == token_type_named_op)
    {
      token_ty *next = stack_head (token_stack);

      if (!next)
	{
#if DEBUG_PERL
	  fprintf (stderr, "%s:%d: pre-fetching next token\n",
		   real_file_name, line_number);
	  fflush (stderr);
#endif
	  next = x_perl_lex (mlp);
	  x_perl_unlex (next);
#if DEBUG_PERL
	  fprintf (stderr, "%s:%d: unshifted next token\n",
		   real_file_name, line_number);
#endif
	}

#if DEBUG_PERL
      fprintf (stderr, "%s:%d: next token is %s\n",
	       real_file_name, line_number, token2string (next));
#endif

      if (next->type == token_type_fat_comma)
	{
	  tp->type = token_type_string;
	  tp->string_type = string_type_q;
#if DEBUG_PERL
	  fprintf (stderr,
		   "%s:%d: token %s mutated to token_type_string\n",
		   real_file_name, line_number, token2string (tp));
#endif
	}
    }

  return tp;
}

static void
x_perl_unlex (token_ty *tp)
{
  stack_unshift (token_stack, tp, free_token);
}

/* ========================= Extracting strings.  ========================== */

static char *
collect_message (message_list_ty *mlp, token_ty *tp, int error_level)
{
  char *string;
  size_t len;

  extract_quotelike_pass3 (tp, error_level);
  string = xstrdup (tp->string);
  len = strlen (tp->string) + 1;

  for (;;)
    {
      int c = phase2_getc ();
      while (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f')
	c = phase2_getc ();
      if (c != '.')
	{
	  phase2_ungetc (c);
	  return string;
	}

      c = phase2_getc ();
      while (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f')
	c = phase2_getc ();
      phase2_ungetc (c);

      if (c == '"' || c == '\'' || c == '`'
	  || (!prefer_division_over_regexp && (c == '/' || c == '?'))
	  || c == 'q')
	{
	  token_ty *qstring = x_perl_lex (mlp);
	  if (qstring->type != token_type_string)
	    {
	      /* assert (qstring->type == token_type_symbol) */
	      x_perl_unlex (qstring);
	      return string;
	    }

	  extract_quotelike_pass3 (qstring, error_level);
	  len += strlen (qstring->string);
	  string = xrealloc (string, len);
	  strcat (string, qstring->string);
	  free_token (qstring);
	}
    }
}

/* The file is broken into tokens.  Scan the token stream, looking for
   a keyword, followed by a left paren, followed by a string.  When we
   see this sequence, we have something to remember.  We assume we are
   looking at a valid C or C++ program, and leave the complaints about
   the grammar to the compiler.

     Normal handling: Look for
       keyword ( ... msgid ... )
     Plural handling: Look for
       keyword ( ... msgid ... msgid_plural ... )

   We use recursion because the arguments before msgid or between msgid
   and msgid_plural can contain subexpressions of the same form.  */


/* Extract messages until the next balanced closing parenthesis.
   Extracted messages are added to MLP.

   When specific arguments shall be extracted, ARG_SG and ARG_PL are
   set to the corresponding argument number or -1 if not applicable.

   Returns the number of requested arguments consumed or -1 for eof.
   If - instead of consuming requested arguments - a complete message
   has been extracted, the return value will be sufficiently high to
   avoid any mis-interpretation.

   States are:

   0 - initial state
   1 - keyword has been seen
   2 - extractable string has been seen
   3 - a dot operator after an extractable string has been seen

   States 2 and 3 are "fragile", the parser will remain in state 2
   as long as only opening parentheses are seen, a transition to
   state 3 is done on appearance of a dot operator, all other tokens
   will cause the parser to fall back to state 1 or 0, eventually
   with an error message about invalid intermixing of constant and
   non-constant strings.

   Likewise, state 3 is fragile.  The parser will remain in state 3
   as long as only closing parentheses are seen, a transition to state
   2 is done on appearance of another (literal!) string, all other
   tokens will cause a warning.  */
static bool
extract_balanced (message_list_ty *mlp, int arg_sg, int arg_pl, int state,
		  token_type_ty delim)
{
  /* Remember the message containing the msgid, for msgid_plural.  */
  message_ty *plural_mp = NULL;

  /* The current argument for a possibly extracted keyword.  Counting
     starts with 1.  */
  int arg_count = 1;

  /* Number of left parentheses seen.  */
  int paren_seen = 0;

  /* The current token.  */
  token_ty *tp = NULL;

  token_type_ty last_token = token_type_eof;

#if DEBUG_PERL
  static int nesting_level = 0;

  ++nesting_level;
#endif

  for (;;)
    {
      int my_last_token = last_token;

      if (tp)
	free_token (tp);

      tp = x_perl_lex (mlp);

      last_token = tp->type;

      if (delim == tp->type)
	{
#if DEBUG_PERL
	  fprintf (stderr, "%s:%d: extract_balanced finished (%d)\n",
		   logical_file_name, tp->line_number, --nesting_level);
#endif
	  free_token (tp);
	  return false;
	}

      switch (tp->type)
	{
	case token_type_symbol:
#if DEBUG_PERL
	  fprintf (stderr, "%s:%d: type symbol (%d) \"%s\"\n",
		   logical_file_name, tp->line_number, nesting_level,
		   tp->string);
#endif

	  /* No need to bother if we extract all strings anyway.  */
	  if (!extract_all)
	    {
	      void *keyword_value;

	      if (find_entry (&keywords, tp->string, strlen (tp->string),
			      &keyword_value) == 0)
		{
		  last_token = token_type_keyword_symbol;

		  arg_sg = (int) (long) keyword_value & ((1 << 10) - 1);
		  arg_pl = (int) (long) keyword_value >> 10;
		  arg_count = 1;

		  state = 2;
		}
	    }
	  continue;

	case token_type_variable:
#if DEBUG_PERL
	  fprintf (stderr, "%s:%d: type variable (%d) \"%s\"\n",
		   logical_file_name, tp->line_number, nesting_level, tp->string);
#endif
	  prefer_division_over_regexp = true;
	  continue;

	case token_type_lparen:
#if DEBUG_PERL
	  fprintf (stderr, "%s:%d: type left parentheses (%d)\n",
		   logical_file_name, tp->line_number, nesting_level);
#endif
	  ++paren_seen;

	  /* No need to recurse if we extract all strings anyway.  */
	  if (extract_all)
	    continue;
	  else
	    {
	      if (extract_balanced (mlp, arg_sg - arg_count + 1,
				    arg_pl - arg_count + 1, state,
				    token_type_rparen))
		{
		  free_token (tp);
		  return true;
		}
	      if (my_last_token == token_type_keyword_symbol)
		arg_sg = arg_pl = -1;
	    }
	  continue;

	case token_type_rparen:
#if DEBUG_PERL
	  fprintf (stderr, "%s:%d: type right parentheses(%d)\n",
		   logical_file_name, tp->line_number, nesting_level);
#endif
	  --paren_seen;

	  /* No need to return if we extract all strings anyway.  */
	  if (extract_all)
	    continue;

	  continue;

	case token_type_comma:
	case token_type_fat_comma:
#if DEBUG_PERL
	  fprintf (stderr, "%s:%d: type comma (%d)\n",
		   logical_file_name, tp->line_number, nesting_level);
#endif
	  /* No need to bother if we extract all strings anyway.  */
	  if (extract_all)
	    continue;
	  ++arg_count;

	  if (arg_count > arg_sg && arg_count > arg_pl)
	    {
	      /* We have missed the argument.  */
	      arg_sg = arg_pl = -1;
	      arg_count = 0;
	    }
#if DEBUG_PERL
	  fprintf (stderr, "%s:%d: arg_count: %d, arg_sg: %d, arg_pl: %d\n",
		   real_file_name, tp->line_number,
		   arg_count, arg_sg, arg_pl);
#endif
	  continue;

	case token_type_string:
#if DEBUG_PERL
	  fprintf (stderr, "%s:%d: type string (%d): \"%s\"\n",
		   logical_file_name, tp->line_number, nesting_level,
		   tp->string);
#endif

	  if (extract_all)
	    {
	      lex_pos_ty pos;

	      pos.file_name = logical_file_name;
	      pos.line_number = tp->line_number;
	      remember_a_message (mlp, collect_message (mlp, tp,
							EXIT_SUCCESS),
				  &pos);
	    }
	  else if (state)
	    {
	      lex_pos_ty pos;

	      pos.file_name = logical_file_name;
	      pos.line_number = tp->line_number;

	      if (arg_count == arg_sg)
		{
		  plural_mp =
		    remember_a_message (mlp, collect_message (mlp, tp,
							      EXIT_FAILURE),
					&pos);
		  arg_sg = -1;
		}
	      else if (arg_count == arg_pl && plural_mp == NULL)
		{
		  if (plural_mp == NULL)
		    error (EXIT_FAILURE, 0, _("\
%s:%d: fatal: plural message seen before singular message\n"),
			   real_file_name, tp->line_number);
		}
	      else if (arg_count == arg_pl)
		{
		  remember_a_message_plural (plural_mp,
					     collect_message (mlp, tp,
							      EXIT_FAILURE),
					     &pos);
		  arg_pl = -1;
		}
	    }

	  if (arg_sg == -1 && arg_pl == -1)
	    {
	      state = 0;
	      plural_mp = NULL;
	    }

	  break;

	case token_type_eof:
#if DEBUG_PERL
	  fprintf (stderr, "%s:%d: type EOF (%d)\n",
		   logical_file_name, tp->line_number, nesting_level);
#endif
	  free_token (tp);
	  return true;

	case token_type_lbrace:
#if DEBUG_PERL
	  fprintf (stderr, "%s:%d: type lbrace (%d)\n",
		   logical_file_name, tp->line_number, nesting_level);
#endif
	  /* No need to recurse if we extract all strings anyway.  */
	  if (extract_all)
	    continue;
	  else
	    {
	      if (extract_balanced (mlp, -1, -1, 0, token_type_rbrace))
		{
		  free_token (tp);
		  return true;
		}
	    }
	  continue;

	case token_type_rbrace:
#if DEBUG_PERL
	  fprintf (stderr, "%s:%d: type rbrace (%d)\n",
		   logical_file_name, tp->line_number, nesting_level);
#endif
	  state = 0;
	  continue;

	case token_type_lbracket:
#if DEBUG_PERL
	  fprintf (stderr, "%s:%d: type lbracket (%d)\n",
		   logical_file_name, tp->line_number, nesting_level);
#endif
	  /* No need to recurse if we extract all strings anyway.  */
	  if (extract_all)
	    continue;
	  else
	    {
	      if (extract_balanced (mlp, -1, -1, 0, token_type_rbracket))
		{
		  free_token (tp);
		  return true;
		}
	    }
	  continue;

	case token_type_rbracket:
#if DEBUG_PERL
	  fprintf (stderr, "%s:%d: type rbracket (%d)\n",
		   logical_file_name, tp->line_number, nesting_level);
#endif
	  state = 0;
	  continue;

	case token_type_semicolon:
#if DEBUG_PERL
	  fprintf (stderr, "%s:%d: type semicolon (%d)\n",
		   logical_file_name, tp->line_number, nesting_level);
#endif
	  state = 0;

	  /* The ultimate sign.  */
	  arg_sg = arg_pl = -1;

	  continue;

	case token_type_dereference:
#if DEBUG_PERL
	  fprintf (stderr, "%s:%d: type dereference (%d)\n",
		   logical_file_name, tp->line_number, nesting_level);
#endif

	  continue;

	case token_type_dot:
#if DEBUG_PERL
	  fprintf (stderr, "%s:%d: type dot (%d)\n",
		   logical_file_name, tp->line_number, nesting_level);
#endif
	  state = 0;
	  continue;

	case token_type_named_op:
#if DEBUG_PERL
	  fprintf (stderr, "%s:%d: type named operator (%d): %s\n",
		   logical_file_name, tp->line_number, nesting_level,
		   tp->string);
#endif
	  state = 0;
	  continue;

	case token_type_regex_op:
#if DEBUG_PERL
	  fprintf (stderr, "%s:%d: type regex operator (%d)\n",
		   logical_file_name, tp->line_number, nesting_level);
#endif
	  continue;

	case token_type_other:
#if DEBUG_PERL
	  fprintf (stderr, "%s:%d: type other (%d)\n",
		   logical_file_name, tp->line_number, nesting_level);
#endif
	  state = 0;
	  continue;

	default:
	  fprintf (stderr, "%s:%d: unknown token type %d\n",
		   real_file_name, tp->line_number, tp->type);
	  abort ();
	}
    }
}

void
extract_perl (FILE *f, const char *real_filename, const char *logical_filename,
	      msgdomain_list_ty *mdlp)
{
  message_list_ty *mlp = mdlp->item[0]->messages;

  fp = f;
  real_file_name = real_filename;
  logical_file_name = xstrdup (logical_filename);
  line_number = 0;

  last_comment_line = -1;
  last_non_comment_line = -1;

  last_token = token_type_semicolon;  /* Safe assumption.  */
  prefer_division_over_regexp = false;

  last_string_finished = false;

  init_keywords ();

  token_stack = (struct stack *) xmalloc (sizeof (struct stack));
  here_eaten = 0;
  end_of_file = false;

  /* Eat tokens until eof is seen.  When extract_balanced returns
     due to an unbalanced closing brace, just restart it.  */
  while (!extract_balanced (mlp, -1, -1, 0, token_type_rbrace))
    ;

  fp = NULL;
  real_file_name = NULL;
  free (logical_file_name);
  logical_file_name = NULL;
  line_number = 0;
  last_token = token_type_semicolon;
  last_string_finished = false;
  stack_free (token_stack);
  free (token_stack);
  token_stack = NULL;
  here_eaten = 0;
  end_of_file = true;
}
