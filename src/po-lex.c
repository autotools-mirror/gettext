/* GNU gettext - internationalization aids
   Copyright (C) 1995-1999, 2000, 2001 Free Software Foundation, Inc.

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

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>

#include "libgettext.h"
#define _(str) gettext(str)

#if HAVE_VPRINTF || HAVE_DOPRNT
# if __STDC__
#  include <stdarg.h>
#  define VA_START(args, lastarg) va_start(args, lastarg)
# else
#  include <varargs.h>
#  define VA_START(args, lastarg) va_start(args)
# endif
#else
# define va_alist a1, a2, a3, a4, a5, a6, a7, a8
# define va_dcl char *a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8;
#endif

#include "po-lex.h"
#include "system.h"
#include "error.h"
#include "open-po.h"
#include "po-gram-gen2.h"

#if HAVE_C_BACKSLASH_A
# define ALERT_CHAR '\a'
#else
# define ALERT_CHAR '\7'
#endif


static FILE *fp;
lex_pos_ty gram_pos;
unsigned int gram_max_allowed_errors = 20;
static int po_lex_obsolete;
const char *po_lex_charset;
#if HAVE_ICONV
iconv_t po_lex_iconv;
#endif
static int pass_comments = 0;
int pass_obsolete_entries = 0;


/* Prototypes for local functions.  */
static int lex_getc PARAMS ((void));
static void lex_ungetc PARAMS ((int __ch));
static int keyword_p PARAMS ((const char *__s));
static int control_sequence PARAMS ((void));


/* Open the PO file FNAME and prepare its lexical analysis.  */
void
lex_open (fname)
     const char *fname;
{
  fp = open_po_file (fname, &gram_pos.file_name);
  if (!fp)
    error (EXIT_FAILURE, errno,
	   _("error while opening \"%s\" for reading"), fname);

  gram_pos.line_number = 1;
  po_lex_obsolete = 0;
  po_lex_charset = NULL;
#if HAVE_ICONV
  po_lex_iconv = (iconv_t)(-1);
#endif
}


/* Terminate lexical analysis and close the current PO file.  */
void
lex_close ()
{
  if (error_message_count > 0)
    error (EXIT_FAILURE, 0,
	   ngettext ("found %d fatal error", "found %d fatal errors",
		     error_message_count),
	   error_message_count);

  if (fp != stdin)
    fclose (fp);
  fp = NULL;
  gram_pos.file_name = 0;
  gram_pos.line_number = 0;
  error_message_count = 0;
  po_lex_obsolete = 0;
  po_lex_charset = NULL;
#if HAVE_ICONV
  if (po_lex_iconv != (iconv_t)(-1))
    {
      iconv_close (po_lex_iconv);
      po_lex_iconv = (iconv_t)(-1);
    }
#endif
}


/* CAUTION: If you change this function, you must also make identical
   changes to the macros of the same name in src/po-lex.h  */

#if !__STDC__ || !defined __GNUC__ || __GNUC__ == 1
/* VARARGS1 */
void
# if defined VA_START && __STDC__
po_gram_error (const char *fmt, ...)
# else
po_gram_error (fmt, va_alist)
     const char *fmt;
     va_dcl
# endif
{
# ifdef VA_START
  va_list ap;
  char *buffer;

  VA_START (ap, fmt);

  vasprintf (&buffer, fmt, ap);
  va_end (ap);
  error_at_line (0, 0, gram_pos.file_name, gram_pos.line_number, "%s", buffer);
# else
  error_at_line (0, 0, gram_pos.file_name, gram_pos.line_number, fmt,
		 a1, a2, a3, a4, a5, a6, a7, a8);
# endif

  /* Some messages need more than one line.  Continuation lines are
     indicated by using "..." at the start of the string.  We don't
     increment the error counter for these continuation lines.  */
  if (*fmt == '.')
    --error_message_count;
  else if (error_message_count >= gram_max_allowed_errors)
    error (EXIT_FAILURE, 0, _("too many errors, aborting"));
}


/* CAUTION: If you change this function, you must also make identical
   changes to the macro of the same name in src/po-lex.h  */

/* VARARGS2 */
void
# if defined VA_START && __STDC__
po_gram_error_at_line (const lex_pos_ty *pp, const char *fmt, ...)
# else
po_gram_error_at_line (pp, fmt, va_alist)
     const lex_pos_ty *pp;
     const char *fmt;
     va_dcl
# endif
{
# ifdef VA_START
  va_list ap;
  char *buffer;

  VA_START (ap, fmt);

  vasprintf (&buffer, fmt, ap);
  va_end (ap);
  error_at_line (0, 0, pp->file_name, pp->line_number, "%s", buffer);
# else
  error_at_line (0, 0, pp->file_name, pp->line_number, fmt,
		 a1, a2, a3, a4, a5, a6, a7, a8);
# endif

  /* Some messages need more than one line, or more than one location.
     Continuation lines are indicated by using "..." at the start of the
     string.  We don't increment the error counter for these
     continuation lines.  */
  if (*fmt == '.')
    --error_message_count;
  else if (error_message_count >= gram_max_allowed_errors)
    error (EXIT_FAILURE, 0, _("too many errors, aborting"));
}
#endif


/* Read a single character, dealing with backslash-newline.  */
static int
lex_getc ()
{
  int c;

  for (;;)
    {
      c = getc (fp);
      switch (c)
	{
	case EOF:
	  if (ferror (fp))
	    error (EXIT_FAILURE, errno,	_("error while reading \"%s\""),
		   gram_pos.file_name);
	  return EOF;

	case '\n':
	  ++gram_pos.line_number;
	  return '\n';

	case '\\':
	  c = getc (fp);
	  if (c != '\n')
	    {
	      if (c != EOF)
		ungetc (c, fp);
	      return '\\';
	    }
	  ++gram_pos.line_number;
	  break;

	default:
	  return c;
	}
    }
}


static void
lex_ungetc (c)
     int c;
{
  switch (c)
    {
    case EOF:
      break;

    case '\n':
      --gram_pos.line_number;
      /* FALLTHROUGH */

    default:
      ungetc (c, fp);
      break;
    }
}


static int
keyword_p (s)
     const char *s;
{
  if (!strcmp (s, "domain"))
    return DOMAIN;
  if (!strcmp (s, "msgid"))
    return MSGID;
  if (!strcmp (s, "msgid_plural"))
    return MSGID_PLURAL;
  if (!strcmp (s, "msgstr"))
    return MSGSTR;
  po_gram_error (_("keyword \"%s\" unknown"), s);
  return NAME;
}


static int
control_sequence ()
{
  int c;
  int val;
  int max;

  c = lex_getc ();
  switch (c)
    {
    case 'n':
      return '\n';

    case 't':
      return '\t';

    case 'b':
      return '\b';

    case 'r':
      return '\r';

    case 'f':
      return '\f';

    case 'v':
      return '\v';

    case 'a':
      return ALERT_CHAR;

    case '\\':
    case '"':
      return c;

    case '0': case '1': case '2': case '3':
    case '4': case '5': case '6': case '7':
      val = 0;
      max = 0;
      for (;;)
	{
	  /* Warning: not portable, can't depend on '0'..'7' ordering.  */
	  val = val * 8 + (c - '0');
	  if (++max == 3)
	    break;
	  c = lex_getc ();
	  switch (c)
	    {
	    case '0': case '1': case '2': case '3':
	    case '4': case '5': case '6': case '7':
	      continue;

	    default:
	      break;
	    }
	  lex_ungetc (c);
	  break;
	}
      return val;

    case 'x':
      c = lex_getc ();
      if (c == EOF || !isxdigit (c))
	break;

      val = 0;
      for (;;)
	{
	  val *= 16;
	  if (isdigit (c))
	    /* Warning: not portable, can't depend on '0'..'9' ordering */
	    val += c - '0';
	  else if (isupper (c))
	    /* Warning: not portable, can't depend on 'A'..'F' ordering */
	    val += c - 'A' + 10;
	  else
	    /* Warning: not portable, can't depend on 'a'..'f' ordering */
	    val += c - 'a' + 10;

	  c = lex_getc ();
	  switch (c)
	    {
	    case '0': case '1': case '2': case '3': case '4':
	    case '5': case '6': case '7': case '8': case '9':
	    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
	    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
	      continue;

	    default:
	      break;
	    }
	  lex_ungetc (c);
	  break;
	}
      return val;

    /* FIXME: \u and \U are not handled.  */
    }
  po_gram_error (_("invalid control sequence"));
  return ' ';
}


/* Return the next token in the PO file.  The return codes are defined
   in "po-gram-gen2.h".  Associated data is put in 'po_gram_lval'.  */
int
po_gram_lex ()
{
  static char *buf;
  static size_t bufmax;
  int c;
  size_t bufpos;

  for (;;)
    {
      c = lex_getc ();
      switch (c)
	{
	case EOF:
	  /* Yacc want this for end of file.  */
	  return 0;

	case '\n':
	  po_lex_obsolete = 0;
	  break;

	case ' ':
	case '\t':
	case '\r':
	case '\f':
	case '\v':
	  break;

	case '#':
	  c = lex_getc ();
	  if (c == '~')
	    /* A pseudo-comment beginning with #~ is found.  This is
	       not a comment.  It is the format for obsolete entries.
	       We simply discard the "#~" prefix.  The following
	       characters are expected to be well formed.  */
	    {
	      po_lex_obsolete = 1;
	      break;
	    }

	  /* Accumulate comments into a buffer.  If we have been asked
 	     to pass comments, generate a COMMENT token, otherwise
 	     discard it.  */
	  if (pass_comments)
	    {
	      bufpos = 0;
	      while (1)
		{
		  if (bufpos >= bufmax)
		    {
		      bufmax += 100;
		      buf = xrealloc (buf, bufmax);
		    }
		  if (c == EOF || c == '\n')
		    break;

		  buf[bufpos++] = c;
		  c = lex_getc ();
		}
	      buf[bufpos] = 0;

	      po_gram_lval.string.string = buf;
	      po_gram_lval.string.pos = gram_pos;
	      po_gram_lval.string.obsolete = po_lex_obsolete;
	      po_lex_obsolete = 0;
	      return COMMENT;
	    }
	  else
	    {
	      /* We do this in separate loop because collecting large
		 comments while they get not passed to the upper layers
		 is not very effective.  */
	      while (c != EOF && c != '\n')
		c = lex_getc ();
	      po_lex_obsolete = 0;
	    }
	  break;

	case '"':
	  /* Accumulate a string.  */
	  {
#if HAVE_ICONV
	    size_t bufmbpos = 0;
#endif

	    bufpos = 0;
	    while (1)
	      {
		if (bufpos >= bufmax)
		  {
		    bufmax += 100;
		    buf = xrealloc (buf, bufmax);
		  }
		c = lex_getc ();
		if (c == EOF)
		  {
		    po_gram_error (_("end-of-file within string"));
		    break;
		  }
		if (c == '\n')
		  {
		    po_gram_error (_("end-of-line within string"));
		    break;
		  }
#if HAVE_ICONV
		/* Interpret c only if it is the first byte of a multi-byte
		   character.  Don't interpret it as ASCII when it is the
		   second byte.  This is needed for the BIG5, BIG5HKSCS, GBK,
		   GB18030, SJIS, JOHAB encodings.  */
		if (po_lex_iconv == (iconv_t)(-1) || bufmbpos == bufpos)
#endif
		  {
		    if (c == '"')
		      break;

		    if (c == '\\')
		      {
			buf[bufpos++] = control_sequence ();
#if HAVE_ICONV
			bufmbpos++;
#endif
			continue;
		      }
		  }

		/* Add c to the accumulator.  */
		buf[bufpos++] = c;
#if HAVE_ICONV
		if (po_lex_iconv != (iconv_t)(-1))
		  {
		    /* If c terminates a multibyte character, set
		       bufmbpos = bufpos.  Otherwise keep bufmbpos
		       pointing at the start of the multibyte character.  */
		    char scratchbuf[64];
		    const char *inptr = &buf[bufmbpos];
		    size_t insize = bufpos - bufmbpos;
		    char *outptr = &scratchbuf[0];
		    size_t outsize = sizeof (scratchbuf);
		    if (iconv (po_lex_iconv,
			       (ICONV_CONST char **) &inptr, &insize,
			       &outptr, &outsize)
			== (size_t)(-1)
			&& errno == EILSEQ)
		      {
			po_gram_error (_("invalid multibyte sequence"));
			bufmbpos = bufpos;
		      }
		    else
		      bufmbpos = inptr - buf;
		  }
#endif
	      }
	    buf[bufpos] = 0;

	    /* FIXME: Treatment of embedded \000 chars is incorrect.  */
	    po_gram_lval.string.string = xstrdup (buf);
	    po_gram_lval.string.pos = gram_pos;
	    po_gram_lval.string.obsolete = po_lex_obsolete;
	    return STRING;
	  }

	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
	case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
	case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
	case 's': case 't': case 'u': case 'v': case 'w': case 'x':
	case 'y': case 'z':
	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
	case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
	case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
	case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
	case 'Y': case 'Z':
	case '_': case '$':
	  bufpos = 0;
	  for (;;)
	    {
	      if (bufpos + 1 >= bufmax)
		{
		  bufmax += 100;
		  buf = xrealloc (buf, bufmax);
		}
	      buf[bufpos++] = c;
	      c = lex_getc ();
	      switch (c)
		{
		default:
		  break;
		case 'a': case 'b': case 'c': case 'd':
		case 'e': case 'f': case 'g': case 'h':
		case 'i': case 'j': case 'k': case 'l':
		case 'm': case 'n': case 'o': case 'p':
		case 'q': case 'r': case 's': case 't':
		case 'u': case 'v': case 'w': case 'x':
		case 'y': case 'z':
		case 'A': case 'B': case 'C': case 'D':
		case 'E': case 'F': case 'G': case 'H':
		case 'I': case 'J': case 'K': case 'L':
		case 'M': case 'N': case 'O': case 'P':
		case 'Q': case 'R': case 'S': case 'T':
		case 'U': case 'V': case 'W': case 'X':
		case 'Y': case 'Z':
		case '_': case '$':
		case '0': case '1': case '2': case '3':
		case '4': case '5': case '6': case '7':
		case '8': case '9':
		  continue;
		}
	      break;
	    }
	  lex_ungetc (c);

	  buf[bufpos] = 0;

	  c = keyword_p (buf);
	  if (c == NAME)
	    {
	      po_gram_lval.string.string = xstrdup (buf);
	      po_gram_lval.string.pos = gram_pos;
	      po_gram_lval.string.obsolete = po_lex_obsolete;
	    }
	  else
	    {
	      po_gram_lval.pos.pos = gram_pos;
	      po_gram_lval.pos.obsolete = po_lex_obsolete;
	    }
	  return c;

	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	  bufpos = 0;
	  for (;;)
	    {
	      if (bufpos + 1 >= bufmax)
		{
		  bufmax += 100;
		  buf = xrealloc (buf, bufmax + 1);
		}
	      buf[bufpos++] = c;
	      c = lex_getc ();
	      switch (c)
		{
		default:
		  break;

		case '0': case '1': case '2': case '3':
		case '4': case '5': case '6': case '7':
		case '8': case '9':
		  continue;
		}
	      break;
	    }
	  lex_ungetc (c);

	  buf[bufpos] = 0;

	  po_gram_lval.number.number = atol (buf);
	  po_gram_lval.number.pos = gram_pos;
	  po_gram_lval.number.obsolete = po_lex_obsolete;
	  return NUMBER;

	case '[':
	  po_gram_lval.pos.pos = gram_pos;
	  po_gram_lval.pos.obsolete = po_lex_obsolete;
	  return '[';

	case ']':
	  po_gram_lval.pos.pos = gram_pos;
	  po_gram_lval.pos.obsolete = po_lex_obsolete;
	  return ']';

	default:
	  /* This will cause a syntax error.  */
	  return JUNK;
	}
    }
}


/* po_gram_lex() can return comments as COMMENT.  Switch this on or off.  */
void
po_lex_pass_comments (flag)
     int flag;
{
  pass_comments = (flag != 0);
}


/* po_gram_lex() can return obsolete entries as if they were normal entries.
   Switch this on or off.  */
void
po_lex_pass_obsolete_entries (flag)
     int flag;
{
  pass_obsolete_entries = (flag != 0);
}
