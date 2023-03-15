/* GNU gettext - internationalization aids
   Copyright (C) 2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2023.  */

#ifndef _STR_DESC_H
#define _STR_DESC_H 1

/* Get size_t, ptrdiff_t.  */
#include <stddef.h>

/* Get bool.  */
#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif


/* Type describing a string that may contain NUL bytes.
   It's merely a descriptor of an array of bytes.  */
typedef struct string_desc_ty string_desc_ty;
struct string_desc_ty
{
  size_t nbytes;
  char *data;
};

/* String descriptors can be passed and returned by value.  */


/* ==== Side-effect-free operations on string descriptors ==== */

/* Return the length of the string S.  */
extern size_t string_desc_length (string_desc_ty s);

/* Return the byte at index I of string S.
   I must be < length(S).  */
extern char string_desc_char_at (string_desc_ty s, size_t i);

/* Return a read-only view of the bytes of S.  */
extern const char * string_desc_data (string_desc_ty s);

/* Return true if S is the empty string.  */
extern bool string_desc_is_empty (string_desc_ty s);

/* Return true if S starts with PREFIX.  */
extern bool string_desc_startswith (string_desc_ty s, string_desc_ty prefix);

/* Return true if S ends with SUFFIX.  */
extern bool string_desc_endswith (string_desc_ty s, string_desc_ty suffix);

/* Return > 0, == 0, or < 0 if A > B, A == B, A < B.
   This uses a lexicographic ordering, where the bytes are compared as
   'unsigned char'.  */
extern int string_desc_cmp (string_desc_ty a, string_desc_ty b);

/* Return the index of the first occurrence of C in S,
   or -1 if there is none.  */
extern ptrdiff_t string_desc_index (string_desc_ty s, char c);

/* Return the index of the last occurrence of C in S,
   or -1 if there is none.  */
extern ptrdiff_t string_desc_last_index (string_desc_ty s, char c);

/* Return the index of the first occurrence of NEEDLE in HAYSTACK,
   or -1 if there is none.  */
extern ptrdiff_t string_desc_contains (string_desc_ty haystack, string_desc_ty needle);

/* Return a string that represents the C string S, of length strlen (S).  */
extern string_desc_ty string_desc_from_c (const char *s);

/* Return the substring of S, starting at offset START and ending at offset END.
   START must be <= END.
   The result is of length END - START.
   The result must not be freed (since its storage is part of the storage
   of S).  */
extern string_desc_ty string_desc_substring (string_desc_ty s, size_t start, size_t end);


/* ==== Memory-allocating operations on string descriptors ==== */

/* Return a string of length N, with uninitialized contents.  */
extern string_desc_ty string_desc_new (size_t n);

/* Return a string of length N, at the given memory address.  */
extern string_desc_ty string_desc_new_addr (size_t n, char *addr);

/* Return a string of length N, filled with C.  */
extern string_desc_ty string_desc_new_filled (size_t n, char c);

/* Return a copy of string S.  */
extern string_desc_ty string_desc_copy (string_desc_ty s);

/* Return the concatenation of N strings.  N must be > 0.  */
extern string_desc_ty string_desc_concat (size_t n, string_desc_ty string1, ...);

/* Return a copy of string S, as a NUL-terminated C string.  */
extern char * string_desc_c (string_desc_ty s);


/* ==== Operations with side effects on string descriptors ==== */

/* Overwrite the byte at index I of string S with C.
   I must be < length(S).  */
extern void string_desc_set_char_at (string_desc_ty s, size_t i, char c);

/* Fill part of S, starting at offset START and ending at offset END,
   with copies of C.
   START must be <= END.  */
extern void string_desc_fill (string_desc_ty s, size_t start, size_t end, char c);

/* Overwrite part of S with T, starting at offset START.
   START + length(T) must be <= length (S).  */
extern void string_desc_overwrite (string_desc_ty s, size_t start, string_desc_ty t);

/* Free S.  */
extern void string_desc_free (string_desc_ty s);


#ifdef __cplusplus
}
#endif


#endif /* _STR_DESC_H */
