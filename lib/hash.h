/* Copyright (C) 1995, 2000 Free Software Foundation, Inc.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, write to the Free Software Foundation, Inc., 59 Temple Place
   - Suite 330, Boston, MA 02111-1307, USA.  */

#ifndef _HASH_H
# define _HASH_H

# include <obstack.h>

typedef struct hash_table
{
  unsigned long int size;
  unsigned long int filled;
  void *first;
  void *table;
  struct obstack mem_pool;
}
hash_table;

# ifndef PARAMS
#  if defined (__GNUC__) || __STDC__
#   define PARAMS(Args) Args
#  else
#   define PARAMS(Args) ()
#  endif
# endif

extern int init_hash PARAMS ((hash_table *htab, unsigned long int init_size));
extern int delete_hash PARAMS ((hash_table *htab));
extern int insert_entry PARAMS ((hash_table *htab,
				 const void *key, size_t keylen,
				 void *data));
extern int find_entry PARAMS ((hash_table *htab,
			       const void *key, size_t keylen,
			       void **result));

extern int iterate_table PARAMS ((hash_table *htab, void **ptr,
				  const void **key, size_t *keylen,
				  void **data));

extern unsigned long int next_prime PARAMS ((unsigned long int seed));

#endif /* not _HASH_H */
