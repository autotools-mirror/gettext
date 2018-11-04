/* Handling strings that are given partially in the source encoding and
   partially in Unicode.
   Copyright (C) 2001-2018 Free Software Foundation, Inc.

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

#ifndef _XGETTEXT_MIXED_STRING_H
#define _XGETTEXT_MIXED_STRING_H

#include <stdbool.h>
#include <stddef.h>

#include "xg-encoding.h"

#ifdef __cplusplus
extern "C" {
#endif


/* A string buffer type that allows appending bytes (in the
   xgettext_current_source_encoding) or Unicode characters.
   Returns the entire string in UTF-8 encoding.  */

struct mixed_string_buffer
{
  /* The part of the string that has already been converted to UTF-8.  */
  char *utf8_buffer;
  size_t utf8_buflen;
  size_t utf8_allocated;
  /* The first half of an UTF-16 surrogate character.  */
  unsigned short utf16_surr;
  /* The part of the string that is still in the source encoding.  */
  char *curr_buffer;
  size_t curr_buflen;
  size_t curr_allocated;
  /* The lexical context.  Used only for error message purposes.  */
  lexical_context_ty lcontext;
  const char *logical_file_name;
  int line_number;
};

/* Initializes a mixed_string_buffer.  */
extern void
       mixed_string_buffer_init (struct mixed_string_buffer *bp,
                                 lexical_context_ty lcontext,
                                 const char *logical_file_name,
                                 int line_number);

/* Creates a fresh mixed_string_buffer.  */
extern struct mixed_string_buffer *
       mixed_string_buffer_alloc (lexical_context_ty lcontext,
                                  const char *logical_file_name,
                                  int line_number);

/* Determines whether a mixed_string_buffer is still empty.  */
extern bool mixed_string_buffer_is_empty (const struct mixed_string_buffer *bp);

/* Appends a character to a mixed_string_buffer.  */
extern void mixed_string_buffer_append_char (struct mixed_string_buffer *bp,
                                             int c);

/* Appends a Unicode character to a mixed_string_buffer.  */
extern void mixed_string_buffer_append_unicode (struct mixed_string_buffer *bp,
                                                int c);

/* Frees the memory pointed to by a 'struct mixed_string_buffer'.  */
extern void mixed_string_buffer_destroy (struct mixed_string_buffer *bp);

/* Frees the memory pointed to by a 'struct mixed_string_buffer'
   and returns the accumulated string in UTF-8.  */
extern char * mixed_string_buffer_result (struct mixed_string_buffer *bp);

/* Frees mixed_string_buffer and returns the accumulated string in UTF-8.  */
extern char * mixed_string_buffer_done (struct mixed_string_buffer *bp);


#ifdef __cplusplus
}
#endif


#endif /* _XGETTEXT_MIXED_STRING_H */
