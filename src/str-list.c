/* GNU gettext - internationalization aids
   Copyright (C) 1995, 1998, 2000, 2001 Free Software Foundation, Inc.

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
#include "str-list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xmalloc.h"


/* Initialize an empty list of strings.  */
void
string_list_init (slp)
     string_list_ty *slp;
{
  slp->item = NULL;
  slp->nitems = 0;
  slp->nitems_max = 0;
}


/* Return a fresh, empty list of strings.  */
string_list_ty *
string_list_alloc ()
{
  string_list_ty *slp;

  slp = (string_list_ty *) xmalloc (sizeof (*slp));
  slp->item = NULL;
  slp->nitems = 0;
  slp->nitems_max = 0;

  return slp;
}


/* Append a single string to the end of a list of strings.  */
void
string_list_append (slp, s)
     string_list_ty *slp;
     const char *s;
{
  /* Grow the list.  */
  if (slp->nitems >= slp->nitems_max)
    {
      size_t nbytes;

      slp->nitems_max = slp->nitems_max * 2 + 4;
      nbytes = slp->nitems_max * sizeof (slp->item[0]);
      slp->item = (const char **) xrealloc (slp->item, nbytes);
    }

  /* Add a copy of the string to the end of the list.  */
  slp->item[slp->nitems++] = xstrdup (s);
}


/* Append a single string to the end of a list of strings, unless it is
   already contained in the list.  */
void
string_list_append_unique (slp, s)
     string_list_ty *slp;
     const char *s;
{
  size_t j;

  /* Do not if the string is already in the list.  */
  for (j = 0; j < slp->nitems; ++j)
    if (strcmp (slp->item[j], s) == 0)
      return;

  /* Grow the list.  */
  if (slp->nitems >= slp->nitems_max)
    {
      slp->nitems_max = slp->nitems_max * 2 + 4;
      slp->item = (const char **) xrealloc (slp->item,
					    slp->nitems_max
					    * sizeof (slp->item[0]));
    }

  /* Add a copy of the string to the end of the list.  */
  slp->item[slp->nitems++] = xstrdup (s);
}


/* Destroy a list of strings.  */
void
string_list_destroy (slp)
     string_list_ty *slp;
{
  size_t j;

  for (j = 0; j < slp->nitems; ++j)
    free ((char *) slp->item[j]);
  if (slp->item != NULL)
    free (slp->item);
}


/* Free a list of strings.  */
void
string_list_free (slp)
     string_list_ty *slp;
{
  size_t j;

  for (j = 0; j < slp->nitems; ++j)
    free ((char *) slp->item[j]);
  if (slp->item != NULL)
    free (slp->item);
  free (slp);
}


/* Return a freshly allocated string obtained by concatenating all the
   strings in the list.  */
char *
string_list_concat (slp)
     const string_list_ty *slp;
{
  size_t len;
  size_t j;
  char *result;
  size_t pos;

  len = 1;
  for (j = 0; j < slp->nitems; ++j)
    len += strlen (slp->item[j]);
  result = (char *) xmalloc (len);
  pos = 0;
  for (j = 0; j < slp->nitems; ++j)
    {
      len = strlen (slp->item[j]);
      memcpy (result + pos, slp->item[j], len);
      pos += len;
    }
  result[pos] = '\0';
  return result;
}


/* Return a freshly allocated string obtained by concatenating all the
   strings in the list, and destroy the list.  */
char *
string_list_concat_destroy (slp)
     string_list_ty *slp;
{
  char *result;

  /* Optimize the most frequent case.  */
  if (slp->nitems == 1)
    {
      result = (char *) slp->item[0];
      free (slp->item);
    }
  else
    {
      result = string_list_concat (slp);
      string_list_destroy (slp);
    }
  return result;
}


/* Return a freshly allocated string obtained by concatenating all the
   strings in the list, separated by spaces.  */
char *
string_list_join (slp)
     const string_list_ty *slp;
{
  size_t len;
  size_t j;
  char *result;
  size_t pos;

  len = 1;
  for (j = 0; j < slp->nitems; ++j)
    {
      if (j)
	++len;
      len += strlen (slp->item[j]);
    }
  result = (char *) xmalloc (len);
  pos = 0;
  for (j = 0; j < slp->nitems; ++j)
    {
      if (j)
	result[pos++] = ' ';
      len = strlen (slp->item[j]);
      memcpy (result + pos, slp->item[j], len);
      pos += len;
    }
  result[pos] = '\0';
  return result;
}


/* Return 1 if s is contained in the list of strings, 0 otherwise.  */
bool
string_list_member (slp, s)
     const string_list_ty *slp;
     const char *s;
{
  size_t j;

  for (j = 0; j < slp->nitems; ++j)
    if (strcmp (slp->item[j], s) == 0)
      return true;
  return false;
}
