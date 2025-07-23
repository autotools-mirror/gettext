/* xgettext OCaml backend.
   Copyright (C) 2020-2025 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2025.

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
# include "config.h"
#endif

/* Specification.  */
#include "x-ocaml.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <error.h>
#include "message.h"
#include "clean-temp.h"
#include "concat-filename.h"
#include "execute.h"
#include "xvasprintf.h"
#include "x-po.h"
#include "xgettext.h"
#include "gettext.h"

/* A convenience macro.  I don't like writing gettext() every time.  */
#define _(str) gettext (str)

/* We don't parse OCaml directly, but instead rely on the 'ocaml-gettext'
   program, that invokes the 'ocaml-xgettext' program.  Both are contained
   in the 'opam' package named 'gettext':
     https://opam.ocaml.org/packages/gettext/
     https://github.com/gildor478/ocaml-gettext
     https://github.com/gildor478/ocaml-gettext/blob/master/doc/reference-manual.md

   Comments start with '(*' and end with '*)' and can be nested.
   Reference: <https://ocaml.org/docs/tour-of-ocaml>
 */


/* ====================== Keyword set customization.  ====================== */

/* This function currently has no effect.  */
void
x_ocaml_extract_all (void)
{
}

/* This function currently has no effect.  */
void
x_ocaml_keyword (const char *keyword)
{
}

/* This function currently has no effect.  */
void
init_flag_table_ocaml (void)
{
}


/* ========================= Extracting strings.  ========================== */

static bool
is_not_header (const message_ty *mp)
{
  return !is_header (mp);
}

void
extract_ocaml (const char *found_in_dir, const char *real_filename,
               const char *logical_filename,
               flag_context_list_table_ty *flag_table,
               msgdomain_list_ty *mdlp)
{
  /* Invoke
       ocaml-gettext --action extract --extract-pot <temp>.pot real_filename  */

  /* First, create a temporary directory where this invocation can place its
     output.  */
  struct temp_dir *tmpdir = create_temp_dir ("ocgt", NULL, false);
  if (tmpdir == NULL)
    exit (EXIT_FAILURE);

  /* Prepare the temporary POT file name.  */
  char *temp_file_name = xconcatenated_filename (tmpdir->dir_name, "temp.pot", NULL);
  register_temp_file (tmpdir, temp_file_name);

  /* Invoke ocaml-gettext.  */
  const char *progname = "ocaml-gettext";
  {
    const char *argv[7];
    int exitstatus;
    /* Prepare arguments.  */
    argv[0] = progname;
    argv[1] = "--action";
    argv[2] = "extract";
    argv[3] = "--extract-pot";
    argv[4] = temp_file_name;
    argv[5] = logical_filename;
    argv[6] = NULL;
    exitstatus = execute (progname, progname, argv, NULL, found_in_dir,
                          true, false, false, false, true, false, NULL);
    if (exitstatus != 0)
      error (EXIT_FAILURE, 0, _("%s subprocess failed with exit code %d"),
             progname, exitstatus);
  }

  /* Read the resulting POT file.  */
  {
    FILE *fp = fopen (temp_file_name, "r");
    if (fp == NULL)
      error (EXIT_FAILURE, 0, _("%s subprocess did not create the expected file"),
             progname);
    char *dummy_filename = xasprintf (_("(output from '%s')"), progname);
    extract_po (fp, temp_file_name, dummy_filename, flag_table, mdlp);
    fclose (fp);
    free (dummy_filename);
  }

  cleanup_temp_dir (tmpdir);

  if (xgettext_omit_header)
    {
      /* Remove the header entry.  */
      if (mdlp->nitems > 0)
        message_list_remove_if_not (mdlp->item[0]->messages, is_not_header);
    }
}
