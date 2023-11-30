/* Keeping track of the flags that apply to a string extracted
   in a certain context.
   Copyright (C) 2001-2018, 2020, 2023 Free Software Foundation, Inc.

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

#ifndef _XGETTEXT_ARGLIST_CONTEXT_H
#define _XGETTEXT_ARGLIST_CONTEXT_H

#include <stdbool.h>

#include "mem-hash-map.h"
#include "message.h"
#include "xg-formatstring.h"

#ifdef __cplusplus
extern "C" {
#endif


/* Context representing some flags w.r.t. a specific format string type.  */
struct formatstring_context_ty
{
  /*enum is_format*/ unsigned int is_format    : 3;
  /*bool*/           unsigned int pass_format  : 1;
};

/* Context representing some flags.  */
typedef struct flag_context_ty flag_context_ty;
struct flag_context_ty
{
  struct formatstring_context_ty for_formatstring[NXFORMATS];
};

/* Context representing some flags, for each possible argument number.
   This is a linked list, sorted according to the argument number.  */
typedef struct flag_context_list_ty flag_context_list_ty;
struct flag_context_list_ty
{
  int argnum;                   /* current argument number, > 0 */
  flag_context_ty flags;        /* flags for current argument */
  flag_context_list_ty *next;
};

/* Iterator through a flag_context_list_ty.  */
typedef struct flag_context_list_iterator_ty flag_context_list_iterator_ty;
struct flag_context_list_iterator_ty
{
  int argnum;                           /* current argument number, > 0 */
  const flag_context_list_ty* head;     /* tail of list */
};
extern flag_context_list_iterator_ty null_context_list_iterator;
extern flag_context_list_iterator_ty passthrough_context_list_iterator;
extern flag_context_list_iterator_ty
       flag_context_list_iterator (flag_context_list_ty *list);
extern flag_context_ty
       flag_context_list_iterator_advance (flag_context_list_iterator_ty *iter);


/* For nearly each backend, we have a separate table mapping a keyword to
   a flag_context_list_ty *.  */
typedef hash_table /* char[] -> flag_context_list_ty * */
        flag_context_list_table_ty;
extern flag_context_list_ty *
       flag_context_list_table_lookup (flag_context_list_table_ty *flag_table,
                                       const void *key, size_t keylen);
/* Insert the pair (VALUE, PASS) as (is_format, pass_format) for the format
   string type FI in the flags of the element numbered ARGNUM of the list
   corresponding to NAME in the TABLE.  */
extern void
       flag_context_list_table_add (flag_context_list_table_ty *table,
                                    size_t fi,
                                    const char *name_start, const char *name_end,
                                    int argnum, enum is_format value, bool pass);


/* A set of arguments to pass to set_format_flag_from_context.  */
struct remembered_message_ty
{
  message_ty *mp;
  bool plural;
  lex_pos_ty pos;
};

/* A list of 'struct remembered_message_ty'.  */
struct remembered_message_list_ty
{
  unsigned int refcount;
  struct remembered_message_ty *item;
  size_t nitems;
  size_t nitems_max;
};

/* Adds an element to a list of 'struct remembered_message_ty'.  */
extern void
       remembered_message_list_append (struct remembered_message_list_ty *list,
                                       struct remembered_message_ty element);

/* Context representing some flags w.r.t. a specific format string type,
   as effective in a region of the input file.  */
struct formatstring_region_ty
{
  enum is_format is_format;
  /* Messages that were remembered in this context.
     This messages list is shared with sub-regions when pass_format was true
     in inheriting_region.  */
  struct remembered_message_list_ty *remembered;
};

/* A region of the input file, in which a given context is in effect, together
   with the messages that were remembered while processing this region.  */
typedef struct flag_region_ty flag_region_ty;
struct flag_region_ty
{
  unsigned int refcount;
  struct formatstring_region_ty for_formatstring[NXFORMATS];
};

/* Creates a region in which the null context is in effect.  */
extern flag_region_ty *
       null_context_region ();

/* Creates a sub-region that inherits from an outer region.  */
extern flag_region_ty *
       inheriting_region (flag_region_ty *outer_region,
                          flag_context_ty modifier_context);

/* Adds a reference to a region.  Returns the region.  */
extern flag_region_ty *
       ref_region (flag_region_ty *region);

/* Drops a reference to a region.
   When the last reference is dropped, the region is freed.  */
extern void
       unref_region (flag_region_ty *region);

/* Assigns the value of B to the variable A.
   Both are of type 'flag_region_ty *'.  B is *not* freshly created.  */
#define assign_region(a, b) \
  do {                             \
    flag_region_ty *_prev_a = (a); \
    (a) = (b);                     \
    ref_region (a);                \
    unref_region (_prev_a);        \
  } while (0)

/* Assigns the value of B to the variable A.
   Both are of type 'flag_region_ty *'.  B is freshly created.  */
#define assign_new_region(a, b) \
  do {                             \
    flag_region_ty *_prev_a = (a); \
    (a) = (b);                     \
    unref_region (_prev_a);        \
  } while (0)


#ifdef __cplusplus
}
#endif


#endif /* _XGETTEXT_ARGLIST_CONTEXT_H */
