/* Concatenates several translation catalogs.
   Copyright (C) 2001 Free Software Foundation, Inc.
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

#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#include "dir-list.h"
#include "error.h"
#include "xerror.h"
#include "progname.h"
#include "message.h"
#include "read-po.h"
#include "write-po.h"
#include "po-charset.h"
#include "msgl-iconv.h"
#include "system.h"
#include "libgettext.h"

#define _(str) gettext (str)


/* Force output of PO file even if empty.  */
static int force_po;

/* Target encoding.  */
static const char *to_code;

/* These variables control which messages are selected.  */
static int more_than = 0;
static int less_than = INT_MAX;

/* If true, use the first available translation.
   If false, merge all available translations into one and fuzzy it.  */
static int use_first;

/* Long options.  */
static const struct option long_options[] =
{
  { "add-location", no_argument, &line_comment, 1 },
  { "directory", required_argument, NULL, 'D' },
  { "escape", no_argument, NULL, 'E' },
  { "files-from", required_argument, NULL, 'f' },
  { "force-po", no_argument, &force_po, 1 },
  { "help", no_argument, NULL, 'h' },
  { "indent", no_argument, NULL, 'i' },
  { "no-escape", no_argument, NULL, 'e' },
  { "no-location", no_argument, &line_comment, 0 },
  { "output-file", required_argument, NULL, 'o' },
  { "sort-by-file", no_argument, NULL, 'F' },
  { "sort-output", no_argument, NULL, 's' },
  { "strict", no_argument, NULL, 'S' },
  { "to-code", required_argument, NULL, 't' },
  { "unique", no_argument, NULL, 'u' },
  { "use-first", no_argument, &use_first, 1 },
  { "version", no_argument, NULL, 'V' },
  { "width", required_argument, NULL, 'w', },
  { "more-than", required_argument, NULL, '>', },
  { "less-than", required_argument, NULL, '<', },
  { NULL, 0, NULL, 0 }
};


/* Prototypes for local functions.  */
static void usage PARAMS ((int status));
static string_list_ty *read_name_from_file PARAMS ((const char *file_name));
static bool is_message_selected PARAMS ((const message_ty *tmp));
static bool is_message_needed PARAMS ((const message_ty *tmp));
static bool is_message_first_needed PARAMS ((const message_ty *tmp));
static msgdomain_list_ty *
       catenate_msgdomain_list PARAMS ((string_list_ty *file_list,
					const char *to_code));


int
main (argc, argv)
     int argc;
     char **argv;
{
  int cnt;
  int optchar;
  bool do_help;
  bool do_version;
  char *output_file;
  const char *files_from;
  string_list_ty *file_list;
  msgdomain_list_ty *result;
  bool sort_by_msgid = false;
  bool sort_by_filepos = false;

  /* Set program name for messages.  */
  program_name = argv[0];
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
  files_from = NULL;

  while ((optchar = getopt_long (argc, argv, "<:>:D:eEf:Fhino:st:uVw:",
				 long_options, NULL)) != EOF)
    switch (optchar)
      {
      case '\0':		/* Long option.  */
	break;

      case '>':
	{
	  int value;
	  char *endp;
	  value = strtol (optarg, &endp, 10);
	  if (endp != optarg)
	    more_than = value;
	}
	break;

      case '<':
	{
	  int value;
	  char *endp;
	  value = strtol (optarg, &endp, 10);
	  if (endp != optarg)
	    less_than = value;
	}
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

      case 't':
	to_code = optarg;
	break;

      case 'u':
        less_than = 2;
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
	/* NOTREACHED */
      }

  /* Verify selected options.  */
  if (!line_comment && sort_by_filepos)
    error (EXIT_FAILURE, 0, _("%s and %s are mutually exclusive"),
	   "--no-location", "--sort-by-file");

  if (sort_by_msgid && sort_by_filepos)
    error (EXIT_FAILURE, 0, _("%s and %s are mutually exclusive"),
	   "--sort-output", "--sort-by-file");

  /* Version information requested.  */
  if (do_version)
    {
      printf ("%s (GNU %s) %s\n", basename (program_name), PACKAGE, VERSION);
      /* xgettext: no-wrap */
      printf (_("Copyright (C) %s Free Software Foundation, Inc.\n\
This is free software; see the source for copying conditions.  There is NO\n\
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\
"),
	      "2001");
      printf (_("Written by %s.\n"), "Bruno Haible");
      exit (EXIT_SUCCESS);
    }

  /* Help is requested.  */
  if (do_help)
    usage (EXIT_SUCCESS);

  /* Determine list of files we have to process.  */
  if (files_from != NULL)
    file_list = read_name_from_file (files_from);
  else
    file_list = string_list_alloc ();
  /* Append names from command line.  */
  for (cnt = optind; cnt < argc; ++cnt)
    string_list_append_unique (file_list, argv[cnt]);

  /* Check the message selection criteria for sanity.  */
  if (more_than >= less_than || less_than < 2)
    error (EXIT_FAILURE, 0,
           _("impossible selection criteria specified (%d < n < %d)"),
           more_than, less_than);

  /* Read input files, then filter, convert and merge messages.  */
  result = catenate_msgdomain_list (file_list, to_code);

  string_list_free (file_list);

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
Usage: %s [OPTION] [INPUTFILE]...\n\
"), program_name);
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Concatenates and merges the specified PO files.\n\
Find messages which are common to two or more of the specified PO files.\n\
By using the --more-than option, greater commonality may be requested\n\
before messages are printed.  Conversely, the --less-than option may be\n\
used to specify less commonality before messages are printed (i.e.\n\
--less-than=2 will only print the unique messages).  Translations,\n\
comments and extract comments will be cumulated, except that if --use-first\n\
is specified, they will be taken from the first PO file to define them.\n\
File positions from all PO files will be cumulated.\n\
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
  INPUTFILE ...                  input files\n\
  -f, --files-from=FILE          get list of input files from FILE\n\
  -D, --directory=DIRECTORY      add DIRECTORY to list for input files search\n\
If input file is -, standard input is read.\n\
"));
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Output file location:\n\
  -o, --output-file=FILE         write output to specified file\n\
The results are written to standard output if no output file is specified\n\
or if it is -.\n\
"));
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Message selection:\n\
  -<, --less-than=NUMBER         print messages with less than this many\n\
                                 definitions, defaults to infinite if not\n\
                                 set\n\
  ->, --more-than=NUMBER         print messages with more than this many\n\
                                 definitions, defaults to 0 if not set\n\
  -u, --unique                   shorthand for --less-than=2, requests\n\
                                 that only unique messages be printed\n\
"));
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Output details:\n\
  -t, --to-code=NAME             encoding for output\n\
      --use-first                use first available translation for each\n\
                                 message, don't merge several translations\n\
  -e, --no-escape                do not use C escapes in output (default)\n\
  -E, --escape                   use C escapes in output, no extended chars\n\
      --force-po                 write PO file even if empty\n\
  -i, --indent                   write the .po file using indented style\n\
      --no-location              do not write '#: filename:line' lines\n\
  -n, --add-location             generate '#: filename:line' lines (default)\n\
      --strict                   write out strict Uniforum conforming .po file\n\
  -w, --width=NUMBER             set output page width\n\
  -s, --sort-output              generate sorted output and remove duplicates\n\
  -F, --sort-by-file             sort output by file location\n\
"));
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Informative output:\n\
  -h, --help                     display this help and exit\n\
  -V, --version                  output version information and exit\n\
"));
      printf ("\n");
      fputs (_("Report bugs to <bug-gnu-utils@gnu.org>.\n"),
	     stdout);
    }

  exit (status);
}


/* Read list of files to process from file.  */
static string_list_ty *
read_name_from_file (file_name)
     const char *file_name;
{
  size_t line_len = 0;
  char *line_buf = NULL;
  FILE *fp;
  string_list_ty *result;

  if (strcmp (file_name, "-") == 0)
    fp = stdin;
  else
    {
      fp = fopen (file_name, "r");
      if (fp == NULL)
	error (EXIT_FAILURE, errno,
	       _("error while opening \"%s\" for reading"), file_name);
    }

  result = string_list_alloc ();

  while (!feof (fp))
    {
      /* Read next line from file.  */
      int len = getline (&line_buf, &line_len, fp);

      /* In case of an error leave loop.  */
      if (len < 0)
	break;

      /* Remove trailing '\n' and trailing whitespace.  */
      if (len > 0 && line_buf[len - 1] == '\n')
	line_buf[--len] = '\0';
      while (len > 0
	     && (line_buf[len - 1] == ' '
		 || line_buf[len - 1] == '\t'
		 || line_buf[len - 1] == '\r'))
	line_buf[--len] = '\0';

      /* Test if we have to ignore the line.  */
      if (*line_buf == '\0' || *line_buf == '#')
	continue;

      string_list_append_unique (result, line_buf);
    }

  /* Free buffer allocated through getline.  */
  if (line_buf != NULL)
    free (line_buf);

  /* Close input stream.  */
  if (fp != stdin)
    fclose (fp);

  return result;
}


static bool
is_message_selected (tmp)
     const message_ty *tmp;
{
  int used = (tmp->used >= 0 ? tmp->used : - tmp->used);

  return (tmp->msgid[0] == '\0') /* keep the header entry */
	 || (used > more_than && used < less_than);
}


static bool
is_message_needed (mp)
     const message_ty *mp;
{
  if ((mp->msgid[0] != '\0' && mp->is_fuzzy) || mp->msgstr[0] == '\0')
    /* Weak translation.  Needed if there are only weak translations.  */
    return mp->tmp->used < 0 && is_message_selected (mp->tmp);
  else
    /* Good translation.  */
    return is_message_selected (mp->tmp);
}


/* The use_first logic.  */
static bool
is_message_first_needed (mp)
     const message_ty *mp;
{
  if (mp->tmp->obsolete && is_message_needed (mp))
    {
      mp->tmp->obsolete = false;
      return true;
    }
  else
    return false;
}


static msgdomain_list_ty *
catenate_msgdomain_list (file_list, to_code)
     string_list_ty *file_list;
     const char *to_code;
{
  const char * const *files = file_list->item;
  size_t nfiles = file_list->nitems;
  msgdomain_list_ty **mdlps;
  const char ***canon_charsets;
  const char ***identifications;
  msgdomain_list_ty *total_mdlp;
  const char *canon_to_code;
  size_t n, j, k;

  /* Read input files.  */
  mdlps =
    (msgdomain_list_ty **) xmalloc (nfiles * sizeof (msgdomain_list_ty *));
  for (n = 0; n < nfiles; n++)
    mdlps[n] = read_po_file (files[n]);

  /* Determine the canonical name of each input file's encoding.  */
  canon_charsets = (const char ***) xmalloc (nfiles * sizeof (const char **));
  for (n = 0; n < nfiles; n++)
    {
      msgdomain_list_ty *mdlp = mdlps[n];
      size_t k;

      canon_charsets[n] =
	(const char **) xmalloc (mdlp->nitems * sizeof (const char *));
      for (k = 0; k < mdlp->nitems; k++)
	{
	  message_list_ty *mlp = mdlp->item[k]->messages;
	  const char *canon_from_code = NULL;

	  if (mlp->nitems > 0)
	    {
	      for (j = 0; j < mlp->nitems; j++)
		if (mlp->item[j]->msgid[0] == '\0' && !mlp->item[j]->obsolete)
		  {
		    const char *header = mlp->item[j]->msgstr;

		    if (header != NULL)
		      {
			const char *charsetstr = strstr (header, "charset=");

			if (charsetstr != NULL)
			  {
			    size_t len;
			    char *charset;
			    const char *canon_charset;

			    charsetstr += strlen ("charset=");
			    len = strcspn (charsetstr, " \t\n");
			    charset = (char *) alloca (len + 1);
			    memcpy (charset, charsetstr, len);
			    charset[len] = '\0';

			    canon_charset = po_charset_canonicalize (charset);
			    if (canon_charset == NULL)
			      error (EXIT_FAILURE, 0,
				     _("\
present charset \"%s\" is not a portable encoding name"),
				     charset);

			    if (canon_from_code == NULL)
			      canon_from_code = canon_charset;
			    else if (canon_from_code != canon_charset)
			      error (EXIT_FAILURE, 0,
				     _("\
two different charsets \"%s\" and \"%s\" in input file"),
				     canon_from_code, canon_charset);
			  }
		      }
		  }
	      if (canon_from_code == NULL)
		{
		  if (k == 0)
		    error (EXIT_FAILURE, 0, _("\
input file `%s' doesn't contain a header entry with a charset specification"),
			   files[n]);
		  else
		    error (EXIT_FAILURE, 0, _("\
domain \"%s\" in input file `%s' doesn't contain a header entry with a charset specification"),
			   mdlp->item[k]->domain, files[n]);
		}
	    }
	  canon_charsets[n][k] = canon_from_code;
	}
    }

  /* Determine textual identifications of each file/domain combination.  */
  identifications = (const char ***) xmalloc (nfiles * sizeof (const char **));
  for (n = 0; n < nfiles; n++)
    {
      const char *filename = basename (files[n]);
      msgdomain_list_ty *mdlp = mdlps[n];
      size_t k;

      identifications[n] =
	(const char **) xmalloc (mdlp->nitems * sizeof (const char *));
      for (k = 0; k < mdlp->nitems; k++)
	{
	  const char *domain = mdlp->item[k]->domain;
	  message_list_ty *mlp = mdlp->item[k]->messages;
	  char *project_id = NULL;

	  for (j = 0; j < mlp->nitems; j++)
	    if (mlp->item[j]->msgid[0] == '\0' && !mlp->item[j]->obsolete)
	      {
		const char *header = mlp->item[j]->msgstr;

		if (header != NULL)
		  {
		    const char *cp = strstr (header, "Project-Id-Version:");

		    if (cp != NULL)
		      {
			const char *endp;

			cp += sizeof ("Project-Id-Version:") - 1;

			endp = strchr (cp, '\n');
			if (endp == NULL)
			  endp = cp + strlen (cp);

			while (cp < endp && *cp == ' ')
			  cp++;

			if (cp < endp)
			  {
			    size_t len = endp - cp;
			    project_id = (char *) xmalloc (len + 1);
			    memcpy (project_id, cp, len);
			    project_id[len] = '\0';
			  }
			break;
		      }
		  }
	      }

	  identifications[n][k] =
	    (project_id != NULL
	     ? (k > 0 ? xasprintf ("%s:%s (%s)", filename, domain, project_id)
		      : xasprintf ("%s (%s)", filename, project_id))
	     : (k > 0 ? xasprintf ("%s:%s", filename, domain)
		      : xasprintf ("%s", filename)));
	}
    }

  /* Create list of resulting messages, but don't fill it.  Only count
     the number of translations for each message.
     If for a message, there is at least one non-fuzzy, non-empty translation,
     use only the non-fuzzy, non-empty translations.  Otherwise use the
     fuzzy or empty translations as well.  */
  total_mdlp = msgdomain_list_alloc ();
  for (n = 0; n < nfiles; n++)
    {
      msgdomain_list_ty *mdlp = mdlps[n];

      for (k = 0; k < mdlp->nitems; k++)
	{
	  const char *domain = mdlp->item[k]->domain;
	  message_list_ty *mlp = mdlp->item[k]->messages;
	  message_list_ty *total_mlp;

	  total_mlp = msgdomain_list_sublist (total_mdlp, domain, 1);

	  for (j = 0; j < mlp->nitems; j++)
	    {
	      message_ty *mp = mlp->item[j];
	      message_ty *tmp;

	      tmp = message_list_search (total_mlp, mp->msgid);
	      if (tmp == NULL)
		{
		  tmp = message_alloc (mp->msgid, mp->msgid_plural, NULL, 0,
				       &mp->pos);
		  tmp->is_fuzzy = true; /* may be set to false later */
		  tmp->is_c_format = undecided; /* may be set to yes/no later */
		  tmp->do_wrap = yes; /* may be set to no later */
		  tmp->obsolete = true; /* may be set to false later */
		  tmp->alternative_count = 0;
		  tmp->alternative = NULL;
		  message_list_append (total_mlp, tmp);
		}

	      if ((mp->msgid[0] != '\0' && mp->is_fuzzy)
		  || mp->msgstr[0] == '\0')
		/* Weak translation.  Counted as negative tmp->used.  */
		{
		  if (tmp->used <= 0)
		    tmp->used--;
		}
	      else
		/* Good translation.  Counted as positive tmp->used.  */
		{
		  if (tmp->used < 0)
		    tmp->used = 0;
		  tmp->used++;
		}
	      mp->tmp = tmp;
	    }
	}
    }

  /* Remove messages that are not used and need not be converted.  */
  for (n = 0; n < nfiles; n++)
    {
      msgdomain_list_ty *mdlp = mdlps[n];

      for (k = 0; k < mdlp->nitems; k++)
	{
	  message_list_ty *mlp = mdlp->item[k]->messages;

	  message_list_remove_if_not (mlp,
				      use_first
				      ? is_message_first_needed
				      : is_message_needed);

	  /* If no messages are remaining, drop the charset.  */
	  if (mlp->nitems == 0)
	    canon_charsets[n][k] = NULL;
	}
    }
  for (k = 0; k < total_mdlp->nitems; k++)
    {
      message_list_ty *mlp = total_mdlp->item[k]->messages;

      message_list_remove_if_not (mlp, is_message_selected);
    }

  /* Determine the target encoding for the remaining messages.  */
  if (to_code != NULL)
    {
      /* Canonicalize target encoding.  */
      canon_to_code = po_charset_canonicalize (to_code);
      if (canon_to_code == NULL)
	error (EXIT_FAILURE, 0,
	       _("target charset \"%s\" is not a portable encoding name."),
	       to_code);
    }
  else
    {
      /* No target encoding was specified.  Test whether the messages are
	 all in a single encoding.  If so, conversion is not needed.  */
      const char *first = NULL;
      const char *second = NULL;
      bool with_UTF8 = false;

      for (n = 0; n < nfiles; n++)
	{
	  msgdomain_list_ty *mdlp = mdlps[n];

	  for (k = 0; k < mdlp->nitems; k++)
	    if (canon_charsets[n][k] != NULL)
	      {
		if (first == NULL)
		  first = canon_charsets[n][k];
		else if (canon_charsets[n][k] != first && second == NULL)
		  second = canon_charsets[n][k];

		if (strcmp (canon_charsets[n][k], "UTF-8") == 0)
		  with_UTF8 = true;
	      }
	}

      if (second != NULL)
	{
	  /* A conversion is needed.  Warn the user since he hasn't asked
	     for it and might be surprised.  */
	  if (with_UTF8)
	    multiline_warning (xasprintf (_("warning: ")),
			       xasprintf (_("\
Input files contain messages in different encodings, UTF-8 among others.\n\
Converting the output to UTF-8.\n\
")));
	  else
	    multiline_warning (xasprintf (_("warning: ")),
			       xasprintf (_("\
Input files contain messages in different encodings, %s and %s among others.\n\
Converting the output to UTF-8.\n\
To select a different output encoding, use the --to-code option.\n\
"), first, second));
	  canon_to_code = po_charset_canonicalize ("UTF-8");
	}
      else
	{
	  /* No conversion needed.  */
	  canon_to_code = NULL;
	}
    }

  /* Now convert the remaining messages to to_code.  */
  if (canon_to_code != NULL)
    for (n = 0; n < nfiles; n++)
      {
	msgdomain_list_ty *mdlp = mdlps[n];

	for (k = 0; k < mdlp->nitems; k++)
	  if (canon_charsets[n][k] != NULL)
	    iconv_message_list (mdlp->item[k]->messages, canon_to_code);
      }

  /* Fill the resulting messages.  */
  for (n = 0; n < nfiles; n++)
    {
      msgdomain_list_ty *mdlp = mdlps[n];

      for (k = 0; k < mdlp->nitems; k++)
	{
	  message_list_ty *mlp = mdlp->item[k]->messages;

	  for (j = 0; j < mlp->nitems; j++)
	    {
	      message_ty *mp = mlp->item[j];
	      message_ty *tmp = mp->tmp;
	      size_t i;

	      /* No need to discard unneeded weak translations here;
		 they have already been filtered out above.  */
	      if (use_first || tmp->used == 1 || tmp->used == -1)
		{
		  /* Copy mp, as only message, into tmp.  */
		  tmp->msgstr = mp->msgstr;
		  tmp->msgstr_len = mp->msgstr_len;
		  tmp->pos = mp->pos;
		  if (mp->comment)
		    for (i = 0; i < mp->comment->nitems; i++)
		      message_comment_append (tmp, mp->comment->item[i]);
		  if (mp->comment_dot)
		    for (i = 0; i < mp->comment_dot->nitems; i++)
		      message_comment_dot_append (tmp,
						  mp->comment_dot->item[i]);
		  for (i = 0; i < mp->filepos_count; i++)
		    message_comment_filepos (tmp, mp->filepos[i].file_name,
					     mp->filepos[i].line_number);
		  tmp->is_fuzzy = mp->is_fuzzy;
		  tmp->is_c_format = mp->is_c_format;
		  tmp->do_wrap = mp->do_wrap;
		  tmp->obsolete = mp->obsolete;
		}
	      else
		{
		  /* Copy mp, among others, into tmp.  */
		  char *id = xasprintf ("#-#-#-#-#  %s  #-#-#-#-#",
					identifications[n][k]);
		  size_t nbytes;

		  if (tmp->alternative_count == 0)
		    tmp->pos = mp->pos;

		  i = tmp->alternative_count;
		  nbytes = (i + 1) * sizeof (struct altstr);
		  tmp->alternative = xrealloc (tmp->alternative, nbytes);
		  tmp->alternative[i].msgstr = mp->msgstr;
		  tmp->alternative[i].msgstr_len = mp->msgstr_len;
		  tmp->alternative[i].msgstr_end =
		    tmp->alternative[i].msgstr + tmp->alternative[i].msgstr_len;
		  tmp->alternative[i].id = id;
		  tmp->alternative_count = i + 1;

		  if (mp->comment)
		    {
		      message_comment_append (tmp, id);
		      for (i = 0; i < mp->comment->nitems; i++)
			message_comment_append (tmp, mp->comment->item[i]);
		    }
		  if (mp->comment_dot)
		    {
		      message_comment_dot_append (tmp, id);
		      for (i = 0; i < mp->comment_dot->nitems; i++)
			message_comment_dot_append (tmp,
						    mp->comment_dot->item[i]);
		    }
		  for (i = 0; i < mp->filepos_count; i++)
		    message_comment_filepos (tmp, mp->filepos[i].file_name,
					     mp->filepos[i].line_number);
		  if (!mp->is_fuzzy)
		    tmp->is_fuzzy = false;
		  if (mp->is_c_format == yes)
		    tmp->is_c_format = yes;
		  else if (mp->is_c_format == no
			   && tmp->is_c_format == undecided)
		    tmp->is_c_format = no;
		  if (mp->do_wrap == no)
		    tmp->do_wrap = no;
		  if (!mp->obsolete)
		    tmp->obsolete = false;
		}
	    }
	}
    }
  for (k = 0; k < total_mdlp->nitems; k++)
    {
      message_list_ty *mlp = total_mdlp->item[k]->messages;

      for (j = 0; j < mlp->nitems; j++)
	{
	  message_ty *tmp = mlp->item[j];

	  if (tmp->alternative_count > 0)
	    {
	      /* Test whether all alternative translations are equal.  */
	      struct altstr *first = &tmp->alternative[0];
	      size_t i;

	      for (i = 0; i < tmp->alternative_count; i++)
		if (!(tmp->alternative[i].msgstr_len == first->msgstr_len
		      && memcmp (tmp->alternative[i].msgstr, first->msgstr,
				 first->msgstr_len) == 0))
		  break;

	      if (i == tmp->alternative_count)
		{
		  /* All alternatives are equal.  */
		  tmp->msgstr = first->msgstr;
		  tmp->msgstr_len = first->msgstr_len;
		}
	      else
		{
		  /* Concatenate the alternative msgstrs into a single one,
		     separated by markers.  */
		  size_t len;
		  const char *p;
		  const char *p_end;
		  char *new_msgstr;
		  char *np;

		  len = 0;
		  for (i = 0; i < tmp->alternative_count; i++)
		    {
		      size_t id_len = strlen (tmp->alternative[i].id);

		      len += tmp->alternative[i].msgstr_len;

		      p = tmp->alternative[i].msgstr;
		      p_end = tmp->alternative[i].msgstr_end;
		      for (; p < p_end; p += strlen (p) + 1)
		        len += id_len + 2;
		    }

		  new_msgstr = (char *) xmalloc (len);
		  np = new_msgstr;
		  for (;;)
		    {
		      /* Test whether there's one more plural form to
			 process.  */
		      for (i = 0; i < tmp->alternative_count; i++)
			if (tmp->alternative[i].msgstr
			    < tmp->alternative[i].msgstr_end)
			  break;
		      if (i == tmp->alternative_count)
			break;

		      /* Process next plural form.  */
		      for (i = 0; i < tmp->alternative_count; i++)
			if (tmp->alternative[i].msgstr
			    < tmp->alternative[i].msgstr_end)
			  {
			    if (np > new_msgstr && np[-1] != '\0'
				&& np[-1] != '\n')
			      *np++ = '\n';

			    len = strlen (tmp->alternative[i].id);
			    memcpy (np, tmp->alternative[i].id, len);
			    np += len;
			    *np++ = '\n';

			    len = strlen (tmp->alternative[i].msgstr);
			    memcpy (np, tmp->alternative[i].msgstr, len);
			    np += len;
			    tmp->alternative[i].msgstr += len + 1;
			  }

		      /* Plural forms are separated by NUL bytes.  */
		      *np++ = '\0';
		    }
		  tmp->msgstr = new_msgstr;
		  tmp->msgstr_len = np - new_msgstr;

		  tmp->is_fuzzy = true;
		}
	    }
	}
    }

  return total_mdlp;
}
