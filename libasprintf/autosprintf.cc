/* Class autosprintf - formatted output to an ostream.
   Copyright (C) 2002 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2002.

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published
   by the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA.  */

/* Specification.  */
#include "autosprintf.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "lib-asprintf.h"

namespace gnu
{

  /* Constructor: takes a format string and the printf arguments.  */
  autosprintf::autosprintf (const char *format, ...)
  {
    va_list args;
    va_start (args, format);
    if (vasprintf (&str, format, args) < 0)
      str = NULL;
    va_end (args);
  }

  /* Copy constructor.  Necessary because the destructor is nontrivial.  */
  autosprintf::autosprintf (const autosprintf& src)
  {
    str = (src.str != NULL ? strdup (src.str) : NULL);
  }

  /* Destructor: frees the temporarily allocated string.  */
  autosprintf::~autosprintf ()
  {
    free (str);
  }

  /* Conversion to string.  */
  autosprintf::operator char * () const
  {
    if (str != NULL)
      {
        size_t length = strlen (str) + 1;
        char *copy = new char[length];
        memcpy (copy, str, length);
        return copy;
      }
    else
      return NULL;
  }
  autosprintf::operator std::string () const
  {
    return std::string (str ? str : "(error in autosprintf)");
  }
}
