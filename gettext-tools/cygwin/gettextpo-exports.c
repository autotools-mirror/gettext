/* List of exported symbols of libgettextpo on Cygwin.
   Copyright (C) 2006 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2006.

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

#include "cygwin/export.h"

VARIABLE(libgettextpo_version)

#if 0 /* not needed - we use --export-all-symbols */
FUNCTION(po_file_check_all)
FUNCTION(po_file_create)
FUNCTION(po_file_domain_header)
FUNCTION(po_file_domains)
FUNCTION(po_file_free)
FUNCTION(po_file_read)
FUNCTION(po_file_read_v2)
FUNCTION(po_file_read_v3)
FUNCTION(po_file_write)
FUNCTION(po_file_write_v2)
FUNCTION(po_filepos_file)
FUNCTION(po_filepos_start_line)
FUNCTION(po_header_field)
FUNCTION(po_header_set_field)
FUNCTION(po_message_add_filepos)
FUNCTION(po_message_check_all)
FUNCTION(po_message_check_format)
FUNCTION(po_message_check_format_v2)
FUNCTION(po_message_comments)
FUNCTION(po_message_create)
FUNCTION(po_message_extracted_comments)
FUNCTION(po_message_filepos)
FUNCTION(po_message_insert)
FUNCTION(po_message_is_format)
FUNCTION(po_message_is_fuzzy)
FUNCTION(po_message_is_obsolete)
FUNCTION(po_message_iterator)
FUNCTION(po_message_iterator_free)
FUNCTION(po_message_msgctxt)
FUNCTION(po_message_msgid)
FUNCTION(po_message_msgid_plural)
FUNCTION(po_message_msgstr)
FUNCTION(po_message_msgstr_plural)
FUNCTION(po_message_remove_filepos)
FUNCTION(po_message_set_comments)
FUNCTION(po_message_set_extracted_comments)
FUNCTION(po_message_set_format)
FUNCTION(po_message_set_fuzzy)
FUNCTION(po_message_set_msgctxt)
FUNCTION(po_message_set_msgid)
FUNCTION(po_message_set_msgid_plural)
FUNCTION(po_message_set_msgstr)
FUNCTION(po_message_set_msgstr_plural)
FUNCTION(po_message_set_obsolete)
FUNCTION(po_next_message)
#endif
