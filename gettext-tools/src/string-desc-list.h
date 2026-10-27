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

#ifndef _STRING_DESC_LIST_H
#define _STRING_DESC_LIST_H 1

/* Get size_t and NULL.  */
#include <stddef.h>

/* Get bool.  */
#include <stdbool.h>

#include "attribute.h"
#include "string-desc.h"


#ifdef __cplusplus
extern "C" {
#endif


/* Type describing list of immutable strings that may contain NUL bytes,
   implemented using a dynamic array.  */
typedef struct string_desc_list_ty string_desc_list_ty;
struct string_desc_list_ty
{
  size_t nitems;
  string_desc_t *item
    COUNTED_BY (nitems);
  size_t nitems_max;
};

/* Initialize an empty list of strings.  */
extern void
       string_desc_list_init (string_desc_list_ty *slp);

/* Return a fresh, empty list of strings.  */
extern string_desc_list_ty *
       string_desc_list_alloc (void);

/* Append a single string to the end of a list of strings.  */
extern void
       string_desc_list_append (string_desc_list_ty *slp, string_desc_t s);

/* Append a freshly allocated single string to the end of a list of strings,
   transferring its ownership to SLP.  */
extern void
       string_desc_list_append_move (string_desc_list_ty *slp,
                                     rw_string_desc_t s);

/* Append a single string to the end of a list of strings, unless it is
   already contained in the list.  */
extern void
       string_desc_list_append_unique (string_desc_list_ty *slp,
                                       string_desc_t s);

/* Destroy a list of strings.  */
extern void
       string_desc_list_destroy (string_desc_list_ty *slp);

/* Free a list of strings.  */
extern void
       string_desc_list_free (string_desc_list_ty *slp);

/* Return a freshly allocated string obtained by concatenating all the
   strings in the list.  */
extern rw_string_desc_t
       string_desc_list_concat (const string_desc_list_ty *slp);

/* Return a freshly allocated string obtained by concatenating all the
   strings in the list, and destroy the list.  */
extern rw_string_desc_t
       string_desc_list_concat_destroy (string_desc_list_ty *slp);

/* Return a freshly allocated string obtained by concatenating all the
   strings in the list, separated by the separator string, terminated
   by the terminator character.  The terminator character is not added if
   drop_redundant_terminator is true and the last string already ends with
   the terminator.  */
extern rw_string_desc_t
       string_desc_list_join (const string_desc_list_ty *slp,
                              string_desc_t separator, char terminator,
                              bool drop_redundant_terminator);

/* Return true if s is contained in the list of strings, false otherwise.  */
extern bool
       string_desc_list_member (const string_desc_list_ty *slp,
                                string_desc_t s);

/* Remove s from the list of strings.  Return the removed string, now owned
   by the caller, or (0, NULL).  */
extern rw_string_desc_t
       string_desc_list_remove (string_desc_list_ty *slp, string_desc_t s);


#ifdef __cplusplus
}
#endif


#endif /* _STRING_DESC_LIST_H */
