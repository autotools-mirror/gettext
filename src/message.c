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

#include <stdlib.h>
#include <string.h>

#include "fstrcmp.h"
#include "message.h"
#include "system.h"


/* Prototypes for local functions.  Needed to ensure compiler checking of
   function argument counts despite of K&R C function definition syntax.  */
static message_ty *message_list_search_fuzzy_inner PARAMS ((
       message_list_ty *mlp, const char *msgid, double *best_weight_p));


enum is_c_format
parse_c_format_description_string (s)
     const char *s;
{
  if (strstr (s, "no-c-format") != NULL)
    return no;
  else if (strstr (s, "impossible-c-format") != NULL)
    return impossible;
  else if (strstr (s, "possible-c-format") != NULL)
    return possible;
  else if (strstr (s, "c-format") != NULL)
    return yes;
  return undecided;
}


int
possible_c_format_p (is_c_format)
     enum is_c_format is_c_format;
{
  return is_c_format == possible || is_c_format == yes;
}


enum is_c_format
parse_c_width_description_string (s)
     const char *s;
{
  if (strstr (s, "no-wrap") != NULL)
    return no;
  else if (strstr (s, "wrap") != NULL)
    return yes;
  return undecided;
}


message_ty *
message_alloc (msgid, msgid_plural, msgstr, msgstr_len, pp)
     const char *msgid;
     const char *msgid_plural;
     const char *msgstr;
     size_t msgstr_len;
     const lex_pos_ty *pp;
{
  message_ty *mp;

  mp = (message_ty *) xmalloc (sizeof (message_ty));
  mp->msgid = msgid;
  mp->msgid_plural = (msgid_plural != NULL ? xstrdup (msgid_plural) : NULL);
  mp->msgstr = msgstr;
  mp->msgstr_len = msgstr_len;
  mp->pos = *pp;
  mp->comment = NULL;
  mp->comment_dot = NULL;
  mp->filepos_count = 0;
  mp->filepos = NULL;
  mp->is_fuzzy = false;
  mp->is_c_format = undecided;
  mp->do_wrap = undecided;
  mp->used = 0;
  mp->obsolete = false;
  return mp;
}


void
message_free (mp)
     message_ty *mp;
{
  size_t j;

  free ((char *) mp->msgid);
  if (mp->msgid_plural != NULL)
    free ((char *) mp->msgid_plural);
  free ((char *) mp->msgstr);
  if (mp->comment != NULL)
    string_list_free (mp->comment);
  if (mp->comment_dot != NULL)
    string_list_free (mp->comment_dot);
  for (j = 0; j < mp->filepos_count; ++j)
    free ((char *) mp->filepos[j].file_name);
  if (mp->filepos != NULL)
    free (mp->filepos);
  free (mp);
}


void
message_comment_append (mp, s)
     message_ty *mp;
     const char *s;
{
  if (mp->comment == NULL)
    mp->comment = string_list_alloc ();
  string_list_append (mp->comment, s);
}


void
message_comment_dot_append (mp, s)
     message_ty *mp;
     const char *s;
{
  if (mp->comment_dot == NULL)
    mp->comment_dot = string_list_alloc ();
  string_list_append (mp->comment_dot, s);
}


void
message_comment_filepos (mp, name, line)
     message_ty *mp;
     const char *name;
     size_t line;
{
  size_t j;
  size_t nbytes;
  lex_pos_ty *pp;

  /* See if we have this position already.  */
  for (j = 0; j < mp->filepos_count; j++)
    {
      pp = &mp->filepos[j];
      if (strcmp (pp->file_name, name) == 0 && pp->line_number == line)
	return;
    }

  /* Extend the list so that we can add a position to it.  */
  nbytes = (mp->filepos_count + 1) * sizeof (mp->filepos[0]);
  mp->filepos = xrealloc (mp->filepos, nbytes);

  /* Insert the position at the end.  Don't sort the file positions here.  */
  pp = &mp->filepos[mp->filepos_count++];
  pp->file_name = xstrdup (name);
  pp->line_number = line;
}


message_ty *
message_copy (mp)
     message_ty *mp;
{
  message_ty *result;
  size_t j;

  result = message_alloc (xstrdup (mp->msgid), mp->msgid_plural,
			  mp->msgstr, mp->msgstr_len, &mp->pos);

  if (mp->comment)
    {
      for (j = 0; j < mp->comment->nitems; ++j)
	message_comment_append (result, mp->comment->item[j]);
    }
  if (mp->comment_dot)
    {
      for (j = 0; j < mp->comment_dot->nitems; ++j)
	message_comment_dot_append (result, mp->comment_dot->item[j]);
    }
  result->is_fuzzy = mp->is_fuzzy;
  result->is_c_format = mp->is_c_format;
  result->do_wrap = mp->do_wrap;
  for (j = 0; j < mp->filepos_count; ++j)
    {
      lex_pos_ty *pp = &mp->filepos[j];
      message_comment_filepos (result, pp->file_name, pp->line_number);
    }
  return result;
}


message_ty *
message_merge (def, ref)
     message_ty *def;
     message_ty *ref;
{
  const char *msgstr;
  size_t msgstr_len;
  message_ty *result;
  size_t j;

  /* Take the msgid from the reference.  When fuzzy matches are made,
     the definition will not be unique, but the reference will be -
     usually because it has only been slightly changed.  */

  /* Take the msgstr from the definition.  The msgstr of the reference
     is usually empty, as it was generated by xgettext.  If we currently
     process the header entry we have to merge the msgstr by using the
     POT-Creation-Date field from the reference.  */
  if (ref->msgid[0] == '\0')
    {
      /* Oh, oh.  The header entry and we have something to fill in.  */
      static const struct
      {
	const char *name;
	size_t len;
      } known_fields[] =
      {
	{ "Project-Id-Version:", sizeof ("Project-Id-Version:") - 1 },
#define PROJECT_ID	0
	{ "POT-Creation-Date:", sizeof ("POT-Creation-Date:") - 1 },
#define POT_CREATION	1
	{ "PO-Revision-Date:", sizeof ("PO-Revision-Date:") - 1 },
#define PO_REVISION	2
	{ "Last-Translator:", sizeof ("Last-Translator:") - 1 },
#define LAST_TRANSLATOR	3
	{ "Language-Team:", sizeof ("Language-Team:") - 1 },
#define LANGUAGE_TEAM	4
	{ "MIME-Version:", sizeof ("MIME-Version:") - 1 },
#define MIME_VERSION	5
	{ "Content-Type:", sizeof ("Content-Type:") - 1 },
#define CONTENT_TYPE	6
	{ "Content-Transfer-Encoding:",
	  sizeof ("Content-Transfer-Encoding:") - 1 }
#define CONTENT_TRANSFER 7
      };
#define UNKNOWN	8
      struct
      {
	const char *string;
	size_t len;
      } header_fields[UNKNOWN + 1];
      const char *cp;
      char *newp;
      size_t len, cnt;

      /* Clear all fields.  */
      memset (header_fields, '\0', sizeof (header_fields));

      cp = def->msgstr;
      while (*cp != '\0')
	{
	  const char *endp = strchr (cp, '\n');
	  int terminated = endp != NULL;

	  if (!terminated)
	    {
	      /* Add a trailing newline.  */
	      char *copy;
	      endp = strchr (cp, '\0');

	      len = endp - cp + 1;

	      copy = (char *) alloca (len + 1);
	      stpcpy (stpcpy (copy, cp), "\n");
	      cp = copy;
	    }
	  else
	    {
	      len = (endp - cp) + 1;
	      ++endp;
	    }

	  /* Compare with any of the known fields.  */
	  for (cnt = 0;
	       cnt < sizeof (known_fields) / sizeof (known_fields[0]);
	       ++cnt)
	    if (strncasecmp (cp, known_fields[cnt].name, known_fields[cnt].len)
		== 0)
	      break;

	  if (cnt < sizeof (known_fields) / sizeof (known_fields[0]))
	    {
	      header_fields[cnt].string = &cp[known_fields[cnt].len];
	      header_fields[cnt].len = len - known_fields[cnt].len;
	    }
	  else
	    {
	      /* It's an unknown field.  Append content to what is already
		 known.  */
	      char *extended =
		(char *) alloca (header_fields[UNKNOWN].len + len + 1);
	      memcpy (extended, header_fields[UNKNOWN].string,
		      header_fields[UNKNOWN].len);
	      memcpy (&extended[header_fields[UNKNOWN].len], cp, len);
	      extended[header_fields[UNKNOWN].len + len] = '\0';
	      header_fields[UNKNOWN].string = extended;
	      header_fields[UNKNOWN].len += len;
	    }

	  cp = endp;
	}

      {
	const char *pot_date_ptr;

	pot_date_ptr = strstr (ref->msgstr, "POT-Creation-Date:");
	if (pot_date_ptr != NULL)
	  {
	    size_t pot_date_len;
	    const char *endp;

	    pot_date_ptr += sizeof ("POT-Creation-Date:") - 1;

	    endp = strchr (pot_date_ptr, '\n');
	    if (endp == NULL)
	      {
		/* Add a trailing newline.  */
		char *extended;
		endp = strchr (pot_date_ptr, '\0');
		pot_date_len = (endp - pot_date_ptr) + 1;
		extended = (char *) alloca (pot_date_len + 1);
		stpcpy (stpcpy (extended, pot_date_ptr), "\n");
		pot_date_ptr = extended;
	      }
	    else
	      pot_date_len = (endp - pot_date_ptr) + 1;

	    header_fields[POT_CREATION].string = pot_date_ptr;
	    header_fields[POT_CREATION].len = pot_date_len;
	  }
      }

      /* Concatenate all the various fields.  */
      len = 0;
      for (cnt = 0; cnt < UNKNOWN; ++cnt)
	if (header_fields[cnt].string != NULL)
	  len += known_fields[cnt].len + header_fields[cnt].len;
      len += header_fields[UNKNOWN].len;

      cp = newp = (char *) xmalloc (len + 1);
      newp[len] = '\0';

#define IF_FILLED(idx)							      \
      if (header_fields[idx].string)					      \
	newp = stpncpy (stpcpy (newp, known_fields[idx].name),		      \
			header_fields[idx].string, header_fields[idx].len)

      IF_FILLED (PROJECT_ID);
      IF_FILLED (POT_CREATION);
      IF_FILLED (PO_REVISION);
      IF_FILLED (LAST_TRANSLATOR);
      IF_FILLED (LANGUAGE_TEAM);
      IF_FILLED (MIME_VERSION);
      IF_FILLED (CONTENT_TYPE);
      IF_FILLED (CONTENT_TRANSFER);
      if (header_fields[UNKNOWN].string != NULL)
	stpcpy (newp, header_fields[UNKNOWN].string);

#undef IF_FILLED

      msgstr = cp;
      msgstr_len = strlen (cp) + 1;
    }
  else
    {
      msgstr = def->msgstr;
      msgstr_len = def->msgstr_len;
    }

  result = message_alloc (xstrdup (ref->msgid), ref->msgid_plural,
			  msgstr, msgstr_len, &def->pos);

  /* Take the comments from the definition file.  There will be none at
     all in the reference file, as it was generated by xgettext.  */
  if (def->comment)
    for (j = 0; j < def->comment->nitems; ++j)
      message_comment_append (result, def->comment->item[j]);

  /* Take the dot comments from the reference file, as they are
     generated by xgettext.  Any in the definition file are old ones
     collected by previous runs of xgettext and msgmerge.  */
  if (ref->comment_dot)
    for (j = 0; j < ref->comment_dot->nitems; ++j)
      message_comment_dot_append (result, ref->comment_dot->item[j]);

  /* The flags are mixed in a special way.  Some informations come
     from the reference message (such as format/no-format), others
     come from the definition file (fuzzy or not).  */
  result->is_fuzzy = def->is_fuzzy;
  result->is_c_format = ref->is_c_format;
  result->do_wrap = ref->do_wrap;

  /* Take the file position comments from the reference file, as they
     are generated by xgettext.  Any in the definition file are old ones
     collected by previous runs of xgettext and msgmerge.  */
  for (j = 0; j < ref->filepos_count; ++j)
    {
      lex_pos_ty *pp = &ref->filepos[j];
      message_comment_filepos (result, pp->file_name, pp->line_number);
    }

  /* All done, return the merged message to the caller.  */
  return result;
}


message_list_ty *
message_list_alloc ()
{
  message_list_ty *mlp;

  mlp = (message_list_ty *) xmalloc (sizeof (message_list_ty));
  mlp->nitems = 0;
  mlp->nitems_max = 0;
  mlp->item = NULL;
  return mlp;
}


void
message_list_free (mlp)
     message_list_ty *mlp;
{
  size_t j;

  for (j = 0; j < mlp->nitems; ++j)
    message_free (mlp->item[j]);
  if (mlp->item)
    free (mlp->item);
  free (mlp);
}


void
message_list_append (mlp, mp)
     message_list_ty *mlp;
     message_ty *mp;
{
  if (mlp->nitems >= mlp->nitems_max)
    {
      size_t nbytes;

      mlp->nitems_max = mlp->nitems_max * 2 + 4;
      nbytes = mlp->nitems_max * sizeof (message_ty *);
      mlp->item = xrealloc (mlp->item, nbytes);
    }
  mlp->item[mlp->nitems++] = mp;
}


void
message_list_prepend (mlp, mp)
     message_list_ty *mlp;
     message_ty *mp;
{
  size_t j;

  if (mlp->nitems >= mlp->nitems_max)
    {
      size_t nbytes;

      mlp->nitems_max = mlp->nitems_max * 2 + 4;
      nbytes = mlp->nitems_max * sizeof (message_ty *);
      mlp->item = xrealloc (mlp->item, nbytes);
    }
  for (j = mlp->nitems; j > 0; j--)
    mlp->item[j] = mlp->item[j - 1];
  mlp->item[0] = mp;
  mlp->nitems++;
}


#if 0 /* unused */
void
message_list_delete_nth (mlp, n)
     message_list_ty *mlp;
     size_t n;
{
  size_t j;

  if (n >= mlp->nitems)
    return;
  message_free (mlp->item[n]);
  for (j = n + 1; j < mlp->nitems; ++j)
    mlp->item[j - 1] = mlp->item[j];
  mlp->nitems--;
}
#endif


void
message_list_remove_if_not (mlp, predicate)
     message_list_ty *mlp;
     message_predicate_ty *predicate;
{
  size_t i, j;

  for (j = 0, i = 0; j < mlp->nitems; j++)
    if (predicate (mlp->item[j]))
      mlp->item[i++] = mlp->item[j];
  mlp->nitems = i;
}


message_ty *
message_list_search (mlp, msgid)
     message_list_ty *mlp;
     const char *msgid;
{
  size_t j;

  for (j = 0; j < mlp->nitems; ++j)
    {
      message_ty *mp;

      mp = mlp->item[j];
      if (strcmp (msgid, mp->msgid) == 0)
	return mp;
    }
  return NULL;
}


static message_ty *
message_list_search_fuzzy_inner (mlp, msgid, best_weight_p)
     message_list_ty *mlp;
     const char *msgid;
     double *best_weight_p;
{
  size_t j;
  message_ty *best_mp;

  best_mp = NULL;
  for (j = 0; j < mlp->nitems; ++j)
    {
      message_ty *mp;

      mp = mlp->item[j];

      if (mp->msgstr != NULL && mp->msgstr[0] != '\0')
	{
	  double weight = fstrcmp (msgid, mp->msgid);
	  if (weight > *best_weight_p)
	    {
	      *best_weight_p = weight;
	      best_mp = mp;
	    }
	}
    }
  return best_mp;
}


message_ty *
message_list_search_fuzzy (mlp, msgid)
     message_list_ty *mlp;
     const char *msgid;
{
  double best_weight;

  best_weight = 0.6;
  return message_list_search_fuzzy_inner (mlp, msgid, &best_weight);
}


message_list_list_ty *
message_list_list_alloc ()
{
  message_list_list_ty *mllp;

  mllp = (message_list_list_ty *) xmalloc (sizeof (message_list_list_ty));
  mllp->nitems = 0;
  mllp->nitems_max = 0;
  mllp->item = NULL;
  return mllp;
}


#if 0 /* unused */
void
message_list_list_free (mllp)
     message_list_list_ty *mllp;
{
  size_t j;

  for (j = 0; j < mllp->nitems; ++j)
    message_list_free (mllp->item[j]);
  if (mllp->item)
    free (mllp->item);
  free (mllp);
}
#endif


void
message_list_list_append (mllp, mlp)
     message_list_list_ty *mllp;
     message_list_ty *mlp;
{
  if (mllp->nitems >= mllp->nitems_max)
    {
      size_t nbytes;

      mllp->nitems_max = mllp->nitems_max * 2 + 4;
      nbytes = mllp->nitems_max * sizeof (message_list_ty *);
      mllp->item = xrealloc (mllp->item, nbytes);
    }
  mllp->item[mllp->nitems++] = mlp;
}


void
message_list_list_append_list (mllp, mllp2)
     message_list_list_ty *mllp;
     message_list_list_ty *mllp2;
{
  size_t j;

  for (j = 0; j < mllp2->nitems; ++j)
    message_list_list_append (mllp, mllp2->item[j]);
}


message_ty *
message_list_list_search (mllp, msgid)
     message_list_list_ty *mllp;
     const char *msgid;
{
  size_t j;

  for (j = 0; j < mllp->nitems; ++j)
    {
      message_list_ty *mlp;
      message_ty *mp;

      mlp = mllp->item[j];
      mp = message_list_search (mlp, msgid);
      if (mp)
        return mp;
    }
  return NULL;
}


message_ty *
message_list_list_search_fuzzy (mllp, msgid)
     message_list_list_ty *mllp;
     const char *msgid;
{
  size_t j;
  double best_weight;
  message_ty *best_mp;

  best_weight = 0.6;
  best_mp = NULL;
  for (j = 0; j < mllp->nitems; ++j)
    {
      message_list_ty *mlp;
      message_ty *mp;

      mlp = mllp->item[j];
      mp = message_list_search_fuzzy_inner (mlp, msgid, &best_weight);
      if (mp)
	best_mp = mp;
    }
  return best_mp;
}


msgdomain_ty*
msgdomain_alloc (domain)
     const char *domain;
{
  msgdomain_ty *mdp;

  mdp = (msgdomain_ty *) xmalloc (sizeof (msgdomain_ty));
  mdp->domain = domain;
  mdp->messages = message_list_alloc ();
  return mdp;
}


void
msgdomain_free (mdp)
     msgdomain_ty *mdp;
{
  message_list_free (mdp->messages);
  free (mdp);
}


msgdomain_list_ty *
msgdomain_list_alloc ()
{
  msgdomain_list_ty *mdlp;

  mdlp = (msgdomain_list_ty *) xmalloc (sizeof (msgdomain_list_ty));
  /* Put the default domain first, so that when we output it,
     we can omit the 'domain' directive.  */
  mdlp->nitems = 1;
  mdlp->nitems_max = 1;
  mdlp->item =
    (msgdomain_ty **) xmalloc (mdlp->nitems_max * sizeof (msgdomain_ty *));
  mdlp->item[0] = msgdomain_alloc (MESSAGE_DOMAIN_DEFAULT);
  return mdlp;
}


#if 0 /* unused */
void
msgdomain_list_free (mdlp)
     msgdomain_list_ty *mdlp;
{
  size_t j;

  for (j = 0; j < mdlp->nitems; ++j)
    msgdomain_free (mdlp->item[j]);
  if (mdlp->item)
    free (mdlp->item);
  free (mdlp);
}
#endif


void
msgdomain_list_append (mdlp, mdp)
     msgdomain_list_ty *mdlp;
     msgdomain_ty *mdp;
{
  if (mdlp->nitems >= mdlp->nitems_max)
    {
      size_t nbytes;

      mdlp->nitems_max = mdlp->nitems_max * 2 + 4;
      nbytes = mdlp->nitems_max * sizeof (msgdomain_ty *);
      mdlp->item = xrealloc (mdlp->item, nbytes);
    }
  mdlp->item[mdlp->nitems++] = mdp;
}


#if 0 /* unused */
void
msgdomain_list_append_list (mdlp, mdlp2)
     msgdomain_list_ty *mdlp;
     msgdomain_list_ty *mdlp2;
{
  size_t j;

  for (j = 0; j < mdlp2->nitems; ++j)
    msgdomain_list_append (mdlp, mdlp2->item[j]);
}
#endif


message_list_ty *
msgdomain_list_sublist (mdlp, domain, create)
     msgdomain_list_ty *mdlp;
     const char *domain;
     int create;
{
  size_t j;

  for (j = 0; j < mdlp->nitems; j++)
    if (strcmp (mdlp->item[j]->domain, domain) == 0)
      return mdlp->item[j]->messages;

  if (create)
    {
      msgdomain_ty *mdp = msgdomain_alloc (domain);
      msgdomain_list_append (mdlp, mdp);
      return mdp->messages;
    }
  else
    return NULL;
}


#if 0 /* unused */
message_ty *
msgdomain_list_search (mdlp, msgid)
     msgdomain_list_ty *mdlp;
     const char *msgid;
{
  size_t j;

  for (j = 0; j < mdlp->nitems; ++j)
    {
      msgdomain_ty *mdp;
      message_ty *mp;

      mdp = mdlp->item[j];
      mp = message_list_search (mdp->messages, msgid);
      if (mp)
        return mp;
    }
  return NULL;
}
#endif


#if 0 /* unused */
message_ty *
msgdomain_list_search_fuzzy (mdlp, msgid)
     msgdomain_list_ty *mdlp;
     const char *msgid;
{
  size_t j;
  double best_weight;
  message_ty *best_mp;

  best_weight = 0.6;
  best_mp = NULL;
  for (j = 0; j < mdlp->nitems; ++j)
    {
      msgdomain_ty *mdp;
      message_ty *mp;

      mdp = mdlp->item[j];
      mp = message_list_search_fuzzy_inner (mdp->messages, msgid, &best_weight);
      if (mp)
	best_mp = mp;
    }
  return best_mp;
}
#endif
