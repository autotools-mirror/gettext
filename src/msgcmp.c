/* GNU gettext - internationalization aids
   Copyright (C) 1995-1998, 2000-2002 Free Software Foundation, Inc.
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
#include "exit.h"
#include "gettext.h"
#include "po.h"

#define _(str) gettext (str)


/* Apply the .pot file to each of the domains in the PO file.  */
static bool multi_domain_mode = false;

/* Long options.  */
static const struct option long_options[] =
{
  { "directory", required_argument, NULL, 'D' },
  { "help", no_argument, NULL, 'h' },
  { "multi-domain", no_argument, NULL, 'm' },
  { "version", no_argument, NULL, 'V' },
  { NULL, 0, NULL, 0 }
};


/* Prototypes for local functions.  Needed to ensure compiler checking of
   function argument counts despite of K&R C function definition syntax.  */
static void usage PARAMS ((int status));
static void match_domain PARAMS ((const char *fn1, const char *fn2,
				  message_list_ty *defmlp,
				  message_list_ty *refmlp, int *nerrors));
static void compare PARAMS ((const char *, const char *));
static msgdomain_list_ty *grammar PARAMS ((const char *filename));
static void compare_constructor PARAMS ((po_ty *that));
static void compare_destructor PARAMS ((po_ty *that));
static void compare_directive_domain PARAMS ((po_ty *that, char *name));
static void compare_directive_message PARAMS ((po_ty *that, char *msgid,
					       lex_pos_ty *msgid_pos,
					       char *msgid_plural,
					       char *msgstr, size_t msgstr_len,
					       lex_pos_ty *msgstr_pos,
					       bool obsolete));


int
main (argc, argv)
     int argc;
     char *argv[];
{
  int optchar;
  bool do_help;
  bool do_version;

  /* Set program name for messages.  */
  set_program_name (argv[0]);
  error_print_progname = maybe_print_progname;
  gram_max_allowed_errors = UINT_MAX;

#ifdef HAVE_SETLOCALE
  /* Set locale via LC_ALL.  */
  setlocale (LC_ALL, "");
#endif

  /* Set the text message domain.  */
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  do_help = false;
  do_version = false;
  while ((optchar = getopt_long (argc, argv, "D:hmV", long_options, NULL))
	 != EOF)
    switch (optchar)
      {
      case '\0':		/* long option */
	break;

      case 'D':
	dir_list_append (optarg);
	break;

      case 'h':
	do_help = true;
	break;

      case 'm':
	multi_domain_mode = true;
	break;

      case 'V':
	do_version = true;
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
	      "1995-1998, 2000-2002");
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

  /* compare the two files */
  compare (argv[optind], argv[optind + 1]);
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
Compare two Uniforum style .po files to check that both contain the same\n\
set of msgid strings.  The def.po file is an existing PO file with the\n\
translations.  The ref.pot file is the last created PO file, or a PO Template\n\
file (generally created by xgettext).  This is useful for checking that\n\
you have translated each and every message in your program.  Where an exact\n\
match cannot be found, fuzzy matching is used to produce better diagnostics.\n\
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
  def.po                      translations\n\
  ref.pot                     references to the sources\n\
  -D, --directory=DIRECTORY   add DIRECTORY to list for input files search\n\
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
Informative output:\n\
  -h, --help                  display this help and exit\n\
  -V, --version               output version information and exit\n\
"));
      printf ("\n");
      fputs (_("Report bugs to <bug-gnu-gettext@gnu.org>.\n"), stdout);
    }

  exit (status);
}


static void
match_domain (fn1, fn2, defmlp, refmlp, nerrors)
     const char *fn1;
     const char *fn2;
     message_list_ty *defmlp;
     message_list_ty *refmlp;
     int *nerrors;
{
  size_t j;

  for (j = 0; j < refmlp->nitems; j++)
    {
      message_ty *refmsg;
      message_ty *defmsg;

      refmsg = refmlp->item[j];

      /* See if it is in the other file.  */
      defmsg = message_list_search (defmlp, refmsg->msgid);
      if (defmsg)
	defmsg->used = 1;
      else
	{
	  /* If the message was not defined at all, try to find a very
	     similar message, it could be a typo, or the suggestion may
	     help.  */
	  (*nerrors)++;
	  defmsg = message_list_search_fuzzy (defmlp, refmsg->msgid);
	  if (defmsg)
	    {
	      po_gram_error_at_line (&refmsg->pos, _("\
this message is used but not defined..."));
	      po_gram_error_at_line (&defmsg->pos, _("\
...but this definition is similar"));
	      defmsg->used = 1;
	    }
	  else
	    po_gram_error_at_line (&refmsg->pos, _("\
this message is used but not defined in %s"), fn1);
	}
    }
}


static void
compare (fn1, fn2)
     const char *fn1;
     const char *fn2;
{
  msgdomain_list_ty *def;
  msgdomain_list_ty *ref;
  int nerrors;
  size_t j, k;
  message_list_ty *empty_list;

  /* This is the master file, created by a human.  */
  def = grammar (fn1);

  /* This is the generated file, created by groping the sources with
     the xgettext program.  */
  ref = grammar (fn2);

  empty_list = message_list_alloc (false);

  /* Every entry in the xgettext generated file must be matched by a
     (single) entry in the human created file.  */
  nerrors = 0;
  if (!multi_domain_mode)
    for (k = 0; k < ref->nitems; k++)
      {
	const char *domain = ref->item[k]->domain;
	message_list_ty *refmlp = ref->item[k]->messages;
	message_list_ty *defmlp;

	defmlp = msgdomain_list_sublist (def, domain, false);
	if (defmlp == NULL)
	  defmlp = empty_list;

	match_domain (fn1, fn2, defmlp, refmlp, &nerrors);
      }
  else
    {
      /* Apply the references messages in the default domain to each of
	 the definition domains.  */
      message_list_ty *refmlp = ref->item[0]->messages;

      for (k = 0; k < def->nitems; k++)
	{
	  message_list_ty *defmlp = def->item[k]->messages;

	  /* Ignore the default message domain if it has no messages.  */
	  if (k > 0 || defmlp->nitems > 0)
	    match_domain (fn1, fn2, defmlp, refmlp, &nerrors);
	}
    }

  /* Look for messages in the definition file, which are not present
     in the reference file, indicating messages which defined but not
     used in the program.  */
  for (k = 0; k < def->nitems; ++k)
    {
      message_list_ty *defmlp = def->item[k]->messages;

      for (j = 0; j < defmlp->nitems; j++)
	{
	  message_ty *defmsg = defmlp->item[j];

	  if (!defmsg->used)
	    po_gram_error_at_line (&defmsg->pos,
				   _("warning: this message is not used"));
	}
    }

  /* Exit with status 1 on any error.  */
  if (nerrors > 0)
    error (EXIT_FAILURE, 0,
	   ngettext ("found %d fatal error", "found %d fatal errors", nerrors),
	   nerrors);
}


/* Local functions.  */

/* This structure defines a derived class of the po_ty class.  (See
   po.h for an explanation.)  */
typedef struct compare_class_ty compare_class_ty;
struct compare_class_ty
{
  /* inherited instance variables, etc */
  PO_BASE_TY

  /* List of messages already appeared in the current file.  */
  msgdomain_list_ty *mdlp;

  /* Name of domain we are currently examining.  */
  char *domain;

  /* List of messages belonging to the current domain.  */
  message_list_ty *mlp;
};

static void
compare_constructor (that)
     po_ty *that;
{
  compare_class_ty *this = (compare_class_ty *) that;

  this->mdlp = msgdomain_list_alloc (true);
  this->domain = MESSAGE_DOMAIN_DEFAULT;
  this->mlp = msgdomain_list_sublist (this->mdlp, this->domain, true);
}


static void
compare_destructor (that)
     po_ty *that;
{
  compare_class_ty *this = (compare_class_ty *) that;

  (void) this;
  /* Do not free this->mdlp and this->mlp.  */
}


static void
compare_directive_domain (that, name)
     po_ty *that;
     char *name;
{
  compare_class_ty *this = (compare_class_ty *)that;
  /* Override current domain name.  Don't free memory.  */
  this->domain = name;
}


static void
compare_directive_message (that, msgid, msgid_pos, msgid_plural,
			   msgstr, msgstr_len, msgstr_pos, obsolete)
     po_ty *that;
     char *msgid;
     lex_pos_ty *msgid_pos;
     char *msgid_plural;
     char *msgstr;
     size_t msgstr_len;
     lex_pos_ty *msgstr_pos;
     bool obsolete;
{
  compare_class_ty *this = (compare_class_ty *) that;
  message_ty *mp;

  /* Select the appropriate sublist of this->mdlp.  */
  this->mlp = msgdomain_list_sublist (this->mdlp, this->domain, true);

  /* See if this message ID has been seen before.  */
  mp = message_list_search (this->mlp, msgid);
  if (mp)
    {
      po_gram_error_at_line (msgid_pos, _("duplicate message definition"));
      po_gram_error_at_line (&mp->pos, _("\
...this is the location of the first definition"));
      free (msgstr);
      free (msgid);
    }
  else
    {
      mp = message_alloc (msgid, msgid_plural, msgstr, msgstr_len, msgstr_pos);
      message_list_append (this->mlp, mp);
    }
}


/* So that the one parser can be used for multiple programs, and also
   use good data hiding and encapsulation practices, an object
   oriented approach has been taken.  An object instance is allocated,
   and all actions resulting from the parse will be through
   invocations of method functions of that object.  */

static po_method_ty compare_methods =
{
  sizeof (compare_class_ty),
  compare_constructor,
  compare_destructor,
  compare_directive_domain,
  compare_directive_message,
  NULL, /* parse_brief */
  NULL, /* parse_debrief */
  NULL, /* comment */
  NULL, /* comment_dot */
  NULL, /* comment_filepos */
  NULL, /* comment_special */
};


static msgdomain_list_ty *
grammar (filename)
     const char *filename;
{
  po_ty *pop;
  msgdomain_list_ty *mdlp;

  pop = po_alloc (&compare_methods);
  po_scan_file (pop, filename);
  mdlp = ((compare_class_ty *)pop)->mdlp;
  po_free (pop);
  return mdlp;
}
