/* Perl format strings.
   Copyright (C) 2002-2003 Free Software Foundation, Inc.
   Written by Guido Flohr <guido@imperia.net>, 2003.

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
#include <alloca.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "format.h"
#include "xmalloc.h"
#include "error.h"
#include "progname.h"
#include "gettext.h"
#include "hash.h"

#define _(str) gettext (str)

/* Perl format strings are currently quite simple.  They consist of 
   place-holders embedded in the string.

   messageFormatPattern := string ("[" messageFormatElement "]" string)*
   
   messageFormatElement := [_A-Za-z][_0-9A-Za-z]+

   However, C format strings are also allowed and used.  The following
   functions are therefore decorators for the C format checker, and
   will only fall back to Perl format if the C check is negative.  */
struct spec
{
  unsigned int directives;
  hash_table hash;
  void* c_format;
};

static void*
format_parse (const char* string, char** invalid_reason)
{
  char* last_pos = (char*) string;
  char* pos;
   struct spec* spec;
  void* c_format = formatstring_c.parse (string, invalid_reason);

  if (c_format == NULL)
    return NULL;

  spec = xcalloc (1, sizeof (struct spec));

  spec->c_format = c_format;

  init_hash (&spec->hash, 13);
  while ((pos = strchr (last_pos, '['))) {
    char* start = pos + 1;
    last_pos = start;

    if (!(*last_pos == '_' 
	  || (*last_pos >= 'A' && *last_pos <= 'Z') 
	  || (*last_pos >= 'a' && *last_pos <= 'z')))
      continue;
    
    ++last_pos;
    
    while (*last_pos == '_' 
	   || (*last_pos >= '0' && *last_pos <= '9') 
	   || (*last_pos >= 'A' && *last_pos <= 'Z') 
	   || (*last_pos >= 'a' && *last_pos <= 'z'))
      ++last_pos;

    if (*last_pos == ']') 
      {
	size_t len = last_pos - start;
	int* hits;

	if (0 == find_entry (&spec->hash, start, len, (void**) &hits))
	  {
	    ++(*hits);
	  }
	else
	  {
	    hits = xmalloc (sizeof *hits);
	    *hits = 1;
	    insert_entry (&spec->hash, start, len, hits);
	  }
	++last_pos;
	++spec->directives;
      }
  }

  return spec;
}

static void
format_free (void* description)
{
  void* ptr = NULL;
  const void* key;
  size_t keylen;
  void* data;

  struct spec* spec = (struct spec*) description;

  if (spec != NULL)
    {
      while (0 == iterate_table (&spec->hash, &ptr, &key, &keylen, &data))
	free (data);

      delete_hash (&spec->hash);
      
      if (spec->c_format)
	formatstring_c.free (spec->c_format);
      
      free (spec);
    }
}

static int
format_get_number_of_directives (void* description)
{
  int c_directives = 0;

  struct spec* spec = (struct spec*) description;
  if (spec->c_format)
    c_directives = formatstring_c.get_number_of_directives (spec->c_format);

  return c_directives + spec->directives;
}

static bool 
format_check (const lex_pos_ty* pos, void* msgid_descr, void* msgstr_descr, 
	      bool equality, bool noisy, const char* pretty_msgstr) 
{
  struct spec* spec1 = (struct spec *) msgid_descr;
  struct spec* spec2 = (struct spec *) msgstr_descr;
  bool result = false;
  void* ptr = NULL;
  const void* key;
  size_t keylen;
  int* hits1;
  int* hits2;

  /* First check the perl arguments.  We are probably faster than format-c.  */
  if (equality)
    {
      /* Pass 1: Check that every format specification in msgid has its
	 counterpart in msgstr.  This is only necessary for equality.  */
      while (0 == iterate_table (&spec1->hash, &ptr, &key, 
				 &keylen, (void**) &hits1))
	{
	  if (0 == find_entry (&spec2->hash, key, keylen, (void**) &hits2))
	    {
	      if (*hits1 != *hits2)
		{
		  result = true;
		  if (noisy)
		    {
		      char* argname = alloca (keylen + 1);
		      memcpy (argname, key, keylen);
		      argname[keylen] = 0;
		      
		      /* The next message shows the limitations of ngettext.
			 It is not smart enough to say "once", "twice",
			 "thrice/%d times", "%d times", ..., and it cannot
			 grok with two variable arguments.  We have to
			 work around the problem.  */
		      error_with_progname = false;
		      error_at_line (0, 0, pos->file_name, pos->line_number,
				 _("\
appearances of named argument '[%s]' do not match \
(%d in original string, %d in '%s')"),
				     argname, *hits1, *hits2, pretty_msgstr);
		      error_with_progname = true;
		    }
		  else
		    return true;
		}
	    }
	  else
	    {
	      result = true;
	      if (noisy)
		{
		  char* argname = alloca (keylen + 1);
		  memcpy (argname, key, keylen);
		  argname[keylen] = 0;
		  
		  error_with_progname = false;
		  error_at_line (0, 0, pos->file_name, pos->line_number,
				 _("\
named argument '[%s]' appears in original string but not in '%s'"),
				 argname, pretty_msgstr);
		  error_with_progname = true;
		}
	      else
		return true;
	    }
	}
    }

  /* Pass 2: Check that the number of format specifications in msgstr
     does not exceed the number of appearances in msgid.  */
  ptr = NULL;
  while (0 == iterate_table (&spec2->hash, &ptr, &key, &keylen, 
			     (void**) &hits2))
    {
      if (0 != find_entry (&spec1->hash, key, keylen, (void**) &hits1))
	{
	  result = true;
	  if (noisy)
	    {
	      char* argname = alloca (keylen + 1);
	      memcpy (argname, key, keylen);
	      argname[keylen] = 0;
	      
	      error_with_progname = false;
	      error_at_line (0, 0, pos->file_name, pos->line_number,
			     _("\
named argument '[%s]' appears only in '%s' but not in the original string"),
			     argname, pretty_msgstr);
	      error_with_progname = true;
	    }
	  else
	    return true;
	}
    }

  if (spec1->c_format && spec2->c_format)
    {
      result |= formatstring_c.check (pos, spec1->c_format, spec2->c_format,
				      equality, noisy, pretty_msgstr);
    }

  return result;
}

struct formatstring_parser formatstring_perl =
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
format_print (descr, line)
     void* descr;
     const char* line;
{
  struct spec *spec = (struct spec *) descr;
  char* ptr = NULL;
  char* key = NULL;
  size_t keylen;
  int data;
  
  printf ("%s=> ", line);
  
  if (spec == NULL)
    {
      printf ("INVALID\n");
      return;
    }
  
  while (iterate_table (&spec->hash, (void**) &ptr, (const void**) &key, 
			&keylen, (void**) &data) == 0) 
    {
      key[keylen - 1] = '\0';
      printf (">>>[%s]<<< ", key);
    }
  
  printf ("\n");
}

int
main ()
{
  char *line = NULL;
  size_t line_len = 0;
  void *descr;
  
  for (;;)
    {      
      if (getline (&line, &line_len, stdin) < 0)
	break;
      
      descr = format_parse (line);
      
      format_print (descr, line);
      
      format_free (descr);
    }
  
  return 0;
}

/*
 * For Emacs M-x compile
 * Local Variables:
 * compile-command: "/bin/sh ../libtool --mode=link gcc -o a.out -static -O -g -Wall -I.. -I../lib -I../intl -DHAVE_CONFIG_H -DTEST format-perl.c ../lib/libgettextlib.la"
 * End:
 */

#endif /* TEST */
