/* Format strings.
   Copyright (C) 2001 Free Software Foundation, Inc.
   Written by Bruno Haible <haible@clisp.cons.org>, 2001.

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

#ifndef _FORMAT_H
#define _FORMAT_H

#include "pos.h"	/* Get lex_pos_ty.  */
#include "message.h"	/* Get NFORMATS.  */

/* This structure describes a format string parser for a language.  */
struct formatstring_parser
{
  /* Parse the given string as a format string.
     Return a freshly allocated structure describing
       1. the argument types/names needed for the format string,
       2. the total number of format directives.
     Return NULL if the string is not a valid format string.  */
  void * (*parse) (const char *string);

  /* Free a format string descriptor, returned by parse().  */
  void (*free) (void *descr);

  /* Return the number of format directives.
     A string that can be output literally has 0 format directives.  */
  int (*get_number_of_directives) (void *descr);

  /* Verify that the argument types/names in msgid_descr and those in
     msgstr_descr are the same.  If not, signal an error using
       error_with_progname = false;
       error_at_line (0, 0, pos->file_name, pos->line_number, ...);
       error_with_progname = true;
     and return true.  Otherwise return false.  */
  bool (*check) (const lex_pos_ty *pos, void *msgid_descr, void *msgstr_descr);
};

/* Format string parsers, each defined in its own file.  */
extern struct formatstring_parser formatstring_c;
extern struct formatstring_parser formatstring_python;
extern struct formatstring_parser formatstring_lisp;
extern struct formatstring_parser formatstring_java;
extern struct formatstring_parser formatstring_ycp;

/* Table of all format string parsers.  */
extern struct formatstring_parser *formatstring_parsers[NFORMATS];

#endif /* _FORMAT_H */
