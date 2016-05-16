/* Routines for locating data files
   Copyright (C) 2016 Free Software Foundation, Inc.

   This file was written by Daiki Ueno <ueno@gnu.org>, 2016.

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
# include <config.h>
#endif

/* Specification.  */
#include "search-path.h"

#include <stdlib.h>
#include <string.h>

#include "concat-filename.h"
#include "xalloc.h"
#include "xmemdup0.h"
#include "xvasprintf.h"

/* Find the standard search path for data files.  Returns a NULL
   terminated list of strings.  */
char **
get_search_path (const char *name)
{
  char **result;
  const char *gettextdatadir;
  const char *gettextdatadirs;
  char *base, *dir;
  size_t n_dirs = 2;

  gettextdatadirs = getenv ("GETTEXTDATADIRS");

  /* If GETTEXTDATADIRS is not set, fallback to XDG_DATA_DIRS.  */
  if (gettextdatadirs == NULL || *gettextdatadirs == '\0')
    gettextdatadirs = getenv ("XDG_DATA_DIRS");

  if (gettextdatadirs != NULL)
    {
      const char *start = gettextdatadirs;

      /* Count the number of valid elements in GETTEXTDATADIRS.  */
      while (*start != '\0')
        {
          char *end = strchrnul (start, ':');

          /* Skip empty element.  */
          if (start != end)
            n_dirs++;

          if (*end == '\0')
            break;

          start = end + 1;
        }
    }

  result = XCALLOC (n_dirs + 1, char *);

  n_dirs = 0;

  gettextdatadir = getenv ("GETTEXTDATADIR");
  if (gettextdatadir == NULL || gettextdatadir[0] == '\0')
    gettextdatadir = GETTEXTDATADIR;

  if (name == NULL)
    dir = xstrdup (gettextdatadir);
  else
    dir = xconcatenated_filename (gettextdatadir, name, NULL);
  result[n_dirs++] = dir;

  if (gettextdatadirs != NULL)
    {
      const char *start = gettextdatadirs;

      /* Count the number of valid elements in GETTEXTDATADIRS.  */
      while (*start != '\0')
        {
          char *end = strchrnul (start, ':');

          /* Skip empty element.  */
          if (start != end)
            {
              base = xmemdup0 (start, end - start);
              if (name == NULL)
                dir = base;
              else
                {
                  dir = xconcatenated_filename (base, name, NULL);
                  free (base);
                }
              result[n_dirs++] = dir;
            }

          if (*end == '\0')
            break;

          start = end + 1;
        }
    }

  base = xasprintf ("%s%s", gettextdatadir, PACKAGE_SUFFIX);
  if (name == NULL)
    dir = base;
  else
    {
      dir = xconcatenated_filename (base, name, NULL);
      free (base);
    }
  result[n_dirs++] = dir;

  return result;
}
