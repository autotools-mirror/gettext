/* Shell format strings.
   Copyright (C) 2003 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2003.

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

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "format.h"
#include "c-ctype.h"
#include "xmalloc.h"
#include "format-invalid.h"
#include "error.h"
#include "error-progname.h"
#include "gettext.h"

#define _(str) gettext (str)

/* Shell format strings are simply strings subjects to variable substitution.
   A variable substitution starts with '$' and is finished by either
   - a nonempty sequence of alphanumeric ASCII characters, the first being
     not a digit, or
   - an opening brace '{', some other characters with balanced '{' and '}',
     and a closing brace '}', or
   - a single ASCII character, like '$' or '?'.

   FIXME: POSIX has more complicated rules for determining the matching brace:
     "Any '}' escaped by a backslash or within a quoted string, and characters
      in embedded arithmetic expansions, command substitutions, and variable
      expansions, shall not be examined in determining the matching '}'."
   Not yet implemented here.
 */

struct named_arg
{
  char *name;
};

struct spec
{
  unsigned int directives;
  unsigned int named_arg_count;
  unsigned int allocated;
  struct named_arg *named;
};


static int
named_arg_compare (const void *p1, const void *p2)
{
  return strcmp (((const struct named_arg *) p1)->name,
		 ((const struct named_arg *) p2)->name);
}

#define INVALID_NON_ASCII_VARIABLE() \
  xstrdup (_("The string refers to a shell variable with a non-ASCII name."))
#define INVALID_EMPTY_VARIABLE() \
  xstrdup (_("The string refers to a shell variable with an empty name."))

static void *
format_parse (const char *format, char **invalid_reason)
{
  struct spec spec;
  struct spec *result;

  spec.directives = 0;
  spec.named_arg_count = 0;
  spec.allocated = 0;
  spec.named = NULL;

  for (; *format != '\0';)
    if (*format++ == '$')
      {
	/* A variable substitution.  */
	char *name;

	spec.directives++;

	if (*format == '{')
	  {
	    unsigned int depth;
	    const char *name_start;
	    const char *name_end;
	    size_t n;

	    name_start = ++format;
	    depth = 0;
	    for (; *format != '\0'; format++)
	      {
		if (*format == '{')
		  depth++;
		else if (*format == '}')
		  {
		    if (depth == 0)
		      break;
		    else
		      depth--;
		  }
		if (!c_isascii (*format))
		  {
		    *invalid_reason = INVALID_NON_ASCII_VARIABLE();
		    goto bad_format;
		  }
	      }
	    if (*format == '\0')
	      {
		*invalid_reason = INVALID_UNTERMINATED_DIRECTIVE ();
		goto bad_format;
	      }
	    name_end = format++;

	    n = name_end - name_start;
	    if (n == 0)
	      {
		*invalid_reason = INVALID_EMPTY_VARIABLE();
		goto bad_format;
	      }
	    name = (char *) xmalloc (n + 1);
	    memcpy (name, name_start, n);
	    name[n] = '\0';
	  }
	else if (c_isalpha (*format) || *format == '_')
	  {
	    const char *name_start;
	    const char *name_end;
	    size_t n;

	    name_start = format;
	    do
	      format++;
	    while (*format != '\0' && (c_isalnum (*format) || *format == '_'));
	    name_end = format;

	    n = name_end - name_start;
	    name = (char *) xmalloc (n + 1);
	    memcpy (name, name_start, n);
	    name[n] = '\0';
	  }
	else if (*format != '\0')
	  {
	    if (!c_isascii (*format))
	      {
		*invalid_reason = INVALID_NON_ASCII_VARIABLE();
		goto bad_format;
	      }
	    name = (char *) xmalloc (2);
	    name[0] = *format++;
	    name[1] = '\0';
	  }
	else
	  {
	    *invalid_reason = INVALID_UNTERMINATED_DIRECTIVE ();
	    goto bad_format;
	  }

	/* Named argument.  */
	if (spec.allocated == spec.named_arg_count)
	  {
	    spec.allocated = 2 * spec.allocated + 1;
	    spec.named = (struct named_arg *) xrealloc (spec.named, spec.allocated * sizeof (struct named_arg));
	  }
	spec.named[spec.named_arg_count].name = name;
	spec.named_arg_count++;
      }

  /* Sort the named argument array, and eliminate duplicates.  */
  if (spec.named_arg_count > 1)
    {
      unsigned int i, j;

      qsort (spec.named, spec.named_arg_count, sizeof (struct named_arg),
	     named_arg_compare);

      /* Remove duplicates: Copy from i to j, keeping 0 <= j <= i.  */
      for (i = j = 0; i < spec.named_arg_count; i++)
	if (j > 0 && strcmp (spec.named[i].name, spec.named[j-1].name) == 0)
	  free (spec.named[i].name);
	else
	  {
	    if (j < i)
	      spec.named[j].name = spec.named[i].name;
	    j++;
	  }
      spec.named_arg_count = j;
    }

  result = (struct spec *) xmalloc (sizeof (struct spec));
  *result = spec;
  return result;

 bad_format:
  if (spec.named != NULL)
    {
      unsigned int i;
      for (i = 0; i < spec.named_arg_count; i++)
	free (spec.named[i].name);
      free (spec.named);
    }
  return NULL;
}

static void
format_free (void *descr)
{
  struct spec *spec = (struct spec *) descr;

  if (spec->named != NULL)
    {
      unsigned int i;
      for (i = 0; i < spec->named_arg_count; i++)
	free (spec->named[i].name);
      free (spec->named);
    }
  free (spec);
}

static int
format_get_number_of_directives (void *descr)
{
  struct spec *spec = (struct spec *) descr;

  return spec->directives;
}

static bool
format_check (const lex_pos_ty *pos, void *msgid_descr, void *msgstr_descr,
	      bool equality, bool noisy, const char *pretty_msgstr)
{
  struct spec *spec1 = (struct spec *) msgid_descr;
  struct spec *spec2 = (struct spec *) msgstr_descr;
  bool err = false;

  if (spec1->named_arg_count + spec2->named_arg_count > 0)
    {
      unsigned int i, j;
      unsigned int n1 = spec1->named_arg_count;
      unsigned int n2 = spec2->named_arg_count;

      /* Check the argument names are the same.
	 Both arrays are sorted.  We search for the first difference.  */
      for (i = 0, j = 0; i < n1 || j < n2; )
	{
	  int cmp = (i >= n1 ? 1 :
		     j >= n2 ? -1 :
		     strcmp (spec1->named[i].name, spec2->named[j].name));

	  if (cmp > 0)
	    {
	      if (noisy)
		{
		  error_with_progname = false;
		  error_at_line (0, 0, pos->file_name, pos->line_number,
				 _("a format specification for argument '%s', as in '%s', doesn't exist in 'msgid'"),
				 spec2->named[j].name, pretty_msgstr);
		  error_with_progname = true;
		}
	      err = true;
	      break;
	    }
	  else if (cmp < 0)
	    {
	      if (equality)
		{
		  if (noisy)
		    {
		      error_with_progname = false;
		      error_at_line (0, 0, pos->file_name, pos->line_number,
				     _("a format specification for argument '%s' doesn't exist in '%s'"),
				     spec1->named[i].name, pretty_msgstr);
		      error_with_progname = true;
		    }
		  err = true;
		  break;
		}
	      else
		i++;
	    }
	  else
	    j++, i++;
	}
    }

  return err;
}


struct formatstring_parser formatstring_sh =
{
  format_parse,
  format_free,
  format_get_number_of_directives,
  format_check
};


#ifdef TEST

/* Test program: Print the argument list specification returned by
   format_parse for strings read from standard input.  */

#include <stdio.h>
#include "getline.h"

static void
format_print (void *descr)
{
  struct spec *spec = (struct spec *) descr;
  unsigned int i;

  if (spec == NULL)
    {
      printf ("INVALID");
      return;
    }

  printf ("{");
  for (i = 0; i < spec->named_arg_count; i++)
    {
      if (i > 0)
	printf (", ");
      printf ("'%s'", spec->named[i].name);
    }
  printf ("}");
}

int
main ()
{
  for (;;)
    {
      char *line = NULL;
      size_t line_size = 0;
      int line_len;
      char *invalid_reason;
      void *descr;

      line_len = getline (&line, &line_size, stdin);
      if (line_len < 0)
	break;
      if (line_len > 0 && line[line_len - 1] == '\n')
	line[--line_len] = '\0';

      invalid_reason = NULL;
      descr = format_parse (line, &invalid_reason);

      format_print (descr);
      printf ("\n");
      if (descr == NULL)
	printf ("%s\n", invalid_reason);

      free (invalid_reason);
      free (line);
    }

  return 0;
}

/*
 * For Emacs M-x compile
 * Local Variables:
 * compile-command: "/bin/sh ../libtool --mode=link gcc -o a.out -static -O -g -Wall -I.. -I../lib -I../intl -DHAVE_CONFIG_H -DTEST format-sh.c ../lib/libgettextlib.la"
 * End:
 */

#endif /* TEST */
