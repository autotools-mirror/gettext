/* Output stream for attributed text, producing ANSI escape sequences.
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

#ifndef _TERM_OSTREAM_H
#define _TERM_OSTREAM_H

#include "ostream.h"


/* Querying and setting of text attributes.
   The stream has a notion of the current text attributes; they apply
   implicitly to all following output.  The attributes are automatically
   reset when the stream is closed.
   Note: Not all terminal types can actually render all attributes adequately.
   For example, xterm cannot render POSTURE_ITALIC nor the combination of
   WEIGHT_BOLD and UNDERLINE_ON.  */

typedef enum
{                     /* RGB components */
  COLOR_BLACK = 0,    /* 000 */
  COLOR_BLUE,         /* 001 */
  COLOR_GREEN,        /* 010 */
  COLOR_CYAN,         /* 011 */
  COLOR_RED,          /* 100 */
  COLOR_MAGENTA,      /* 101 */
  COLOR_YELLOW,       /* 110 */
  COLOR_WHITE,        /* 111 */
  COLOR_DEFAULT = -1  /* unknown */
} term_color_t;

typedef enum
{
  WEIGHT_NORMAL = 0,
  WEIGHT_BOLD,
  WEIGHT_DEFAULT = WEIGHT_NORMAL
} term_weight_t;

typedef enum
{
  POSTURE_NORMAL = 0,
  POSTURE_ITALIC, /* same as oblique */
  POSTURE_DEFAULT = POSTURE_NORMAL
} term_posture_t;

typedef enum
{
  UNDERLINE_OFF = 0,
  UNDERLINE_ON,
  UNDERLINE_DEFAULT = UNDERLINE_OFF
} term_underline_t;

struct term_ostream : struct ostream
{
methods:

  /* Get/set the text color.  */
  term_color_t get_color (term_ostream_t stream);
  void         set_color (term_ostream_t stream, term_color_t color);

  /* Get/set the background color.  */
  term_color_t get_bgcolor (term_ostream_t stream);
  void         set_bgcolor (term_ostream_t stream, term_color_t color);

  /* Get/set the font weight.  */
  term_weight_t get_weight (term_ostream_t stream);
  void          set_weight (term_ostream_t stream, term_weight_t weight);

  /* Get/set the font posture.  */
  term_posture_t get_posture (term_ostream_t stream);
  void           set_posture (term_ostream_t stream, term_posture_t posture);

  /* Get/set the text underline decoration.  */
  term_underline_t get_underline (term_ostream_t stream);
  void             set_underline (term_ostream_t stream,
				  term_underline_t underline);
};


#ifdef __cplusplus
extern "C" {
#endif


/* Create an output stream referring to the file descriptor FD.
   FILENAME is used only for error messages.
   The resulting stream will be line-buffered.
   Note that the resulting stream must be closed before FD can be closed.  */
extern term_ostream_t term_ostream_create (int fd, const char *filename);


#ifdef __cplusplus
}
#endif

#endif /* _TERM_OSTREAM_H */
