/* GNU gettext - internationalization aids
   Copyright (C) 1995-1998, 2000, 2001 Free Software Foundation, Inc.

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
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_LIMITS_H
# include <limits.h>
#endif

#if HAVE_ICONV
#include <iconv.h>
#endif

#include "write-po.h"
#include "c-ctype.h"
#include "linebreak.h"
#include "system.h"
#include "error.h"
#include "libgettext.h"


/* Our regular abbreviation.  */
#define _(str) gettext (str)


/* Prototypes for local functions.  */
static const char *make_c_format_description_string PARAMS ((enum is_c_format,
							     int debug));
static const char *make_c_width_description_string PARAMS ((enum is_c_format));
static int significant_c_format_p PARAMS ((enum is_c_format __is_c_format));
static void wrap PARAMS ((FILE *__fp, const char *__line_prefix,
			  const char *__name, const char *__value,
			  enum is_wrap do_wrap, const char *__charset));
static void print_blank_line PARAMS ((FILE *__fp));
static void message_print PARAMS ((const message_ty *__mp, FILE *__fp,
				   const char *__domain, const char *__charset,
				   int blank_line, int __debug));
static void message_print_obsolete PARAMS ((const message_ty *__mp, FILE *__fp,
					    const char *__domain,
					    const char *__charset,
					    int blank_line));
static int msgid_cmp PARAMS ((const void *__va, const void *__vb));
static int filepos_cmp PARAMS ((const void *__va, const void *__vb));


/* This variable controls the page width when printing messages.
   Defaults to PAGE_WIDTH if not set.  Zero (0) given to message_page_-
   width_set will result in no wrapping being performed.  */
static size_t page_width = PAGE_WIDTH;

void
message_page_width_set (n)
     size_t n;
{
  if (n == 0)
    {
      page_width = INT_MAX;
      return;
    }

  if (n < 20)
    n = 20;

  page_width = n;
}


/* These three variables control the output style of the message_print
   function.  Interface functions for them are to be used.  */
static int indent;
static int uniforum;
static int escape;

void
message_print_style_indent ()
{
  indent = 1;
}

void
message_print_style_uniforum ()
{
  uniforum = 1;
}

void
message_print_style_escape (flag)
     int flag;
{
  escape = (flag != 0);
}


/* Local functions.  */


static const char *
make_c_format_description_string (is_c_format, debug)
     enum is_c_format is_c_format;
     int debug;
{
  const char *result = NULL;

  switch (is_c_format)
    {
    case possible:
      if (debug)
	{
	  result = " possible-c-format";
	  break;
	}
      /* FALLTHROUGH */
    case yes:
      result = " c-format";
      break;
    case impossible:
      result = " impossible-c-format";
      break;
    case no:
      result = " no-c-format";
      break;
    case undecided:
      result = " undecided";
      break;
    default:
      abort ();
    }

  return result;
}


static int
significant_c_format_p (is_c_format)
     enum is_c_format is_c_format;
{
  return is_c_format != undecided && is_c_format != impossible;
}


static const char *
make_c_width_description_string (do_wrap)
     enum is_wrap do_wrap;
{
  const char *result = NULL;

  switch (do_wrap)
    {
    case yes:
      result = " wrap";
      break;
    case no:
      result = " no-wrap";
      break;
    default:
      abort ();
    }

  return result;
}


static void
wrap (fp, line_prefix, name, value, do_wrap, charset)
     FILE *fp;
     const char *line_prefix;
     const char *name;
     const char *value;
     enum is_wrap do_wrap;
     const char *charset;
{
  const char *s;
  int first_line;
#if HAVE_ICONV
  const char *envval;
  iconv_t conv;
#endif

#if HAVE_ICONV
  /* The old Solaris/openwin msgfmt and GNU msgfmt <= 0.10.35 don't know
     about multibyte encodings, and require a spurious backslash after
     every multibyte character whose last byte is 0x5C.  Some programs,
     like vim, distribute PO files in this broken format.  It is important
     for such programs that GNU msgmerge continues to support this old
     PO file format when the Makefile requests it.  */
  envval = getenv ("OLD_PO_FILE_OUTPUT");
  if (envval != NULL && *envval != '\0')
    /* Write a PO file in old format, with extraneous backslashes.  */
    conv = (iconv_t)(-1);
  else
    /* Avoid glibc-2.1 bug with EUC-KR.  */
# if (__GLIBC__ - 0 == 2 && __GLIBC_MINOR__ - 0 <= 1) && !defined _LIBICONV_VERSION
    if (strcmp (charset, "EUC-KR") == 0)
      conv = (iconv_t)(-1);
    else
# endif
    /* Use iconv() to parse multibyte characters.  */
    conv = iconv_open ("UTF-8", charset);
#endif

  /* Loop over the '\n' delimited portions of value.  */
  s = value;
  first_line = 1;
  do
    {
      /* The \a and \v escapes were added by the ANSI C Standard.
         Prior to the Standard, most compilers did not have them.
         Because we need the same program on all platforms we don't
         provide support for them here.  */
      static const char escapes[] = "\b\f\n\r\t";
      static const char escape_names[] = "bfnrt";

      const char *es;
      const char *ep;
      size_t portion_len;
      char *portion;
      char *overrides;
      char *linebreaks;
      char *pp;
      char *op;
      int startcol, startcol_after_break, width;
      size_t i;

      for (es = s; *es != '\0'; )
	if (*es++ == '\n')
	  break;

      /* Expand escape sequences in each portion.  */
      for (ep = s, portion_len = 0; ep < es; ep++)
	{
	  char c = *ep;
	  const char *esc = strchr (escapes, c);
	  if (esc != NULL)
	    portion_len += 2;
	  else if (escape && !c_isprint ((unsigned char) c))
	    portion_len += 4;
	  else if (c == '\\' || c == '"')
	    portion_len += 2;
	  else
	    {
#if HAVE_ICONV
	      if (conv != (iconv_t)(-1))
		{
		  /* Skip over a complete multi-byte character.  Don't
		     interpret the second byte of a multi-byte character as
		     ASCII.  This is needed for the BIG5, BIG5HKSCS, GBK,
		     GB18030, SJIS, JOHAB encodings.  */
		  char scratchbuf[64];
		  const char *inptr = ep;
		  size_t insize;
		  char *outptr = &scratchbuf[0];
		  size_t outsize = sizeof (scratchbuf);
		  size_t res;

		  res = (size_t)(-1);
		  for (insize = 1; inptr + insize <= es; insize++)
		    {
		      res = iconv (conv,
				   (ICONV_CONST char **) &inptr, &insize,
				   &outptr, &outsize);
		      if (!(res == (size_t)(-1) && errno == EINVAL))
			break;
		    }
		  if (res == (size_t)(-1))
		    {
		      if (errno == EILSEQ)
			{
			  error (0, 0, _("invalid multibyte sequence"));
			  continue;
			}
		      else
			abort ();
		    }
		  insize = inptr - ep;
		  portion_len += insize;
		  ep += insize - 1;
		}
	      else
#endif
		portion_len += 1;
	    }
	}
      portion = (char *) xmalloc (portion_len);
      overrides = (char *) xmalloc (portion_len);
      memset (overrides, UC_BREAK_UNDEFINED, portion_len);
      for (ep = s, pp = portion, op = overrides; ep < es; ep++)
	{
	  char c = *ep;
	  const char *esc = strchr (escapes, c);
	  if (esc != NULL)
	    {
	      *pp++ = '\\';
	      *pp++ = c = escape_names[esc - escapes];
	      op++;
	      *op++ = UC_BREAK_PROHIBITED;
	      /* We warn about any use of escape sequences beside
		 '\n' and '\t'.  */
	      if (c != 'n' && c != 't')
		error (0, 0, _("\
internationalized messages should not contain the `\\%c' escape sequence"),
		       c);
	    }
	  else if (escape && !c_isprint ((unsigned char) c))
	    {
	      *pp++ = '\\';
	      *pp++ = '0' + (((unsigned char) c >> 6) & 7);
	      *pp++ = '0' + (((unsigned char) c >> 3) & 7);
	      *pp++ = '0' + ((unsigned char) c & 7);
	      op++;
	      *op++ = UC_BREAK_PROHIBITED;
	      *op++ = UC_BREAK_PROHIBITED;
	      *op++ = UC_BREAK_PROHIBITED;
	    }
	  else if (c == '\\' || c == '"')
	    {
	      *pp++ = '\\';
	      *pp++ = c;
	      op++;
	      *op++ = UC_BREAK_PROHIBITED;
	    }
	  else
	    {
#if HAVE_ICONV
	      if (conv != (iconv_t)(-1))
		{
		  /* Copy a complete multi-byte character.  Don't
		     interpret the second byte of a multi-byte character as
		     ASCII.  This is needed for the BIG5, BIG5HKSCS, GBK,
		     GB18030, SJIS, JOHAB encodings.  */
		  char scratchbuf[64];
		  const char *inptr = ep;
		  size_t insize;
		  char *outptr = &scratchbuf[0];
		  size_t outsize = sizeof (scratchbuf);
		  size_t res;

		  res = (size_t)(-1);
		  for (insize = 1; inptr + insize <= es; insize++)
		    {
		      res = iconv (conv,
				   (ICONV_CONST char **) &inptr, &insize,
				   &outptr, &outsize);
		      if (!(res == (size_t)(-1) && errno == EINVAL))
			break;
		    }
		  if (res == (size_t)(-1))
		    {
		      if (errno == EILSEQ)
			{
			  error (0, 0, _("invalid multibyte sequence"));
			  continue;
			}
		      else
			abort ();
		    }
		  insize = inptr - ep;
		  memcpy (pp, ep, insize);
		  pp += insize;
		  op += insize;
		  ep += insize - 1;
		}
	      else
#endif
		{
		  *pp++ = c;
		  op++;
		}
	    }
	}

      /* Don't break immediately before the "\n" at the end.  */
      if (es > s && es[-1] == '\n')
	overrides[portion_len - 2] = UC_BREAK_PROHIBITED;

      linebreaks = (char *) xmalloc (portion_len);

      /* Subsequent lines after a break are all indented.
	 See INDENT-S.  */
      startcol_after_break = (line_prefix ? strlen (line_prefix) : 0);
      if (indent)
	startcol_after_break = (startcol_after_break + 8) & ~7;
      startcol_after_break++;

      /* The line width.  Allow room for the closing quote character.  */
      width = (do_wrap == no ? INT_MAX : page_width) - 1;
      /* Adjust for indentation of subsequent lines.  */
      width -= startcol_after_break;

    recompute:
      /* The line starts with different things depending on whether it
	 is the first line, and if we are using the indented style.
	 See INDENT-F.  */
      startcol = (line_prefix ? strlen (line_prefix) : 0);
      if (first_line)
	{
	  startcol += strlen (name);
	  if (indent)
	    startcol = (startcol + 8) & ~7;
	  else
	    startcol++;
	}
      else
	{
	  if (indent)
	    startcol = (startcol + 8) & ~7;
	}
      /* Allow room for the opening quote character.  */
      startcol++;
      /* Adjust for indentation of subsequent lines.  */
      startcol -= startcol_after_break;

      /* Do line breaking on the portion.  */
      mbs_width_linebreaks (portion, portion_len, width, startcol, 0,
			    overrides, charset, linebreaks);

      /* If this is the first line, and we are not using the indented
	 style, and the line would wrap, then use an empty first line
	 and restart.  */
      if (first_line && !indent
	  && portion_len > 0
	  && (*es != '\0'
	      || startcol > width
	      || memchr (linebreaks, UC_BREAK_POSSIBLE, portion_len) != NULL))
	{
	  if (line_prefix != NULL)
	    fputs (line_prefix, fp);
	  fputs (name, fp);
	  fputs (" \"\"\n", fp);
	  first_line = 0;
	  /* Recompute startcol and linebreaks.  */
	  goto recompute;
	}

      /* Print the beginning of the line.  This will depend on whether
	 this is the first line, and if the indented style is being
	 used.  INDENT-F.  */
      if (line_prefix != NULL)
        fputs (line_prefix, fp);
      if (first_line)
	{
	  fputs (name, fp);
	  putc (indent ? '\t' : ' ', fp);
	  first_line = 0;
	}
      else
	{
	  if (indent)
	    putc ('\t', fp);
	}

      /* Print the portion itself, with linebreaks where necessary.  */
      putc ('"', fp);
      for (i = 0; i < portion_len; i++)
	{
	  if (linebreaks[i] == UC_BREAK_POSSIBLE)
	    {
	      fputs ("\"\n", fp);
	      /* INDENT-S.  */
	      if (line_prefix != NULL)
		fputs (line_prefix, fp);
	      if (indent)
		putc ('\t', fp);
	      putc ('"', fp);
	    }
	  putc (portion[i], fp);
	}
      fputs ("\"\n", fp);

      free (linebreaks);
      free (overrides);
      free (portion);

      s = es;
    }
  while (*s);

#if HAVE_ICONV
  if (conv != (iconv_t)(-1))
    iconv_close (conv);
#endif
}


static void
print_blank_line (fp)
     FILE *fp;
{
  if (uniforum)
    fputs ("#\n", fp);
  else
    putc ('\n', fp);
}


static void
message_print (mp, fp, domain, charset, blank_line, debug)
     const message_ty *mp;
     FILE *fp;
     const char *domain;
     const char *charset;
     int blank_line;
     int debug;
{
  message_variant_ty *mvp;
  int first;
  size_t j;

  /* Find the relevant message variant.  If there isn't one, remember
     this using a NULL pointer.  */
  mvp = NULL;
  first = 0;

  for (j = 0; j < mp->variant_count; ++j)
    {
      if (strcmp (domain, mp->variant[j].domain) == 0)
	{
	  mvp = &mp->variant[j];
	  first = (j == 0);
	  break;
	}
    }

  /* Separate messages with a blank line.  Uniforum doesn't like blank
     lines, so use an empty comment (unless there already is one).  */
  if (blank_line && (!uniforum
		     || mp->comment == NULL
		     || mp->comment->nitems == 0
		     || mp->comment->item[0][0] != '\0'))
    print_blank_line (fp);

  /* The first variant of a message will have the comments attached to
     it.  We can't attach them to all variants in case we are read in
     again, multiplying the number of comment lines.  Usually there is
     only one variant.  */
  if (first)
    {
      if (mp->comment != NULL)
	for (j = 0; j < mp->comment->nitems; ++j)
	  {
	    const char *s = mp->comment->item[j];
	    do
	      {
		const char *e;
		putc ('#', fp);
		if (*s != '\0' && *s != ' ')
		  putc (' ', fp);
		e = strchr (s, '\n');
		if (e == NULL)
		  {
		    fputs (s, fp);
		    s = NULL;
		  }
		else
		  {
		    fwrite (s, 1, e - s, fp);
		    s = e + 1;
		  }
		putc ('\n', fp);
	      }
	    while (s != NULL);
	  }

      if (mp->comment_dot != NULL)
	for (j = 0; j < mp->comment_dot->nitems; ++j)
	  {
	    const char *s = mp->comment_dot->item[j];
	    putc ('#', fp);
	    putc ('.', fp);
	    if (*s != '\0' && *s != ' ')
	      putc (' ', fp);
	    fputs (s, fp);
	    putc ('\n', fp);
	  }
    }

  /* Print the file position comments for every domain.  This will
     help a human who is trying to navigate the sources.  There is no
     problem of getting repeat positions, because duplicates are
     checked for.  */
  if (mp->filepos_count != 0)
    {
      if (uniforum)
	for (j = 0; j < mp->filepos_count; ++j)
	  {
	    lex_pos_ty *pp = &mp->filepos[j];
	    char *cp = pp->file_name;
	    while (cp[0] == '.' && cp[1] == '/')
	      cp += 2;
	    /* There are two Sun formats to choose from: SunOS and
	       Solaris.  Use the Solaris form here.  */
	    fprintf (fp, "# File: %s, line: %ld\n",
		     cp, (long) pp->line_number);
	  }
      else
	{
	  size_t column;

	  fputs ("#:", fp);
	  column = 2;
	  for (j = 0; j < mp->filepos_count; ++j)
	    {
	      lex_pos_ty *pp;
	      char buffer[20];
	      char *cp;
	      size_t len;

	      pp = &mp->filepos[j];
	      cp = pp->file_name;
	      while (cp[0] == '.' && cp[1] == '/')
		cp += 2;
	      sprintf (buffer, "%ld", (long) pp->line_number);
	      len = strlen (cp) + strlen (buffer) + 2;
	      if (column > 2 && column + len >= page_width)
		{
		  fputs ("\n#:", fp);
		  column = 2;
		}
	      fprintf (fp, " %s:%s", cp, buffer);
	      column += len;
	    }
	  putc ('\n', fp);
	}
    }

  /* Print flag information in special comment.  */
  if (first && ((mp->is_fuzzy && mvp != NULL && mvp->msgstr[0] != '\0')
		|| significant_c_format_p (mp->is_c_format)
		|| mp->do_wrap == no))
    {
      int first_flag = 1;

      putc ('#', fp);
      putc (',', fp);

      /* We don't print the fuzzy flag if the msgstr is empty.  This
	 might be introduced by the user but we want to normalize the
	 output.  */
      if (mp->is_fuzzy && mvp != NULL && mvp->msgstr[0] != '\0')
	{
	  fputs (" fuzzy", fp);
	  first_flag = 0;
	}

      if (significant_c_format_p (mp->is_c_format))
	{
	  if (!first_flag)
	    putc (',', fp);

	  fputs (make_c_format_description_string (mp->is_c_format, debug),
		 fp);
	  first_flag = 0;
	}

      if (mp->do_wrap == no)
	{
	  if (!first_flag)
	    putc (',', fp);

	  fputs (make_c_width_description_string (mp->do_wrap), fp);
	  first_flag = 0;
	}

      putc ('\n', fp);
    }

  /* Print each of the message components.  Wrap them nicely so they
     are as readable as possible.  If there is no recorded msgstr for
     this domain, emit an empty string.  */
  wrap (fp, NULL, "msgid", mp->msgid, mp->do_wrap, charset);
  if (mp->msgid_plural != NULL)
    wrap (fp, NULL, "msgid_plural", mp->msgid_plural, mp->do_wrap, charset);

  if (mp->msgid_plural == NULL)
    wrap (fp, NULL, "msgstr", mvp ? mvp->msgstr : "", mp->do_wrap, charset);
  else
    {
      char prefix_buf[20];
      unsigned int i;

      if (mvp)
	{
	  const char *p;

	  for (p = mvp->msgstr, i = 0;
	       p < mvp->msgstr + mvp->msgstr_len;
	       p += strlen (p) + 1, i++)
	    {
	      sprintf (prefix_buf, "msgstr[%u]", i);
	      wrap (fp, NULL, prefix_buf, p, mp->do_wrap, charset);
	    }
	}
      else
	{
	  for (i = 0; i < 2; i++)
	    {
	      sprintf (prefix_buf, "msgstr[%u]", i);
	      wrap (fp, NULL, prefix_buf, "", mp->do_wrap, charset);
	    }
	}
    }
}


static void
message_print_obsolete (mp, fp, domain, charset, blank_line)
     const message_ty *mp;
     FILE *fp;
     const char *domain;
     const char *charset;
     int blank_line;
{
  message_variant_ty *mvp;
  size_t j;

  /* Find the relevant message variant.  If there isn't one, remember
     this using a NULL pointer.  */
  mvp = NULL;

  for (j = 0; j < mp->variant_count; ++j)
    {
      if (strcmp (domain, mp->variant[j].domain) == 0)
	{
	  mvp = &mp->variant[j];
	  break;
	}
    }

  /* If no msgstr is found or it is the empty string we print nothing.  */
  if (mvp == NULL || mvp->msgstr[0] == '\0')
    return;

  /* Separate messages with a blank line.  Uniforum doesn't like blank
     lines, so use an empty comment (unless there already is one).  */
  if (blank_line)
    print_blank_line (fp);

  /* Print translator comment if available.  */
  if (mp->comment)
    for (j = 0; j < mp->comment->nitems; ++j)
      {
	const char *s = mp->comment->item[j];
	do
	  {
	    const char *e;
	    putc ('#', fp);
	    if (*s != '\0' && *s != ' ')
	      putc (' ', fp);
	    e = strchr (s, '\n');
	    if (e == NULL)
	      {
		fputs (s, fp);
		s = NULL;
	      }
	    else
	      {
		fwrite (s, 1, e - s, fp);
		s = e + 1;
	      }
	    putc ('\n', fp);
	  }
	while (s != NULL);
      }

  /* Print flag information in special comment.  */
  if (mp->is_fuzzy)
    {
      int first = 1;

      putc ('#', fp);
      putc (',', fp);

      if (mp->is_fuzzy)
	{
	  fputs (" fuzzy", fp);
	  first = 0;
	}

      putc ('\n', fp);
    }

  /* Print each of the message components.  Wrap them nicely so they
     are as readable as possible.  */
  wrap (fp, "#~ ", "msgid", mp->msgid, mp->do_wrap, charset);
  if (mp->msgid_plural != NULL)
    wrap (fp, "#~ ", "msgid_plural", mp->msgid_plural, mp->do_wrap, charset);

  if (mp->msgid_plural == NULL)
    wrap (fp, "#~ ", "msgstr", mvp->msgstr, mp->do_wrap, charset);
  else
    {
      char prefix_buf[20];
      unsigned int i;
      const char *p;

      for (p = mvp->msgstr, i = 0;
	   p < mvp->msgstr + mvp->msgstr_len;
	   p += strlen (p) + 1, i++)
	{
	  sprintf (prefix_buf, "msgstr[%u]", i);
	  wrap (fp, "#~ ", prefix_buf, p, mp->do_wrap, charset);
	}
    }
}


void
message_list_print (mlp, filename, force, debug)
     message_list_ty *mlp;
     const char *filename;
     int force;
     int debug;
{
  FILE *fp;
  size_t i, j, k;
  string_list_ty *dl;
  int blank_line;

  /* We will not write anything if we have no message or only the
     header entry.  */
  if (force == 0
      && (mlp->nitems == 0
	  || (mlp->nitems == 1 && *mlp->item[0]->msgid == '\0')))
    return;

  /* Build the list of domains.  */
  dl = string_list_alloc ();
  for (j = 0; j < mlp->nitems; ++j)
    {
      message_ty *mp = mlp->item[j];
      for (k = 0; k < mp->variant_count; ++k)
	string_list_append_unique (dl, mp->variant[k].domain);
    }

  /* Open the output file.  */
  if (filename != NULL && strcmp (filename, "-") != 0
      && strcmp (filename, "/dev/stdout") != 0)
    {
      fp = fopen (filename, "w");
      if (fp == NULL)
	error (EXIT_FAILURE, errno, _("cannot create output file \"%s\""),
	       filename);
    }
  else
    {
      fp = stdout;
      /* xgettext:no-c-format */
      filename = _("standard output");
    }

  /* Write out the messages for each domain.  */
  blank_line = 0;
  for (k = 0; k < dl->nitems; ++k)
    {
      const char *header;
      char *charset;

      /* If there is only one domain, and that domain is the default,
	 don't bother emitting the domain name, because it is the
	 default.  */
      if (dl->nitems != 1 || strcmp (dl->item[0], MESSAGE_DOMAIN_DEFAULT) != 0)
	{
	  if (blank_line)
	    print_blank_line (fp);
	  fprintf (fp, "domain \"%s\"\n", dl->item[k]);
	  blank_line = 1;
	}

      /* Search the header entry.  */
      header = NULL;
      for (j = 0; j < mlp->nitems; ++j)
	if (*mlp->item[j]->msgid == '\0' && mlp->item[j]->obsolete == 0)
	  {
	    for (i = 0; i < mlp->item[j]->variant_count; i++)
	      if (strcmp (dl->item[k], mlp->item[j]->variant[i].domain) == 0)
		{
		  header = mlp->item[j]->variant[i].msgstr;
		  break;
		}
	    break;
	  }

      /* Extract the charset name.  */
      charset = "ASCII";
      if (header != NULL)
	{
	  const char *charsetstr = strstr (header, "charset=");

	  if (charsetstr != NULL)
	    {
	      size_t len;

	      charsetstr += strlen ("charset=");
	      len = strcspn (charsetstr, " \t\n");
	      charset = (char *) alloca (len + 1);
	      memcpy (charset, charsetstr, len);
	      charset[len] = '\0';
	    }
	}

      /* Write out each of the messages for this domain.  */
      for (j = 0; j < mlp->nitems; ++j)
	if (mlp->item[j]->obsolete == 0)
	  {
	    message_print (mlp->item[j], fp, dl->item[k], charset,
			   blank_line, debug);
	    blank_line = 1;
	  }

      /* Write out each of the obsolete messages for this domain.  */
      for (j = 0; j < mlp->nitems; ++j)
	if (mlp->item[j]->obsolete != 0)
	  {
	    message_print_obsolete (mlp->item[j], fp, dl->item[k], charset,
				    blank_line);
	    blank_line = 1;
	  }
    }
  string_list_free (dl);

  /* Make sure nothing went wrong.  */
  if (fflush (fp))
    error (EXIT_FAILURE, errno, _("error while writing \"%s\" file"),
	   filename);
  fclose (fp);
}


static int
msgid_cmp (va, vb)
     const void *va;
     const void *vb;
{
  const message_ty *a = *(const message_ty **) va;
  const message_ty *b = *(const message_ty **) vb;
  /* Because msgids normally contain only ASCII characters, it is OK to
     sort them as if we were in the C locale. And strcoll() in the C locale
     is the same as strcmp().  */
  return strcmp (a->msgid, b->msgid);
}


void
message_list_sort_by_msgid (mlp)
     message_list_ty *mlp;
{
  qsort (mlp->item, mlp->nitems, sizeof (mlp->item[0]), msgid_cmp);
}


static int
filepos_cmp (va, vb)
     const void *va;
     const void *vb;
{
  const message_ty *a = *(const message_ty **) va;
  const message_ty *b = *(const message_ty **) vb;
  int cmp;

  /* No filepos is smaller than any other filepos.  */
  if (a->filepos_count == 0)
    {
      if (b->filepos_count != 0)
	return -1;
    }
  if (b->filepos_count == 0)
    return 1;

  /* Compare on the file names...  */
  cmp = strcmp (a->filepos[0].file_name, b->filepos[0].file_name);
  if (cmp != 0)
    return cmp;

  /* If they are equal, compare on the line numbers...  */
  cmp = a->filepos[0].line_number - b->filepos[0].line_number;
  if (cmp != 0)
    return cmp;

  /* If they are equal, compare on the msgid strings.  */
  /* Because msgids normally contain only ASCII characters, it is OK to
     sort them as if we were in the C locale. And strcoll() in the C locale
     is the same as strcmp().  */
  return strcmp (a->msgid, b->msgid);
}


void
message_list_sort_by_filepos (mlp)
    message_list_ty *mlp;
{
  qsort (mlp->item, mlp->nitems, sizeof (mlp->item[0]), filepos_cmp);
}
