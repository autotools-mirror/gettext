/* Message list test for equality.
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

/* Specification.  */
#include "msgl-equal.h"

#include <string.h>


/* Prototypes for local functions.  Needed to ensure compiler checking of
   function argument counts despite of K&R C function definition syntax.  */
static inline bool pos_equal PARAMS ((const lex_pos_ty *pos1,
				      const lex_pos_ty *pos2));
static inline bool msgdomain_equal PARAMS ((const msgdomain_ty *mdp1,
					    const msgdomain_ty *mdp2));


static inline bool
pos_equal (pos1, pos2)
     const lex_pos_ty *pos1;
     const lex_pos_ty *pos2;
{
  return ((pos1->file_name == pos2->file_name
	   || strcmp (pos1->file_name, pos2->file_name) == 0)
	  && pos1->line_number == pos2->line_number);
}

bool
string_list_equal (slp1, slp2)
     const string_list_ty *slp1;
     const string_list_ty *slp2;
{
  size_t i, i1, i2;

  i1 = (slp1 != NULL ? slp1->nitems : 0);
  i2 = (slp2 != NULL ? slp2->nitems : 0);
  if (i1 != i2)
    return false;
  for (i = 0; i < i1; i++)
    if (strcmp (slp1->item[i], slp2->item[i]) != 0)
      return false;
  return true;
}

bool
message_equal (mp1, mp2)
     const message_ty *mp1;
     const message_ty *mp2;
{
  size_t i, i1, i2;

  if (strcmp (mp1->msgid, mp2->msgid) != 0)
    return false;

  if (!(mp1->msgid_plural != NULL
	? mp2->msgid_plural != NULL
	  && strcmp (mp1->msgid_plural, mp2->msgid_plural) == 0
	: mp2->msgid_plural == NULL))
    return false;

  if (mp1->msgstr_len != mp2->msgstr_len)
    return false;
  if (memcmp (mp1->msgstr, mp2->msgstr, mp1->msgstr_len) != 0)
    return false;

  if (!pos_equal (&mp1->pos, &mp2->pos))
    return false;

  if (!string_list_equal (mp1->comment, mp2->comment))
    return false;

  if (!string_list_equal (mp1->comment_dot, mp2->comment_dot))
    return false;

  i1 = mp1->filepos_count;
  i2 = mp2->filepos_count;
  if (i1 != i2)
    return false;
  for (i = 0; i < i1; i++)
    if (!pos_equal (&mp1->filepos[i], &mp2->filepos[i]))
      return false;

  if (mp1->is_fuzzy != mp2->is_fuzzy)
    return false;

  for (i = 0; i < NFORMATS; i++)
    if (mp1->is_format[i] != mp2->is_format[i])
      return false;

  if (mp1->obsolete != mp2->obsolete)
    return false;

  return true;
}

bool
message_list_equal (mlp1, mlp2)
     const message_list_ty *mlp1;
     const message_list_ty *mlp2;
{
  size_t i, i1, i2;

  i1 = mlp1->nitems;
  i2 = mlp2->nitems;
  if (i1 != i2)
    return false;
  for (i = 0; i < i1; i++)
    if (!message_equal (mlp1->item[i], mlp2->item[i]))
      return false;
  return true;
}

static inline bool
msgdomain_equal (mdp1, mdp2)
     const msgdomain_ty *mdp1;
     const msgdomain_ty *mdp2;
{
  return (strcmp (mdp1->domain, mdp2->domain) == 0
	  && message_list_equal (mdp1->messages, mdp2->messages));
}

bool
msgdomain_list_equal (mdlp1, mdlp2)
     const msgdomain_list_ty *mdlp1;
     const msgdomain_list_ty *mdlp2;
{
  size_t i, i1, i2;

  i1 = mdlp1->nitems;
  i2 = mdlp2->nitems;
  if (i1 != i2)
    return false;
  for (i = 0; i < i1; i++)
    if (!msgdomain_equal (mdlp1->item[i], mdlp2->item[i]))
      return false;
  return true;
}
