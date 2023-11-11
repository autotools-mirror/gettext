/* GNU gettext - internationalization aids
   Copyright (C) 1995-2023 Free Software Foundation, Inc.

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

#ifndef _PO_GRAM_H
#define _PO_GRAM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Input, output, and local variables of a PO parser instance.  */
struct po_parser_state
{
  /* Input variables.  */
  /* Output variables.  */
  /* Local variables.  */
  long plural_counter;
};

extern int po_gram_parse (struct po_parser_state *ps);

#ifdef __cplusplus
}
#endif

#endif
