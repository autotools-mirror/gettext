/* Writing binary .mo files.
   Copyright (C) 1995-1998, 2000, 2001 Free Software Foundation, Inc.
   Written by Ulrich Drepper <drepper@gnu.ai.mit.edu>, April 1995.

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

/* AIX 3 forces us to put this declaration at the beginning of the file.  */
#if defined _AIX && !defined __GNUC__
 #pragma alloca
#endif

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* Specification.  */
#include "write-mo.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/param.h>
#include <stdlib.h>
#include <string.h>

#include "msgfmt.h"

/* These two include files describe the binary .mo format.  */
#include "gmo.h"
#include "hash-string.h"

#include "error.h"
#include "hash.h"
#include "message.h"
#include "system.h"
#include "gettext.h"

#define _(str) gettext (str)

/* Usually defined in <sys/param.h>.  */
#ifndef roundup
# if defined __GNUC__ && __GNUC__ >= 2
#  define roundup(x, y) ({typeof(x) _x = (x); typeof(y) _y = (y); \
			  ((_x + _y - 1) / _y) * _y; })
# else
#  define roundup(x, y) ((((x)+((y)-1))/(y))*(y))
# endif	/* GNU CC2  */
#endif /* roundup  */


/* Alignment of strings in resulting .mo file.  */
size_t alignment;

/* True if no hash table in .mo is wanted.  */
bool no_hash_table;


/* Prototypes for local functions.  Needed to ensure compiler checking of
   function argument counts despite of K&R C function definition syntax.  */
static int compare_id PARAMS ((const void *pval1, const void *pval2));
static void write_table PARAMS ((FILE *output_file, message_list_ty *mlp));


/* Define the data structure which we need to represent the data to
   be written out.  */
struct id_str_pair
{
  const char *id;
  size_t id_len;
  const char *id_plural;
  size_t id_plural_len;
  const char *str;
  size_t str_len;
};


static int
compare_id (pval1, pval2)
     const void *pval1;
     const void *pval2;
{
  return strcmp (((struct id_str_pair *) pval1)->id,
		 ((struct id_str_pair *) pval2)->id);
}


static void
write_table (output_file, mlp)
     FILE *output_file;
     message_list_ty *mlp;
{
  static char null = '\0';
  /* This should be explained:
     Each string has an associate hashing value V, computed by a fixed
     function.  To locate the string we use open addressing with double
     hashing.  The first index will be V % M, where M is the size of the
     hashing table.  If no entry is found, iterating with a second,
     independent hashing function takes place.  This second value will
     be 1 + V % (M - 2).
     The approximate number of probes will be

       for unsuccessful search:  (1 - N / M) ^ -1
       for successful search:    - (N / M) ^ -1 * ln (1 - N / M)

     where N is the number of keys.

     If we now choose M to be the next prime bigger than 4 / 3 * N,
     we get the values
			 4   and   1.85  resp.
     Because unsuccesful searches are unlikely this is a good value.
     Formulas: [Knuth, The Art of Computer Programming, Volume 3,
		Sorting and Searching, 1973, Addison Wesley]  */
  nls_uint32 hash_tab_size =
    (no_hash_table ? 0 : next_prime ((mlp->nitems * 4) / 3));
  nls_uint32 *hash_tab;

  /* Header of the .mo file to be written.  */
  struct mo_file_header header;
  struct id_str_pair *msg_arr;
  size_t cnt, j;
  message_ty *entry;
  struct string_desc sd;

  /* Fill the structure describing the header.  */
  header.magic = _MAGIC;		/* Magic number.  */
  header.revision = MO_REVISION_NUMBER;	/* Revision number of file format.  */
  header.nstrings = mlp->nitems;	/* Number of strings.  */
  header.orig_tab_offset = sizeof (header);
			/* Offset of table for original string offsets.  */
  header.trans_tab_offset = sizeof (header)
			    + mlp->nitems * sizeof (struct string_desc);
			/* Offset of table for translation string offsets.  */
  header.hash_tab_size = hash_tab_size;	/* Size of used hashing table.  */
  header.hash_tab_offset =
	no_hash_table ? 0 : sizeof (header)
			    + 2 * (mlp->nitems * sizeof (struct string_desc));
			/* Offset of hashing table.  */

  /* Write the header out.  */
  fwrite (&header, sizeof (header), 1, output_file);

  /* Allocate table for the all elements of the hashing table.  */
  msg_arr = (struct id_str_pair *) alloca (mlp->nitems * sizeof (msg_arr[0]));

  /* Read values from list into array.  */
  for (j = 0; j < mlp->nitems; j++)
    {
      entry = mlp->item[j];

      msg_arr[j].id = entry->msgid;
      msg_arr[j].id_len = strlen (entry->msgid) + 1;
      msg_arr[j].id_plural = entry->msgid_plural;
      msg_arr[j].id_plural_len =
	(entry->msgid_plural != NULL ? strlen (entry->msgid_plural) + 1 : 0);
      msg_arr[j].str = entry->msgstr;
      msg_arr[j].str_len = entry->msgstr_len;
    }

  /* Sort the table according to original string.  */
  qsort (msg_arr, mlp->nitems, sizeof (msg_arr[0]), compare_id);

  /* Set offset to first byte after all the tables.  */
  sd.offset = roundup (sizeof (header)
		       + mlp->nitems * sizeof (sd)
		       + mlp->nitems * sizeof (sd)
		       + hash_tab_size * sizeof (nls_uint32),
		       alignment);

  /* Write out length and starting offset for all original strings.  */
  for (cnt = 0; cnt < mlp->nitems; ++cnt)
    {
      /* Subtract 1 because of the terminating NUL.  */
      sd.length = msg_arr[cnt].id_len + msg_arr[cnt].id_plural_len - 1;
      fwrite (&sd, sizeof (sd), 1, output_file);
      sd.offset += roundup (sd.length + 1, alignment);
    }

  /* Write out length and starting offset for all translation strings.  */
  for (cnt = 0; cnt < mlp->nitems; ++cnt)
    {
      /* Subtract 1 because of the terminating NUL.  */
      sd.length = msg_arr[cnt].str_len - 1;
      fwrite (&sd, sizeof (sd), 1, output_file);
      sd.offset += roundup (sd.length + 1, alignment);
    }

  /* Skip this part when no hash table is needed.  */
  if (!no_hash_table)
    {
      /* Allocate room for the hashing table to be written out.  */
      hash_tab = (nls_uint32 *) alloca (hash_tab_size * sizeof (nls_uint32));
      memset (hash_tab, '\0', hash_tab_size * sizeof (nls_uint32));

      /* Insert all value in the hash table, following the algorithm described
	 above.  */
      for (cnt = 0; cnt < mlp->nitems; ++cnt)
	{
	  nls_uint32 hash_val = hash_string (msg_arr[cnt].id);
	  nls_uint32 idx = hash_val % hash_tab_size;

	  if (hash_tab[idx] != 0)
	    {
	      /* We need the second hashing function.  */
	      nls_uint32 c = 1 + (hash_val % (hash_tab_size - 2));

	      do
		if (idx >= hash_tab_size - c)
		  idx -= hash_tab_size - c;
		else
		  idx += c;
	      while (hash_tab[idx] != 0);
	    }

	  hash_tab[idx] = cnt + 1;
	}

      /* Write the hash table out.  */
      fwrite (hash_tab, sizeof (nls_uint32), hash_tab_size, output_file);
    }

  /* Write bytes to make first string to be aligned.  */
  cnt = sizeof (header) + 2 * mlp->nitems * sizeof (sd)
	+ hash_tab_size * sizeof (nls_uint32);
  fwrite (&null, 1, roundup (cnt, alignment) - cnt, output_file);

  /* Now write the original strings.  */
  for (cnt = 0; cnt < mlp->nitems; ++cnt)
    {
      size_t len = msg_arr[cnt].id_len + msg_arr[cnt].id_plural_len;

      fwrite (msg_arr[cnt].id, msg_arr[cnt].id_len, 1, output_file);
      if (msg_arr[cnt].id_plural_len > 0)
	fwrite (msg_arr[cnt].id_plural, msg_arr[cnt].id_plural_len, 1,
		output_file);
      fwrite (&null, 1, roundup (len, alignment) - len, output_file);
    }

  /* Now write the translation strings.  */
  for (cnt = 0; cnt < mlp->nitems; ++cnt)
    {
      size_t len = msg_arr[cnt].str_len;

      fwrite (msg_arr[cnt].str, len, 1, output_file);
      fwrite (&null, 1, roundup (len, alignment) - len, output_file);
    }
}


int
msgdomain_write_mo (mlp, domain_name, file_name)
     message_list_ty *mlp;
     const char *domain_name;
     const char *file_name;
{
  FILE *output_file;

  /* If no entry for this domain don't even create the file.  */
  if (mlp->nitems != 0)
    {
      if (strcmp (domain_name, "-") == 0)
	{
	  output_file = stdout;
	  SET_BINARY (fileno (output_file));
	}
      else
	{
	  output_file = fopen (file_name, "wb");
	  if (output_file == NULL)
	    {
	      error (0, errno, _("error while opening \"%s\" for writing"),
		     file_name);
	      return 1;
	    }
	}

      if (output_file != NULL)
	{
	  write_table (output_file, mlp);

	  /* Make sure nothing went wrong.  */
	  if (fflush (output_file) || ferror (output_file))
	    error (EXIT_FAILURE, errno, _("error while writing \"%s\" file"),
		   file_name);

	  if (output_file != stdout)
	    fclose (output_file);
	}
    }

  return 0;
}
