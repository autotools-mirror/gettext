/* Reading PO files.
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

/* Specification.  */
#include "read-po.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "po.h"
#include "xmalloc.h"
#include "gettext.h"

#define _(str) gettext (str)


/* If nonzero, remember comments for file name and line number for each
   msgid, if present in the reference input.  Defaults to true.  */
int line_comment = 1;

/* If false, duplicate msgids in the same domain and file generate an error.
   If true, such msgids are allowed; the caller should treat them
   appropriately.  Defaults to false.  */
bool allow_duplicates = false;


/* This structure defines a derived class of the po_ty class.  (See
   po.h for an explanation.)  */
typedef struct readall_class_ty readall_class_ty;
struct readall_class_ty
{
  /* inherited instance variables, etc.  */
  PO_BASE_TY

  /* List of messages already appeared in the current file.  */
  msgdomain_list_ty *mdlp;

  /* Name of domain we are currently examining.  */
  char *domain;

  /* List of messages belonging to the current domain.  */
  message_list_ty *mlp;

  /* Accumulate comments for next message directive.  */
  string_list_ty *comment;
  string_list_ty *comment_dot;

  /* Flags transported in special comments.  */
  bool is_fuzzy;
  enum is_format is_format[NFORMATS];
  enum is_wrap do_wrap;

  /* Accumulate filepos comments for the next message directive.  */
  size_t filepos_count;
  lex_pos_ty *filepos;
};


/* Prototypes for local functions.  Needed to ensure compiler checking of
   function argument counts despite of K&R C function definition syntax.  */
static void readall_constructor PARAMS ((po_ty *that));
static void readall_destructor PARAMS ((po_ty *that));
static void readall_directive_domain PARAMS ((po_ty *that, char *name));
static void readall_directive_message PARAMS ((po_ty *that, char *msgid,
					       lex_pos_ty *msgid_pos,
					       char *msgid_plural,
					       char *msgstr, size_t msgstr_len,
					       lex_pos_ty *msgstr_pos,
					       bool obsolete));
static void readall_parse_brief PARAMS ((po_ty *that));
static void readall_comment PARAMS ((po_ty *that, const char *s));
static void readall_comment_dot PARAMS ((po_ty *that, const char *s));
static void readall_comment_special PARAMS ((po_ty *that, const char *s));
static void readall_comment_filepos PARAMS ((po_ty *that, const char *name,
					     size_t line));


/* Prepare for first message.  */
static void
readall_constructor (that)
     po_ty *that;
{
  readall_class_ty *this = (readall_class_ty *) that;
  size_t i;

  this->mdlp = msgdomain_list_alloc (!allow_duplicates);
  this->domain = MESSAGE_DOMAIN_DEFAULT;
  this->mlp = msgdomain_list_sublist (this->mdlp, this->domain, true);
  this->comment = NULL;
  this->comment_dot = NULL;
  this->filepos_count = 0;
  this->filepos = NULL;
  this->is_fuzzy = false;
  for (i = 0; i < NFORMATS; i++)
    this->is_format[i] = undecided;
  this->do_wrap = undecided;
}


static void
readall_destructor (that)
     po_ty *that;
{
  readall_class_ty *this = (readall_class_ty *) that;
  size_t j;

  /* Do not free this->mdlp and this->mlp.  */
  if (this->comment != NULL)
    string_list_free (this->comment);
  if (this->comment_dot != NULL)
    string_list_free (this->comment_dot);
  for (j = 0; j < this->filepos_count; ++j)
    free (this->filepos[j].file_name);
  if (this->filepos != NULL)
    free (this->filepos);
}


/* Process 'domain' directive from .po file.  */
static void
readall_directive_domain (that, name)
     po_ty *that;
     char *name;
{
  size_t j;

  readall_class_ty *this = (readall_class_ty *) that;
  /* Override current domain name.  Don't free memory.  */
  this->domain = name;

  /* If there are accumulated comments, throw them away, they are
     probably part of the file header, or about the domain directive,
     and will be unrelated to the next message.  */
  if (this->comment != NULL)
    {
      string_list_free (this->comment);
      this->comment = NULL;
    }
  if (this->comment_dot != NULL)
    {
      string_list_free (this->comment_dot);
      this->comment_dot = NULL;
    }
  for (j = 0; j < this->filepos_count; ++j)
    free (this->filepos[j].file_name);
  if (this->filepos != NULL)
    free (this->filepos);
  this->filepos_count = 0;
  this->filepos = NULL;
}


/* Process 'msgid'/'msgstr' pair from .po file.  */
static void
readall_directive_message (that, msgid, msgid_pos, msgid_plural,
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
  readall_class_ty *this = (readall_class_ty *) that;
  message_ty *mp;
  size_t j, i;

  /* Select the appropriate sublist of this->mdlp.  */
  this->mlp = msgdomain_list_sublist (this->mdlp, this->domain, true);

  if (allow_duplicates && msgid[0] != '\0')
    /* Doesn't matter if this message ID has been seen before.  */
    mp = NULL;
  else
    /* See if this message ID has been seen before.  */
    mp = message_list_search (this->mlp, msgid);

  if (mp)
    {
      po_gram_error_at_line (msgid_pos, _("duplicate message definition"));
      po_gram_error_at_line (&mp->pos, _("\
...this is the location of the first definition"));
      free (msgstr);
      free (msgid);
    }
  else
    {
      mp = message_alloc (msgid, msgid_plural, msgstr, msgstr_len, msgstr_pos);
      message_list_append (this->mlp, mp);
      mp->obsolete = obsolete;
    }

  /* Add the accumulated comments to the message.  Clear the
     accumulation in preparation for the next message.  */
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
  for (j = 0; j < this->filepos_count; ++j)
    {
      lex_pos_ty *pp;

      pp = &this->filepos[j];
      message_comment_filepos (mp, pp->file_name, pp->line_number);
      free (pp->file_name);
    }
  mp->is_fuzzy = this->is_fuzzy;
  for (i = 0; i < NFORMATS; i++)
    mp->is_format[i] = this->is_format[i];
  mp->do_wrap = this->do_wrap;

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
readall_parse_brief (that)
     po_ty *that;
{
  po_lex_pass_comments (true);
}


static void
readall_comment (that, s)
     po_ty *that;
     const char *s;
{
  readall_class_ty *this = (readall_class_ty *) that;

  if (this->comment == NULL)
    this->comment = string_list_alloc ();
  string_list_append (this->comment, s);
}


static void
readall_comment_dot (that, s)
     po_ty *that;
     const char *s;
{
  readall_class_ty *this = (readall_class_ty *) that;

  if (this->comment_dot == NULL)
    this->comment_dot = string_list_alloc ();
  string_list_append (this->comment_dot, s);
}


/* Test for '#, fuzzy' comments and warn.  */
static void
readall_comment_special (that, s)
     po_ty *that;
     const char *s;
{
  readall_class_ty *this = (readall_class_ty *) that;

  po_parse_comment_special (s, &this->is_fuzzy, this->is_format,
			    &this->do_wrap);
}


static void
readall_comment_filepos (that, name, line)
     po_ty *that;
     const char *name;
     size_t line;
{
  readall_class_ty *this = (readall_class_ty *) that;
  size_t nbytes;
  lex_pos_ty *pp;

  if (!line_comment)
    return;
  nbytes = (this->filepos_count + 1) * sizeof (this->filepos[0]);
  this->filepos = xrealloc (this->filepos, nbytes);
  pp = &this->filepos[this->filepos_count++];
  pp->file_name = xstrdup (name);
  pp->line_number = line;
}


/* So that the one parser can be used for multiple programs, and also
   use good data hiding and encapsulation practices, an object
   oriented approach has been taken.  An object instance is allocated,
   and all actions resulting from the parse will be through
   invocations of method functions of that object.  */

static po_method_ty readall_methods =
{
  sizeof (readall_class_ty),
  readall_constructor,
  readall_destructor,
  readall_directive_domain,
  readall_directive_message,
  readall_parse_brief,
  NULL, /* parse_debrief */
  readall_comment,
  readall_comment_dot,
  readall_comment_filepos,
  readall_comment_special
};


msgdomain_list_ty *
read_po (fp, real_filename, logical_filename)
     FILE *fp;
     const char *real_filename;
     const char *logical_filename;
{
  po_ty *pop;
  msgdomain_list_ty *mdlp;

  pop = po_alloc (&readall_methods);
  po_lex_pass_obsolete_entries (true);
  po_scan (pop, fp, real_filename, logical_filename);
  mdlp = ((readall_class_ty *) pop)->mdlp;
  po_free (pop);
  return mdlp;
}


msgdomain_list_ty *
read_po_file (filename)
     const char *filename;
{
  po_ty *pop;
  msgdomain_list_ty *mdlp;

  pop = po_alloc (&readall_methods);
  po_lex_pass_obsolete_entries (true);
  po_scan_file (pop, filename);
  mdlp = ((readall_class_ty *) pop)->mdlp;
  po_free (pop);
  return mdlp;
}
