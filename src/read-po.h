/* Reading PO files.
   Copyright (C) 1995-1998, 2000, 2001 Free Software Foundation, Inc.
   This file was written by Bruno Haible <haible@clisp.cons.org>.

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

#ifndef _READ_PO_H
#define _READ_PO_H

#include "message.h"

/* If nonzero, remember comments for file name and line number for each
   msgid, if present in the reference input.  Defaults to true.  */
extern int line_comment;

/* Read the input file with the name INPUT_NAME.  The ending .po is added
   if necessary.  If INPUT_NAME is not an absolute file name and the file is
   not found, the list of directories in "dir-list.h" is searched.  Returns
   a list of messages.  */
extern msgdomain_list_ty *read_po_file PARAMS ((const char *input_name));

#endif /* _READ_PO_H */
