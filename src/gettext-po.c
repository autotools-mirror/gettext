/* Public API for GNU gettext PO files.
   Copyright (C) 2003 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2003.

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
# include <config.h>
#endif

/* Specification.  */
#include "gettext-po.h"

#include <stdio.h>
#include <stdlib.h>

#include "message.h"
#include "xmalloc.h"
#include "read-po.h"


struct po_file
{
  msgdomain_list_ty *mdlp;
  const char *real_filename;
  const char *logical_filename;
  const char **domains;
};

struct po_message_iterator
{
  message_list_ty *mlp;
  size_t index;
};

/* A po_message_t is actually a 'struct message_ty *'.  */


/* Read a PO file into memory.
   Return its contents.  Upon failure, return NULL and set errno.  */

po_file_t
po_file_read (const char *filename)
{
  FILE *fp;
  po_file_t file;

  fp = fopen (filename, "r");
  if (fp == NULL)
    return NULL;
  file = (struct po_file *) xmalloc (sizeof (struct po_file));
  file->real_filename = filename;
  file->logical_filename = filename;
  file->mdlp = read_po (fp, file->real_filename, file->logical_filename);
  file->domains = NULL;
  fclose (fp);
  return file;
}


/* Free a PO file from memory.  */

void
po_file_free (po_file_t file)
{
  msgdomain_list_free (file->mdlp);
  if (file->domains != NULL)
    free (file->domains);
  free (file);
}


/* Return the names of the domains covered by a PO file in memory.  */

const char * const *
po_file_domains (po_file_t file)
{
  if (file->domains == NULL)
    {
      size_t n = file->mdlp->nitems;
      const char **domains =
	(const char **) xmalloc ((n + 1) * sizeof (const char *));
      size_t j;

      for (j = 0; j < n; j++)
	domains[j] = file->mdlp->item[j]->domain;
      domains[n] = NULL;

      file->domains = domains;
    }

  return file->domains;
}


/* Create an iterator for traversing a domain of a PO file in memory.
   The domain NULL denotes the default domain.  */

po_message_iterator_t
po_message_iterator (po_file_t file, const char *domain)
{
  po_message_iterator_t iterator;

  if (domain == NULL)
    domain = MESSAGE_DOMAIN_DEFAULT;

  iterator =
    (struct po_message_iterator *)
    xmalloc (sizeof (struct po_message_iterator));
  iterator->mlp = msgdomain_list_sublist (file->mdlp, domain, false);
  iterator->index = 0;

  return iterator;
}


/* Free an iterator.  */

void
po_message_iterator_free (po_message_iterator_t iterator)
{
  free (iterator);
}


/* Return the next message, and advance the iterator.
   Return NULL at the end of the message list.  */

po_message_t
po_next_message (po_message_iterator_t iterator)
{
  if (iterator->index < iterator->mlp->nitems)
    return (po_message_t) iterator->mlp->item[iterator->index++];
  else
    return NULL;
}


/* Return the msgid (untranslated English string) of a message.  */

const char *
po_message_msgid (po_message_t message)
{
  message_ty *mp = (message_ty *) message;

  return mp->msgid;
}


/* Return the msgid_plural (untranslated English plural string) of a message,
   or NULL for a message without plural.  */

const char *
po_message_msgid_plural (po_message_t message)
{
  message_ty *mp = (message_ty *) message;

  return mp->msgid_plural;
}


/* Return the msgstr (translation) of a message.
   Return the empty string for an untranslated message.  */

const char *
po_message_msgstr (po_message_t message)
{
  message_ty *mp = (message_ty *) message;

  return mp->msgstr;
}


/* Return the msgstr[index] for a message with plural handling, or
   NULL when the index is out of range or for a message without plural.  */

const char *
po_message_msgstr_plural (po_message_t message, int index)
{
  message_ty *mp = (message_ty *) message;

  if (mp->msgid_plural != NULL && index >= 0)
    {
      const char *p;
      const char *p_end = mp->msgstr + mp->msgstr_len;

      for (p = mp->msgstr; ; p += strlen (p) + 1, index--)
	{
	  if (p >= p_end)
	    return NULL;
	  if (index == 0)
	    break;
	}
      return p;
    }
  else
    return NULL;
}
