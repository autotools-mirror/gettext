/* Extracts strings from C source file to Uniforum style .po file.
   Copyright (C) 1995-1998, 2000-2002 Free Software Foundation, Inc.
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

#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <locale.h>
#include <limits.h>

#include "xgettext.h"
#include "dir-list.h"
#include "file-list.h"
#include "str-list.h"
#include "error.h"
#include "progname.h"
#include "basename.h"
#include "xerror.h"
#include "xmalloc.h"
#include "strstr.h"
#include "xerror.h"
#include "exit.h"
#include "pathname.h"
#include "strcase.h"
#include "stpcpy.h"
#include "po.h"
#include "message.h"
#include "po-time.h"
#include "write-po.h"
#include "format.h"
#include "gettext.h"

/* A convenience macro.  I don't like writing gettext() every time.  */
#define _(str) gettext (str)


#include "x-c.h"
#include "x-po.h"
#include "x-python.h"
#include "x-lisp.h"
#include "x-elisp.h"
#include "x-librep.h"
#include "x-java.h"
#include "x-awk.h"
#include "x-ycp.h"
#include "x-tcl.h"
#include "x-rst.h"
#include "x-glade.h"


/* If nonzero add all comments immediately preceding one of the keywords. */
static bool add_all_comments = false;

/* Tag used in comment of prevailing domain.  */
static char *comment_tag;

/* Compare tokens with keywords using substring matching instead of
   equality.  */
bool substring_match;

/* Name of default domain file.  If not set defaults to messages.po.  */
static const char *default_domain;

/* If called with --debug option the output reflects whether format
   string recognition is done automatically or forced by the user.  */
static int do_debug;

/* Content of .po files with symbols to be excluded.  */
message_list_ty *exclude;

/* Force output of PO file even if empty.  */
static int force_po;

/* Copyright holder of the output file and the translations.  */
static const char *copyright_holder = "THE PACKAGE'S COPYRIGHT HOLDER";

/* String used as prefix for msgstr.  */
static const char *msgstr_prefix;

/* String used as suffix for msgstr.  */
static const char *msgstr_suffix;

/* Directory in which output files are created.  */
static char *output_dir;

/* If nonzero omit header with information about this run.  */
int xgettext_omit_header;

/* Long options.  */
static const struct option long_options[] =
{
  { "add-comments", optional_argument, NULL, 'c' },
  { "add-location", no_argument, &line_comment, 1 },
  { "c++", no_argument, NULL, 'C' },
  { "copyright-holder", required_argument, NULL, CHAR_MAX + 1 },
  { "debug", no_argument, &do_debug, 1 },
  { "default-domain", required_argument, NULL, 'd' },
  { "directory", required_argument, NULL, 'D' },
  { "escape", no_argument, NULL, 'E' },
  { "exclude-file", required_argument, NULL, 'x' },
  { "extract-all", no_argument, NULL, 'a' },
  { "files-from", required_argument, NULL, 'f' },
  { "force-po", no_argument, &force_po, 1 },
  { "foreign-user", no_argument, NULL, CHAR_MAX + 2 },
  { "help", no_argument, NULL, 'h' },
  { "indent", no_argument, NULL, 'i' },
  { "join-existing", no_argument, NULL, 'j' },
  { "keyword", optional_argument, NULL, 'k' },
  { "keyword-substring", no_argument, NULL, 'K'},
  { "language", required_argument, NULL, 'L' },
  { "msgstr-prefix", optional_argument, NULL, 'm' },
  { "msgstr-suffix", optional_argument, NULL, 'M' },
  { "no-escape", no_argument, NULL, 'e' },
  { "no-location", no_argument, &line_comment, 0 },
  { "no-wrap", no_argument, NULL, CHAR_MAX + 3 },
  { "omit-header", no_argument, &xgettext_omit_header, 1 },
  { "output", required_argument, NULL, 'o' },
  { "output-dir", required_argument, NULL, 'p' },
  { "sort-by-file", no_argument, NULL, 'F' },
  { "sort-output", no_argument, NULL, 's' },
  { "strict", no_argument, NULL, 'S' },
  { "string-limit", required_argument, NULL, 'l' },
  { "trigraphs", no_argument, NULL, 'T' },
  { "version", no_argument, NULL, 'V' },
  { "width", required_argument, NULL, 'w', },
  { NULL, 0, NULL, 0 }
};


/* The extractors must all be functions returning void and taking three
   arguments designating the input stream and one message domain list argument
   in which to add the messages.  */
typedef void (*extractor_func) PARAMS ((FILE *fp, const char *real_filename,
					const char *logical_filename,
					msgdomain_list_ty *mdlp));


/* Prototypes for local functions.  Needed to ensure compiler checking of
   function argument counts despite of K&R C function definition syntax.  */
static void usage PARAMS ((int status))
#if defined __GNUC__ && ((__GNUC__ == 2 && __GNUC_MINOR__ > 4) || __GNUC__ > 2)
	__attribute__ ((noreturn))
#endif
;
static void exclude_directive_domain PARAMS ((po_ty *pop, char *name));
static void exclude_directive_message PARAMS ((po_ty *pop, char *msgid,
					       lex_pos_ty *msgid_pos,
					       char *msgid_plural,
					       char *msgstr, size_t msgstr_len,
					       lex_pos_ty *msgstr_pos,
					       bool obsolete));
static void read_exclusion_file PARAMS ((char *file_name));
static FILE *xgettext_open PARAMS ((const char *fn, char **logical_file_name_p,
				    char **real_file_name_p));
static void extract_from_file PARAMS ((const char *file_name,
				       extractor_func extractor,
				       msgdomain_list_ty *mdlp));
static message_ty *construct_header PARAMS ((void));
static void finalize_header PARAMS ((msgdomain_list_ty *mdlp));
static extractor_func language_to_extractor PARAMS ((const char *name));
static const char *extension_to_language PARAMS ((const char *extension));


int
main (argc, argv)
     int argc;
     char *argv[];
{
  int cnt;
  int optchar;
  bool do_help = false;
  bool do_version = false;
  msgdomain_list_ty *mdlp;
  bool join_existing = false;
  bool sort_by_msgid = false;
  bool sort_by_filepos = false;
  const char *file_name;
  const char *files_from = NULL;
  string_list_ty *file_list;
  char *output_file = NULL;
  extractor_func extractor = NULL;

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

  /* Set initial value of variables.  */
  default_domain = MESSAGE_DOMAIN_DEFAULT;

  while ((optchar = getopt_long (argc, argv,
				 "ac::Cd:D:eEf:Fhijk::l:L:m::M::no:p:sTVw:x:",
				 long_options, NULL)) != EOF)
    switch (optchar)
      {
      case '\0':		/* Long option.  */
	break;
      case 'a':
	x_c_extract_all ();
	x_python_extract_all ();
	x_lisp_extract_all ();
	x_elisp_extract_all ();
	x_librep_extract_all ();
	x_java_extract_all ();
	x_awk_extract_all ();
	x_tcl_extract_all ();
	x_glade_extract_all ();
	break;
      case 'c':
	if (optarg == NULL)
	  {
	    add_all_comments = true;
	    comment_tag = NULL;
	  }
	else
	  {
	    add_all_comments = false;
	    comment_tag = optarg;
	    /* We ignore leading white space.  */
	    while (isspace ((unsigned char) *comment_tag))
	      ++comment_tag;
	  }
	break;
      case 'C':
	extractor = language_to_extractor ("C++");
	break;
      case 'd':
	default_domain = optarg;
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
      case 'f':
	files_from = optarg;
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
      case 'j':
	join_existing = true;
	break;
      case 'k':
	if (optarg == NULL || *optarg != '\0')
	  {
	    x_c_keyword (optarg);
	    x_python_keyword (optarg);
	    x_lisp_keyword (optarg);
	    x_elisp_keyword (optarg);
	    x_librep_keyword (optarg);
	    x_java_keyword (optarg);
	    x_awk_keyword (optarg);
	    x_tcl_keyword (optarg);
	    x_glade_keyword (optarg);
	  }
	break;
      case 'K':
	substring_match = true;
	break;
      case 'l':
	/* Accepted for backward compatibility with 0.10.35.  */
	break;
      case 'L':
	extractor = language_to_extractor (optarg);
	break;
      case 'm':
	/* -m takes an optional argument.  If none is given "" is assumed. */
	msgstr_prefix = optarg == NULL ? "" : optarg;
	break;
      case 'M':
	/* -M takes an optional argument.  If none is given "" is assumed. */
	msgstr_suffix = optarg == NULL ? "" : optarg;
	break;
      case 'n':
	line_comment = 1;
	break;
      case 'o':
	output_file = optarg;
	break;
      case 'p':
	{
	  size_t len = strlen (optarg);

	  if (output_dir != NULL)
	    free (output_dir);

	  if (optarg[len - 1] == '/')
	    output_dir = xstrdup (optarg);
	  else
	    output_dir = xasprintf ("%s/", optarg);
	}
	break;
      case 's':
	sort_by_msgid = true;
	break;
      case 'S':
	message_print_style_uniforum ();
	break;
      case 'T':
	x_c_trigraphs ();
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
      case 'x':
	read_exclusion_file (optarg);
	break;
      case CHAR_MAX + 1:	/* --copyright-holder */
	copyright_holder = optarg;
	break;
      case CHAR_MAX + 2:	/* --foreign-user */
	copyright_holder = "";
	break;
      case CHAR_MAX + 3:	/* --no-wrap */
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
	      "1995-1998, 2000-2002");
      printf (_("Written by %s.\n"), "Ulrich Drepper");
      exit (EXIT_SUCCESS);
    }

  /* Help is requested.  */
  if (do_help)
    usage (EXIT_SUCCESS);

  /* Verify selected options.  */
  if (!line_comment && sort_by_filepos)
    error (EXIT_FAILURE, 0, _("%s and %s are mutually exclusive"),
	   "--no-location", "--sort-by-file");

  if (sort_by_msgid && sort_by_filepos)
    error (EXIT_FAILURE, 0, _("%s and %s are mutually exclusive"),
	   "--sort-output", "--sort-by-file");

  if (join_existing && strcmp (default_domain, "-") == 0)
    error (EXIT_FAILURE, 0, _("\
--join-existing cannot be used when output is written to stdout"));

  if (!x_c_any_keywords ())
    {
      error (0, 0, _("\
xgettext cannot work without keywords to look for"));
      usage (EXIT_FAILURE);
    }

  /* Test whether we have some input files given.  */
  if (files_from == NULL && optind >= argc)
    {
      error (EXIT_SUCCESS, 0, _("no input file given"));
      usage (EXIT_FAILURE);
    }

  /* Canonize msgstr prefix/suffix.  */
  if (msgstr_prefix != NULL && msgstr_suffix == NULL)
    msgstr_suffix = "";
  else if (msgstr_prefix == NULL && msgstr_suffix != NULL)
    msgstr_prefix = "";

  /* Default output directory is the current directory.  */
  if (output_dir == NULL)
    output_dir = ".";

  /* Construct the name of the output file.  If the default domain has
     the special name "-" we write to stdout.  */
  if (output_file)
    {
      if (IS_ABSOLUTE_PATH (output_file) || strcmp (output_file, "-") == 0)
	file_name = xstrdup (output_file);
      else
	/* Please do NOT add a .po suffix! */
	file_name = concatenated_pathname (output_dir, output_file, NULL);
    }
  else if (strcmp (default_domain, "-") == 0)
    file_name = "-";
  else
    file_name = concatenated_pathname (output_dir, default_domain, ".po");

  /* Determine list of files we have to process.  */
  if (files_from != NULL)
    file_list = read_names_from_file (files_from);
  else
    file_list = string_list_alloc ();
  /* Append names from command line.  */
  for (cnt = optind; cnt < argc; ++cnt)
    string_list_append_unique (file_list, argv[cnt]);

  /* Allocate a message list to remember all the messages.  */
  mdlp = msgdomain_list_alloc (true);

  /* Generate a header, so that we know how and when this PO file was
     created.  */
  if (!xgettext_omit_header)
    message_list_append (mdlp->item[0]->messages, construct_header ());

  /* Read in the old messages, so that we can add to them.  */
  if (join_existing)
    {
      /* Temporarily reset the directory list to empty, because file_name
	 is an output file and therefore should not be searched for.  */
      void *saved_directory_list = dir_list_save_reset ();

      extract_from_file (file_name, extract_po, mdlp);

      dir_list_restore (saved_directory_list);
    }

  /* Process all input files.  */
  for (cnt = 0; cnt < file_list->nitems; ++cnt)
    {
      const char *fname;
      extractor_func this_file_extractor;

      fname = file_list->item[cnt];

      if (extractor)
        this_file_extractor = extractor;
      else
	{
	  const char *extension;
	  const char *language;

	  /* Work out what the file extension is.  */
	  extension = strrchr (fname, '/');
	  if (!extension)
	    extension = fname;
	  extension = strrchr (extension, '.');
	  if (extension)
	    ++extension;
	  else
	    extension = "";

	  /* Derive the language from the extension, and the extractor
	     function from the language.  */
	  language = extension_to_language (extension);
	  if (language == NULL)
	    {
	      error (0, 0, _("\
warning: file `%s' extension `%s' is unknown; will try C"), fname, extension);
	      language = "C";
	    }
	  this_file_extractor = language_to_extractor (language);
	}

      /* Extract the strings from the file.  */
      extract_from_file (fname, this_file_extractor, mdlp);
    }
  string_list_free (file_list);

  /* Finalize the constructed header.  */
  if (!xgettext_omit_header)
    finalize_header (mdlp);

  /* Sorting the list of messages.  */
  if (sort_by_filepos)
    msgdomain_list_sort_by_filepos (mdlp);
  else if (sort_by_msgid)
    msgdomain_list_sort_by_msgid (mdlp);

  /* Write the PO file.  */
  msgdomain_list_print (mdlp, file_name, force_po, do_debug);

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
Usage: %s [OPTION] [INPUTFILE]...\n\
"), program_name);
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Extract translatable strings from given input files.\n\
"));
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Mandatory arguments to long options are mandatory for short options too.\n\
Similarly for optional arguments.\n\
"));
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Input file location:\n\
  INPUTFILE ...                  input files\n\
  -f, --files-from=FILE          get list of input files from FILE\n\
  -D, --directory=DIRECTORY      add DIRECTORY to list for input files search\n\
If input file is -, standard input is read.\n\
"));
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Output file location:\n\
  -d, --default-domain=NAME      use NAME.po for output (instead of messages.po)\n\
  -o, --output=FILE              write output to specified file\n\
  -p, --output-dir=DIR           output files will be placed in directory DIR\n\
If output file is -, output is written to standard output.\n\
"));
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Choice of input file language:\n\
  -L, --language=NAME            recognise the specified language\n\
                                   (C, C++, ObjectiveC, PO, Python, Lisp,\n\
                                   EmacsLisp, librep, Java, awk, YCP, Tcl,\n\
                                   RST, Glade)\n\
  -C, --c++                      shorthand for --language=C++\n\
By default the language is guessed depending on the input file name extension.\n\
"));
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Operation mode:\n\
  -j, --join-existing            join messages with existing file\n\
  -x, --exclude-file=FILE.po     entries from FILE.po are not extracted\n\
  -c, --add-comments[=TAG]       place comment block with TAG (or those\n\
                                 preceding keyword lines) in output file\n\
"));
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Language=C/C++ specific options:\n\
  -a, --extract-all              extract all strings\n\
  -k, --keyword[=WORD]           additional keyword to be looked for (without\n\
                                 WORD means not to use default keywords)\n\
  -T, --trigraphs                understand ANSI C trigraphs for input\n\
      --debug                    more detailed formatstring recognition result\n\
"));
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Output details:\n\
  -e, --no-escape                do not use C escapes in output (default)\n\
  -E, --escape                   use C escapes in output, no extended chars\n\
      --force-po                 write PO file even if empty\n\
  -i, --indent                   write the .po file using indented style\n\
      --no-location              do not write '#: filename:line' lines\n\
  -n, --add-location             generate '#: filename:line' lines (default)\n\
      --strict                   write out strict Uniforum conforming .po file\n\
  -w, --width=NUMBER             set output page width\n\
      --no-wrap                  do not break long message lines, longer than\n\
                                 the output page width, into several lines\n\
  -s, --sort-output              generate sorted output\n\
  -F, --sort-by-file             sort output by file location\n\
      --omit-header              don't write header with `msgid \"\"' entry\n\
      --copyright-holder=STRING  set copyright holder in output\n\
      --foreign-user             omit FSF copyright in output for foreign user\n\
  -m, --msgstr-prefix[=STRING]   use STRING or \"\" as prefix for msgstr entries\n\
  -M, --msgstr-suffix[=STRING]   use STRING or \"\" as suffix for msgstr entries\n\
"));
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Informative output:\n\
  -h, --help                     display this help and exit\n\
  -V, --version                  output version information and exit\n\
"));
      printf ("\n");
      fputs (_("Report bugs to <bug-gnu-gettext@gnu.org>.\n"),
	     stdout);
    }

  exit (status);
}


static void
exclude_directive_domain (pop, name)
     po_ty *pop;
     char *name;
{
  po_gram_error_at_line (&gram_pos,
			 _("this file may not contain domain directives"));
}


static void
exclude_directive_message (pop, msgid, msgid_pos, msgid_plural,
			   msgstr, msgstr_len, msgstr_pos, obsolete)
     po_ty *pop;
     char *msgid;
     lex_pos_ty *msgid_pos;
     char *msgid_plural;
     char *msgstr;
     size_t msgstr_len;
     lex_pos_ty *msgstr_pos;
     bool obsolete;
{
  message_ty *mp;

  /* See if this message ID has been seen before.  */
  if (exclude == NULL)
    exclude = message_list_alloc (true);
  mp = message_list_search (exclude, msgid);
  if (mp != NULL)
    free (msgid);
  else
    {
      mp = message_alloc (msgid, msgid_plural, "", 1, msgstr_pos);
      /* Do not free msgid.  */
      message_list_append (exclude, mp);
    }

  /* All we care about is the msgid.  Throw the msgstr away.
     Don't even check for duplicate msgids.  */
  free (msgstr);
}


/* So that the one parser can be used for multiple programs, and also
   use good data hiding and encapsulation practices, an object
   oriented approach has been taken.  An object instance is allocated,
   and all actions resulting from the parse will be through
   invocations of method functions of that object.  */

static po_method_ty exclude_methods =
{
  sizeof (po_ty),
  NULL, /* constructor */
  NULL, /* destructor */
  exclude_directive_domain,
  exclude_directive_message,
  NULL, /* parse_brief */
  NULL, /* parse_debrief */
  NULL, /* comment */
  NULL, /* comment_dot */
  NULL, /* comment_filepos */
  NULL, /* comment_special */
};


static void
read_exclusion_file (file_name)
     char *file_name;
{
  po_ty *pop;

  pop = po_alloc (&exclude_methods);
  po_scan_file (pop, file_name);
  po_free (pop);
}


void
split_keywordspec (spec, endp, argnum1p, argnum2p)
     const char *spec;
     const char **endp;
     int *argnum1p;
     int *argnum2p;
{
  const char *p;

  /* Start parsing from the end.  */
  p = spec + strlen (spec);
  if (p > spec && isdigit ((unsigned char) p[-1]))
    {
      const char *last_arg;

      do
	p--;
      while (p > spec && isdigit ((unsigned char) p[-1]));

      last_arg = p;

      if (p > spec && p[-1] == ',')
	{
	  p--;

	  if (p > spec && isdigit ((unsigned char) p[-1]))
	    {
	      const char *first_arg;

	      do
		p--;
	      while (p > spec && isdigit ((unsigned char) p[-1]));

	      first_arg = p;

	      if (p > spec && p[-1] == ':')
		{
		  /* Parsed "KEYWORD:ARGNUM1,ARGNUM2".  */
		  char *dummy;

		  *endp = p - 1;
		  *argnum1p = strtol (first_arg, &dummy, 10);
		  *argnum2p = strtol (last_arg, &dummy, 10);
		  return;
		}
	    }
	}
      else if (p > spec && p[-1] == ':')
	{
	  /* Parsed "KEYWORD:ARGNUM1.  */
	  char *dummy;

	  *endp = p - 1;
	  *argnum1p = strtol (last_arg, &dummy, 10);
	  *argnum2p = 0;
	  return;
	}
    }
  /* Parsed "KEYWORD".  */
  *endp = p + strlen (p);
  *argnum1p = 0;
  *argnum2p = 0;
}


static string_list_ty *comment;

void
xgettext_comment_add (str)
     const char *str;
{
  if (comment == NULL)
    comment = string_list_alloc ();
  string_list_append (comment, str);
}

const char *
xgettext_comment (n)
     size_t n;
{
  if (comment == NULL || n >= comment->nitems)
    return NULL;
  return comment->item[n];
}

void
xgettext_comment_reset ()
{
  if (comment != NULL)
    {
      string_list_free (comment);
      comment = NULL;
    }
}



static FILE *
xgettext_open (fn, logical_file_name_p, real_file_name_p)
     const char *fn;
     char **logical_file_name_p;
     char **real_file_name_p;
{
  FILE *fp;
  char *new_name;
  char *logical_file_name;

  if (strcmp (fn, "-") == 0)
    {
      new_name = xstrdup (_("standard input"));
      logical_file_name = xstrdup (new_name);
      fp = stdin;
    }
  else if (IS_ABSOLUTE_PATH (fn))
    {
      new_name = xstrdup (fn);
      fp = fopen (fn, "r");
      if (fp == NULL)
	error (EXIT_FAILURE, errno, _("\
error while opening \"%s\" for reading"), fn);
      logical_file_name = xstrdup (new_name);
    }
  else
    {
      int j;

      for (j = 0; ; ++j)
	{
	  const char *dir = dir_list_nth (j);

	  if (dir == NULL)
	    error (EXIT_FAILURE, ENOENT, _("\
error while opening \"%s\" for reading"), fn);

	  new_name = concatenated_pathname (dir, fn, NULL);

	  fp = fopen (new_name, "r");
	  if (fp != NULL)
	    break;

	  if (errno != ENOENT)
	    error (EXIT_FAILURE, errno, _("\
error while opening \"%s\" for reading"), new_name);
	  free (new_name);
	}

      /* Note that the NEW_NAME variable contains the actual file name
	 and the logical file name is what is reported by xgettext.  In
	 this case NEW_NAME is set to the file which was found along the
	 directory search path, and LOGICAL_FILE_NAME is is set to the
	 file name which was searched for.  */
      logical_file_name = xstrdup (fn);
    }

  *logical_file_name_p = logical_file_name;
  *real_file_name_p = new_name;
  return fp;
}


static void
extract_from_file (file_name, extractor, mdlp)
     const char *file_name;
     extractor_func extractor;
     msgdomain_list_ty *mdlp;
{
  char *logical_file_name;
  char *real_file_name;
  FILE *fp = xgettext_open (file_name, &logical_file_name, &real_file_name);

  extractor (fp, real_file_name, logical_file_name, mdlp);

  if (fp != stdin)
    fclose (fp);
  free (logical_file_name);
  free (real_file_name);
}



/* Language dependent format string parser.
   NULL if the language has no notion of format strings.  */
static struct formatstring_parser *current_formatstring_parser;


message_ty *
remember_a_message (mlp, string, pos)
     message_list_ty *mlp;
     char *string;
     lex_pos_ty *pos;
{
  enum is_format is_format[NFORMATS];
  enum is_wrap do_wrap;
  char *msgid;
  message_ty *mp;
  char *msgstr;
  size_t i;

  msgid = string;

  /* See whether we shall exclude this message.  */
  if (exclude != NULL && message_list_search (exclude, msgid) != NULL)
    {
      /* Tell the lexer to reset its comment buffer, so that the next
	 message gets the correct comments.  */
      xgettext_comment_reset ();

      return NULL;
    }

  for (i = 0; i < NFORMATS; i++)
    is_format[i] = undecided;
  do_wrap = undecided;

  if (msgid[0] == '\0' && !xgettext_omit_header)
    {
      char buffer[21];

      error_with_progname = false;
      if (pos->line_number == (size_t)(-1))
	buffer[0] = '\0';
      else
	sprintf (buffer, ":%ld", (long) pos->line_number);
      multiline_warning (xasprintf ("%s%s: warning: ", pos->file_name, buffer),
			 xstrdup (_("\
Empty msgid.  It is reserved by GNU gettext:\n\
gettext(\"\") returns the header entry with\n\
meta information, not the empty string.\n")));
      error_with_progname = true;
    }

  /* See if we have seen this message before.  */
  mp = message_list_search (mlp, msgid);
  if (mp != NULL)
    {
      free (msgid);
      for (i = 0; i < NFORMATS; i++)
	is_format[i] = mp->is_format[i];
      do_wrap = mp->do_wrap;
    }
  else
    {
      static lex_pos_ty dummypos = { __FILE__, __LINE__ };

      /* Construct the msgstr from the prefix and suffix, otherwise use the
	 empty string.  */
      if (msgstr_prefix)
	{
	  msgstr = (char *) xmalloc (strlen (msgstr_prefix)
				     + strlen (msgid)
				     + strlen (msgstr_suffix) + 1);
	  stpcpy (stpcpy (stpcpy (msgstr, msgstr_prefix), msgid),
		  msgstr_suffix);
	}
      else
	msgstr = "";

      /* Allocate a new message and append the message to the list.  */
      mp = message_alloc (msgid, NULL, msgstr, strlen (msgstr) + 1, &dummypos);
      /* Do not free msgid.  */
      message_list_append (mlp, mp);
    }

  /* Ask the lexer for the comments it has seen.  Only do this for the
     first instance, otherwise there could be problems; especially if
     the same comment appears before each.  */
  if (!mp->comment_dot)
    {
      int j;
      bool add_all_remaining_comments;

      add_all_remaining_comments = add_all_comments;
      for (j = 0; ; ++j)
	{
	  const char *s = xgettext_comment (j);
	  const char *t;
	  if (s == NULL)
	    break;

	  /* To reduce the possibility of unwanted matches be do a two
	     step match: the line must contain `xgettext:' and one of
	     the possible format description strings.  */
	  if ((t = strstr (s, "xgettext:")) != NULL)
	    {
	      bool tmp_fuzzy;
	      enum is_format tmp_format[NFORMATS];
	      enum is_wrap tmp_wrap;
	      bool interesting;

	      t += strlen ("xgettext:");

	      po_parse_comment_special (t, &tmp_fuzzy, tmp_format, &tmp_wrap);

	      interesting = false;
	      for (i = 0; i < NFORMATS; i++)
		if (tmp_format[i] != undecided)
		  {
		    is_format[i] = tmp_format[i];
		    interesting = true;
		  }
	      if (tmp_wrap != undecided)
		{
		  do_wrap = tmp_wrap;
		  interesting = true;
		}

	      /* If the "xgettext:" marker was followed by an interesting
		 keyword, and we updated our is_format/do_wrap variables,
		 we don't print the comment as a #. comment.  */
	      if (interesting)
		continue;
	    }
	  /* When the comment tag is seen, it drags in not only the line
	     which it starts, but all remaining comment lines.  */
	  if (add_all_remaining_comments
	      || (add_all_remaining_comments =
		    (comment_tag != NULL
		     && strncmp (s, comment_tag, strlen (comment_tag)) == 0)))
	    message_comment_dot_append (mp, s);
	}
    }

  /* If it is not already decided, through programmer comments, whether the
     msgid is a format string, examine the msgid.  This is a heuristic.  */
  for (i = 0; i < NFORMATS; i++)
    {
      if (is_format[i] == undecided
	  && formatstring_parsers[i] == current_formatstring_parser)
	{
	  struct formatstring_parser *parser = formatstring_parsers[i];
	  void *descr = parser->parse (mp->msgid);

	  if (descr != NULL)
	    {
	      /* msgid is a valid format string.  We mark only those msgids
		 as format strings which contain at least one format directive
		 and thus are format strings with a high probability.  We
		 don't mark strings without directives as format strings,
		 because that would force the programmer to add
		 "xgettext: no-c-format" anywhere where a translator wishes
		 to use a percent sign.  So, the msgfmt checking will not be
		 perfect.  Oh well.  */
	      if (parser->get_number_of_directives (descr) > 0)
		is_format[i] = possible;

	      parser->free (descr);
	    }
	  else
	    /* msgid is not a valid format string.  */
	    is_format[i] = impossible;
	}
      mp->is_format[i] = is_format[i];
    }

  mp->do_wrap = do_wrap == no ? no : yes;	/* By default we wrap.  */

  /* Remember where we saw this msgid.  */
  if (line_comment)
    message_comment_filepos (mp, pos->file_name, pos->line_number);

  /* Tell the lexer to reset its comment buffer, so that the next
     message gets the correct comments.  */
  xgettext_comment_reset ();

  return mp;
}


void
remember_a_message_plural (mp, string, pos)
     message_ty *mp;
     char *string;
     lex_pos_ty *pos;
{
  char *msgid_plural;
  char *msgstr1;
  size_t msgstr1_len;
  char *msgstr;
  size_t i;

  msgid_plural = string;

  /* See if the message is already a plural message.  */
  if (mp->msgid_plural == NULL)
    {
      mp->msgid_plural = msgid_plural;

      /* Construct the first plural form from the prefix and suffix,
	 otherwise use the empty string.  The translator will have to
	 provide additional plural forms.  */
      if (msgstr_prefix)
	{
	  msgstr1 = (char *) xmalloc (strlen (msgstr_prefix)
				      + strlen (msgid_plural)
				      + strlen (msgstr_suffix) + 1);
	  stpcpy (stpcpy (stpcpy (msgstr1, msgstr_prefix), msgid_plural),
		  msgstr_suffix);
	}
      else
	msgstr1 = "";
      msgstr1_len = strlen (msgstr1) + 1;
      msgstr = (char *) xmalloc (mp->msgstr_len + msgstr1_len);
      memcpy (msgstr, mp->msgstr, mp->msgstr_len);
      memcpy (msgstr + mp->msgstr_len, msgstr1, msgstr1_len);
      mp->msgstr = msgstr;
      mp->msgstr_len = mp->msgstr_len + msgstr1_len;

      /* If it is not already decided, through programmer comments or
	 the msgid, whether the msgid is a format string, examine the
	 msgid_plural.  This is a heuristic.  */
      for (i = 0; i < NFORMATS; i++)
	if (formatstring_parsers[i] == current_formatstring_parser
	    && (mp->is_format[i] == undecided || mp->is_format[i] == possible))
	  {
	    struct formatstring_parser *parser = formatstring_parsers[i];
	    void *descr = parser->parse (mp->msgid_plural);

	    if (descr != NULL)
	      {
		/* Same heuristic as in remember_a_message.  */
		if (parser->get_number_of_directives (descr) > 0)
		  mp->is_format[i] = possible;

		parser->free (descr);
	      }
	    else
	      /* msgid_plural is not a valid format string.  */
	      mp->is_format[i] = impossible;
	  }
    }
  else
    free (msgid_plural);
}


static message_ty *
construct_header ()
{
  time_t now;
  char *timestring;
  message_ty *mp;
  char *msgstr;
  static lex_pos_ty pos = { __FILE__, __LINE__, };

  time (&now);
  timestring = po_strftime (&now);

  msgstr = xasprintf ("\
Project-Id-Version: PACKAGE VERSION\n\
POT-Creation-Date: %s\n\
PO-Revision-Date: YEAR-MO-DA HO:MI+ZONE\n\
Last-Translator: FULL NAME <EMAIL@ADDRESS>\n\
Language-Team: LANGUAGE <LL@li.org>\n\
MIME-Version: 1.0\n\
Content-Type: text/plain; charset=CHARSET\n\
Content-Transfer-Encoding: 8bit\n",
		      timestring);
  free (timestring);

  mp = message_alloc ("", NULL, msgstr, strlen (msgstr) + 1, &pos);

  message_comment_append (mp,
			  copyright_holder[0] != '\0'
			  ? xasprintf ("\
SOME DESCRIPTIVE TITLE.\n\
Copyright (C) YEAR %s\n\
This file is distributed under the same license as the PACKAGE package.\n\
FIRST AUTHOR <EMAIL@ADDRESS>, YEAR.\n",
				       copyright_holder)
			  : "\
SOME DESCRIPTIVE TITLE.\n\
This file is put in the public domain.\n\
FIRST AUTHOR <EMAIL@ADDRESS>, YEAR.\n");

  mp->is_fuzzy = true;

  return mp;
}

static void
finalize_header (mdlp)
     msgdomain_list_ty *mdlp;
{
  /* If the generated PO file has plural forms, add a Plural-Forms template
     to the constructed header.  */
  bool has_plural;
  size_t i, j;

  has_plural = false;
  for (i = 0; i < mdlp->nitems; i++)
    {
      message_list_ty *mlp = mdlp->item[i]->messages;

      for (j = 0; j < mlp->nitems; j++)
	{
	  message_ty *mp = mlp->item[j];

	  if (mp->msgid_plural != NULL)
	    {
	      has_plural = true;
	      break;
	    }
	}
      if (has_plural)
	break;
    }

  if (has_plural)
    {
      message_ty *header = message_list_search (mdlp->item[0]->messages, "");
      if (header != NULL
	  && strstr (header->msgstr, "Plural-Forms:") == NULL)
	{
	  size_t insertpos = strlen (header->msgstr);
	  const char *suffix;
	  size_t suffix_len;
	  char *new_msgstr;

	  suffix = "\nPlural-Forms: nplurals=INTEGER; plural=EXPRESSION;\n";
	  if (insertpos == 0 || header->msgstr[insertpos-1] == '\n')
	    suffix++;
	  suffix_len = strlen (suffix);
	  new_msgstr = (char *) xmalloc (header->msgstr_len + suffix_len);
	  memcpy (new_msgstr, header->msgstr, insertpos);
	  memcpy (new_msgstr + insertpos, suffix, suffix_len);
	  memcpy (new_msgstr + insertpos + suffix_len,
		  header->msgstr + insertpos,
		  header->msgstr_len - insertpos);
	  header->msgstr = new_msgstr;
	  header->msgstr_len = header->msgstr_len + suffix_len;
	}
    }
}


#define SIZEOF(a) (sizeof(a) / sizeof(a[0]))
#define ENDOF(a) ((a) + SIZEOF(a))


static extractor_func
language_to_extractor (name)
     const char *name;
{
  typedef struct table_ty table_ty;
  struct table_ty
  {
    const char *name;
    extractor_func func;
    struct formatstring_parser *formatstring_parser;
  };

  static table_ty table[] =
  {
    SCANNERS_C
    SCANNERS_PO
    SCANNERS_PYTHON
    SCANNERS_LISP
    SCANNERS_ELISP
    SCANNERS_LIBREP
    SCANNERS_JAVA
    SCANNERS_AWK
    SCANNERS_YCP
    SCANNERS_TCL
    SCANNERS_RST
    SCANNERS_GLADE
    /* Here will follow more languages and their scanners: perl, etc...
       Make sure new scanners honor the --exclude-file option.  */
  };

  table_ty *tp;

  for (tp = table; tp < ENDOF(table); ++tp)
    if (strcasecmp (name, tp->name) == 0)
      {
	/* XXX Ugly side effect.  */
	current_formatstring_parser = tp->formatstring_parser;

	return tp->func;
      }

  error (EXIT_FAILURE, 0, _("language `%s' unknown"), name);
  /* NOTREACHED */
  return NULL;
}


static const char *
extension_to_language (extension)
     const char *extension;
{
  typedef struct table_ty table_ty;
  struct table_ty
  {
    const char *extension;
    const char *language;
  };

  static table_ty table[] =
  {
    EXTENSIONS_C
    EXTENSIONS_PO
    EXTENSIONS_PYTHON
    EXTENSIONS_LISP
    EXTENSIONS_ELISP
    EXTENSIONS_LIBREP
    EXTENSIONS_JAVA
    EXTENSIONS_AWK
    EXTENSIONS_YCP
    EXTENSIONS_TCL
    EXTENSIONS_RST
    EXTENSIONS_GLADE
    /* Here will follow more file extensions: sh, pl ... */
  };

  table_ty *tp;

  for (tp = table; tp < ENDOF(table); ++tp)
    if (strcmp (extension, tp->extension) == 0)
      return tp->language;
  return NULL;
}
