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


/* Prototypes for local functions.  */
static message_ty *message_list_search_fuzzy_inner PARAMS ((
       message_list_ty *__mlp, const char *__msgid, double *__best_weight_p));



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
message_alloc (msgid, msgid_plural)
     char *msgid;
     const char *msgid_plural;
{
  message_ty *mp;

  mp = xmalloc (sizeof (message_ty));
  mp->msgid = msgid;
  mp->msgid_plural = (msgid_plural != NULL ? xstrdup (msgid_plural) : NULL);
  mp->comment = NULL;
  mp->comment_dot = NULL;
  mp->filepos_count = 0;
  mp->filepos = NULL;
  mp->variant_count = 0;
  mp->variant = NULL;
  mp->used = 0;
  mp->obsolete = 0;
  mp->is_fuzzy = 0;
  mp->is_c_format = undecided;
  mp->do_wrap = undecided;
  return mp;
}


void
message_free (mp)
     message_ty *mp;
{
  size_t j;

  if (mp->comment != NULL)
    string_list_free (mp->comment);
  if (mp->comment_dot != NULL)
    string_list_free (mp->comment_dot);
  free ((char *) mp->msgid);
  if (mp->msgid_plural != NULL)
    free ((char *) mp->msgid_plural);
  for (j = 0; j < mp->variant_count; ++j)
    free ((char *) mp->variant[j].msgstr);
  if (mp->variant != NULL)
    free (mp->variant);
  for (j = 0; j < mp->filepos_count; ++j)
    free ((char *) mp->filepos[j].file_name);
  if (mp->filepos != NULL)
    free (mp->filepos);
  free (mp);
}


message_variant_ty *
message_variant_search (mp, domain)
     message_ty *mp;
     const char *domain;
{
  size_t j;
  message_variant_ty *mvp;

  for (j = 0; j < mp->variant_count; ++j)
    {
      mvp = &mp->variant[j];
      if (0 == strcmp (domain, mvp->domain))
	return mvp;
    }
  return 0;
}


void
message_variant_append (mp, domain, msgstr, msgstr_len, pp)
     message_ty *mp;
     const char *domain;
     const char *msgstr;
     size_t msgstr_len;
     const lex_pos_ty *pp;
{
  size_t nbytes;
  message_variant_ty *mvp;

  nbytes = (mp->variant_count + 1) * sizeof (mp->variant[0]);
  mp->variant = xrealloc (mp->variant, nbytes);
  mvp = &mp->variant[mp->variant_count++];
  mvp->domain = domain;
  mvp->msgstr = msgstr;
  mvp->msgstr_len = msgstr_len;
  mvp->pos = *pp;
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


message_ty *
message_copy (mp)
     message_ty *mp;
{
  message_ty *result;
  size_t j;

  result = message_alloc (xstrdup (mp->msgid), mp->msgid_plural);

  for (j = 0; j < mp->variant_count; ++j)
    {
      message_variant_ty *mvp = &mp->variant[j];
      message_variant_append (result, mvp->domain, mvp->msgstr, mvp->msgstr_len,
			      &mvp->pos);
    }
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
  message_ty *result;
  const char *pot_date_ptr = NULL;
  size_t pot_date_len = 0;
  size_t j;

  /* Take the msgid from the reference.  When fuzzy matches are made,
     the definition will not be unique, but the reference will be -
     usually because it has a typo.  */
  result = message_alloc (xstrdup (ref->msgid), ref->msgid_plural);

  /* If msgid is the header entry (i.e., "") we find the
     POT-Creation-Date line in the reference.  */
  if (ref->msgid[0] == '\0')
    {
      pot_date_ptr = strstr (ref->variant[0].msgstr, "POT-Creation-Date:");
      if (pot_date_ptr != NULL)
	{
	  const char *endp;

	  pot_date_ptr += sizeof ("POT-Creation-Date:") - 1;

	  endp = strchr (pot_date_ptr, '\n');
	  if (endp == NULL)
	    {
	      char *extended;
	      endp = strchr (pot_date_ptr, '\0');
	      pot_date_len = (endp - pot_date_ptr) + 1;
	      extended = (char *) alloca (pot_date_len + 1);
	      stpcpy (stpcpy (extended, pot_date_ptr), "\n");
	      pot_date_ptr = extended;
	    }
	  else
	    pot_date_len = (endp - pot_date_ptr) + 1;

	  if (pot_date_len == 0)
	    pot_date_ptr = NULL;
	}
    }

  /* Take the variant list from the definition.  The msgstr of the
     refences will be empty, as they were generated by xgettext.  If
     we currently process the header entry we have to merge the msgstr
     by using the POT-Creation-Date field from the .pot file.  */
  for (j = 0; j < def->variant_count; ++j)
    {
      message_variant_ty *mvp = &def->variant[j];

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

	  cp = mvp->msgstr;
	  while (*cp != '\0')
	    {
	      const char *endp = strchr (cp, '\n');
	      int terminated = endp != NULL;

	      if (!terminated)
		{
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
		if (strncasecmp (cp, known_fields[cnt].name,
				 known_fields[cnt].len) == 0)
		  break;

	      if (cnt < sizeof (known_fields) / sizeof (known_fields[0]))
		{
		  header_fields[cnt].string = &cp[known_fields[cnt].len];
		  header_fields[cnt].len = len - known_fields[cnt].len;
		}
	      else
		{
		  /* It's an unknown field.  Append content to what is
		     already known.  */
		  char *extended = (char *) alloca (header_fields[UNKNOWN].len
						    + len + 1);
		  memcpy (extended, header_fields[UNKNOWN].string,
			  header_fields[UNKNOWN].len);
		  memcpy (&extended[header_fields[UNKNOWN].len], cp, len);
		  extended[header_fields[UNKNOWN].len + len] = '\0';
		  header_fields[UNKNOWN].string = extended;
		  header_fields[UNKNOWN].len += len;
		}

	      cp = endp;
	    }

	  if (pot_date_ptr != NULL)
	    {
	      header_fields[POT_CREATION].string = pot_date_ptr;
	      header_fields[POT_CREATION].len = pot_date_len;
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
	  if (header_fields[idx].string)				      \
	    newp = stpncpy (stpcpy (newp, known_fields[idx].name),	      \
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

	  message_variant_append (result, mvp->domain, cp, strlen (cp) + 1,
				  &mvp->pos);
	}
      else
	message_variant_append (result, mvp->domain, mvp->msgstr,
				mvp->msgstr_len, &mvp->pos);
    }

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


void
message_comment_filepos (mp, name, line)
     message_ty *mp;
     const char *name;
     size_t line;
{
  size_t nbytes;
  lex_pos_ty *pp;
  int min, max;
  int j;

  /* See if we have this position already.  They are kept in sorted
     order, so use a binary chop.  */
  /* FIXME: use bsearch */
  min = 0;
  max = (int) mp->filepos_count - 1;
  while (min <= max)
    {
      int mid;
      int cmp;

      mid = (min + max) / 2;
      pp = &mp->filepos[mid];
      cmp = strcmp (pp->file_name, name);
      if (cmp == 0)
	cmp = (int) pp->line_number - line;
      if (cmp == 0)
	return;
      if (cmp < 0)
	min = mid + 1;
      else
	max = mid - 1;
    }

  /* Extend the list so that we can add an position to it.  */
  nbytes = (mp->filepos_count + 1) * sizeof (mp->filepos[0]);
  mp->filepos = xrealloc (mp->filepos, nbytes);

  /* Shuffle the rest of the list up one, so that we can insert the
     position at ``min''.  */
  /* FIXME: use memmove */
  for (j = mp->filepos_count; j > min; --j)
    mp->filepos[j] = mp->filepos[j - 1];
  mp->filepos_count++;

  /* Insert the postion into the empty slot.  */
  pp = &mp->filepos[min];
  pp->file_name = xstrdup (name);
  pp->line_number = line;
}


message_list_ty *
message_list_alloc ()
{
  message_list_ty *mlp;

  mlp = xmalloc (sizeof (message_list_ty));
  mlp->nitems = 0;
  mlp->nitems_max = 0;
  mlp->item = 0;
  return mlp;
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
      if (0 == strcmp (msgid, mp->msgid))
	return mp;
    }
  return 0;
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
      size_t k;
      double weight;
      message_ty *mp;

      mp = mlp->item[j];

      for (k = 0; k < mp->variant_count; ++k)
	if (mp->variant[k].msgstr != NULL && mp->variant[k].msgstr[0] != '\0')
	  break;
      if (k >= mp->variant_count)
	continue;

      weight = fstrcmp (msgid, mp->msgid);
      if (weight > *best_weight_p)
	{
	  *best_weight_p = weight;
	  best_mp = mp;
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


message_list_list_ty *
message_list_list_alloc ()
{
  message_list_list_ty *mllp;

  mllp = xmalloc (sizeof (message_list_list_ty));
  mllp->nitems = 0;
  mllp->nitems_max = 0;
  mllp->item = 0;
  return mllp;
}


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
  return 0;
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
