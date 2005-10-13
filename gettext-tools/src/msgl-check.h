/* Checking of messages in PO files.
   Copyright (C) 2005 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2005.

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
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  */

#ifndef _MSGL_CHECK_H
#define _MSGL_CHECK_H 1

#include "message.h"
#include "pos.h"


#ifdef __cplusplus
extern "C" {
#endif


/* Perform plural expression checking.
   Return nonzero if an error was seen.  */
extern int check_plural (message_list_ty *mlp);

/* Perform all checks on a message.
   Return nonzero if an error was seen.  */
extern int check_message (const message_ty *mp,
			  const lex_pos_ty *msgid_pos,
			  int check_newlines,
			  int check_format_strings,
			  int check_header,
			  int check_compatibility,
			  int check_accelerators, char accelerator_char);


#ifdef __cplusplus
}
#endif

#endif /* _MSGL_CHECK_H */
