/* msgfmt specific message representation
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

#ifndef _MSGFMT_H
#define _MSGFMT_H

#include "pos.h"

/* Contains information about the definition of one translation.
   The msgid is the hash table key.  This is a mini 'struct message_ty'.  */
struct hashtable_entry
{
  char *msgid_plural;	/* The msgid's plural, if present.  */
  char *msgstr;		/* The msgstr strings.  */
  size_t msgstr_len;	/* The number of bytes in msgstr, including NULs.  */
  lex_pos_ty pos;	/* Position in the source PO file.  */
};

#endif /* _MSGFMT_H */
