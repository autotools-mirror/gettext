/* Reading binary .mo files.
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

/* Specification.  */
#include "read-mo.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* This include file describes the main part of binary .mo format.  */
#include "gmo.h"

#include "error.h"
#include "xmalloc.h"
#include "system.h"
#include "message.h"
#include "gettext.h"

#define _(str) gettext (str)


/* This defines the byte order within the file.  It needs to be set
   appropriately once we have the file open.  */
static enum { MO_LITTLE_ENDIAN, MO_BIG_ENDIAN } endian;


/* Prototypes for local functions.  Needed to ensure compiler checking of
   function argument counts despite of K&R C function definition syntax.  */
static nls_uint32 read32 PARAMS ((FILE *fp, const char *fn));
static void seek32 PARAMS ((FILE *fp, const char *fn, long offset));
static char *string32 PARAMS ((FILE *fp, const char *fn, long offset,
			       size_t *lengthp));


/* This function reads a 32-bit number from the file, and assembles it
   according to the current ``endian'' setting.  */
static nls_uint32
read32 (fp, fn)
     FILE *fp;
     const char *fn;
{
  int c1, c2, c3, c4;

  c1 = getc (fp);
  if (c1 == EOF)
    {
    bomb:
      if (ferror (fp))
	error (EXIT_FAILURE, errno, _("error while reading \"%s\""), fn);
      error (EXIT_FAILURE, 0, _("file \"%s\" truncated"), fn);
    }
  c2 = getc (fp);
  if (c2 == EOF)
    goto bomb;
  c3 = getc (fp);
  if (c3 == EOF)
    goto bomb;
  c4 = getc (fp);
  if (c4 == EOF)
    goto bomb;
  if (endian == MO_LITTLE_ENDIAN)
    return (((nls_uint32) c1)
	    | ((nls_uint32) c2 << 8)
	    | ((nls_uint32) c3 << 16)
	    | ((nls_uint32) c4 << 24));

  return (((nls_uint32) c1 << 24)
	  | ((nls_uint32) c2 << 16)
	  | ((nls_uint32) c3 << 8)
	  | ((nls_uint32) c4));
}


static void
seek32 (fp, fn, offset)
     FILE *fp;
     const char *fn;
     long offset;
{
  if (fseek (fp, offset, 0) < 0)
    error (EXIT_FAILURE, errno, _("seek \"%s\" offset %ld failed"),
	   fn, offset);
}


static char *
string32 (fp, fn, offset, lengthp)
     FILE *fp;
     const char *fn;
     long offset;
     size_t *lengthp;
{
  long length;
  char *buffer;
  long n;

  /* Read the string_desc structure, describing where in the file to
     find the string.  */
  seek32 (fp, fn, offset);
  length = read32 (fp, fn);
  offset = read32 (fp, fn);

  /* Allocate memory for the string to be read into.  Leave space for
     the NUL on the end.  */
  buffer = (char *) xmalloc (length + 1);

  /* Read in the string.  Complain if there is an error or it comes up
     short.  Add the NUL ourselves.  */
  seek32 (fp, fn, offset);
  n = fread (buffer, 1, length + 1, fp);
  if (n != length + 1)
    {
      if (ferror (fp))
	error (EXIT_FAILURE, errno, _("error while reading \"%s\""), fn);
      error (EXIT_FAILURE, 0, _("file \"%s\" truncated"), fn);
    }
  if (buffer[length] != '\0')
    {
      error (EXIT_FAILURE, 0,
	     _("file \"%s\" contains a not NUL terminated string"), fn);
    }

  /* Return the string to the caller.  */
  *lengthp = length + 1;
  return buffer;
}


/* This function reads an existing .mo file.  */
void
read_mo_file (mlp, fn)
     message_list_ty *mlp;
     const char *fn;
{
  FILE *fp;
  struct mo_file_header header;
  int j;

  if (strcmp (fn, "-") == 0 || strcmp (fn, "/dev/stdin") == 0)
    {
      fp = stdin;
      SET_BINARY (fileno (fp));
    }
  else
    {
      fp = fopen (fn, "rb");
      if (fp == NULL)
	error (EXIT_FAILURE, errno,
	       _("error while opening \"%s\" for reading"), fn);
    }

  /* We must grope the file to determine which endian it is.
     Perversity of the universe tends towards maximum, so it will
     probably not match the currently executing architecture.  */
  endian = MO_BIG_ENDIAN;
  header.magic = read32 (fp, fn);
  if (header.magic != _MAGIC)
    {
      endian = MO_LITTLE_ENDIAN;
      seek32 (fp, fn, 0L);
      header.magic = read32 (fp, fn);
      if (header.magic != _MAGIC)
	{
	unrecognised:
	  error (EXIT_FAILURE, 0, _("file \"%s\" is not in GNU .mo format"),
		 fn);
	}
    }

  /* Fill the structure describing the header.  */
  header.revision = read32 (fp, fn);
  if (header.revision != MO_REVISION_NUMBER)
    goto unrecognised;
  header.nstrings = read32 (fp, fn);
  header.orig_tab_offset = read32 (fp, fn);
  header.trans_tab_offset = read32 (fp, fn);
  header.hash_tab_size = read32 (fp, fn);
  header.hash_tab_offset = read32 (fp, fn);

  for (j = 0; j < header.nstrings; ++j)
    {
      static lex_pos_ty pos = { __FILE__, __LINE__ };
      message_ty *mp;
      char *msgid;
      size_t msgid_len;
      char *msgstr;
      size_t msgstr_len;

      /* Read the msgid.  */
      msgid = string32 (fp, fn, header.orig_tab_offset + j * 8, &msgid_len);

      /* Read the msgstr.  */
      msgstr = string32 (fp, fn, header.trans_tab_offset + j * 8, &msgstr_len);

      mp = message_alloc (msgid,
			  (strlen (msgid) + 1 < msgid_len
			   ? msgid + strlen (msgid) + 1
			   : NULL),
			  msgstr, msgstr_len, &pos);
      message_list_append (mlp, mp);
    }

  if (fp != stdin)
    fclose (fp);
}
