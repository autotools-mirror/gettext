/* xgettext C/C++/ObjectiveC backend.
   Copyright (C) 1995-1998, 2000-2002 Free Software Foundation, Inc.

   This file was written by Peter Miller <millerp@canb.auug.org.au>

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
#include "x-c.h"
#include "xgettext.h"
#include "error.h"
#include "progname.h"
#include "xmalloc.h"
#include "exit.h"
#include "hash.h"
#include "gettext.h"

#define _(s) gettext(s)

#if HAVE_C_BACKSLASH_A
# define ALERT_CHAR '\a'
#else
# define ALERT_CHAR '\7'
#endif


/* The ANSI C standard defines several phases of translation:

   1. Terminate line by \n, regardless of the external representation
      of a text line.  Stdio does this for us.

   2. Convert trigraphs to their single character equivalents.

   3. Concatenate each line ending in backslash (\) with the following
      line.

   4. Replace each comment with a space character.

   5. Parse each resulting logical line as preprocessing tokens a
      white space.

   6. Recognize and carry out directives (it also expands macros on
      non-directive lines, which we do not do here).

   7. Replaces escape sequences within character strings with their
      single character equivalents (we do this in step 5, because we
      don't have to worry about the #include argument).

   8. Concatenates adjacent string literals to form single string
      literals (because we don't expand macros, there are a few things
      we will miss).

   9. Converts the remaining preprocessing tokens to C tokens and
      discards any white space from the translation unit.

   This lexer implements the above, and presents the scanner (in
   xgettext.c) with a stream of C tokens.  The comments are
   accumulated in a buffer, and given to xgettext when asked for.  */

enum xgettext_token_type_ty
{
  xgettext_token_type_eof,
  xgettext_token_type_keyword,
  xgettext_token_type_lparen,
  xgettext_token_type_rparen,
  xgettext_token_type_comma,
  xgettext_token_type_string_literal,
  xgettext_token_type_symbol
};
typedef enum xgettext_token_type_ty xgettext_token_type_ty;

typedef struct xgettext_token_ty xgettext_token_ty;
struct xgettext_token_ty
{
  xgettext_token_type_ty type;

  /* These fields are used only for xgettext_token_type_keyword.  */
  int argnum1;
  int argnum2;

  /* This field is used only for xgettext_token_type_string_literal.  */
  char *string;

  /* These fields are only for
       xgettext_token_type_keyword,
       xgettext_token_type_string_literal.  */
  lex_pos_ty pos;
};


enum token_type_ty
{
  token_type_character_constant,
  token_type_eof,
  token_type_eoln,
  token_type_hash,
  token_type_lparen,
  token_type_rparen,
  token_type_comma,
  token_type_name,
  token_type_number,
  token_type_string_literal,
  token_type_symbol,
  token_type_white_space
};
typedef enum token_type_ty token_type_ty;

typedef struct token_ty token_ty;
struct token_ty
{
  token_type_ty type;
  char *string;		/* for token_type_name, token_type_string_literal */
  long number;
  int line_number;
};


/* Prototypes for local functions.  Needed to ensure compiler checking of
   function argument counts despite of K&R C function definition syntax.  */
static void init_keywords PARAMS ((void));
static int phase1_getc PARAMS ((void));
static void phase1_ungetc PARAMS ((int c));
static int phase2_getc PARAMS ((void));
static void phase2_ungetc PARAMS ((int c));
static int phase3_getc PARAMS ((void));
static void phase3_ungetc PARAMS ((int c));
static inline void comment_start PARAMS ((void));
static inline void comment_add PARAMS ((int c));
static inline void comment_line_end PARAMS ((size_t chars_to_remove));
static int phase4_getc PARAMS ((void));
static void phase4_ungetc PARAMS ((int c));
static int phase7_getc PARAMS ((void));
static void phase7_ungetc PARAMS ((int c));
static inline void free_token PARAMS ((token_ty *tp));
static void phase5_get PARAMS ((token_ty *tp));
static void phase5_unget PARAMS ((token_ty *tp));
static void phaseX_get PARAMS ((token_ty *tp));
static void phase6_get PARAMS ((token_ty *tp));
static void phase6_unget PARAMS ((token_ty *tp));
static bool is_inttypes_macro PARAMS ((const char *name));
static void phase8a_get PARAMS ((token_ty *tp));
static void phase8a_unget PARAMS ((token_ty *tp));
static void phase8_get PARAMS ((token_ty *tp));
static void x_c_lex PARAMS ((xgettext_token_ty *tp));
static bool extract_parenthesized PARAMS ((message_list_ty *mlp,
					   int commas_to_skip,
					   int plural_commas));


/* ========================= Lexer customization.  ========================= */

static bool trigraphs = false;

void
x_c_trigraphs ()
{
  trigraphs = true;
}


/* ====================== Keyword set customization.  ====================== */

/* If true extract all strings.  */
static bool extract_all = false;

static hash_table keywords;
static bool default_keywords = true;


void
x_c_extract_all ()
{
  extract_all = true;
}


void
x_c_keyword (name)
     const char *name;
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

bool
x_c_any_keywords ()
{
  return (keywords.filled > 0) || default_keywords;
}

/* Finish initializing the keywords hash table.
   Called after argument processing, before each file is processed.  */
static void
init_keywords ()
{
  if (default_keywords)
    {
      x_c_keyword ("gettext");
      x_c_keyword ("dgettext:2");
      x_c_keyword ("dcgettext:2");
      x_c_keyword ("ngettext:1,2");
      x_c_keyword ("dngettext:2,3");
      x_c_keyword ("dcngettext:2,3");
      x_c_keyword ("gettext_noop");
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


/* 1. Terminate line by \n, regardless of the external representation of
   a text line.  Stdio does this for us, we just need to check that
   there are no I/O errors, and cope with potentially 2 characters of
   pushback, not just the one that ungetc can cope with.  */

/* Maximum used guaranteed to be < 4.  */
static unsigned char phase1_pushback[4];
static int phase1_pushback_length;


static int
phase1_getc ()
{
  int c;

  if (phase1_pushback_length)
    {
      c = phase1_pushback[--phase1_pushback_length];
      if (c == '\n')
	++line_number;
      return c;
    }
  for (;;)
    {
      c = getc (fp);
      switch (c)
	{
	case EOF:
	  if (ferror (fp))
	    {
	    bomb:
	      error (EXIT_FAILURE, errno, _("\
error while reading \"%s\""), real_file_name);
	    }
	  return EOF;

	case '\n':
	  ++line_number;
	  return '\n';

	case '\\':
	  c = getc (fp);
	  if (c == EOF)
	    {
	      if (ferror (fp))
		goto bomb;
	      return '\\';
	    }
	  if (c != '\n')
	    {
	      ungetc (c, fp);
	      return '\\';
	    }
	  ++line_number;
	  break;

	default:
	  return c;
	}
    }
}


static void
phase1_ungetc (c)
     int c;
{
  switch (c)
    {
    case EOF:
      break;

    case '\n':
      --line_number;
      /* FALLTHROUGH */

    default:
      phase1_pushback[phase1_pushback_length++] = c;
      break;
    }
}


/* 2. Convert trigraphs to their single character equivalents.  Most
   sane human beings vomit copiously at the mention of trigraphs, which
   is why they are an option.  */

/* Maximum used guaranteed to be < 4.  */
static unsigned char phase2_pushback[4];
static int phase2_pushback_length;


static int
phase2_getc ()
{
  int c;

  if (phase2_pushback_length)
    return phase2_pushback[--phase2_pushback_length];
  if (!trigraphs)
    return phase1_getc ();

  c = phase1_getc ();
  if (c != '?')
    return c;
  c = phase1_getc ();
  if (c != '?')
    {
      phase1_ungetc (c);
      return '?';
    }
  c = phase1_getc ();
  switch (c)
    {
    case '(':
      return '[';
    case '/':
      return '\\';
    case ')':
      return ']';
    case '\'':
      return '^';
    case '<':
      return '{';
    case '!':
      return '|';
    case '>':
      return '}';
    case '-':
      return '~';
    case '#':
      return '=';
    }
  phase1_ungetc (c);
  phase1_ungetc ('?');
  return '?';
}


static void
phase2_ungetc (c)
     int c;
{
  if (c != EOF)
    phase2_pushback[phase2_pushback_length++] = c;
}


/* 3. Concatenate each line ending in backslash (\) with the following
   line.  Basically, all you need to do is elide "\\\n" sequences from
   the input.  */

/* Maximum used guaranteed to be < 4.  */
static unsigned char phase3_pushback[4];
static int phase3_pushback_length;


static int
phase3_getc ()
{
  if (phase3_pushback_length)
    return phase3_pushback[--phase3_pushback_length];
  for (;;)
    {
      int c = phase2_getc ();
      if (c != '\\')
	return c;
      c = phase2_getc ();
      if (c != '\n')
	{
	  phase2_ungetc (c);
	  return '\\';
	}
    }
}


static void
phase3_ungetc (c)
     int c;
{
  if (c != EOF)
    phase3_pushback[phase3_pushback_length++] = c;
}


/* Accumulating comments.  */

static char *buffer;
static size_t bufmax;
static size_t buflen;

static inline void
comment_start ()
{
  buflen = 0;
}

static inline void
comment_add (c)
     int c;
{
  if (buflen >= bufmax)
    {
      bufmax += 100;
      buffer = xrealloc (buffer, bufmax);
    }
  buffer[buflen++] = c;
}

static inline void
comment_line_end (chars_to_remove)
     size_t chars_to_remove;
{
  buflen -= chars_to_remove;
  while (buflen >= 1
	 && (buffer[buflen - 1] == ' ' || buffer[buflen - 1] == '\t'))
    --buflen;
  if (chars_to_remove == 0 && buflen >= bufmax)
    {
      bufmax += 100;
      buffer = xrealloc (buffer, bufmax);
    }
  buffer[buflen] = '\0';
  xgettext_comment_add (buffer);
}


/* These are for tracking whether comments count as immediately before
   keyword.  */
static int last_comment_line;
static int last_non_comment_line;
static int newline_count;


/* 4. Replace each comment that is not inside a character constant or
   string literal with a space character.  We need to remember the
   comment for later, because it may be attached to a keyword string.
   We also optionally understand C++ comments.  */

static int
phase4_getc ()
{
  int c;
  bool last_was_star;

  c = phase3_getc ();
  if (c != '/')
    return c;
  c = phase3_getc ();
  switch (c)
    {
    default:
      phase3_ungetc (c);
      return '/';

    case '*':
      /* C comment.  */
      comment_start ();
      last_was_star = false;
      for (;;)
	{
	  c = phase3_getc ();
	  if (c == EOF)
	    break;
	  /* We skip all leading white space, but not EOLs.  */
	  if (!(buflen == 0 && (c == ' ' || c == '\t')))
	    comment_add (c);
	  switch (c)
	    {
	    case '\n':
	      comment_line_end (1);
	      comment_start ();
	      last_was_star = false;
	      continue;

	    case '*':
	      last_was_star = true;
	      continue;

	    case '/':
	      if (last_was_star)
		{
		  comment_line_end (2);
		  break;
		}
	      /* FALLTHROUGH */

	    default:
	      last_was_star = false;
	      continue;
	    }
	  break;
	}
      last_comment_line = newline_count;
      return ' ';

    case '/':
      /* C++ or ISO C 99 comment.  */
      comment_start ();
      for (;;)
	{
	  c = phase3_getc ();
	  if (c == '\n' || c == EOF)
	    break;
	  comment_add (c);
	}
      comment_line_end (0);
      last_comment_line = newline_count;
      return '\n';
    }
}


static void
phase4_ungetc (c)
     int c;
{
  phase3_ungetc (c);
}


/* 7. Replace escape sequences within character strings with their
   single character equivalents.  This is called from phase 5, because
   we don't have to worry about the #include argument.  There are
   pathological cases which could bite us (like the DOS directory
   separator), but just pretend it can't happen.  */

#define P7_QUOTES (1000 + '"')
#define P7_QUOTE (1000 + '\'')
#define P7_NEWLINE (1000 + '\n')

static int
phase7_getc ()
{
  int c, n, j;

  /* Use phase 3, because phase 4 elides comments.  */
  c = phase3_getc ();

  /* Return a magic newline indicator, so that we can distinguish
     between the user requesting a newline in the string (e.g. using
     "\n" or "\012") from the user failing to terminate the string or
     character constant.  The ANSI C standard says: 3.1.3.4 Character
     Constants contain ``any character except single quote, backslash or
     newline; or an escape sequence'' and 3.1.4 String Literals contain
     ``any character except double quote, backslash or newline; or an
     escape sequence''.

     Most compilers give a fatal error in this case, however gcc is
     stupidly silent, even though this is a very common typo.  OK, so
     gcc --pedantic will tell me, but that gripes about too much other
     stuff.  Could I have a ``gcc -Wnewline-in-string'' option, or
     better yet a ``gcc -fno-newline-in-string'' option, please?  Gcc is
     also inconsistent between string literals and character constants:
     you may not embed newlines in character constants; try it, you get
     a useful diagnostic.  --PMiller  */
  if (c == '\n')
    return P7_NEWLINE;

  if (c == '"')
    return P7_QUOTES;
  if (c == '\'')
    return P7_QUOTE;
  if (c != '\\')
    return c;
  c = phase3_getc ();
  switch (c)
    {
    default:
      /* Unknown escape sequences really should be an error, but just
	 ignore them, and let the real compiler complain.  */
      phase3_ungetc (c);
      return '\\';

    case '"':
    case '\'':
    case '?':
    case '\\':
      return c;

    case 'a':
      return ALERT_CHAR;
    case 'b':
      return '\b';

      /* The \e escape is preculiar to gcc, and assumes an ASCII
         character set (or superset).  We don't provide support for it
         here.  */

    case 'f':
      return '\f';
    case 'n':
      return '\n';
    case 'r':
      return '\r';
    case 't':
      return '\t';
    case 'v':
      return '\v';

    case 'x':
      c = phase3_getc ();
      switch (c)
	{
	default:
	  phase3_ungetc (c);
	  phase3_ungetc ('x');
	  return '\\';

	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
	  break;
	}
      n = 0;
      for (;;)
	{
	  switch (c)
	    {
	    default:
	      phase3_ungetc (c);
	      return n;
	      break;

	    case '0': case '1': case '2': case '3': case '4':
	    case '5': case '6': case '7': case '8': case '9':
	      n = n * 16 + c - '0';
	      break;

	    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
	      n = n * 16 + 10 + c - 'A';
	      break;

	    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
	      n = n * 16 + 10 + c - 'a';
	      break;
	    }
	  c = phase3_getc ();
	}
      return n;

    case '0': case '1': case '2': case '3':
    case '4': case '5': case '6': case '7':
      n = 0;
      for (j = 0; j < 3; ++j)
	{
	  n = n * 8 + c - '0';
	  c = phase3_getc ();
	  switch (c)
	    {
	    default:
	      break;

	    case '0': case '1': case '2': case '3':
	    case '4': case '5': case '6': case '7':
	      continue;
	    }
	  break;
	}
      phase3_ungetc (c);
      return n;
    }
}


static void
phase7_ungetc (c)
     int c;
{
  phase3_ungetc (c);
}


/* Free the memory pointed to by a 'struct token_ty'.  */
static inline void
free_token (tp)
     token_ty *tp;
{
  if (tp->type == token_type_name || tp->type == token_type_string_literal)
    free (tp->string);
}


/* 5. Parse each resulting logical line as preprocessing tokens and
   white space.  Preprocessing tokens and C tokens don't always match.  */

/* Maximum used guaranteed to be < 4.  */
static token_ty phase5_pushback[4];
static int phase5_pushback_length;


static void
phase5_get (tp)
     token_ty *tp;
{
  static char *buffer;
  static int bufmax;
  int bufpos;
  int c;

  if (phase5_pushback_length)
    {
      *tp = phase5_pushback[--phase5_pushback_length];
      return;
    }
  tp->string = NULL;
  tp->number = 0;
  tp->line_number = line_number;
  c = phase4_getc ();
  switch (c)
    {
    case EOF:
      tp->type = token_type_eof;
      return;

    case '\n':
      tp->type = token_type_eoln;
      return;

    case ' ':
    case '\f':
    case '\t':
      for (;;)
	{
	  c = phase4_getc ();
	  switch (c)
	    {
	    case ' ':
	    case '\f':
	    case '\t':
	      continue;

	    default:
	      phase4_ungetc (c);
	      break;
	    }
	  break;
	}
      tp->type = token_type_white_space;
      return;

    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
    case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
    case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
    case 'V': case 'W': case 'X': case 'Y': case 'Z':
    case '_':
    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
    case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
    case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
    case 'v': case 'w': case 'x': case 'y': case 'z':
      bufpos = 0;
      for (;;)
	{
	  if (bufpos >= bufmax)
	    {
	      bufmax += 100;
	      buffer = xrealloc (buffer, bufmax);
	    }
	  buffer[bufpos++] = c;
	  c = phase4_getc ();
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
	      phase4_ungetc (c);
	      break;
	    }
	  break;
	}
      if (bufpos >= bufmax)
	{
	  bufmax += 100;
	  buffer = xrealloc (buffer, bufmax);
	}
      buffer[bufpos] = 0;
      tp->string = xstrdup (buffer);
      tp->type = token_type_name;
      return;

    case '.':
      c = phase4_getc ();
      phase4_ungetc (c);
      switch (c)
	{
	default:
	  tp->type = token_type_symbol;
	  return;

	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	  c = '.';
	  break;
	}
      /* FALLTHROUGH */

    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      /* The preprocessing number token is more "generous" than the C
	 number tokens.  This is mostly due to token pasting (another
	 thing we can ignore here).  */
      bufpos = 0;
      for (;;)
	{
	  if (bufpos >= bufmax)
	    {
	      bufmax += 100;
	      buffer = xrealloc (buffer, bufmax);
	    }
	  buffer[bufpos++] = c;
	  c = phase4_getc ();
	  switch (c)
	    {
	    case 'e':
	    case 'E':
	      if (bufpos >= bufmax)
		{
		  bufmax += 100;
		  buffer = xrealloc (buffer, bufmax);
		}
	      buffer[bufpos++] = c;
	      c = phase4_getc ();
	      if (c != '+' || c != '-')
		{
		  phase4_ungetc (c);
		  break;
		}
	      continue;

	    case 'A': case 'B': case 'C': case 'D':           case 'F':
	    case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
	    case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
	    case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
	    case 'Y': case 'Z':
	    case 'a': case 'b': case 'c': case 'd':           case 'f':
	    case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
	    case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
	    case 's': case 't': case 'u': case 'v': case 'w': case 'x':
	    case 'y': case 'z':
	    case '0': case '1': case '2': case '3': case '4':
	    case '5': case '6': case '7': case '8': case '9':
	    case '.':
	      continue;

	    default:
	      phase4_ungetc (c);
	      break;
	    }
	  break;
	}
      if (bufpos >= bufmax)
	{
	  bufmax += 100;
	  buffer = xrealloc (buffer, bufmax);
	}
      buffer[bufpos] = 0;
      tp->type = token_type_number;
      tp->number = atol (buffer);
      return;

    case '\'':
      /* We could worry about the 'L' before wide character constants,
	 but ignoring it has no effect unless one of the keywords is
	 "L".  Just pretend it won't happen.  Also, we don't need to
	 remember the character constant.  */
      for (;;)
	{
	  c = phase7_getc ();
	  if (c == P7_NEWLINE)
	    {
	      error_with_progname = false;
	      error (0, 0, _("%s:%d: warning: unterminated character constant"),
		     logical_file_name, line_number - 1);
	      error_with_progname = true;
	      phase7_ungetc ('\n');
	      break;
	    }
	  if (c == EOF || c == P7_QUOTE)
	    break;
	}
      tp->type = token_type_character_constant;
      return;

    case '"':
      /* We could worry about the 'L' before wide string constants,
	 but since gettext's argument is not a wide character string,
	 let the compiler complain about the argument not matching the
	 prototype.  Just pretend it won't happen.  */
      bufpos = 0;
      for (;;)
	{
	  c = phase7_getc ();
	  if (c == P7_NEWLINE)
	    {
	      error_with_progname = false;
	      error (0, 0, _("%s:%d: warning: unterminated string literal"),
		     logical_file_name, line_number - 1);
	      error_with_progname = true;
	      phase7_ungetc ('\n');
	      break;
	    }
	  if (c == EOF || c == P7_QUOTES)
	    break;
	  if (c == P7_QUOTE)
	    c = '\'';
	  if (bufpos >= bufmax)
	    {
	      bufmax += 100;
	      buffer = xrealloc (buffer, bufmax);
	    }
	  buffer[bufpos++] = c;
	}
      if (bufpos >= bufmax)
	{
	  bufmax += 100;
	  buffer = xrealloc (buffer, bufmax);
	}
      buffer[bufpos] = 0;
      tp->type = token_type_string_literal;
      tp->string = xstrdup (buffer);
      return;

    case '(':
      tp->type = token_type_lparen;
      return;

    case ')':
      tp->type = token_type_rparen;
      return;

    case ',':
      tp->type = token_type_comma;
      return;

    case '#':
      tp->type = token_type_hash;
      return;

    default:
      /* We could carefully recognize each of the 2 and 3 character
        operators, but it is not necessary, as we only need to recognize
        gettext invocations.  Don't bother.  */
      tp->type = token_type_symbol;
      return;
    }
}


static void
phase5_unget (tp)
     token_ty *tp;
{
  if (tp->type != token_type_eof)
    phase5_pushback[phase5_pushback_length++] = *tp;
}


/* X. Recognize a leading # symbol.  Leave leading hash as a hash, but
   turn hash in the middle of a line into a plain symbol token.  This
   makes the phase 6 easier.  */

static void
phaseX_get (tp)
     token_ty *tp;
{
  static bool middle;	/* false at the beginning of a line, true otherwise.  */

  phase5_get (tp);

  if (tp->type == token_type_eoln || tp->type == token_type_eof)
    middle = false;
  else
    {
      if (middle)
	{
	  /* Turn hash in the middle of a line into a plain symbol token.  */
	  if (tp->type == token_type_hash)
	    tp->type = token_type_symbol;
	}
      else
	{
	  /* When we see leading whitespace followed by a hash sign,
	     discard the leading white space token.  The hash is all
	     phase 6 is interested in.  */
	  if (tp->type == token_type_white_space)
	    {
	      token_ty next;

	      phase5_get (&next);
	      if (next.type == token_type_hash)
		*tp = next;
	      else
		phase5_unget (&next);
	    }
	  middle = true;
	}
    }
}


/* 6. Recognize and carry out directives (it also expands macros on
   non-directive lines, which we do not do here).  The only directive
   we care about are the #line and #define directive.  We throw all the
   others away.  */

/* Maximum used guaranteed to be < 4.  */
static token_ty phase6_pushback[4];
static int phase6_pushback_length;


static void
phase6_get (tp)
     token_ty *tp;
{
  static token_ty *buf;
  static int bufmax;
  int bufpos;
  int j;

  if (phase6_pushback_length)
    {
      *tp = phase6_pushback[--phase6_pushback_length];
      return;
    }
  for (;;)
    {
      /* Get the next token.  If it is not a '#' at the beginning of a
	 line (ignoring whitespace), return immediately.  */
      phaseX_get (tp);
      if (tp->type != token_type_hash)
	return;

      /* Accumulate the rest of the directive in a buffer, until the
	 "define" keyword is seen or until end of line.  */
      bufpos = 0;
      for (;;)
	{
	  phaseX_get (tp);
	  if (tp->type == token_type_eoln || tp->type == token_type_eof)
	    break;

	  /* Before the "define" keyword and inside other directives
	     white space is irrelevant.  So just throw it away.  */
	  if (tp->type != token_type_white_space)
	    {
	      /* If it is a #define directive, return immediately,
		 thus treating the body of the #define directive like
		 normal input.  */
	      if (bufpos == 0
		  && tp->type == token_type_name
		  && strcmp (tp->string, "define") == 0)
		return;

	      /* Accumulate.  */
	      if (bufpos >= bufmax)
		{
		  bufmax += 100;
		  buf = xrealloc (buf, bufmax * sizeof (buf[0]));
		}
	      buf[bufpos++] = *tp;
	    }
	}

      /* If it is a #line directive, with no macros to expand, act on
	 it.  Ignore all other directives.  */
      if (bufpos >= 3 && buf[0].type == token_type_name
	  && strcmp (buf[0].string, "line") == 0
	  && buf[1].type == token_type_number
	  && buf[2].type == token_type_string_literal)
	{
	  logical_file_name = xstrdup (buf[2].string);
	  line_number = buf[1].number;
	}
      if (bufpos >= 2 && buf[0].type == token_type_number
	  && buf[1].type == token_type_string_literal)
	{
	  logical_file_name = xstrdup (buf[1].string);
	  line_number = buf[0].number;
	}

      /* Release the storage held by the directive.  */
      for (j = 0; j < bufpos; ++j)
	free_token (&buf[j]);

      /* We must reset the selected comments.  */
      xgettext_comment_reset ();
    }
}


static void
phase6_unget (tp)
     token_ty *tp;
{
  if (tp->type != token_type_eof)
    phase6_pushback[phase6_pushback_length++] = *tp;
}


/* 8a. Convert ISO C 99 section 7.8.1 format string directives to string
   literal placeholders.  */

/* Test for an ISO C 99 section 7.8.1 format string directive.  */
static bool
is_inttypes_macro (name)
     const char *name;
{
  /* Syntax:
     P R I { d | i | o | u | x | X }
     { { | LEAST | FAST } { 8 | 16 | 32 | 64 } | MAX | PTR }  */
  if (name[0] == 'P' && name[1] == 'R' && name[2] == 'I')
    {
      name += 3;
      if (name[0] == 'd' || name[0] == 'i' || name[0] == 'o' || name[0] == 'u'
	  || name[0] == 'x' || name[0] == 'X')
	{
	  name += 1;
	  if (name[0] == 'M' && name[1] == 'A' && name[2] == 'X'
	      && name[3] == '\0')
	    return true;
	  if (name[0] == 'P' && name[1] == 'T' && name[2] == 'R'
	      && name[3] == '\0')
	    return true;
	  if (name[0] == 'L' && name[1] == 'E' && name[2] == 'A'
	      && name[3] == 'S' && name[4] == 'T')
	    name += 5;
	  else if (name[0] == 'F' && name[1] == 'A' && name[2] == 'S'
		   && name[3] == 'T')
	    name += 4;
	  if (name[0] == '8' && name[1] == '\0')
	    return true;
	  if (name[0] == '1' && name[1] == '6' && name[2] == '\0')
	    return true;
	  if (name[0] == '3' && name[1] == '2' && name[2] == '\0')
	    return true;
	  if (name[0] == '6' && name[1] == '4' && name[2] == '\0')
	    return true;
	}
    }
  return false;
}

static void
phase8a_get (tp)
     token_ty *tp;
{
  phase6_get (tp);
  if (tp->type == token_type_name && is_inttypes_macro (tp->string))
    {
      /* Turn PRIdXXX into "<PRIdXXX>".  */
      size_t len = strlen (tp->string);
      char *new_string = (char *) xmalloc (len + 3);
      new_string[0] = '<';
      memcpy (new_string + 1, tp->string, len);
      new_string[len + 1] = '>';
      new_string[len + 2] = '\0';
      free (tp->string);
      tp->string = new_string;
      tp->type = token_type_string_literal;
    }
}

static void
phase8a_unget (tp)
     token_ty *tp;
{
  phase6_unget (tp);
}


/* 8. Concatenate adjacent string literals to form single string
   literals (because we don't expand macros, there are a few things we
   will miss).  */

static void
phase8_get (tp)
     token_ty *tp;
{
  phase8a_get (tp);
  if (tp->type != token_type_string_literal)
    return;
  for (;;)
    {
      token_ty tmp;
      size_t len;

      phase8a_get (&tmp);
      if (tmp.type == token_type_white_space)
	continue;
      if (tmp.type == token_type_eoln)
	continue;
      if (tmp.type != token_type_string_literal)
	{
	  phase8a_unget (&tmp);
	  return;
	}
      len = strlen (tp->string);
      tp->string = xrealloc (tp->string, len + strlen (tmp.string) + 1);
      strcpy (tp->string + len, tmp.string);
      free (tmp.string);
    }
}


/* 9. Convert the remaining preprocessing tokens to C tokens and
   discards any white space from the translation unit.  */

static void
x_c_lex (tp)
     xgettext_token_ty *tp;
{
  for (;;)
    {
      token_ty token;
      void *keyword_value;

      phase8_get (&token);
      switch (token.type)
	{
	case token_type_eof:
	  tp->type = xgettext_token_type_eof;
	  return;

	case token_type_white_space:
	  break;

	case token_type_eoln:
	  /* We have to track the last occurrence of a string.  One
	     mode of xgettext allows to group an extracted message
	     with a comment for documentation.  The rule which states
	     which comment is assumed to be grouped with the message
	     says it should immediately precede it.  Our
	     interpretation: between the last line of the comment and
	     the line in which the keyword is found must be no line
	     with non-white space tokens.  */
	  ++newline_count;
	  if (last_non_comment_line > last_comment_line)
	    xgettext_comment_reset ();
	  break;

	case token_type_name:
	  last_non_comment_line = newline_count;

	  if (find_entry (&keywords, token.string, strlen (token.string),
			  &keyword_value)
	      == 0)
	    {
	      tp->type = xgettext_token_type_keyword;
	      tp->argnum1 = (int) (long) keyword_value & ((1 << 10) - 1);
	      tp->argnum2 = (int) (long) keyword_value >> 10;
	      tp->pos.file_name = logical_file_name;
	      tp->pos.line_number = token.line_number;
	    }
	  else
	    tp->type = xgettext_token_type_symbol;
	  free (token.string);
	  return;

	case token_type_lparen:
	  last_non_comment_line = newline_count;

	  tp->type = xgettext_token_type_lparen;
	  return;

	case token_type_rparen:
	  last_non_comment_line = newline_count;

	  tp->type = xgettext_token_type_rparen;
	  return;

	case token_type_comma:
	  last_non_comment_line = newline_count;

	  tp->type = xgettext_token_type_comma;
	  return;

	case token_type_string_literal:
	  last_non_comment_line = newline_count;

	  tp->type = xgettext_token_type_string_literal;
	  tp->string = token.string;
	  tp->pos.file_name = logical_file_name;
	  tp->pos.line_number = token.line_number;
	  return;

	default:
	  last_non_comment_line = newline_count;

	  tp->type = xgettext_token_type_symbol;
	  return;
	}
    }
}


/* ========================= Extracting strings.  ========================== */

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
   When a specific argument shall be extracted, COMMAS_TO_SKIP >= 0 and,
   if also a plural argument shall be extracted, PLURAL_COMMAS > 0,
   otherwise PLURAL_COMMAS = 0.
   When no specific argument shall be extracted, COMMAS_TO_SKIP < 0.
   Return true upon eof, false upon closing parenthesis.  */
static bool
extract_parenthesized (mlp, commas_to_skip, plural_commas)
     message_list_ty *mlp;
     int commas_to_skip;
     int plural_commas;
{
  /* Remember the message containing the msgid, for msgid_plural.  */
  message_ty *plural_mp = NULL;

  /* 0 when no keyword has been seen.  1 right after a keyword is seen.  */
  int state;
  /* Parameters of the keyword just seen.  Defined only in state 1.  */
  int next_commas_to_skip = -1;
  int next_plural_commas = 0;

  /* Start state is 0.  */
  state = 0;

  for (;;)
    {
      xgettext_token_ty token;

      x_c_lex (&token);
      switch (token.type)
	{
	case xgettext_token_type_keyword:
	  /* No need to bother if we extract all strings anyway.  */
	  if (extract_all)
	    continue;
	  next_commas_to_skip = token.argnum1 - 1;
	  next_plural_commas = (token.argnum2 > token.argnum1
				? token.argnum2 - token.argnum1 : 0);
	  state = 1;
	  continue;

	case xgettext_token_type_lparen:
	  /* No need to recurse if we extract all strings anyway.  */
	  if (extract_all)
	    continue;
	  if (state
	      ?  extract_parenthesized (mlp, next_commas_to_skip,
					next_plural_commas)
	      : extract_parenthesized (mlp, -1, 0))
	    return true;
	  state = 0;
	  continue;

	case xgettext_token_type_rparen:
	  /* No need to return if we extract all strings anyway.  */
	  if (extract_all)
	    continue;
	  return false;

	case xgettext_token_type_comma:
	  /* No need to bother if we extract all strings anyway.  */
	  if (extract_all)
	    continue;
	  if (commas_to_skip >= 0)
	    {
	      if (commas_to_skip > 0)
		commas_to_skip--;
	      else
		if (plural_mp != NULL && plural_commas > 0)
		  {
		    commas_to_skip = plural_commas - 1;
		    plural_commas = 0;
		  }
		else
		  commas_to_skip = -1;
	    }
	  state = 0;
	  continue;

	case xgettext_token_type_string_literal:
	  if (extract_all)
	    remember_a_message (mlp, token.string, &token.pos);
	  else
	    {
	      if (commas_to_skip == 0)
		{
		  if (plural_mp == NULL)
		    {
		      /* Seen an msgid.  */
		      message_ty *mp = remember_a_message (mlp, token.string,
							   &token.pos);
		      if (plural_commas > 0)
			plural_mp = mp;
		    }
		  else
		    {
		      /* Seen an msgid_plural.  */
		      remember_a_message_plural (plural_mp, token.string,
						 &token.pos);
		      plural_mp = NULL;
		    }
		}
	      else
		free (token.string);
	      state = 0;
	    }
	  continue;

	case xgettext_token_type_symbol:
	  state = 0;
	  continue;

	case xgettext_token_type_eof:
	  return true;

	default:
	  abort ();
	}
    }
}


void
extract_c (f, real_filename, logical_filename, mdlp)
     FILE *f;
     const char *real_filename;
     const char *logical_filename;
     msgdomain_list_ty *mdlp;
{
  message_list_ty *mlp = mdlp->item[0]->messages;

  fp = f;
  real_file_name = real_filename;
  logical_file_name = xstrdup (logical_filename);
  line_number = 1;

  newline_count = 0;
  last_comment_line = -1;
  last_non_comment_line = -1;

  init_keywords ();

  /* Eat tokens until eof is seen.  When extract_parenthesized returns
     due to an unbalanced closing parenthesis, just restart it.  */
  while (!extract_parenthesized (mlp, -1, 0))
    ;

  /* Close scanner.  */
  fp = NULL;
  real_file_name = NULL;
  logical_file_name = NULL;
  line_number = 0;
}
