/* Charset handling while reading PO files.
   Copyright (C) 2001 Free Software Foundation, Inc.
   Written by Bruno Haible <haible@clisp.cons.org>, 2001.

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

#include <stdio.h>
#include <stdlib.h>

#include "po-charset.h"
#include "error.h"
#include "system.h"
#include "mbswidth.h"
#include "libgettext.h"

extern const char *program_name;

#define _(str) gettext (str)

#define SIZEOF(a) (sizeof(a) / sizeof(a[0]))

const char *
po_charset_canonicalize (charset)
     const char *charset;
{
  /* The list of charsets supported by glibc's iconv() and by the portable
     iconv() across platforms.  Taken from intl/config.charset.  */
  static const char *standard_charsets[] =
  {
    "ASCII", "ANSI_X3.4-1968", "US-ASCII",	/* i = 0..2 */
    "ISO-8859-1", "ISO_8859-1",			/* i = 3, 4 */
    "ISO-8859-2", "ISO_8859-2",
    "ISO-8859-3", "ISO_8859-3",
    "ISO-8859-4", "ISO_8859-4",
    "ISO-8859-5", "ISO_8859-5",
    "ISO-8859-6", "ISO_8859-6",
    "ISO-8859-7", "ISO_8859-7",
    "ISO-8859-8", "ISO_8859-8",
    "ISO-8859-9", "ISO_8859-9",
    "ISO-8859-13", "ISO_8859-13",
    "ISO-8859-15", "ISO_8859-15",		/* i = 23, 24 */
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
  size_t i;

  for (i = 0; i < SIZEOF (standard_charsets); i++)
    if (strcasecmp (charset, standard_charsets[i]) == 0)
      return standard_charsets[i < 3 ? 0 : i < 25 ? ((i - 3) & ~1) + 3 : i];
  return NULL;
}

/* The PO file's encoding, as specified in the header entry.  */
static const char *po_lex_charset;

#if HAVE_ICONV
/* Converter from the PO file's encoding to UTF-8.  */
iconv_t po_lex_iconv;
#endif

void
po_lex_charset_init ()
{
  po_lex_charset = NULL;
#if HAVE_ICONV
  po_lex_iconv = (iconv_t)(-1);
#endif
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
po_lex_charset_set (header_entry, filename)
     const char *header_entry;
     const char *filename;
{
  /* Verify the validity of CHARSET.  It is necessary
     1. for the correct treatment of multibyte characters containing
	0x5C bytes in the PO lexer,
     2. so that at run time, gettext() can call iconv() to convert
	msgstr.  */
  const char *charsetstr = strstr (header_entry, "charset=");

  if (charsetstr != NULL)
    {
      size_t len;
      char *charset;
      const char *canon_charset;

      charsetstr += strlen ("charset=");
      len = strcspn (charsetstr, " \t\n");
      charset = (char *) alloca (len + 1);
      memcpy (charset, charsetstr, len);
      charset[len] = '\0';

      canon_charset = po_charset_canonicalize (charset);
      if (canon_charset == NULL)
	{
	  /* Don't warn for POT files, because POT files usually contain
	     only ASCII msgids.  */
	  size_t filenamelen = strlen (filename);

	  if (!(filenamelen >= 4
		&& memcmp (filename + filenamelen - 4, ".pot", 4) == 0
		&& strcmp (charset, "CHARSET") == 0))
	    {
	      char *prefix;
	      char *msg;

	      asprintf (&prefix, _("%s: warning: "), filename);
	      asprintf (&msg, _("\
Charset \"%s\" is not a portable encoding name.\n\
Message conversion to user's charset might not work.\n"),
			charset);
	      if (prefix == NULL || msg == NULL)
		error (EXIT_FAILURE, 0, _("memory exhausted"));
	      multiline_warning (prefix, msg);
	    }
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

	  po_lex_charset = canon_charset;
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
		  size_t i;
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

		  asprintf (&prefix, _("%s: warning: "), filename);
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

		  asprintf (&prefix, _("%s: warning: "), filename);
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
      /* Don't warn for POT files, because POT files usually contain
	 only ASCII msgids.  */
      size_t filenamelen = strlen (filename);

      if (!(filenamelen >= 4
	    && memcmp (filename + filenamelen - 4, ".pot", 4) == 0))
	{
	  char *prefix;
	  char *msg;

	  asprintf (&prefix, _("%s: warning: "), filename);
	  asprintf (&msg, _("\
Charset missing in header.\n\
Message conversion to user's charset will not work.\n"));
	  if (prefix == NULL || msg == NULL)
	    error (EXIT_FAILURE, 0, _("memory exhausted"));
	  multiline_warning (prefix, msg);
	}
    }
}

void
po_lex_charset_close ()
{
  po_lex_charset = NULL;
#if HAVE_ICONV
  if (po_lex_iconv != (iconv_t)(-1))
    {
      iconv_close (po_lex_iconv);
      po_lex_iconv = (iconv_t)(-1);
    }
#endif
}
