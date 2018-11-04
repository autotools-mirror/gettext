/* Extracting a message.  Accumulating the message list.
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

#ifndef _XGETTEXT_MESSAGE_H
#define _XGETTEXT_MESSAGE_H

#include "message.h"
#include "pos.h"
#include "rc-str-list.h"

#include "xg-arglist-context.h"
#include "xg-encoding.h"

#ifdef __cplusplus
extern "C" {
#endif


/* Add a message to the list of extracted messages.
   msgctxt must be either NULL or a malloc()ed string; its ownership is passed
   to the callee.
   MSGID must be a malloc()ed string; its ownership is passed to the callee.
   POS->file_name must be allocated with indefinite extent.
   EXTRACTED_COMMENT is a comment that needs to be copied into the POT file,
   or NULL.
   COMMENT may be savable_comment, or it may be a saved copy of savable_comment
   (then add_reference must be used when saving it, and drop_reference while
   dropping it).  Clear savable_comment.
   Return the new or found message, or NULL if the message is excluded.  */
extern message_ty *remember_a_message (message_list_ty *mlp,
                                       char *msgctxt,
                                       char *msgid,
                                       flag_context_ty context,
                                       lex_pos_ty *pos,
                                       const char *extracted_comment,
                                       refcounted_string_list_ty *comment);

/* Add an msgid_plural to a message previously returned by
   remember_a_message.
   STRING must be a malloc()ed string; its ownership is passed to the callee.
   POS->file_name must be allocated with indefinite extent.
   COMMENT may be savable_comment, or it may be a saved copy of savable_comment
   (then add_reference must be used when saving it, and drop_reference while
   dropping it).  Clear savable_comment.  */
extern void remember_a_message_plural (message_ty *mp,
                                       char *string,
                                       flag_context_ty context,
                                       lex_pos_ty *pos,
                                       refcounted_string_list_ty *comment);


#ifdef __cplusplus
}
#endif


#endif /* _XGETTEXT_MESSAGE_H */
