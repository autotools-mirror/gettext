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

/* Specification.  */
#include "po.h"

#include <stdlib.h>
#include <string.h>

#include "po-charset.h"
#include "po-hash.h"
#include "xmalloc.h"

/* Prototypes for local functions.  Needed to ensure compiler checking of
   function argument counts despite of K&R C function definition syntax.  */
static void po_parse_brief PARAMS ((po_ty *pop));
static void po_parse_debrief PARAMS ((po_ty *pop));

/* Methods used indirectly by po_scan.  */
static void po_directive_domain PARAMS ((po_ty *pop, char *name));
static void po_directive_message PARAMS ((po_ty *pop, char *msgid,
					  lex_pos_ty *msgid_pos,
					  char *msgid_plural,
					  char *msgstr, size_t msgstr_len,
					  lex_pos_ty *msgstr_pos,
					  bool obsolete));
static void po_comment PARAMS ((po_ty *pop, const char *s));
static void po_comment_dot PARAMS ((po_ty *pop, const char *s));
static void po_comment_filepos PARAMS ((po_ty *pop, const char *name,
					size_t line));
static void po_comment_special PARAMS ((po_ty *pop, const char *s));

/* Local variables.  */
static po_ty *callback_arg;


po_ty *
po_alloc (pomp)
     po_method_ty *pomp;
{
  po_ty *pop;

  pop = (po_ty *) xmalloc (pomp->size);
  pop->method = pomp;
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
po_scan (pop, fp, real_filename, logical_filename)
     po_ty *pop;
     FILE *fp;
     const char *real_filename;
     const char *logical_filename;
{
  extern int po_gram_parse PARAMS ((void));

  /* The parse will call the po_callback_... functions (see below)
     when the various directive are recognised.  The callback_arg
     variable is used to tell these functions which instance is to
     have the relevant method invoked.  */
  callback_arg = pop;

  /* Parse the stream's content.  */
  lex_start (fp, real_filename, logical_filename);
  po_parse_brief (pop);
  po_gram_parse ();
  po_parse_debrief (pop);
  lex_end ();
  callback_arg = NULL;
}


void
po_scan_file (pop, filename)
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
		      msgstr, msgstr_len, msgstr_pos, obsolete)
     po_ty *pop;
     char *msgid;
     lex_pos_ty *msgid_pos;
     char *msgid_plural;
     char *msgstr;
     size_t msgstr_len;
     lex_pos_ty *msgstr_pos;
     bool obsolete;
{
  if (pop->method->directive_message)
    pop->method->directive_message (pop, msgid, msgid_pos, msgid_plural,
				    msgstr, msgstr_len, msgstr_pos, obsolete);
}


void
po_callback_message (msgid, msgid_pos, msgid_plural,
		     msgstr, msgstr_len, msgstr_pos,
		     obsolete)
     char *msgid;
     lex_pos_ty *msgid_pos;
     char *msgid_plural;
     char *msgstr;
     size_t msgstr_len;
     lex_pos_ty *msgstr_pos;
     bool obsolete;
{
  /* assert(callback_arg); */

  /* Test for header entry.  Ignore fuzziness of the header entry.  */
  if (msgid[0] == '\0' && !obsolete)
    po_lex_charset_set (msgstr, gram_pos.file_name);

  po_directive_message (callback_arg, msgid, msgid_pos, msgid_plural,
			msgstr, msgstr_len, msgstr_pos, obsolete);
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
	  && s[3] == 'l' && s[4] == 'e' && s[5] == ':'
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
     size_t line;
{
  if (pop->method->comment_filepos)
    pop->method->comment_filepos (pop, name, line);
}


void
po_callback_comment_filepos (name, line)
     const char *name;
     size_t line;
{
  /* assert(callback_arg); */
  po_comment_filepos (callback_arg, name, line);
}


/* Parse a special comment and put the result in *fuzzyp, formatp, *wrapp.  */
void
po_parse_comment_special (s, fuzzyp, formatp, wrapp)
     const char *s;
     bool *fuzzyp;
     enum is_format formatp[NFORMATS];
     enum is_wrap *wrapp;
{
  size_t i;

  *fuzzyp = false;
  for (i = 0; i < NFORMATS; i++)
    formatp[i] = undecided;
  *wrapp = undecided;

  while (*s != '\0')
    {
      const char *t;

      /* Skip whitespace.  */
      while (*s != '\0' && strchr ("\n \t\r\f\v,", *s) != NULL)
	s++;

      /* Collect a token.  */
      t = s;
      while (*s != '\0' && strchr ("\n \t\r\f\v,", *s) == NULL)
	s++;
      if (s != t)
	{
	  size_t len = s - t;

	  /* Accept fuzzy flag.  */
	  if (len == 5 && memcmp (t, "fuzzy", 5) == 0)
	    {
	      *fuzzyp = true;
	      continue;
	    }

	  /* Accept format description.  */
	  if (len >= 7 && memcmp (t + len - 7, "-format", 7) == 0)
	    {
	      const char *p;
	      size_t n;
	      enum is_format value;

	      p = t;
	      n = len - 7;

	      if (n >= 3 && memcmp (p, "no-", 3) == 0)
		{
		  p += 3;
		  n -= 3;
		  value = no;
		}
	      else if (n >= 9 && memcmp (p, "possible-", 9) == 0)
		{
		  p += 9;
		  n -= 9;
		  value = possible;
		}
	      else if (n >= 11 && memcmp (p, "impossible-", 11) == 0)
		{
		  p += 11;
		  n -= 11;
		  value = impossible;
		}
	      else
		value = yes;

	      for (i = 0; i < NFORMATS; i++)
		if (strlen (format_language[i]) == n
		    && memcmp (format_language[i], p, n) == 0)
		  {
		    formatp[i] = value;
		    break;
		  }
	      if (i < NFORMATS)
		continue;
	    }

	  /* Accept wrap description.  */
	  if (len == 4 && memcmp (t, "wrap", 4) == 0)
	    {
	      *wrapp = yes;
	      continue;
	    }
	  if (len == 7 && memcmp (t, "no-wrap", 7) == 0)
	    {
	      *wrapp = no;
	      continue;
	    }

	  /* Unknown special comment marker.  It may have been generated
	     from a future xgettext version.  Ignore it.  */
	}
    }
}
