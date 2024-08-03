/* Reading PO files.
   Copyright (C) 1995-2024 Free Software Foundation, Inc.
   This file was written by Peter Miller <millerp@canb.auug.org.au>

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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* Specification.  */
#include "read-catalog-file.h"

#include "open-catalog.h"
#include "xerror-handler.h"


msgdomain_list_ty *
read_catalog_file (const char *filename, catalog_input_format_ty input_syntax)
{
  char *real_filename;
  FILE *fp = open_catalog_file (filename, &real_filename, true);
  msgdomain_list_ty *result;

  result = read_catalog_stream (fp, real_filename, filename, input_syntax,
                                textmode_xerror_handler);

  if (fp != stdin)
    fclose (fp);

  return result;
}
