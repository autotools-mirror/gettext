/* xgettext common functions.
   Copyright (C) 2001-2002 Free Software Foundation, Inc.
   Written by Peter Miller <millerp@canb.auug.org.au>
   and Bruno Haible <haible@clisp.cons.org>, 2001.

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

#ifndef _XGETTEXT_H
#define _XGETTEXT_H

#include <stddef.h>
#include "message.h"
#include "pos.h"

/* Borrowed from read-po.h.  */
extern int line_comment;

/* If true, omit the header entry.
   If false, keep the header entry present in the input.  */
extern int xgettext_omit_header;

extern bool substring_match;

/* Split keyword spec into keyword, argnum1, argnum2.  */
extern void split_keywordspec PARAMS ((const char *spec, const char **endp,
				       int *argnum1p, int *argnum2p));

/* List of messages whose msgids must not be extracted, or NULL.
   Used by remember_a_message().  */
extern message_list_ty *exclude;

/* Comment handling: There is a list of automatic comments that may be appended
   to the next message.  Used by remember_a_message().  */
extern void xgettext_comment_add PARAMS ((const char *str));
extern const char *xgettext_comment PARAMS ((size_t n));
extern void xgettext_comment_reset PARAMS ((void));

/* Add a message to the list of extracted messages.
   string must be malloc()ed string; its ownership is passed to the callee.
   pos->file_name must be allocated with indefinite extent.  */
extern message_ty *remember_a_message PARAMS ((message_list_ty *mlp,
					       char *string, lex_pos_ty *pos));
/* Add an msgid_plural to a message previously returned by
   remember_a_message.
   string must be malloc()ed string; its ownership is passed to the callee.
   pos->file_name must be allocated with indefinite extent.  */
extern void remember_a_message_plural PARAMS ((message_ty *mp,
					       char *string, lex_pos_ty *pos));


#endif /* _XGETTEXT_H */
