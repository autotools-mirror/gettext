/* Manipulates attributes of messages in translation catalogs.
   Copyright (C) 2001-2002 Free Software Foundation, Inc.
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


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#include "dir-list.h"
#include "error.h"
#include "progname.h"
#include "basename.h"
#include "message.h"
#include "read-po.h"
#include "write-po.h"
#include "exit.h"
#include "gettext.h"

#define _(str) gettext (str)


/* Force output of PO file even if empty.  */
static int force_po;

/* Bit mask of subsets to remove.  */
enum
{
  REMOVE_UNTRANSLATED	= 1 << 0,
  REMOVE_TRANSLATED	= 1 << 1,
  REMOVE_FUZZY		= 1 << 2,
  REMOVE_NONFUZZY	= 1 << 3,
  REMOVE_OBSOLETE	= 1 << 4,
  REMOVE_NONOBSOLETE	= 1 << 5
};
static int to_remove;

/* Bit mask of actions to perform on all messages.  */
enum
{
  SET_FUZZY		= 1 << 0,
  RESET_FUZZY		= 1 << 1,
  SET_OBSOLETE		= 1 << 2,
  RESET_OBSOLETE	= 1 << 3
};
static int to_change;

/* Long options.  */
static const struct option long_options[] =
{
  { "add-location", no_argument, &line_comment, 1 },
  { "clear-fuzzy", no_argument, NULL, CHAR_MAX + 8 },
  { "clear-obsolete", no_argument, NULL, CHAR_MAX + 10 },
  { "directory", required_argument, NULL, 'D' },
  { "escape", no_argument, NULL, 'E' },
  { "force-po", no_argument, &force_po, 1 },
  { "fuzzy", no_argument, NULL, CHAR_MAX + 11 },
  { "help", no_argument, NULL, 'h' },
  { "indent", no_argument, NULL, 'i' },
  { "no-escape", no_argument, NULL, 'e' },
  { "no-fuzzy", no_argument, NULL, CHAR_MAX + 3 },
  { "no-location", no_argument, &line_comment, 0 },
  { "no-obsolete", no_argument, NULL, CHAR_MAX + 5 },
  { "no-wrap", no_argument, NULL, CHAR_MAX + 13 },
  { "obsolete", no_argument, NULL, CHAR_MAX + 12 },
  { "only-fuzzy", no_argument, NULL, CHAR_MAX + 4 },
  { "only-obsolete", no_argument, NULL, CHAR_MAX + 6 },
  { "output-file", required_argument, NULL, 'o' },
  { "set-fuzzy", no_argument, NULL, CHAR_MAX + 7 },
  { "set-obsolete", no_argument, NULL, CHAR_MAX + 9 },
  { "sort-by-file", no_argument, NULL, 'F' },
  { "sort-output", no_argument, NULL, 's' },
  { "strict", no_argument, NULL, 'S' },
  { "translated", no_argument, NULL, CHAR_MAX + 1 },
  { "untranslated", no_argument, NULL, CHAR_MAX + 2 },
  { "version", no_argument, NULL, 'V' },
  { "width", required_argument, NULL, 'w', },
  { NULL, 0, NULL, 0 }
};


/* Prototypes for local functions.  Needed to ensure compiler checking of
   function argument counts despite of K&R C function definition syntax.  */
static void usage PARAMS ((int status));
static bool is_message_selected PARAMS ((const message_ty *mp));
static void process_message_list PARAMS ((message_list_ty *mlp));
static msgdomain_list_ty *
       process_msgdomain_list PARAMS ((msgdomain_list_ty *mdlp));


int
main (argc, argv)
     int argc;
     char **argv;
{
  int optchar;
  bool do_help;
  bool do_version;
  char *output_file;
  const char *input_file;
  msgdomain_list_ty *result;
  bool sort_by_msgid = false;
  bool sort_by_filepos = false;

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

  /* Set default values for variables.  */
  do_help = false;
  do_version = false;
  output_file = NULL;
  input_file = NULL;

  while ((optchar = getopt_long (argc, argv, "D:eEFhino:sVw:", long_options,
				 NULL)) != EOF)
    switch (optchar)
      {
      case '\0':		/* Long option.  */
	break;

      case 'D':
	dir_list_append (optarg);
	break;

      case 'e':
	message_print_style_escape (false);
	break;

      case 'E':
	message_print_style_escape (true);
	break;

      case 'F':
	sort_by_filepos = true;
        break;

      case 'h':
	do_help = true;
	break;

      case 'i':
	message_print_style_indent ();
	break;

      case 'n':
	line_comment = 1;
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

      case CHAR_MAX + 1: /* --translated */
	to_remove |= REMOVE_UNTRANSLATED;
	break;

      case CHAR_MAX + 2: /* --untranslated */
	to_remove |= REMOVE_TRANSLATED;
	break;

      case CHAR_MAX + 3: /* --no-fuzzy */
	to_remove |= REMOVE_FUZZY;
	break;

      case CHAR_MAX + 4: /* --only-fuzzy */
	to_remove |= REMOVE_NONFUZZY;
	break;

      case CHAR_MAX + 5: /* --no-obsolete */
	to_remove |= REMOVE_OBSOLETE;
	break;

      case CHAR_MAX + 6: /* --only-obsolete */
	to_remove |= REMOVE_NONOBSOLETE;
	break;

      case CHAR_MAX + 7: /* --set-fuzzy */
	to_change |= SET_FUZZY;
	break;

      case CHAR_MAX + 8: /* --clear-fuzzy */
	to_change |= RESET_FUZZY;
	break;

      case CHAR_MAX + 9: /* --set-obsolete */
	to_change |= SET_OBSOLETE;
	break;

      case CHAR_MAX + 10: /* --clear-obsolete */
	to_change |= RESET_OBSOLETE;
	break;

      case CHAR_MAX + 11: /* --fuzzy */
	to_remove |= REMOVE_NONFUZZY;
	to_change |= RESET_FUZZY;
	break;

      case CHAR_MAX + 12: /* --obsolete */
	to_remove |= REMOVE_NONOBSOLETE;
	to_change |= RESET_OBSOLETE;
	break;

      case CHAR_MAX + 13: /* --no-wrap */
	message_page_width_ignore ();
	break;

      default:
	usage (EXIT_FAILURE);
	/* NOTREACHED */
      }

  /* Version information requested.  */
  if (do_version)
    {
      printf ("%s (GNU %s) %s\n", basename (program_name), PACKAGE, VERSION);
      /* xgettext: no-wrap */
      printf (_("Copyright (C) %s Free Software Foundation, Inc.\n\
This is free software; see the source for copying conditions.  There is NO\n\
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\
"),
	      "2001-2002");
      printf (_("Written by %s.\n"), "Bruno Haible");
      exit (EXIT_SUCCESS);
    }

  /* Help is requested.  */
  if (do_help)
    usage (EXIT_SUCCESS);

  /* Test whether we have an .po file name as argument.  */
  if (optind == argc)
    input_file = "-";
  else if (optind + 1 == argc)
    input_file = argv[optind];
  else
    {
      error (EXIT_SUCCESS, 0, _("at most one input file allowed"));
      usage (EXIT_FAILURE);
    }

  /* Verify selected options.  */
  if (!line_comment && sort_by_filepos)
    error (EXIT_FAILURE, 0, _("%s and %s are mutually exclusive"),
	   "--no-location", "--sort-by-file");

  if (sort_by_msgid && sort_by_filepos)
    error (EXIT_FAILURE, 0, _("%s and %s are mutually exclusive"),
	   "--sort-output", "--sort-by-file");

  /* Read input file.  */
  result = read_po_file (input_file);

  /* Filter the messages and manipulate the attributes.  */
  result = process_msgdomain_list (result);

  /* Sorting the list of messages.  */
  if (sort_by_filepos)
    msgdomain_list_sort_by_filepos (result);
  else if (sort_by_msgid)
    msgdomain_list_sort_by_msgid (result);

  /* Write the PO file.  */
  msgdomain_list_print (result, output_file, force_po, false);

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
Usage: %s [OPTION] [INPUTFILE]\n\
"), program_name);
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Filters the messages of a translation catalog according to their attributes,\n\
and manipulates the attributes.\n\
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
  INPUTFILE                   input PO file\n\
  -D, --directory=DIRECTORY   add DIRECTORY to list for input files search\n\
If no input file is given or if it is -, standard input is read.\n\
"));
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Output file location:\n\
  -o, --output-file=FILE      write output to specified file\n\
The results are written to standard output if no output file is specified\n\
or if it is -.\n\
"));
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Message selection:\n\
      --translated            keep translated, remove untranslated messages\n\
      --untranslated          keep untranslated, remove translated messages\n\
      --no-fuzzy              remove 'fuzzy' marked messages\n\
      --only-fuzzy            keep 'fuzzy' marked messages\n\
      --no-obsolete           remove obsolete #~ messages\n\
      --only-obsolete         keep obsolete #~ messages\n\
"));
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Attribute manipulation:\n\
      --set-fuzzy             set all messages 'fuzzy'\n\
      --clear-fuzzy           set all messages non-'fuzzy'\n\
      --set-obsolete          set all messages obsolete\n\
      --clear-obsolete        set all messages non-obsolete\n\
      --fuzzy                 synonym for --only-fuzzy --clear-fuzzy\n\
      --obsolete              synonym for --only-obsolete --clear-obsolete\n\
"));
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Output details:\n\
  -e, --no-escape             do not use C escapes in output (default)\n\
  -E, --escape                use C escapes in output, no extended chars\n\
      --force-po              write PO file even if empty\n\
  -i, --indent                write the .po file using indented style\n\
      --no-location           do not write '#: filename:line' lines\n\
  -n, --add-location          generate '#: filename:line' lines (default)\n\
      --strict                write out strict Uniforum conforming .po file\n\
  -w, --width=NUMBER          set output page width\n\
      --no-wrap               do not break long message lines, longer than\n\
                              the output page width, into several lines\n\
  -s, --sort-output           generate sorted output\n\
  -F, --sort-by-file          sort output by file location\n\
"));
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Informative output:\n\
  -h, --help                  display this help and exit\n\
  -V, --version               output version information and exit\n\
"));
      printf ("\n");
      fputs (_("Report bugs to <bug-gnu-gettext@gnu.org>.\n"),
	     stdout);
    }

  exit (status);
}


/* Return true if a message should be kept.  */
static bool
is_message_selected (mp)
     const message_ty *mp;
{
  /* Always keep the header entry.  */
  if (mp->msgid[0] == '\0')
    return true;

  if ((to_remove & (REMOVE_UNTRANSLATED | REMOVE_TRANSLATED))
      && (mp->msgstr[0] == '\0'
	  ? to_remove & REMOVE_UNTRANSLATED
	  : to_remove & REMOVE_TRANSLATED))
    return false;

  if ((to_remove & (REMOVE_FUZZY | REMOVE_NONFUZZY))
      && (mp->is_fuzzy
	  ? to_remove & REMOVE_FUZZY
	  : to_remove & REMOVE_NONFUZZY))
    return false;

  if ((to_remove & (REMOVE_OBSOLETE | REMOVE_NONOBSOLETE))
      && (mp->obsolete
	  ? to_remove & REMOVE_OBSOLETE
	  : to_remove & REMOVE_NONOBSOLETE))
    return false;

  return true;
}


static void
process_message_list (mlp)
     message_list_ty *mlp;
{
  /* Keep only the selected messages.  */
  message_list_remove_if_not (mlp, is_message_selected);

  /* Change the attributes.  */
  if (to_change)
    {
      size_t j;

      for (j = 0; j < mlp->nitems; j++)
	{
	  message_ty *mp = mlp->item[j];

	  if (to_change & SET_FUZZY)
	    mp->is_fuzzy = true;
	  if (to_change & RESET_FUZZY)
	    mp->is_fuzzy = false;
	  /* Always keep the header entry non-obsolete.  */
	  if ((to_change & SET_OBSOLETE) && (mp->msgid[0] != '\0'))
	    mp->obsolete = true;
	  if (to_change & RESET_OBSOLETE)
	    mp->obsolete = false;
	}
    }
}


static msgdomain_list_ty *
process_msgdomain_list (mdlp)
     msgdomain_list_ty *mdlp;
{
  size_t k;

  for (k = 0; k < mdlp->nitems; k++)
    process_message_list (mdlp->item[k]->messages);

  return mdlp;
}
