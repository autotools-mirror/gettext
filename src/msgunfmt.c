/* msgunfmt - converts binary .mo files to Uniforum style .po files
   Copyright (C) 1995-1998, 2000, 2001 Free Software Foundation, Inc.
   Written by Ulrich Drepper <drepper@gnu.ai.mit.edu>, April 1995.

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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#include "error.h"
#include "progname.h"
#include "system.h"
#include "message.h"
#include "read-mo.h"
#include "write-po.h"
#include "libgettext.h"

#define _(str) gettext (str)


/* Force output of PO file even if empty.  */
static int force_po;

/* Long options.  */
static const struct option long_options[] =
{
  { "escape", no_argument, NULL, 'E' },
  { "force-po", no_argument, &force_po, 1 },
  { "help", no_argument, NULL, 'h' },
  { "indent", no_argument, NULL, 'i' },
  { "no-escape", no_argument, NULL, 'e' },
  { "output-file", required_argument, NULL, 'o' },
  { "sort-output", no_argument, NULL, 's' },
  { "strict", no_argument, NULL, 'S' },
  { "version", no_argument, NULL, 'V' },
  { "width", required_argument, NULL, 'w', },
  { NULL, 0, NULL, 0 }
};


/* Prototypes for local functions.  Needed to ensure compiler checking of
   function argument counts despite of K&R C function definition syntax.  */
static void usage PARAMS ((int status));


int
main (argc, argv)
     int argc;
     char **argv;
{
  int optchar;
  bool do_help = false;
  bool do_version = false;
  const char *output_file = "-";
  message_list_ty *mlp;
  msgdomain_list_ty *result;
  bool sort_by_msgid = false;

  /* Set program name for messages.  */
  set_program_name (argv[0]);
  error_print_progname = maybe_print_progname;

#ifdef HAVE_SETLOCALE
  /* Set locale via LC_ALL.  */
  setlocale (LC_ALL, "");
#endif

  /* Set the text message domain.  */
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  while ((optchar = getopt_long (argc, argv, "eEhio:sVw:", long_options, NULL))
	 != EOF)
    switch (optchar)
      {
      case '\0':
	/* long option */
	break;

      case 'e':
	message_print_style_escape (false);
	break;

      case 'E':
	message_print_style_escape (true);
	break;

      case 'h':
	do_help = true;
	break;

      case 'i':
	message_print_style_indent ();
	break;

      case 'o':
	output_file = optarg;
	break;

      case 's':
	sort_by_msgid = true;
	break;

      case 'S':
	message_print_style_uniforum ();
	break;

      case 'V':
	do_version = true;
	break;

      case 'w':
	{
	  int value;
	  char *endp;
	  value = strtol (optarg, &endp, 10);
	  if (endp != optarg)
	    message_page_width_set (value);
	}
	break;

      default:
	usage (EXIT_FAILURE);
	break;
      }

  /* Version information is requested.  */
  if (do_version)
    {
      printf ("%s (GNU %s) %s\n", basename (program_name), PACKAGE, VERSION);
      /* xgettext: no-wrap */
      printf (_("Copyright (C) %s Free Software Foundation, Inc.\n\
This is free software; see the source for copying conditions.  There is NO\n\
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\
"),
	      "1995-1998, 2000, 2001");
      printf (_("Written by %s.\n"), "Ulrich Drepper");
      exit (EXIT_SUCCESS);
    }

  /* Help is requested.  */
  if (do_help)
    usage (EXIT_SUCCESS);

  /* Read the given .mo file. */
  mlp = message_list_alloc ();
  if (optind < argc)
    {
      do
	read_mo_file (mlp, argv[optind]);
      while (++optind < argc);
    }
  else
    read_mo_file (mlp, "-");

  result = msgdomain_list_alloc ();
  result->item[0]->messages = mlp;

  /* Sorting the list of messages.  */
  if (sort_by_msgid)
    msgdomain_list_sort_by_msgid (result);

  /* Write the resulting message list to the given .po file.  */
  msgdomain_list_print (result, output_file, force_po, false);

  /* No problems.  */
  exit (EXIT_SUCCESS);
}


/* Display usage information and exit.  */
static void
usage (status)
     int status;
{
  if (status != EXIT_SUCCESS)
    fprintf (stderr, _("Try `%s --help' for more information.\n"),
	     program_name);
  else
    {
      /* xgettext: no-wrap */
      printf (_("\
Usage: %s [OPTION] [FILE]...\n\
"), program_name);
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Convert binary message catalog to Uniforum style .po file.\n\
"));
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Mandatory arguments to long options are mandatory for short options too.\n\
"));
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Input file location:\n\
  FILE ...                 input .mo files\n\
If no input file is given or if it is -, standard input is read.\n\
"));
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Output file location:\n\
  -o, --output-file=FILE   write output to specified file\n\
The results are written to standard output if no output file is specified\n\
or if it is -.\n\
"));
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Output details:\n\
  -e, --no-escape          do not use C escapes in output (default)\n\
  -E, --escape             use C escapes in output, no extended chars\n\
      --force-po           write PO file even if empty\n\
  -i, --indent             write indented output style\n\
      --strict             write strict uniforum style\n\
  -w, --width=NUMBER       set output page width\n\
  -s, --sort-output        generate sorted output and remove duplicates\n\
"));
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Informative output:\n\
  -h, --help               display this help and exit\n\
  -V, --version            output version information and exit\n\
"));
      printf ("\n");
      fputs (_("Report bugs to <bug-gnu-gettext@gnu.org>.\n"),
	     stdout);
    }

  exit (status);
}
