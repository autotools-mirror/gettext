/* xgettext PO backend.
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
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "message.h"
#include "x-po.h"
#include "xgettext.h"
#include "xmalloc.h"
#include "po.h"
#include "po-lex.h"
#include "gettext.h"

/* A convenience macro.  I don't like writing gettext() every time.  */
#define _(str) gettext (str)


/* Prototypes for local functions.  Needed to ensure compiler checking of
   function argument counts despite of K&R C function definition syntax.  */
static void extract_constructor PARAMS ((po_ty *that));
static void extract_directive_domain PARAMS ((po_ty *that, char *name));
static void extract_directive_message PARAMS ((po_ty *that, char *msgid,
					       lex_pos_ty *msgid_pos,
					       char *msgid_plural,
					       char *msgstr, size_t msgstr_len,
					       lex_pos_ty *msgstr_pos,
					       bool obsolete));
static void extract_parse_brief PARAMS ((po_ty *that));
static void extract_comment PARAMS ((po_ty *that, const char *s));
static void extract_comment_dot PARAMS ((po_ty *that, const char *s));
static void extract_comment_filepos PARAMS ((po_ty *that, const char *name,
					     size_t line));
static void extract_comment_special PARAMS ((po_ty *that, const char *s));


typedef struct extract_class_ty extract_class_ty;
struct extract_class_ty
{
  /* Inherited instance variables and methods.  */
  PO_BASE_TY

  /* Cumulative list of messages.  */
  message_list_ty *mlp;

  /* Cumulative comments for next message.  */
  string_list_ty *comment;
  string_list_ty *comment_dot;

  bool is_fuzzy;
  enum is_format is_format[NFORMATS];
  enum is_wrap do_wrap;

  size_t filepos_count;
  lex_pos_ty *filepos;
};


static void
extract_constructor (that)
     po_ty *that;
{
  extract_class_ty *this = (extract_class_ty *) that;
  size_t i;

  this->mlp = NULL; /* actually set in extract_po, below */
  this->comment = NULL;
  this->comment_dot = NULL;
  this->is_fuzzy = false;
  for (i = 0; i < NFORMATS; i++)
    this->is_format[i] = undecided;
  this->do_wrap = undecided;
  this->filepos_count = 0;
  this->filepos = NULL;
}


static void
extract_directive_domain (that, name)
     po_ty *that;
     char *name;
{
  po_gram_error_at_line (&gram_pos,
			 _("this file may not contain domain directives"));
}


static void
extract_directive_message (that, msgid, msgid_pos, msgid_plural,
			   msgstr, msgstr_len, msgstr_pos, obsolete)
     po_ty *that;
     char *msgid;
     lex_pos_ty *msgid_pos;
     char *msgid_plural;
     char *msgstr;
     size_t msgstr_len;
     lex_pos_ty *msgstr_pos;
     bool obsolete;
{
  extract_class_ty *this = (extract_class_ty *)that;
  message_ty *mp;
  size_t j, i;

  /* See whether we shall exclude this message.  */
  if (exclude != NULL && message_list_search (exclude, msgid) != NULL)
    goto discard;

  /* If the msgid is the empty string, it is the old header.  Throw it
     away, we have constructed a new one.
     But if no new one was constructed, keep the old header.  This is useful
     because the old header may contain a charset= directive.  */
  if (*msgid == '\0' && !xgettext_omit_header)
    {
      discard:
      free (msgid);
      free (msgstr);
      if (this->comment != NULL)
	string_list_free (this->comment);
      if (this->comment_dot != NULL)
	string_list_free (this->comment_dot);
      if (this->filepos != NULL)
	free (this->filepos);
      this->comment = NULL;
      this->comment_dot = NULL;
      this->filepos_count = 0;
      this->filepos = NULL;
      this->is_fuzzy = false;
      for (i = 0; i < NFORMATS; i++)
	this->is_format[i] = undecided;
      this->do_wrap = undecided;
      return;
    }

  /* See if this message ID has been seen before.  */
  mp = message_list_search (this->mlp, msgid);
  if (mp)
    {
      if (msgstr_len != mp->msgstr_len
	  || memcmp (msgstr, mp->msgstr, msgstr_len) != 0)
	{
	  po_gram_error_at_line (msgid_pos, _("duplicate message definition"));
	  po_gram_error_at_line (&mp->pos, _("\
...this is the location of the first definition"));
	}
      free (msgid);
      free (msgstr);
    }
  else
    {
      mp = message_alloc (msgid, msgid_plural, msgstr, msgstr_len, msgstr_pos);
      message_list_append (this->mlp, mp);
    }

  /* Add the accumulated comments to the message.  Clear the
     accumulation in preparation for the next message. */
  if (this->comment != NULL)
    {
      for (j = 0; j < this->comment->nitems; ++j)
	message_comment_append (mp, this->comment->item[j]);
      string_list_free (this->comment);
      this->comment = NULL;
    }
  if (this->comment_dot != NULL)
    {
      for (j = 0; j < this->comment_dot->nitems; ++j)
	message_comment_dot_append (mp, this->comment_dot->item[j]);
      string_list_free (this->comment_dot);
      this->comment_dot = NULL;
    }
  mp->is_fuzzy = this->is_fuzzy;
  for (i = 0; i < NFORMATS; i++)
    mp->is_format[i] = this->is_format[i];
  mp->do_wrap = this->do_wrap;
  for (j = 0; j < this->filepos_count; ++j)
    {
      lex_pos_ty *pp;

      pp = &this->filepos[j];
      message_comment_filepos (mp, pp->file_name, pp->line_number);
      free (pp->file_name);
    }
  if (this->filepos != NULL)
    free (this->filepos);
  this->filepos_count = 0;
  this->filepos = NULL;
  this->is_fuzzy = false;
  for (i = 0; i < NFORMATS; i++)
    this->is_format[i] = undecided;
  this->do_wrap = undecided;
}


static void
extract_parse_brief (that)
     po_ty *that;
{
  po_lex_pass_comments (true);
}


static void
extract_comment (that, s)
     po_ty *that;
     const char *s;
{
  extract_class_ty *this = (extract_class_ty *) that;

  if (this->comment == NULL)
    this->comment = string_list_alloc ();
  string_list_append (this->comment, s);
}


static void
extract_comment_dot (that, s)
     po_ty *that;
     const char *s;
{
  extract_class_ty *this = (extract_class_ty *) that;

  if (this->comment_dot == NULL)
    this->comment_dot = string_list_alloc ();
  string_list_append (this->comment_dot, s);
}


static void
extract_comment_filepos (that, name, line)
     po_ty *that;
     const char *name;
     size_t line;
{
  extract_class_ty *this = (extract_class_ty *) that;
  size_t nbytes;
  lex_pos_ty *pp;

  /* Write line numbers only if -n option is given.  */
  if (line_comment != 0)
    {
      nbytes = (this->filepos_count + 1) * sizeof (this->filepos[0]);
      this->filepos = xrealloc (this->filepos, nbytes);
      pp = &this->filepos[this->filepos_count++];
      pp->file_name = xstrdup (name);
      pp->line_number = line;
    }
}


static void
extract_comment_special (that, s)
     po_ty *that;
     const char *s;
{
  extract_class_ty *this = (extract_class_ty *) that;

  po_parse_comment_special (s, &this->is_fuzzy, this->is_format,
			    &this->do_wrap);
}


/* So that the one parser can be used for multiple programs, and also
   use good data hiding and encapsulation practices, an object
   oriented approach has been taken.  An object instance is allocated,
   and all actions resulting from the parse will be through
   invocations of method functions of that object.  */

static po_method_ty extract_methods =
{
  sizeof (extract_class_ty),
  extract_constructor,
  NULL, /* destructor */
  extract_directive_domain,
  extract_directive_message,
  extract_parse_brief,
  NULL, /* parse_debrief */
  extract_comment,
  extract_comment_dot,
  extract_comment_filepos,
  extract_comment_special
};


void
extract_po (fp, real_filename, logical_filename, mdlp)
     FILE *fp;
     const char *real_filename;
     const char *logical_filename;
     msgdomain_list_ty *mdlp;
{
  po_ty *pop = po_alloc (&extract_methods);
  ((extract_class_ty *) pop)->mlp = mdlp->item[0]->messages;
  po_scan (pop, fp, real_filename, logical_filename);
  po_free (pop);
}
