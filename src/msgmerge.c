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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <getopt.h>
#include <limits.h>
#include <stdbool.h>
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
#include "system.h"
#include "po.h"
#include "msgl-equal.h"
#include "backupfile.h"
#include "copy-file.h"
#include "libgettext.h"

#define _(str) gettext (str)


/* If true do not print unneeded messages.  */
static bool quiet;

/* Verbosity level.  */
static int verbosity_level;

/* Force output of PO file even if empty.  */
static int force_po;

/* Apply the .pot file to each of the domains in the PO file.  */
static bool multi_domain_mode = false;

/* List of user-specified compendiums.  */
static message_list_list_ty *compendiums;

/* Update mode.  */
static bool update_mode = false;
static const char *version_control_string;
static const char *backup_suffix_string;

/* Long options.  */
static const struct option long_options[] =
{
  { "add-location", no_argument, &line_comment, 1 },
  { "backup", required_argument, NULL, CHAR_MAX + 1 },
  { "compendium", required_argument, NULL, 'C', },
  { "directory", required_argument, NULL, 'D' },
  { "escape", no_argument, NULL, 'E' },
  { "force-po", no_argument, &force_po, 1 },
  { "help", no_argument, NULL, 'h' },
  { "indent", no_argument, NULL, 'i' },
  { "multi-domain", no_argument, NULL, 'm' },
  { "no-escape", no_argument, NULL, 'e' },
  { "no-location", no_argument, &line_comment, 0 },
  { "output-file", required_argument, NULL, 'o' },
  { "quiet", no_argument, NULL, 'q' },
  { "sort-by-file", no_argument, NULL, 'F' },
  { "sort-output", no_argument, NULL, 's' },
  { "silent", no_argument, NULL, 'q' },
  { "strict", no_argument, NULL, CHAR_MAX + 2 },
  { "suffix", required_argument, NULL, CHAR_MAX + 3 },
  { "update", no_argument, NULL, 'U' },
  { "verbose", no_argument, NULL, 'v' },
  { "version", no_argument, NULL, 'V' },
  { "width", required_argument, NULL, 'w', },
  { NULL, 0, NULL, 0 }
};


struct statistics
{
  size_t merged;
  size_t fuzzied;
  size_t missing;
  size_t obsolete;
};


/* Prototypes for local functions.  Needed to ensure compiler checking of
   function argument counts despite of K&R C function definition syntax.  */
static void usage PARAMS ((int status));
static void compendium PARAMS ((const char *filename));
static void match_domain PARAMS ((const char *fn1, const char *fn2,
				  message_list_list_ty *definitions,
				  message_list_ty *refmlp,
				  message_list_ty *resultmlp,
				  struct statistics *stats,
				  unsigned int *processed));
static msgdomain_list_ty *merge PARAMS ((const char *fn1, const char *fn2,
					 msgdomain_list_ty **defp));


int
main (argc, argv)
     int argc;
     char **argv;
{
  int opt;
  bool do_help;
  bool do_version;
  char *output_file;
  msgdomain_list_ty *def;
  msgdomain_list_ty *result;
  bool sort_by_filepos = false;
  bool sort_by_msgid = false;

  /* Set program name for messages.  */
  set_program_name (argv[0]);
  error_print_progname = maybe_print_progname;
  verbosity_level = 0;
  quiet = false;
  gram_max_allowed_errors = UINT_MAX;

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

  while ((opt
	  = getopt_long (argc, argv, "C:D:eEFhimo:qsUvVw:", long_options, NULL))
	 != EOF)
    switch (opt)
      {
      case '\0':		/* Long option.  */
	break;

      case 'C':
	compendium (optarg);
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

      case 'm':
	multi_domain_mode = true;
	break;

      case 'o':
	output_file = optarg;
	break;

      case 'q':
	quiet = true;
	break;

      case 's':
        sort_by_msgid = true;
        break;

      case 'U':
	update_mode = true;
	break;

      case 'v':
	++verbosity_level;
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

      case CHAR_MAX + 1: /* --backup */
	version_control_string = optarg;
	break;

      case CHAR_MAX + 2: /* --strict */
	message_print_style_uniforum ();
	break;

      case CHAR_MAX + 3: /* --suffix */
	backup_suffix_string = optarg;
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
      printf (_("Written by %s.\n"), "Peter Miller");
      exit (EXIT_SUCCESS);
    }

  /* Help is requested.  */
  if (do_help)
    usage (EXIT_SUCCESS);

  /* Test whether we have an .po file name as argument.  */
  if (optind >= argc)
    {
      error (EXIT_SUCCESS, 0, _("no input files given"));
      usage (EXIT_FAILURE);
    }
  if (optind + 2 != argc)
    {
      error (EXIT_SUCCESS, 0, _("exactly 2 input files required"));
      usage (EXIT_FAILURE);
    }

  /* Verify selected options.  */
  if (update_mode)
    {
      if (output_file != NULL)
	{
	  error (EXIT_FAILURE, 0, _("%s and %s are mutually exclusive"),
		 "--update", "--output-file");
	}
    }
  else
    {
      if (version_control_string != NULL)
	{
	  error (EXIT_SUCCESS, 0, _("%s is only valid with %s"),
		 "--backup", "--update");
	  usage (EXIT_FAILURE);
	}
      if (backup_suffix_string != NULL)
	{
	  error (EXIT_SUCCESS, 0, _("%s is only valid with %s"),
		 "--suffix", "--update");
	  usage (EXIT_FAILURE);
	}
    }

  if (!line_comment && sort_by_filepos)
    error (EXIT_FAILURE, 0, _("%s and %s are mutually exclusive"),
	   "--no-location", "--sort-by-file");

  if (sort_by_msgid && sort_by_filepos)
    error (EXIT_FAILURE, 0, _("%s and %s are mutually exclusive"),
	   "--sort-output", "--sort-by-file");

  /* Merge the two files.  */
  result = merge (argv[optind], argv[optind + 1], &def);

  /* Sort the results.  */
  if (sort_by_filepos)
    msgdomain_list_sort_by_filepos (result);
  else if (sort_by_msgid)
    msgdomain_list_sort_by_msgid (result);

  if (update_mode)
    {
      /* Do nothing if the original file and the result are equal.  */
      if (!msgdomain_list_equal (def, result))
	{
	  /* Back up def.po.  */
	  enum backup_type backup_type;
	  char *backup_file;

	  output_file = argv[optind];

	  if (backup_suffix_string == NULL)
	    {
	      backup_suffix_string = getenv ("SIMPLE_BACKUP_SUFFIX");
	      if (backup_suffix_string != NULL
		  && backup_suffix_string[0] == '\0')
		backup_suffix_string = NULL;
	    }
	  if (backup_suffix_string != NULL)
	    simple_backup_suffix = backup_suffix_string;

	  backup_type = xget_version (_("backup type"), version_control_string);
	  if (backup_type != none)
	    {
	      backup_file = find_backup_file_name (output_file, backup_type);
	      copy_file (output_file, backup_file);
	    }

	  /* Write the merged message list out.  */
	  msgdomain_list_print (result, output_file, true, false);
	}
    }
  else
    {
      /* Write the merged message list out.  */
      msgdomain_list_print (result, output_file, force_po, false);
    }

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
Usage: %s [OPTION] def.po ref.pot\n\
"), program_name);
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Merges two Uniforum style .po files together.  The def.po file is an\n\
existing PO file with translations which will be taken over to the newly\n\
created file as long as they still match; comments will be preserved,\n\
but extracted comments and file positions will be discarded.  The ref.pot\n\
file is the last created PO file with up-to-date source references but\n\
old translations, or a PO Template file (generally created by xgettext);\n\
any translations or comments in the file will be discarded, however dot\n\
comments and file positions will be preserved.  Where an exact match\n\
cannot be found, fuzzy matching is used to produce better results.\n\
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
  def.po                      translations referring to old sources\n\
  ref.pot                     references to new sources\n\
  -D, --directory=DIRECTORY   add DIRECTORY to list for input files search\n\
  -C, --compendium=FILE       additional library of message translations,\n\
                              may be specified more than once\n\
"));
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Operation mode:\n\
  -U, --update                update def.po,\n\
                              do nothing if def.po already up to date\n\
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
Output file location in update mode:\n\
The result is written back to def.po.\n\
      --backup=CONTROL        make a backup of def.po\n\
      --suffix=SUFFIX         override the usual backup suffix\n\
The version control method may be selected via the --backup option or through\n\
the VERSION_CONTROL environment variable.  Here are the values:\n\
  none, off       never make backups (even if --backup is given)\n\
  numbered, t     make numbered backups\n\
  existing, nil   numbered if numbered backups exist, simple otherwise\n\
  simple, never   always make simple backups\n\
The backup suffix is `~', unless set with --suffix or the SIMPLE_BACKUP_SUFFIX\n\
environment variable.\n\
"));
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Operation modifiers:\n\
  -m, --multi-domain          apply ref.pot to each of the domains in def.po\n\
"));
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Output details:\n\
  -e, --no-escape             do not use C escapes in output (default)\n\
  -E, --escape                use C escapes in output, no extended chars\n\
      --force-po              write PO file even if empty\n\
  -i, --indent                indented output style\n\
      --no-location           suppress '#: filename:line' lines\n\
      --add-location          preserve '#: filename:line' lines (default)\n\
      --strict                strict Uniforum output style\n\
  -w, --width=NUMBER          set output page width\n\
  -s, --sort-output           generate sorted output\n\
  -F, --sort-by-file          sort output by file location\n\
"));
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Informative output:\n\
  -h, --help                  display this help and exit\n\
  -V, --version               output version information and exit\n\
  -v, --verbose               increase verbosity level\n\
  -q, --quiet, --silent       suppress progress indicators\n\
"));
      printf ("\n");
      fputs (_("Report bugs to <bug-gnu-gettext@gnu.org>.\n"),
	     stdout);
    }

  exit (status);
}


static void
compendium (filename)
    const char *filename;
{
  msgdomain_list_ty *mdlp;
  size_t k;

  mdlp = read_po_file (filename);
  if (!compendiums)
    compendiums = message_list_list_alloc ();
  for (k = 0; k < mdlp->nitems; k++)
    message_list_list_append (compendiums, mdlp->item[k]->messages);
}


#define DOT_FREQUENCY 10

static void
match_domain (fn1, fn2, definitions, refmlp, resultmlp, stats, processed)
     const char *fn1;
     const char *fn2;
     message_list_list_ty *definitions;
     message_list_ty *refmlp;
     message_list_ty *resultmlp;
     struct statistics *stats;
     unsigned int *processed;
{
  size_t j;

  for (j = 0; j < refmlp->nitems; j++, (*processed)++)
    {
      message_ty *refmsg;
      message_ty *defmsg;

      /* Because merging can take a while we print something to signal
	 we are not dead.  */
      if (!quiet && verbosity_level <= 1 && *processed % DOT_FREQUENCY == 0)
	fputc ('.', stderr);

      refmsg = refmlp->item[j];

      /* See if it is in the other file.  */
      defmsg = message_list_list_search (definitions, refmsg->msgid);
      if (defmsg)
	{
	  /* Merge the reference with the definition: take the #. and
	     #: comments from the reference, take the # comments from
	     the definition, take the msgstr from the definition.  Add
	     this merged entry to the output message list.  */
	  message_ty *mp = message_merge (defmsg, refmsg);

	  message_list_append (resultmlp, mp);

	  /* Remember that this message has been used, when we scan
	     later to see if anything was omitted.  */
	  defmsg->used = 1;
	  stats->merged++;
	}
      else if (refmsg->msgid[0] != '\0')
	{
	  /* If the message was not defined at all, try to find a very
	     similar message, it could be a typo, or the suggestion may
	     help.  */
	  defmsg = message_list_list_search_fuzzy (definitions, refmsg->msgid);
	  if (defmsg)
	    {
	      message_ty *mp;

	      if (verbosity_level > 1)
		{
		  po_gram_error_at_line (&refmsg->pos, _("\
this message is used but not defined..."));
		  po_gram_error_at_line (&defmsg->pos, _("\
...but this definition is similar"));
		}

	      /* Merge the reference with the definition: take the #. and
		 #: comments from the reference, take the # comments from
		 the definition, take the msgstr from the definition.  Add
		 this merged entry to the output message list.  */
	      mp = message_merge (defmsg, refmsg);

	      mp->is_fuzzy = true;

	      message_list_append (resultmlp, mp);

	      /* Remember that this message has been used, when we scan
		 later to see if anything was omitted.  */
	      defmsg->used = 1;
	      stats->fuzzied++;
	      if (!quiet && verbosity_level <= 1)
		/* Always print a dot if we handled a fuzzy match.  */
		fputc ('.', stderr);
	    }
	  else
	    {
	      message_ty *mp;

	      if (verbosity_level > 1)
		po_gram_error_at_line (&refmsg->pos, _("\
this message is used but not defined in %s"), fn1);

	      mp = message_copy (refmsg);

	      message_list_append (resultmlp, mp);
	      stats->missing++;
	    }
	}
    }
}

static msgdomain_list_ty *
merge (fn1, fn2, defp)
     const char *fn1;			/* definitions */
     const char *fn2;			/* references */
     msgdomain_list_ty **defp;		/* return definition list */
{
  msgdomain_list_ty *def;
  msgdomain_list_ty *ref;
  size_t j, k;
  unsigned int processed;
  struct statistics stats;
  msgdomain_list_ty *result;
  message_list_list_ty *definitions;
  message_list_ty *empty_list;

  stats.merged = stats.fuzzied = stats.missing = stats.obsolete = 0;

  /* This is the definitions file, created by a human.  */
  def = read_po_file (fn1);

  /* Create the set of places to look for message definitions: a list
     whose first element will be definitions for the current domain, and
     whose other elements come from the compendiums.  */
  definitions = message_list_list_alloc ();
  message_list_list_append (definitions, NULL);
  if (compendiums)
    message_list_list_append_list (definitions, compendiums);
  empty_list = message_list_alloc ();

  /* This is the references file, created by groping the sources with
     the xgettext program.  */
  ref = read_po_file (fn2);
  /* Add a dummy header entry, if the references file contains none.  */
  for (k = 0; k < ref->nitems; k++)
    if (message_list_search (ref->item[k]->messages, "") == NULL)
      {
	static lex_pos_ty pos = { __FILE__, __LINE__ };
	message_ty *refheader = message_alloc ("", NULL, "", 1, &pos);

	message_list_prepend (ref->item[k]->messages, refheader);
      }

  result = msgdomain_list_alloc ();
  processed = 0;

  /* Every reference must be matched with its definition. */
  if (!multi_domain_mode)
    for (k = 0; k < ref->nitems; k++)
      {
	const char *domain = ref->item[k]->domain;
	message_list_ty *refmlp = ref->item[k]->messages;
	message_list_ty *resultmlp =
	  msgdomain_list_sublist (result, domain, true);

	definitions->item[0] = msgdomain_list_sublist (def, domain, false);
	if (definitions->item[0] == NULL)
	  definitions->item[0] = empty_list;

	match_domain (fn1, fn2, definitions, refmlp, resultmlp,
		      &stats, &processed);
      }
  else
    {
      /* Apply the references messages in the default domain to each of
	 the definition domains.  */
      message_list_ty *refmlp = ref->item[0]->messages;

      for (k = 0; k < def->nitems; k++)
	{
	  const char *domain = def->item[k]->domain;
	  message_list_ty *defmlp = def->item[k]->messages;

	  /* Ignore the default message domain if it has no messages.  */
	  if (k > 0 || defmlp->nitems > 0)
	    {
	      message_list_ty *resultmlp =
		msgdomain_list_sublist (result, domain, true);

	      definitions->item[0] = defmlp;

	      match_domain (fn1, fn2, definitions, refmlp, resultmlp,
			    &stats, &processed);
	    }
	}
    }

  /* Look for messages in the definition file, which are not present
     in the reference file, indicating messages which defined but not
     used in the program.  Don't scan the compendium(s).  */
  for (k = 0; k < def->nitems; ++k)
    {
      const char *domain = def->item[k]->domain;
      message_list_ty *defmlp = def->item[k]->messages;

      for (j = 0; j < defmlp->nitems; j++)
	{
	  message_ty *defmsg = defmlp->item[j];

	  if (!defmsg->used)
	    {
	      /* Remember the old translation although it is not used anymore.
		 But we mark it as obsolete.  */
	      message_ty *mp;

	      mp = message_copy (defmsg);
	      mp->obsolete = true;

	      message_list_append (msgdomain_list_sublist (result, domain, true),
				   mp);
	      stats.obsolete++;
	    }
	}
    }

  /* Report some statistics.  */
  if (verbosity_level > 0)
    fprintf (stderr, _("%s\
Read %ld old + %ld reference, \
merged %ld, fuzzied %ld, missing %ld, obsolete %ld.\n"),
	     !quiet && verbosity_level <= 1 ? "\n" : "",
	     (long) def->nitems, (long) ref->nitems,
	     (long) stats.merged, (long) stats.fuzzied, (long) stats.missing,
	     (long) stats.obsolete);
  else if (!quiet)
    fputs (_(" done.\n"), stderr);

  /* Return results.  */
  *defp = def;
  return result;
}
