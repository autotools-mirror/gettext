/* GNU gettext - internationalization aids
   Copyright (C) 1995-1996, 1998, 2000, 2001 Free Software Foundation, Inc.

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
# include "config.h"
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "po.h"
#include "po-hash.h"
#include "system.h"
#include "mbswidth.h"
#include "libgettext.h"

extern const char *program_name;

#define _(str) gettext (str)

#define SIZEOF(a) (sizeof(a) / sizeof(a[0]))

/* Prototypes for local functions.  */
static void po_parse_brief PARAMS ((po_ty *__pop));
static void po_parse_debrief PARAMS ((po_ty *__pop));

/* Methods used indirectly by po_scan.  */
static void po_directive_domain PARAMS ((po_ty *__pop, char *__name));
static void po_directive_message PARAMS ((po_ty *__pop, char *__msgid,
					  lex_pos_ty *__msgid_pos,
					  char *__msgid_plural,
					  char *__msgstr, size_t __msgstr_len,
					  lex_pos_ty *__msgstr_pos));
static void po_comment PARAMS ((po_ty *__pop, const char *__s));
static void po_comment_dot PARAMS ((po_ty *__pop, const char *__s));
static void po_comment_filepos PARAMS ((po_ty *__pop, const char *__name,
					int __line));
static void po_comment_special PARAMS ((po_ty *pop, const char *s));

/* Local variables.  */
static po_ty *callback_arg;


po_ty *
po_alloc (pomp)
     po_method_ty *pomp;
{
  po_ty *pop;

  pop = xmalloc (pomp->size);
  pop->method = pomp;
  pop->next_is_fuzzy = 0;
  if (pomp->constructor)
    pomp->constructor (pop);
  return pop;
}


void
po_free (pop)
     po_ty *pop;
{
  if (pop->method->destructor)
    pop->method->destructor (pop);
  free (pop);
}


void
po_scan (pop, filename)
     po_ty *pop;
     const char *filename;
{
  extern int po_gram_parse PARAMS ((void));

  /* The parse will call the po_callback_... functions (see below)
     when the various directive are recognised.  The callback_arg
     variable is used to tell these functions which instance is to
     have the relevant method invoked.  */
  callback_arg = pop;

  /* Open the file and parse it.  */
  lex_open (filename);
  po_parse_brief (pop);
  po_gram_parse ();
  po_parse_debrief (pop);
  lex_close ();
  callback_arg = NULL;
}


static void
po_parse_brief (pop)
     po_ty *pop;
{
  if (pop->method->parse_brief)
    pop->method->parse_brief (pop);
}


static void
po_parse_debrief (pop)
     po_ty *pop;
{
  if (pop->method->parse_debrief)
    pop->method->parse_debrief (pop);
}


static void
po_directive_domain (pop, name)
     po_ty *pop;
     char *name;
{
  if (pop->method->directive_domain)
    pop->method->directive_domain (pop, name);
}


void
po_callback_domain (name)
     char *name;
{
  /* assert(callback_arg); */
  po_directive_domain (callback_arg, name);
}


static void
po_directive_message (pop, msgid, msgid_pos, msgid_plural,
		      msgstr, msgstr_len, msgstr_pos)
     po_ty *pop;
     char *msgid;
     lex_pos_ty *msgid_pos;
     char *msgid_plural;
     char *msgstr;
     size_t msgstr_len;
     lex_pos_ty *msgstr_pos;
{
  if (pop->method->directive_message)
    pop->method->directive_message (pop, msgid, msgid_pos, msgid_plural,
				    msgstr, msgstr_len, msgstr_pos);
}


/* Emit a multiline warning to stderr, consisting of MESSAGE, with the
   first line prefixed with PREFIX and the remaining lines prefixed with
   the same amount of spaces.  Reuse the spaces of the previous call if
   PREFIX is NULL.  Free the PREFIX and MESSAGE when done.  */
static void
multiline_warning (prefix, message)
     char *prefix;
     char *message;
{
  static int width;
  const char *cp;
  int i;

  fflush (stdout);

  cp = message;

  if (prefix != NULL)
    {
      fputs (prefix, stderr);
      width = mbswidth (prefix, 0);
      free (prefix);
      goto after_indent;
    }

  while (1)
    {
      const char *np;

      for (i = width; i > 0; i--)
	putc (' ', stderr);

    after_indent:
      np = strchr (cp, '\n');

      if (np == NULL || np[1] == '\0')
	{
	  fputs (cp, stderr);
	  break;
	}

      np++;
      fwrite (cp, 1, np - cp, stderr);
      cp = np;
    }

  free (message);
}

void
po_callback_message (msgid, msgid_pos, msgid_plural,
		     msgstr, msgstr_len, msgstr_pos)
     char *msgid;
     lex_pos_ty *msgid_pos;
     char *msgid_plural;
     char *msgstr;
     size_t msgstr_len;
     lex_pos_ty *msgstr_pos;
{
  /* assert(callback_arg); */

  /* Test for header entry.  */
  if (msgid[0] == '\0' && !callback_arg->next_is_fuzzy)
    {
      /* Verify the validity of CHARSET.  It is necessary
	 1. for the correct treatment of multibyte characters containing
	    0x5C bytes in the PO lexer,
	 2. so that at run time, gettext() can call iconv() to convert
	    msgstr.  */
      const char *charsetstr = strstr (msgstr, "charset=");

      if (charsetstr != NULL)
	{
	  /* The list of charsets supported by glibc's iconv() and by
	     the portable iconv() across platforms.  Taken from
	     intl/config.charset.  */
	  static const char *standard_charsets[] =
	  {
	    "ASCII", "ANSI_X3.4-1968", "US-ASCII",
	    "ISO-8859-1", "ISO_8859-1",
	    "ISO-8859-2", "ISO_8859-2",
	    "ISO-8859-3", "ISO_8859-3",
	    "ISO-8859-4", "ISO_8859-4",
	    "ISO-8859-5", "ISO_8859-5",
	    "ISO-8859-6", "ISO_8859-6",
	    "ISO-8859-7", "ISO_8859-7",
	    "ISO-8859-8", "ISO_8859-8",
	    "ISO-8859-9", "ISO_8859-9",
	    "ISO-8859-13", "ISO_8859-13",
	    "ISO-8859-15", "ISO_8859-15",
	    "KOI8-R",
	    "KOI8-U",
	    "CP850",
	    "CP866",
	    "CP874",
	    "CP932",
	    "CP949",
	    "CP950",
	    "CP1250",
	    "CP1251",
	    "CP1252",
	    "CP1253",
	    "CP1254",
	    "CP1255",
	    "CP1256",
	    "CP1257",
	    "GB2312",
	    "EUC-JP",
	    "EUC-KR",
	    "EUC-TW",
	    "BIG5",
	    "BIG5HKSCS",
	    "GBK",
	    "GB18030",
	    "SJIS",
	    "JOHAB",
	    "TIS-620",
	    "VISCII",
	    "UTF-8"
	  };
	  size_t len;
	  char *charset;
	  size_t i;

	  charsetstr += strlen ("charset=");
	  len = strcspn (charsetstr, " \t\n");
	  charset = (char *) alloca (len + 1);
	  memcpy (charset, charsetstr, len);
	  charset[len] = '\0';

	  for (i = 0; i < SIZEOF (standard_charsets); i++)
	    if (strcasecmp (charset, standard_charsets[i]) == 0)
	      break;
	  if (i == SIZEOF (standard_charsets))
	    {
	      char *prefix;
	      char *msg;

	      asprintf (&prefix, _("%s: warning: "), gram_pos.file_name);
	      asprintf (&msg, _("\
Charset \"%s\" is not a portable encoding name.\n\
Message conversion to user's charset might not work.\n"),
			charset);
	      if (prefix == NULL || msg == NULL)
		error (EXIT_FAILURE, 0, _("memory exhausted"));
	      multiline_warning (prefix, msg);
	    }
	  else
	    {
	      /* The list of encodings in standard_charsets which have
		 double-byte characters ending in 0x5C.  For these encodings,
		 the string parser is likely to be confused if it can't see
		 the character boundaries.  */
	      static const char *weird_charsets[] =
	      {
		"BIG5",
		"BIG5HKSCS",
		"GBK",
		"GB18030",
		"SJIS",
		"JOHAB"
	      };
	      const char *envval;

	      po_lex_charset = standard_charsets[i];
#if HAVE_ICONV
	      if (po_lex_iconv != (iconv_t)(-1))
		iconv_close (po_lex_iconv);
#endif

	      /* The old Solaris/openwin msgfmt and GNU msgfmt <= 0.10.35
		 don't know about multibyte encodings, and require a spurious
		 backslash after every multibyte character whose last byte is
		 0x5C.  Some programs, like vim, distribute PO files in this
		 broken format.  GNU msgfmt must continue to support this old
		 PO file format when the Makefile requests it.  */
	      envval = getenv ("OLD_PO_FILE_INPUT");
	      if (envval != NULL && *envval != '\0')
		{
		  /* Assume the PO file is in old format, with extraneous
		     backslashes.  */
#if HAVE_ICONV
		  po_lex_iconv = (iconv_t)(-1);
#endif
		}
	      else
		{
		  /* Use iconv() to parse multibyte characters.  */
#if HAVE_ICONV
		  /* Avoid glibc-2.1 bug with EUC-KR.  */
# if (__GLIBC__ - 0 == 2 && __GLIBC_MINOR__ - 0 <= 1) && !defined _LIBICONV_VERSION
		  if (strcmp (po_lex_charset, "EUC-KR") == 0)
		    po_lex_iconv = (iconv_t)(-1);
		  else
# endif
		  po_lex_iconv = iconv_open ("UTF-8", po_lex_charset);
		  if (po_lex_iconv == (iconv_t)(-1))
		    {
		      const char *note;
		      char *prefix;
		      char *msg;

		      for (i = 0; i < SIZEOF (weird_charsets); i++)
			if (strcmp (po_lex_charset, weird_charsets[i]) == 0)
			  break;
		      if (i < SIZEOF (weird_charsets))
			note = _("Continuing anyway, expect parse errors.");
		      else
			note = _("Continuing anyway.");

		      asprintf (&prefix, _("%s: warning: "), gram_pos.file_name);
		      asprintf (&msg, _("\
Charset \"%s\" is not supported. %s relies on iconv(),\n\
and iconv() does not support \"%s\".\n"),
				po_lex_charset, basename (program_name),
				po_lex_charset);
		      if (prefix == NULL || msg == NULL)
			error (EXIT_FAILURE, 0, _("memory exhausted"));
		      multiline_warning (prefix, msg);

# if !defined _LIBICONV_VERSION
		      asprintf (&msg, _("\
Installing GNU libiconv and then reinstalling GNU gettext\n\
would fix this problem.\n"));
		      if (msg == NULL)
			error (EXIT_FAILURE, 0, _("memory exhausted"));
		      multiline_warning (NULL, msg);
# endif

		      asprintf (&msg, _("%s\n"), note);
		      if (msg == NULL)
			error (EXIT_FAILURE, 0, _("memory exhausted"));
		      multiline_warning (NULL, msg);
		    }
#else
		  for (i = 0; i < SIZEOF (weird_charsets); i++)
		    if (strcmp (po_lex_charset, weird_charsets[i]) == 0)
		      break;
		  if (i < SIZEOF (weird_charsets))
		    {
		      const char *note =
			_("Continuing anyway, expect parse errors.");
		      char *prefix;
		      char *msg;

		      asprintf (&prefix, _("%s: warning: "), gram_pos.file_name);
		      asprintf (&msg, _("\
Charset \"%s\" is not supported. %s relies on iconv().\n\
This version was built without iconv().\n"),
				po_lex_charset, basename (program_name));
		      if (prefix == NULL || msg == NULL)
			error (EXIT_FAILURE, 0, _("memory exhausted"));
		      multiline_warning (prefix, msg);

		      asprintf (&msg, _("\
Installing GNU libiconv and then reinstalling GNU gettext\n\
would fix this problem.\n"));
		      if (msg == NULL)
			error (EXIT_FAILURE, 0, _("memory exhausted"));
		      multiline_warning (NULL, msg);

		      asprintf (&msg, _("%s\n"), note);
		      if (msg == NULL)
			error (EXIT_FAILURE, 0, _("memory exhausted"));
		      multiline_warning (NULL, msg);
		    }
#endif
		}
	    }
	}
      else
	{
	  char *prefix;
	  char *msg;

	  asprintf (&prefix, _("%s: warning: "), gram_pos.file_name);
	  asprintf (&msg, _("\
Charset missing in header.\n\
Message conversion to user's charset will not work.\n"));
	  if (prefix == NULL || msg == NULL)
	    error (EXIT_FAILURE, 0, _("memory exhausted"));
	  multiline_warning (prefix, msg);
	}
    }

  po_directive_message (callback_arg, msgid, msgid_pos, msgid_plural,
			msgstr, msgstr_len, msgstr_pos);

  /* Prepare for next message.  */
  callback_arg->next_is_fuzzy = 0;
}


static void
po_comment_special (pop, s)
     po_ty *pop;
     const char *s;
{
  if (pop->method->comment_special != NULL)
    pop->method->comment_special (pop, s);
}


static void
po_comment (pop, s)
     po_ty *pop;
     const char *s;
{
  if (pop->method->comment != NULL)
    pop->method->comment (pop, s);
}


static void
po_comment_dot (pop, s)
     po_ty *pop;
     const char *s;
{
  if (pop->method->comment_dot != NULL)
    pop->method->comment_dot (pop, s);
}


/* This function is called by po_gram_lex() whenever a comment is
   seen.  It analyzes the comment to see what sort it is, and then
   dispatces it to the appropriate method.  */
void
po_callback_comment (s)
     const char *s;
{
  /* assert(callback_arg); */
  if (*s == '.')
    po_comment_dot (callback_arg, s + 1);
  else if (*s == ':')
    {
      /* Parse the file location string.  If the parse succeeds, the
	 appropriate callback will be invoked.  If the parse fails,
	 the po_hash function will return non-zero - so pretend it was
	 a normal comment.  */
      if (po_hash (s + 1) == 0)
	/* Do nothing, it is a GNU-style file pos line.  */ ;
      else
	po_comment (callback_arg, s + 1);
    }
  else if (*s == ',' || *s == '!')
    {
      /* Get all entries in the special comment line.  */
      if (strstr (s + 1, "fuzzy") != NULL)
	callback_arg->next_is_fuzzy = 1;
      po_comment_special (callback_arg, s + 1);
    }
  else
    {
      /* It looks like a plain vanilla comment, but Solaris-style file
	 position lines do, too.  Rather than parse the lot, only look
	 at lines that could start with "# File..." This minimizes
	 memory leaks on failed parses.  If the parse succeeds, the
	 appropriate callback will be invoked.  */
      if (s[0] == ' ' && (s[1] == 'F' || s[1] == 'f') && s[2] == 'i'
	  && po_hash (s) == 0)
	/* Do nothing, it is a Sun-style file pos line.  */ ;
      else
	po_comment (callback_arg, s);
    }
}


static void
po_comment_filepos (pop, name, line)
     po_ty *pop;
     const char *name;
     int line;
{
  if (pop->method->comment_filepos)
    pop->method->comment_filepos (pop, name, line);
}


void
po_callback_comment_filepos (name, line)
     const char *name;
     int line;
{
  /* assert(callback_arg); */
  po_comment_filepos (callback_arg, name, line);
}
