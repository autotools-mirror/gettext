/* Converts Uniforum style .po files to binary .mo files
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

#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include "dir-list.h"
#include "error.h"
#include "hash.h"
#include "progname.h"
#include "basename.h"
#include "xerror.h"
#include "getline.h"
#include "format.h"
#include "xmalloc.h"
#include "plural-exp.h"
#include "strstr.h"
#include "system.h"
#include "msgfmt.h"
#include "write-mo.h"
#include "write-java.h"

#include "libgettext.h"
#include "message.h"
#include "po.h"

#define _(str) gettext (str)

#define SIZEOF(a) (sizeof(a) / sizeof(a[0]))

/* This structure defines a derived class of the po_ty class.  (See
   po.h for an explanation.)  */
typedef struct msgfmt_class_ty msgfmt_class_ty;
struct msgfmt_class_ty
{
  /* inherited instance variables, etc */
  PO_BASE_TY

  bool is_fuzzy;
  enum is_format is_format[NFORMATS];
  enum is_wrap do_wrap;

  bool has_header_entry;
};

/* Contains exit status for case in which no premature exit occurs.  */
static int exit_status;

/* If true include even fuzzy translations in output file.  */
static bool include_all = false;

/* Specifies name of the output file.  */
static const char *output_file_name;

/* Java mode output file specification.  */
static bool java_mode;
static bool assume_java2;
static const char *java_resource_name;
static const char *java_locale_name;
static const char *java_class_directory;

/* We may have more than one input file.  Domains with same names in
   different files have to merged.  So we need a list of tables for
   each output file.  */
struct msg_domain
{
  /* List for mapping message IDs to message strings.  */
  message_list_ty *mlp;
  /* Table for mapping message IDs to message strings.  */
  hash_table symbol_tab;
  /* Name of domain these ID/String pairs are part of.  */
  const char *domain_name;
  /* Output file name.  */
  const char *file_name;
  /* Link to the next domain.  */
  struct msg_domain *next;
};
static struct msg_domain *domain_list;
static struct msg_domain *current_domain;

/* Be more verbose.  Use only 'fprintf' and 'multiline_warning' but not
   'error' or 'multiline_error' to emit verbosity messages, because 'error'
   and 'multiline_error' during PO file parsing cause the program to exit
   with EXIT_FAILURE.  See function lex_end().  */
bool verbose = false;

/* If true check strings according to format string rules for the
   language.  */
static bool check_format_strings = false;

/* If true check the header entry is present and complete.  */
static bool check_header = false;

/* Check that domain directives can be satisfied.  */
static bool check_domain = false;

/* Check that msgfmt's behaviour is semantically compatible with
   X/Open msgfmt or XView msgfmt.  */
static bool check_compatibility = false;

/* If true, consider that strings containing an '&' are menu items and
   the '&' designates a keyboard accelerator, and verify that the translations
   also have a keyboard accelerator.  */
static bool check_accelerators = false;
static char accelerator_char = '&';

/* Counters for statistics on translations for the processed files.  */
static int msgs_translated;
static int msgs_untranslated;
static int msgs_fuzzy;

/* If not zero print statistics about translation at the end.  */
static int do_statistics;

/* Long options.  */
static const struct option long_options[] =
{
  { "alignment", required_argument, NULL, 'a' },
  { "check", no_argument, NULL, 'c' },
  { "check-accelerators", optional_argument, NULL, CHAR_MAX + 1 },
  { "check-compatibility", no_argument, NULL, 'C' },
  { "check-domain", no_argument, NULL, CHAR_MAX + 2 },
  { "check-format", no_argument, NULL, CHAR_MAX + 3 },
  { "check-header", no_argument, NULL, CHAR_MAX + 4 },
  { "directory", required_argument, NULL, 'D' },
  { "help", no_argument, NULL, 'h' },
  { "java", no_argument, NULL, 'j' },
  { "java2", no_argument, NULL, CHAR_MAX + 5 },
  { "locale", required_argument, NULL, 'l' },
  { "no-hash", no_argument, NULL, CHAR_MAX + 6 },
  { "output-file", required_argument, NULL, 'o' },
  { "resource", required_argument, NULL, 'r' },
  { "statistics", no_argument, &do_statistics, 1 },
  { "strict", no_argument, NULL, 'S' },
  { "use-fuzzy", no_argument, NULL, 'f' },
  { "verbose", no_argument, NULL, 'v' },
  { "version", no_argument, NULL, 'V' },
  { NULL, 0, NULL, 0 }
};


/* Prototypes for local functions.  Needed to ensure compiler checking of
   function argument counts despite of K&R C function definition syntax.  */
static void usage PARAMS ((int status))
#if defined __GNUC__ && ((__GNUC__ == 2 && __GNUC_MINOR__ >= 5) || __GNUC__ > 2)
	__attribute__ ((noreturn))
#endif
;
static const char *add_mo_suffix PARAMS ((const char *));
static struct msg_domain *new_domain PARAMS ((const char *name,
					      const char *file_name));
#if HAVE_SIGINFO
static void sigfpe_handler PARAMS ((int sig, siginfo_t *sip, void *scp));
#else
static void sigfpe_handler PARAMS ((int sig));
#endif
static void install_sigfpe_handler PARAMS ((void));
static void uninstall_sigfpe_handler PARAMS ((void));
static void check_plural_eval PARAMS ((struct expression *plural_expr,
				       unsigned long nplurals_value,
				       const lex_pos_ty *header_pos));
static void check_plural PARAMS ((message_list_ty *mlp));
static void check_pair PARAMS ((const char *msgid, const lex_pos_ty *msgid_pos,
				const char *msgid_plural,
				const char *msgstr, size_t msgstr_len,
				const lex_pos_ty *msgstr_pos,
				enum is_format is_format[NFORMATS]));
static void format_constructor PARAMS ((po_ty *that));
static void format_debrief PARAMS ((po_ty *));
static void format_directive_domain PARAMS ((po_ty *pop, char *name));
static void format_directive_message PARAMS ((po_ty *pop, char *msgid,
					      lex_pos_ty *msgid_pos,
					      char *msgid_plural,
					      char *msgstr, size_t msgstr_len,
					      lex_pos_ty *msgstr_pos,
					      bool obsolete));
static void format_comment_special PARAMS ((po_ty *pop, const char *s));
static void read_po_file PARAMS ((char *filename));


int
main (argc, argv)
     int argc;
     char *argv[];
{
  int opt;
  bool do_help = false;
  bool do_version = false;
  bool strict_uniforum = false;
  struct msg_domain *domain;

  /* Set default value for global variables.  */
  alignment = DEFAULT_OUTPUT_ALIGNMENT;

  /* Set program name for messages.  */
  set_program_name (argv[0]);
  error_print_progname = maybe_print_progname;
  error_one_per_line = 1;
  exit_status = EXIT_SUCCESS;

#ifdef HAVE_SETLOCALE
  /* Set locale via LC_ALL.  */
  setlocale (LC_ALL, "");
#endif

  /* Set the text message domain.  */
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  while ((opt = getopt_long (argc, argv, "a:cCd:D:fhjl:o:r:vV", long_options,
			     NULL))
	 != EOF)
    switch (opt)
      {
      case '\0':		/* Long option.  */
	break;
      case 'a':
	{
	  char *endp;
	  size_t new_align = strtoul (optarg, &endp, 0);

	  if (endp != optarg)
	    alignment = new_align;
	}
	break;
      case 'c':
	check_domain = true;
	check_format_strings = true;
	check_header = true;
	break;
      case 'C':
	check_compatibility = true;
	break;
      case 'd':
	java_class_directory = optarg;
	break;
      case 'D':
	dir_list_append (optarg);
	break;
      case 'f':
	include_all = true;
	break;
      case 'h':
	do_help = true;
	break;
      case 'j':
	java_mode = true;
	break;
      case 'l':
	java_locale_name = optarg;
	break;
      case 'o':
	output_file_name = optarg;
	break;
      case 'r':
	java_resource_name = optarg;
	break;
      case 'S':
	strict_uniforum = true;
	break;
      case 'v':
	verbose = true;
	break;
      case 'V':
	do_version = true;
	break;
      case CHAR_MAX + 1:
	check_accelerators = true;
	if (optarg != NULL)
	  {
	    if (optarg[0] != '\0' && ispunct ((unsigned char) optarg[0])
		&& optarg[1] == '\0')
	      accelerator_char = optarg[0];
	    else
	      error (EXIT_FAILURE, 0,
		     _("the argument to %s should be a single punctuation character"),
		     "--check-accelerators");
	  }
	break;
      case CHAR_MAX + 2:
	check_domain = true;
	break;
      case CHAR_MAX + 3:
	check_format_strings = true;
	break;
      case CHAR_MAX + 4:
	check_header = true;
	break;
      case CHAR_MAX + 5:
	java_mode = true;
	assume_java2 = true;
	break;
      case CHAR_MAX + 6:
	no_hash_table = true;
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

  /* Test whether we have a .po file name as argument.  */
  if (optind >= argc)
    {
      error (EXIT_SUCCESS, 0, _("no input file given"));
      usage (EXIT_FAILURE);
    }

  /* Check for contradicting options.  */
  if (java_mode)
    {
      if (output_file_name != NULL)
	{
	  error (EXIT_FAILURE, 0, _("%s and %s are mutually exclusive"),
		 "--java-mode", "--output-file");
	}
      if (java_class_directory == NULL)
	{
	  error (EXIT_SUCCESS, 0,
		 _("%s requires a \"-d directory\" specification"),
		 "--java-mode");
	  usage (EXIT_FAILURE);
	}
    }
  else
    {
      if (java_resource_name != NULL)
	{
	  error (EXIT_SUCCESS, 0, _("%s is only valid with %s"),
		 "--resource", "--java-mode");
	  usage (EXIT_FAILURE);
	}
      if (java_locale_name != NULL)
	{
	  error (EXIT_SUCCESS, 0, _("%s is only valid with %s"),
		 "--locale", "--java-mode");
	  usage (EXIT_FAILURE);
	}
      if (java_class_directory != NULL)
	{
	  error (EXIT_SUCCESS, 0, _("%s is only valid with %s"),
		 "-d", "--java-mode");
	  usage (EXIT_FAILURE);
	}
    }

  /* The -o option determines the name of the domain and therefore
     the output file.  */
  if (output_file_name != NULL)
    current_domain =
      new_domain (output_file_name,
		  strict_uniforum ? add_mo_suffix (output_file_name)
				  : output_file_name);

  /* Prepare PO file reader.  We need to see the comments because inexact
     translations must be reported.  */
  po_lex_pass_comments (true);

  /* Process all given .po files.  */
  while (argc > optind)
    {
      /* Remember that we currently have not specified any domain.  This
	 is of course not true when we saw the -o option.  */
      if (output_file_name == NULL)
	current_domain = NULL;

      /* And process the input file.  */
      read_po_file (argv[optind]);

      ++optind;
    }

  /* Check the plural expression is present if needed and has valid syntax.  */
  if (check_header)
    for (domain = domain_list; domain != NULL; domain = domain->next)
      check_plural (domain->mlp);

  /* Now write out all domains.  */
  for (domain = domain_list; domain != NULL; domain = domain->next)
    {
      if (java_mode)
	{
	  if (msgdomain_write_java (domain->mlp, java_resource_name,
				    java_locale_name, java_class_directory,
				    assume_java2))
	    exit_status = EXIT_FAILURE;
	}
      else
	{
	  if (msgdomain_write_mo (domain->mlp, domain->domain_name,
				  domain->file_name))
	    exit_status = EXIT_FAILURE;
	}

      /* List is not used anymore.  */
      message_list_free (domain->mlp);
      /* Hashing table is not used anymore.  */
      delete_hash (&domain->symbol_tab);
    }

  /* Print statistics if requested.  */
  if (verbose || do_statistics)
    {
      fprintf (stderr,
	       ngettext ("%d translated message", "%d translated messages",
			 msgs_translated),
	       msgs_translated);
      if (msgs_fuzzy > 0)
	fprintf (stderr,
		 ngettext (", %d fuzzy translation", ", %d fuzzy translations",
			   msgs_fuzzy),
		 msgs_fuzzy);
      if (msgs_untranslated > 0)
	fprintf (stderr,
		 ngettext (", %d untranslated message",
			   ", %d untranslated messages",
			   msgs_untranslated),
		 msgs_untranslated);
      fputs (".\n", stderr);
    }

  exit (exit_status);
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
Usage: %s [OPTION] filename.po ...\n\
"), program_name);
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Generate binary message catalog from textual translation description.\n\
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
  filename.po ...             input files\n\
  -D, --directory=DIRECTORY   add DIRECTORY to list for input files search\n\
If input file is -, standard input is read.\n\
"));
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Operation mode:\n\
  -j, --java                  Java mode: generate a Java ResourceBundle class\n\
      --java2                 like --java, and assume Java2 (JDK 1.2 or higher)\n\
"));
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Output file location:\n\
  -o, --output-file=FILE      write output to specified file\n\
      --strict                enable strict Uniforum mode\n\
If output file is -, output is written to standard output.\n\
"));
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Output file location in Java mode:\n\
  -r, --resource=RESOURCE     resource name\n\
  -l, --locale=LOCALE         locale name, either language or language_COUNTRY\n\
  -d DIRECTORY                base directory of classes directory hierarchy\n\
The class name is determined by appending the locale name to the resource name,\n\
separated with an underscore.  The -d option is mandatory.  The class is\n\
written under the specified directory.\n\
"));
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Input file interpretation:\n\
  -c, --check                 perform all the checks implied by\n\
                                --check-format, --check-header, --check-domain\n\
      --check-format          check language dependent format strings\n\
      --check-header          verify presence and contents of the header entry\n\
      --check-domain          check for conflicts between domain directives\n\
                                and the --output-file option\n\
  -C, --check-compatibility   check that GNU msgfmt behaves like X/Open msgfmt\n\
      --check-accelerators[=CHAR]  check presence of keyboard accelerators for\n\
                                menu items\n\
  -f, --use-fuzzy             use fuzzy entries in output\n\
"));
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Output details:\n\
  -a, --alignment=NUMBER      align strings to NUMBER bytes (default: %d)\n\
      --no-hash               binary file will not include the hash table\n\
"), DEFAULT_OUTPUT_ALIGNMENT);
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Informative output:\n\
  -h, --help                  display this help and exit\n\
  -V, --version               output version information and exit\n\
      --statistics            print statistics about translations\n\
  -v, --verbose               increase verbosity level\n\
"));
      printf ("\n");
      fputs (_("Report bugs to <bug-gnu-gettext@gnu.org>.\n"), stdout);
    }

  exit (status);
}


static const char *
add_mo_suffix (fname)
     const char *fname;
{
  size_t len;
  char *result;

  len = strlen (fname);
  if (len > 3 && memcmp (fname + len - 3, ".mo", 3) == 0)
    return fname;
  if (len > 4 && memcmp (fname + len - 4, ".gmo", 4) == 0)
    return fname;
  result = (char *) xmalloc (len + 4);
  stpcpy (stpcpy (result, fname), ".mo");
  return result;
}


static struct msg_domain *
new_domain (name, file_name)
     const char *name;
     const char *file_name;
{
  struct msg_domain **p_dom = &domain_list;

  while (*p_dom != NULL && strcmp (name, (*p_dom)->domain_name) != 0)
    p_dom = &(*p_dom)->next;

  if (*p_dom == NULL)
    {
      struct msg_domain *domain;

      domain = (struct msg_domain *) xmalloc (sizeof (struct msg_domain));
      domain->mlp = message_list_alloc ();
      if (init_hash (&domain->symbol_tab, 100) != 0)
	error (EXIT_FAILURE, errno, _("while creating hash table"));
      domain->domain_name = name;
      domain->file_name = file_name;
      domain->next = NULL;
      *p_dom = domain;
    }

  return *p_dom;
}


static sigjmp_buf sigfpe_exit;

#if HAVE_SIGINFO

static int sigfpe_code;

/* Signal handler called in case of arithmetic exception (e.g. division
   by zero) during plural_eval.  */
static void
sigfpe_handler (sig, sip, scp)
     int sig;
     siginfo_t *sip;
     void *scp;
{
  sigfpe_code = sip->si_code;
  siglongjmp (sigfpe_exit, 1);
}

#else

/* Signal handler called in case of arithmetic exception (e.g. division
   by zero) during plural_eval.  */
static void
sigfpe_handler (sig)
     int sig;
{
  siglongjmp (sigfpe_exit, 1);
}

#endif

static void
install_sigfpe_handler ()
{
#if HAVE_SIGINFO
  struct sigaction action;
  action.sa_sigaction = sigfpe_handler;
  action.sa_flags = SA_SIGINFO;
  sigemptyset (&action.sa_mask);
  sigaction (SIGFPE, &action, (struct sigaction *) NULL);
#else
  signal (SIGFPE, sigfpe_handler);
#endif
}

static void
uninstall_sigfpe_handler ()
{
#if HAVE_SIGINFO
  struct sigaction action;
  action.sa_handler = SIG_DFL;
  action.sa_flags = 0;
  sigemptyset (&action.sa_mask);
  sigaction (SIGFPE, &action, (struct sigaction *) NULL);
#else
  signal (SIGFPE, SIG_DFL);
#endif
}

/* Check the values returned by plural_eval.  */
static void
check_plural_eval (plural_expr, nplurals_value, header_pos)
     struct expression *plural_expr;
     unsigned long nplurals_value;
     const lex_pos_ty *header_pos;
{
  if (sigsetjmp (sigfpe_exit, 1) == 0)
    {
      unsigned long n;

      /* Protect against arithmetic exceptions.  */
      install_sigfpe_handler ();

      for (n = 0; n <= 1000; n++)
	{
	  unsigned long val = plural_eval (plural_expr, n);

	  if ((long) val < 0)
	    {
	      /* End of protection against arithmetic exceptions.  */
	      uninstall_sigfpe_handler ();

	      error_with_progname = false;
	      error_at_line (0, 0,
			     header_pos->file_name, header_pos->line_number,
			     _("plural expression can produce negative values"));
	      error_with_progname = true;
	      exit_status = EXIT_FAILURE;
	      return;
	    }
	  else if (val >= nplurals_value)
	    {
	      /* End of protection against arithmetic exceptions.  */
	      uninstall_sigfpe_handler ();

	      error_with_progname = false;
	      error_at_line (0, 0,
			     header_pos->file_name, header_pos->line_number,
			     _("nplurals = %lu but plural expression can produce values as large as %lu"),
			     nplurals_value, val);
	      error_with_progname = true;
	      exit_status = EXIT_FAILURE;
	      return;
	    }
	}

      /* End of protection against arithmetic exceptions.  */
      uninstall_sigfpe_handler ();
    }
  else
    {
      /* Caught an arithmetic exception.  */
      const char *msg;

      /* End of protection against arithmetic exceptions.  */
      uninstall_sigfpe_handler ();

#if HAVE_SIGINFO
      switch (sigfpe_code)
#endif
	{
#if HAVE_SIGINFO
# ifdef FPE_INTDIV
	case FPE_INTDIV:
	  msg = _("plural expression can produce division by zero");
	  break;
# endif
# ifdef FPE_INTOVF
	case FPE_INTOVF:
	  msg = _("plural expression can produce integer overflow");
	  break;
# endif
	default:
#endif
	  msg = _("plural expression can produce arithmetic exceptions, possibly division by zero");
	}

      error_with_progname = false;
      error_at_line (0, 0, header_pos->file_name, header_pos->line_number, msg);
      error_with_progname = true;
      exit_status = EXIT_FAILURE;
    }
}


/* Perform plural expression checking.  */
static void
check_plural (mlp)
     message_list_ty *mlp;
{
  const lex_pos_ty *has_plural;
  unsigned long min_nplurals;
  const lex_pos_ty *min_pos;
  unsigned long max_nplurals;
  const lex_pos_ty *max_pos;
  size_t j;
  message_ty *header;

  /* Determine whether mlp has plural entries.  */
  has_plural = NULL;
  min_nplurals = ULONG_MAX;
  min_pos = NULL;
  max_nplurals = 0;
  max_pos = NULL;
  for (j = 0; j < mlp->nitems; j++) {
    message_ty *mp = mlp->item[j];

    if (mp->msgid_plural != NULL) {
      const char *p;
      const char *p_end;
      unsigned long n;

      if (has_plural == NULL)
	has_plural = &mp->pos;

      n = 0;
      for (p = mp->msgstr, p_end = p + mp->msgstr_len;
	   p < p_end;
	   p += strlen (p) + 1)
	n++;
      if (min_nplurals > n) {
	min_nplurals = n;
	min_pos = &mp->pos;
      }
      if (max_nplurals > n) {
	max_nplurals = n;
	min_pos = &mp->pos;
      }
    }
  }

  /* Look at the plural entry for this domain.
     Cf, function extract_plural_expression.  */
  header = message_list_search (mlp, "");
  if (header != NULL)
    {
      const char *nullentry;
      const char *plural;
      const char *nplurals;

      nullentry = header->msgstr;

      plural = strstr (nullentry, "plural=");
      nplurals = strstr (nullentry, "nplurals=");
      if (plural == NULL && has_plural != NULL)
	{
	  error_with_progname = false;
	  error_at_line (0, 0, has_plural->file_name, has_plural->line_number,
			 _("message catalog has plural form translations..."));
	  --error_message_count;
	  error_at_line (0, 0, header->pos.file_name, header->pos.line_number,
			 _("...but header entry lacks a \"plural=EXPRESSION\" attribute"));
	  error_with_progname = true;
	  exit_status = EXIT_FAILURE;
	}
      if (nplurals == NULL && has_plural != NULL)
	{
	  error_with_progname = false;
	  error_at_line (0, 0, has_plural->file_name, has_plural->line_number,
			 _("message catalog has plural form translations..."));
	  --error_message_count;
	  error_at_line (0, 0, header->pos.file_name, header->pos.line_number,
			 _("...but header entry lacks a \"nplurals=INTEGER\" attribute"));
	  error_with_progname = true;
	  exit_status = EXIT_FAILURE;
	}
      if (plural != NULL && nplurals != NULL)
	{
	  const char *endp;
	  unsigned long int nplurals_value;
	  struct parse_args args;
	  struct expression *plural_expr;

	  /* First check the number.  */
	  nplurals += 9;
	  while (*nplurals != '\0' && isspace ((unsigned char) *nplurals))
	    ++nplurals;
	  endp = nplurals;
	  nplurals_value = 0;
	  if (*nplurals >= '0' && *nplurals <= '9')
	    nplurals_value = strtoul (nplurals, (char **) &endp, 10);
	  if (nplurals == endp)
	    {
	      error_with_progname = false;
	      error_at_line (0, 0,
			     header->pos.file_name, header->pos.line_number,
			     _("invalid nplurals value"));
	      error_with_progname = true;
	      exit_status = EXIT_FAILURE;
	    }

	  /* Then check the expression.  */
	  plural += 7;
	  args.cp = plural;
	  if (parse_plural_expression (&args) != 0)
	    {
	      error_with_progname = false;
	      error_at_line (0, 0,
			     header->pos.file_name, header->pos.line_number,
			     _("invalid plural expression"));
	      error_with_progname = true;
	      exit_status = EXIT_FAILURE;
	    }
	  plural_expr = args.res;

	  /* See whether nplurals and plural fit together.  */
	  if (exit_status != EXIT_FAILURE)
	    check_plural_eval (plural_expr, nplurals_value, &header->pos);

	  /* Check the number of plurals of the translations.  */
	  if (exit_status != EXIT_FAILURE)
	    {
	      if (min_nplurals < nplurals_value)
		{
		  error_with_progname = false;
		  error_at_line (0, 0,
				 header->pos.file_name, header->pos.line_number,
				 _("nplurals = %lu..."), nplurals_value);
		  --error_message_count;
		  error_at_line (0, 0, min_pos->file_name, min_pos->line_number,
				 _("...but some messages have only %lu plural forms"),
				 min_nplurals);
		  error_with_progname = true;
		  exit_status = EXIT_FAILURE;
		}
	      else if (max_nplurals > nplurals_value)
		{
		  error_with_progname = false;
		  error_at_line (0, 0,
				 header->pos.file_name, header->pos.line_number,
				 _("nplurals = %lu..."), nplurals_value);
		  --error_message_count;
		  error_at_line (0, 0, max_pos->file_name, max_pos->line_number,
				 _("...but some messages have %lu plural forms"),
				 max_nplurals);
		  error_with_progname = true;
		  exit_status = EXIT_FAILURE;
		}
	      /* The only valid case is max_nplurals <= n <= min_nplurals,
		 which means either has_plural == NULL or
		 max_nplurals = n = min_nplurals.  */
	    }
	}
    }
  else if (has_plural != NULL)
    {
      error_with_progname = false;
      error_at_line (0, 0, has_plural->file_name, has_plural->line_number,
		     _("message catalog has plural form translations, but lacks a header entry with \"Plural-Forms: nplurals=INTEGER; plural=EXPRESSION;\""));
      error_with_progname = true;
      exit_status = EXIT_FAILURE;
    }
}


/* Perform miscellaneous checks on a message.  */
static void
check_pair (msgid, msgid_pos, msgid_plural, msgstr, msgstr_len, msgstr_pos,
	    is_format)
     const char *msgid;
     const lex_pos_ty *msgid_pos;
     const char *msgid_plural;
     const char *msgstr;
     size_t msgstr_len;
     const lex_pos_ty *msgstr_pos;
     enum is_format is_format[NFORMATS];
{
  int has_newline;
  size_t i;
  unsigned int j;
  const char *p;

  /* If the msgid string is empty we have the special entry reserved for
     information about the translation.  */
  if (msgid[0] == '\0')
    return;

  /* Test 1: check whether all or none of the strings begin with a '\n'.  */
  has_newline = (msgid[0] == '\n');
#define TEST_NEWLINE(p) (p[0] == '\n')
  if (msgid_plural != NULL)
    {
      if (TEST_NEWLINE(msgid_plural) != has_newline)
	{
	  error_with_progname = false;
	  error_at_line (0, 0, msgid_pos->file_name, msgid_pos->line_number,
			 _("\
`msgid' and `msgid_plural' entries do not both begin with '\\n'"));
	  error_with_progname = true;
	  exit_status = EXIT_FAILURE;
	}
      for (p = msgstr, j = 0; p < msgstr + msgstr_len; p += strlen (p) + 1, j++)
	if (TEST_NEWLINE(p) != has_newline)
	  {
	    error_with_progname = false;
	    error_at_line (0, 0, msgid_pos->file_name, msgid_pos->line_number,
			   _("\
`msgid' and `msgstr[%u]' entries do not both begin with '\\n'"), j);
	    error_with_progname = true;
	    exit_status = EXIT_FAILURE;
	  }
    }
  else
    {
      if (TEST_NEWLINE(msgstr) != has_newline)
	{
	  error_with_progname = false;
	  error_at_line (0, 0, msgid_pos->file_name, msgid_pos->line_number,
			 _("\
`msgid' and `msgstr' entries do not both begin with '\\n'"));
	  error_with_progname = true;
	  exit_status = EXIT_FAILURE;
	}
    }
#undef TEST_NEWLINE

  /* Test 2: check whether all or none of the strings end with a '\n'.  */
  has_newline = (msgid[strlen (msgid) - 1] == '\n');
#define TEST_NEWLINE(p) (p[0] != '\0' && p[strlen (p) - 1] == '\n')
  if (msgid_plural != NULL)
    {
      if (TEST_NEWLINE(msgid_plural) != has_newline)
	{
	  error_with_progname = false;
	  error_at_line (0, 0, msgid_pos->file_name, msgid_pos->line_number,
			 _("\
`msgid' and `msgid_plural' entries do not both end with '\\n'"));
	  error_with_progname = true;
	  exit_status = EXIT_FAILURE;
	}
      for (p = msgstr, j = 0; p < msgstr + msgstr_len; p += strlen (p) + 1, j++)
	if (TEST_NEWLINE(p) != has_newline)
	  {
	    error_with_progname = false;
	    error_at_line (0, 0, msgid_pos->file_name, msgid_pos->line_number,
			   _("\
`msgid' and `msgstr[%u]' entries do not both end with '\\n'"), j);
	    error_with_progname = true;
	    exit_status = EXIT_FAILURE;
	  }
    }
  else
    {
      if (TEST_NEWLINE(msgstr) != has_newline)
	{
	  error_with_progname = false;
	  error_at_line (0, 0, msgid_pos->file_name, msgid_pos->line_number,
			 _("\
`msgid' and `msgstr' entries do not both end with '\\n'"));
	  error_with_progname = true;
	  exit_status = EXIT_FAILURE;
	}
    }
#undef TEST_NEWLINE

  if (check_compatibility && msgid_plural != NULL)
    {
      error_with_progname = false;
      error_at_line (0, 0, msgid_pos->file_name, msgid_pos->line_number,
		     _("plural handling is a GNU gettext extension"));
      error_with_progname = true;
      exit_status = EXIT_FAILURE;
    }

  if (check_format_strings && msgid_plural == NULL)
    /* Test 3: Check whether both formats strings contain the same number
       of format specifications.
       We check only those messages for which the msgid's is_format flag
       is one of 'yes' or 'possible'.  We don't check msgids with is_format
       'no' or 'impossible', to obey the programmer's order.  We don't check
       msgids with is_format 'undecided' because that would introduce too
       many checks, thus forcing the programmer to add "xgettext: no-c-format"
       anywhere where a translator wishes to use a percent sign.  */
    for (i = 0; i < NFORMATS; i++)
      if (possible_format_p (is_format[i]))
	{
	  /* At runtime, we can assume the program passes arguments that
	     fit well for msgid.  We must signal an error if msgstr wants
	     more arguments that msgid accepts.
	     If msgstr wants fewer arguments than msgid, it wouldn't lead
	     to a crash at runtime, but we nevertheless give an error because
	     1) this situation occurs typically after the programmer has
		added some arguments to msgid, so we must make the translator
		specially aware of it (more than just "fuzzy"),
	     2) it is generally wrong if a translation wants to ignore
		arguments that are used by other translations.  */

	  struct formatstring_parser *parser = formatstring_parsers[i];
	  void *msgid_descr = parser->parse (msgid);

	  if (msgid_descr != NULL)
	    {
	      void *msgstr_descr = parser->parse (msgstr);

	      if (msgstr_descr != NULL)
		{
		  if (parser->check (msgid_pos, msgid_descr, msgstr_descr))
		    exit_status = EXIT_FAILURE;

		  parser->free (msgstr_descr);
		}
	      else
		{
		  error_with_progname = false;
		  error_at_line (0, 0, msgid_pos->file_name,
				 msgid_pos->line_number,
				 _("\
'msgstr' is not a valid %s format string, unlike 'msgid'"),
				 format_language_pretty[i]);
		  error_with_progname = true;
		  exit_status = EXIT_FAILURE;
		}

	      parser->free (msgid_descr);
	    }
	}

  if (check_accelerators && msgid_plural == NULL)
    /* Test 4: Check that if msgid is a menu item with a keyboard accelerator,
       the msgstr has an accelerator as well.  A keyboard accelerator is
       designated by an immediately preceding '&'.  We cannot check whether
       two accelerators collide, only whether the translator has bothered
       thinking about them.  */
    {
      const char *p;

      /* We are only interested in msgids that contain exactly one '&'.  */
      p = strchr (msgid, accelerator_char);
      if (p != NULL && strchr (p + 1, accelerator_char) == NULL)
	{
	  /* Count the number of '&' in msgstr.  */
	  unsigned int count = 0;

	  for (p = msgstr; (p = strchr (p, accelerator_char)) != NULL; p++)
	    count++;

	  if (count == 0)
	    {
	      error_with_progname = false;
	      error_at_line (0, 0, msgid_pos->file_name, msgid_pos->line_number,
			     _("msgstr lacks the keyboard accelerator mark '%c'"),
			     accelerator_char);
	      error_with_progname = true;
	    }
	  else if (count > 1)
	    {
	      error_with_progname = false;
	      error_at_line (0, 0, msgid_pos->file_name, msgid_pos->line_number,
			     _("msgstr has too many keyboard accelerator marks '%c'"),
			     accelerator_char);
	      error_with_progname = true;
	    }
	}
    }
}


/* The rest of the file is similar to read-po.c.  The differences are:
   - The result is both a message_list_ty and a hash table mapping
     msgid -> message_ty.  This is useful to speed up the duplicate lookup.
   - Comments are not stored, they are discarded right away.
   - The header entry check is performed on-the-fly.
 */


/* Prepare for first message.  */
static void
format_constructor (that)
     po_ty *that;
{
  msgfmt_class_ty *this = (msgfmt_class_ty *) that;
  size_t i;

  this->is_fuzzy = false;
  for (i = 0; i < NFORMATS; i++)
    this->is_format[i] = undecided;
  this->do_wrap = undecided;
  this->has_header_entry = false;
}


/* Some checks after whole file is read.  */
static void
format_debrief (that)
     po_ty *that;
{
  msgfmt_class_ty *this = (msgfmt_class_ty *) that;

  /* Test whether header entry was found.  */
  if (check_header && !this->has_header_entry)
    {
      multiline_error (xasprintf ("%s: ", gram_pos.file_name),
		       xasprintf (_("\
warning: PO file header missing, fuzzy, or invalid\n")));
      multiline_error (NULL,
		       xasprintf (_("\
warning: charset conversion will not work\n")));
    }
}


/* Process `domain' directive from .po file.  */
static void
format_directive_domain (pop, name)
     po_ty *pop;
     char *name;
{
  /* If no output file was given, we change it with each `domain'
     directive.  */
  if (!java_mode && output_file_name == NULL)
    {
      size_t correct;

      correct = strcspn (name, INVALID_PATH_CHAR);
      if (name[correct] != '\0')
	{
	  exit_status = EXIT_FAILURE;
	  if (correct == 0)
	    {
	      error (0, 0, _("\
domain name \"%s\" not suitable as file name"), name);
	      return;
	    }
	  else
	    error (0, 0, _("\
domain name \"%s\" not suitable as file name: will use prefix"), name);
	  name[correct] = '\0';
	}

      /* Set new domain.  */
      current_domain = new_domain (name, add_mo_suffix (name));
    }
  else
    {
      if (check_domain)
	error (0, 0, _("`domain %s' directive ignored"), name);

      /* NAME was allocated in po-gram.y but is not used anywhere.  */
      free (name);
    }
}


/* Process `msgid'/`msgstr' pair from .po file.  */
static void
format_directive_message (that, msgid_string, msgid_pos, msgid_plural,
			  msgstr_string, msgstr_len, msgstr_pos, obsolete)
     po_ty *that;
     char *msgid_string;
     lex_pos_ty *msgid_pos;
     char *msgid_plural;
     char *msgstr_string;
     size_t msgstr_len;
     lex_pos_ty *msgstr_pos;
     bool obsolete;
{
  msgfmt_class_ty *this = (msgfmt_class_ty *) that;
  message_ty *entry;
  size_t i;

  /* Don't emit untranslated entries.  Also don't emit fuzzy entries, unless
     --use-fuzzy was specified.  But ignore fuzziness of the header entry.  */
  if (msgstr_string[0] == '\0'
      || (!include_all && this->is_fuzzy && msgid_string[0] != '\0'))
    {
      if (check_compatibility)
	{
	  error_with_progname = false;
	  error_at_line (0, 0, msgstr_pos->file_name, msgstr_pos->line_number,
			 (msgstr_string[0] == '\0'
			  ? _("empty `msgstr' entry ignored")
			  : _("fuzzy `msgstr' entry ignored")));
	  error_with_progname = true;
	}

      /* Increment counter for fuzzy/untranslated messages.  */
      if (msgstr_string[0] == '\0')
	++msgs_untranslated;
      else
	++msgs_fuzzy;

      /* Free strings allocated in po-gram.y.  */
      free (msgstr_string);
    }
  else
    {
      /* Test for header entry.  */
      if (msgid_string[0] == '\0')
	{
	  this->has_header_entry = true;

	  /* Do some more tests on test contents of the header entry.  */
	  if (check_header)
	    {
	      static const char *required_fields[] =
	      {
		"Project-Id-Version", "PO-Revision-Date",
		"Last-Translator", "Language-Team", "MIME-Version",
		"Content-Type", "Content-Transfer-Encoding"
	      };
	      static const char *default_values[] =
	      {
		"PACKAGE VERSION", "YEAR-MO-DA", "FULL NAME", "LANGUAGE",
		NULL, "text/plain; charset=CHARSET", "ENCODING"
	      };
	      const size_t nfields = SIZEOF (required_fields);
	      int initial = -1;
	      int cnt;

	      for (cnt = 0; cnt < nfields; ++cnt)
		{
		  char *endp = strstr (msgstr_string, required_fields[cnt]);

		  if (endp == NULL)
		    error (0, 0, _("headerfield `%s' missing in header"),
			   required_fields[cnt]);
		  else if (endp != msgstr_string && endp[-1] != '\n')
		    error (0, 0, _("\
header field `%s' should start at beginning of line"),
			   required_fields[cnt]);
		  else if (default_values[cnt] != NULL
			   && strncmp (default_values[cnt],
				       endp + strlen (required_fields[cnt]) + 2,
				       strlen (default_values[cnt])) == 0)
		    {
		      if (initial != -1)
			{
			  error (0, 0, _("\
some header fields still have the initial default value"));
			  initial = -1;
			  break;
			}
		      else
			initial = cnt;
		    }
		}

	      if (initial != -1)
		error (0, 0, _("field `%s' still has initial default value"),
		       required_fields[initial]);
	    }
	}
      else
	/* We don't count the header entry in the statistic so place the
	   counter incrementation here.  */
	if (this->is_fuzzy)
	  ++msgs_fuzzy;
	else
	  ++msgs_translated;

      /* We found a valid pair of msgid/msgstr.
	 Construct struct to describe msgstr definition.  */
      entry = message_alloc (NULL, NULL, NULL, 0, msgstr_pos);
      entry->msgid = msgid_string;
      entry->msgid_plural = msgid_plural;
      entry->msgstr = msgstr_string;
      entry->msgstr_len = msgstr_len;

      /* Do some more checks on both strings.  */
      check_pair (msgid_string, msgid_pos, msgid_plural,
		  msgstr_string, msgstr_len, msgstr_pos,
		  this->is_format);

      /* Check whether already a domain is specified.  If not use default
	 domain.  */
      if (current_domain == NULL)
	current_domain = new_domain (MESSAGE_DOMAIN_DEFAULT,
				     add_mo_suffix (MESSAGE_DOMAIN_DEFAULT));

      /* We insert the ID/string pair into the hashing table.  But we have
	 to take care for duplicates.  */
      if (insert_entry (&current_domain->symbol_tab, msgid_string,
			strlen (msgid_string) + 1, entry))
	{
	  /* We don't need the just constructed entry.  */
	  free (entry);

	  /* We give a fatal error about this, regardless whether the
	     translations are equal or different.  This is for consistency
	     with msgmerge, msgcat and others.  The user can use the
	     msguniq program to get rid of duplicates.  */
	  find_entry (&current_domain->symbol_tab, msgid_string,
		      strlen (msgid_string) + 1, (void **) &entry);
	  if (msgstr_len != entry->msgstr_len
	      || memcmp (msgstr_string, entry->msgstr, msgstr_len) != 0)
	    {
	      po_gram_error_at_line (msgid_pos, _("\
duplicate message definition"));
	      po_gram_error_at_line (&entry->pos, _("\
...this is the location of the first definition"));
	    }

	  /* We don't need the just constructed entries'
	     parameter string (allocated in po-gram.y).  */
	  free (msgid_string);
	  free (msgstr_string);
	}
      else
	message_list_append (current_domain->mlp, entry);
  }

  /* Prepare for next message.  */
  this->is_fuzzy = false;
  for (i = 0; i < NFORMATS; i++)
    this->is_format[i] = undecided;
  this->do_wrap = undecided;
}


/* Test for `#, fuzzy' comments and warn.  */
static void
format_comment_special (that, s)
     po_ty *that;
     const char *s;
{
  msgfmt_class_ty *this = (msgfmt_class_ty *) that;
  bool fuzzy;

  po_parse_comment_special (s, &fuzzy, this->is_format, &this->do_wrap);

  if (fuzzy)
    {
      static bool warned = false;

      if (!include_all && check_compatibility && !warned)
	{
	  warned = true;
	  error (0, 0, _("\
%s: warning: source file contains fuzzy translation"),
		 gram_pos.file_name);
	}

      this->is_fuzzy = true;
    }
}


/* So that the one parser can be used for multiple programs, and also
   use good data hiding and encapsulation practices, an object
   oriented approach has been taken.  An object instance is allocated,
   and all actions resulting from the parse will be through
   invocations of method functions of that object.  */

static po_method_ty format_methods =
{
  sizeof (msgfmt_class_ty),
  format_constructor, /* constructor */
  NULL, /* destructor */
  format_directive_domain,
  format_directive_message,
  NULL, /* parse_brief */
  format_debrief, /* parse_debrief */
  NULL, /* comment */
  NULL, /* comment_dot */
  NULL, /* comment_filepos */
  format_comment_special /* comment */
};


/* Read .po file FILENAME and store translation pairs.  */
static void
read_po_file (filename)
     char *filename;
{
  po_ty *pop;

  pop = po_alloc (&format_methods);
  po_scan_file (pop, filename);
  po_free (pop);
}
