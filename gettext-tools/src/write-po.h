/* GNU gettext - internationalization aids
   Copyright (C) 1995-1998, 2000-2002 Free Software Foundation, Inc.

   This file was written by Peter Miller <millerp@canb.auug.org.au>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free SoftwareFoundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#ifndef _WRITE_PO_H
#define _WRITE_PO_H

#include "message.h"

#include <stdbool.h>

extern void
       message_page_width_set (size_t width);
extern void
       message_page_width_ignore (void);
extern void
       message_print_style_indent (void);
extern void
       message_print_style_uniforum (void);
extern void
       message_print_style_escape (bool flag);

extern void
       msgdomain_list_print (msgdomain_list_ty *mdlp,
			     const char *filename,
			     bool force, bool debug);
extern void
       msgdomain_list_sort_by_msgid (msgdomain_list_ty *mdlp);
extern void
       msgdomain_list_sort_by_filepos (msgdomain_list_ty *mdlp);

#endif /* _WRITE_PO_H */
