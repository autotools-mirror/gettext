/* GNU gettext - internationalization aids
   Copyright (C) 1995-2026 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Bruno Haible.  */

#include <config.h>

/* Specification.  */
#include "string-desc-list.h"

#include <stdlib.h>

#include "xalloc.h"
#include "xstring-desc.h"


/* Initialize an empty list of strings.  */
void
string_desc_list_init (string_desc_list_ty *slp)
{
  slp->item = NULL;
  slp->nitems = 0;
  slp->nitems_max = 0;
}


/* Return a fresh, empty list of strings.  */
string_desc_list_ty *
string_desc_list_alloc ()
{
  string_desc_list_ty *slp = XMALLOC (string_desc_list_ty);
  slp->item = NULL;
  slp->nitems = 0;
  slp->nitems_max = 0;

  return slp;
}


/* Append a single string to the end of a list of strings.  */
void
string_desc_list_append (string_desc_list_ty *slp, string_desc_t s)
{
  /* Grow the list.  */
  if (slp->nitems >= slp->nitems_max)
    {
      slp->nitems_max = slp->nitems_max * 2 + 4;
      size_t nbytes = slp->nitems_max * sizeof (slp->item[0]);
      slp->item = (string_desc_t *) xrealloc (slp->item, nbytes);
    }

  /* Add a copy of the string to the end of the list.  */
  size_t item_index = slp->nitems++;
  slp->item[item_index] = sd_readonly (xsd_copy (s));
}


/* Append a freshly allocated single string to the end of a list of strings,
   transferring its ownership to SLP.  */
void
string_desc_list_append_move (string_desc_list_ty *slp, rw_string_desc_t s)
{
  /* Grow the list.  */
  if (slp->nitems >= slp->nitems_max)
    {
      slp->nitems_max = slp->nitems_max * 2 + 4;
      size_t nbytes = slp->nitems_max * sizeof (slp->item[0]);
      slp->item = (string_desc_t *) xrealloc (slp->item, nbytes);
    }

  /* Add the string itself to the end of the list.  */
  size_t item_index = slp->nitems++;
  slp->item[item_index] = sd_readonly (s);
}


/* Append a single string to the end of a list of strings, unless it is
   already contained in the list.  */
void
string_desc_list_append_unique (string_desc_list_ty *slp, string_desc_t s)
{
  /* Do nothing if the string is already in the list.  */
  for (size_t j = 0; j < slp->nitems; ++j)
    if (sd_equals (slp->item[j], s))
      return;

  /* Grow the list.  */
  if (slp->nitems >= slp->nitems_max)
    {
      slp->nitems_max = slp->nitems_max * 2 + 4;
      size_t nbytes = slp->nitems_max * sizeof (slp->item[0]);
      slp->item = (string_desc_t *) xrealloc (slp->item, nbytes);
    }

  /* Add a copy of the string to the end of the list.  */
  size_t item_index = slp->nitems++;
  slp->item[item_index] = sd_readonly (xsd_copy (s));
}


/* Destroy a list of strings.  */
void
string_desc_list_destroy (string_desc_list_ty *slp)
{
  for (size_t j = 0; j < slp->nitems; ++j)
    sd_free (sd_readwrite (slp->item[j]));
  if (slp->item != NULL)
    free (slp->item);
}


/* Free a list of strings.  */
void
string_desc_list_free (string_desc_list_ty *slp)
{
  for (size_t j = 0; j < slp->nitems; ++j)
    sd_free (sd_readwrite (slp->item[j]));
  if (slp->item != NULL)
    free (slp->item);
  free (slp);
}


/* Return a freshly allocated string obtained by concatenating all the
   strings in the list.  */
rw_string_desc_t
string_desc_list_concat (const string_desc_list_ty *slp)
{
  rw_string_desc_t result;
  {
    idx_t len = 0;
    for (size_t j = 0; j < slp->nitems; ++j)
      len += sd_length (slp->item[j]);
    result = xsd_new (len);
  }
  {
    idx_t pos = 0;
    for (size_t j = 0; j < slp->nitems; ++j)
      {
        idx_t len = sd_length (slp->item[j]);
        sd_overwrite (result, pos, slp->item[j]);
        pos += len;
      }
  }
  return result;
}


/* Return a freshly allocated string obtained by concatenating all the
   strings in the list, and destroy the list.  */
rw_string_desc_t
string_desc_list_concat_destroy (string_desc_list_ty *slp)
{
  rw_string_desc_t result;

  /* Optimize the most frequent case.  */
  if (slp->nitems == 1)
    {
      result = sd_readwrite (slp->item[0]);
      free (slp->item);
    }
  else
    {
      result = string_desc_list_concat (slp);
      string_desc_list_destroy (slp);
    }
  return result;
}


/* Return a freshly allocated string obtained by concatenating all the
   strings in the list, separated by the separator string, terminated
   by the terminator character.  The terminator character is not added if
   drop_redundant_terminator is true and the last string already ends with
   the terminator. */
rw_string_desc_t
string_desc_list_join (const string_desc_list_ty *slp, string_desc_t separator,
                       char terminator, bool drop_redundant_terminator)
{
  idx_t separator_len = sd_length (separator);

  rw_string_desc_t result;
  {
    idx_t len = 0;
    for (size_t j = 0; j < slp->nitems; ++j)
      {
        if (j > 0)
          len += separator_len;
        len += sd_length (slp->item[j]);
      }
    if (terminator)
      ++len;
    result = xsd_new (len);
  }
  {
    idx_t pos = 0;
    for (size_t j = 0; j < slp->nitems; ++j)
      {
        if (j > 0)
          {
            sd_overwrite (result, pos, separator);
            pos += separator_len;
          }
        idx_t len = sd_length (slp->item[j]);
        sd_overwrite (result, pos, slp->item[j]);
        pos += len;
      }
    {
      idx_t len;
      if (terminator
          && !(drop_redundant_terminator
               && slp->nitems > 0
               && (len = sd_length (slp->item[slp->nitems - 1])) > 0
               && sd_char_at (slp->item[slp->nitems - 1], len - 1) == terminator))
        sd_set_char_at (result, pos, terminator);
    }
  }
  return result;
}


/* Return true if s is contained in the list of strings, false otherwise.  */
bool
string_desc_list_member (const string_desc_list_ty *slp, string_desc_t s)
{
  for (size_t j = 0; j < slp->nitems; ++j)
    if (sd_equals (slp->item[j], s))
      return true;
  return false;
}


/* Remove s from the list of strings.  Return the removed string, now owned
   by the caller, or (0, NULL).  */
rw_string_desc_t
string_desc_list_remove (string_desc_list_ty *slp, string_desc_t s)
{
  for (size_t j = 0; j < slp->nitems; ++j)
    if (sd_equals (slp->item[j], s))
      {
        string_desc_t found = slp->item[j];
        if (slp->nitems > j + 1)
          memmove (&slp->item[j + 1], &slp->item[j],
                   (slp->nitems - j - 1) * sizeof (string_desc_t));
        slp->nitems--;
        return sd_readwrite (found);
      }
  return sd_readwrite (sd_new_empty ());
}
