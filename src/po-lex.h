/* GNU gettext - internationalization aids
   Copyright (C) 1995-1998, 2000, 2001 Free Software Foundation, Inc.

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
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#ifndef _PO_LEX_H
#define _PO_LEX_H

#include <sys/types.h>
#if HAVE_ICONV
#include <iconv.h>
#endif
#include "error.h"
#include "pos.h"

/* Lexical analyzer for reading PO files.  */


/* Global variables from po-lex.c.  */

/* Current position within the PO file.  */
extern lex_pos_ty gram_pos;

/* Number of parse errors within a PO file that cause the program to
   terminate.  Cf. error_message_count, declared in <error.h>.  */
extern unsigned int gram_max_allowed_errors;

/* The PO file's encoding, as specified in the header entry.  */
extern const char *po_lex_charset;

#if HAVE_ICONV
/* Converter from the PO file's encoding to UTF-8.  */
extern iconv_t po_lex_iconv;
#endif

/* Nonzero if obsolete entries shall be considered as valid.  */
extern int pass_obsolete_entries;


/* Open the PO file FNAME and prepare its lexical analysis.  */
extern void lex_open PARAMS ((const char *__fname));

/* Terminate lexical analysis and close the current PO file.  */
extern void lex_close PARAMS ((void));

/* Return the next token in the PO file.  The return codes are defined
   in "po-gram-gen2.h".  Associated data is put in 'po_gram_lval.  */
extern int po_gram_lex PARAMS ((void));

/* po_gram_lex() can return comments as COMMENT.  Switch this on or off.  */
extern void po_lex_pass_comments PARAMS ((int __flag));

/* po_gram_lex() can return obsolete entries as if they were normal entries.
   Switch this on or off.  */
extern void po_lex_pass_obsolete_entries PARAMS ((int __flag));


/* ISO C 99 is smart enough to allow optimizations like this.  */
#if __STDC__ && (defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L)

/* CAUTION: If you change this macro, you must also make identical
   changes to the function of the same name in src/po-lex.c  */

# define po_gram_error(fmt, ...)					    \
  do {									    \
    error_at_line (0, 0, gram_pos.file_name, gram_pos.line_number,	    \
		    fmt, __VA_ARGS__);					    \
    if (*fmt == '.')							    \
      --error_message_count;						    \
    else if (error_message_count >= gram_max_allowed_errors)		    \
      error (1, 0, _("too many errors, aborting"));			    \
  } while (0)


/* CAUTION: If you change this macro, you must also make identical
   changes to the function of the same name in src/po-lex.c  */

# define po_gram_error_at_line(pos, fmt, ...)				    \
  do {									    \
    error_at_line (0, 0, (pos)->file_name, (pos)->line_number,		    \
		    fmt, __VA_ARGS__);					    \
    if (*fmt == '.')							    \
      --error_message_count;						    \
    else if (error_message_count >= gram_max_allowed_errors)		    \
      error (1, 0, _("too many errors, aborting"));			    \
  } while (0)

/* GCC is also smart enough to allow optimizations like this.  */
#elif __STDC__ && defined __GNUC__ && __GNUC__ >= 2

/* CAUTION: If you change this macro, you must also make identical
   changes to the function of the same name in src/po-lex.c  */

# define po_gram_error(fmt, args...)					    \
  do {									    \
    error_at_line (0, 0, gram_pos.file_name, gram_pos.line_number,	    \
		    fmt, ## args);					    \
    if (*fmt == '.')							    \
      --error_message_count;						    \
    else if (error_message_count >= gram_max_allowed_errors)		    \
      error (1, 0, _("too many errors, aborting"));			    \
  } while (0)


/* CAUTION: If you change this macro, you must also make identical
   changes to the function of the same name in src/po-lex.c  */

# define po_gram_error_at_line(pos, fmt, args...)			    \
  do {									    \
    error_at_line (0, 0, (pos)->file_name, (pos)->line_number,		    \
		    fmt, ## args);					    \
    if (*fmt == '.')							    \
      --error_message_count;						    \
    else if (error_message_count >= gram_max_allowed_errors)		    \
      error (1, 0, _("too many errors, aborting"));			    \
  } while (0)

#else
extern void po_gram_error PARAMS ((const char *__fmt, ...));
extern void po_gram_error_at_line PARAMS ((const lex_pos_ty *__pos,
					   const char *__fmt, ...));
#endif


/* Contains information about the definition of one translation.  */
struct msgstr_def
{
  char *msgstr;
  size_t msgstr_len;
};

#endif
