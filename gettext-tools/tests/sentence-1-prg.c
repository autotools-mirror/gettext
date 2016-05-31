/* Test of sentence handling.
   Copyright (C) 2015-2016 Free Software Foundation, Inc.
   Written by Daiki Ueno <ueno@gnu.org>, 2015.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "sentence.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int
main (int argc, char **argv)
{
  char buffer[1024];

  while (1)
    {
      const char *result;
      ucs4_t ending_char;
      char *p;

      if (!fgets (buffer, sizeof buffer, stdin))
        break;

      sentence_end_required_spaces = atoi (buffer);

      memset (buffer, 0, sizeof buffer);
      p = buffer;
      while (1)
        {
          p = fgets (p, sizeof buffer - (buffer - p), stdin);
          if (p == NULL)
            break;
          if (*p == '\n')
            break;
          p = strchr (p, '\n') + 1;
        }
      if (p == NULL)
        break;

      *(p - 1) = '\0';

      result = sentence_end (buffer, &ending_char);
      printf ("%X\n%s\n\n", ending_char, result);
    }

  return 0;
}
