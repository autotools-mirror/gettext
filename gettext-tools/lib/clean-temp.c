/* Temporary directories and temporary files with automatic cleanup.
   Copyright (C) 2001, 2003, 2006 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2006.

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
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  */


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/* Specification.  */
#include "clean-temp.h"

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#include "error.h"
#include "fatal-signal.h"
#include "pathmax.h"
#include "tmpdir.h"
#include "mkdtemp.h"
#include "xalloc.h"
#include "xallocsa.h"
#include "gettext.h"

#define _(str) gettext (str)


/* The use of 'volatile' in the types below (and ISO C 99 section 5.1.2.3.(5))
   ensure that while constructing or modifying the data structures, the field
   values are written to memory in the order of the C statements.  So the
   signal handler can rely on these field values to be up to date.  */

/* Registry for a single temporary directory.
   'struct temp_dir' from the public header file overlaps with this.  */
struct tempdir
{
  /* The absolute pathname of the directory.  */
  char * volatile dirname;
  /* Absolute pathnames of subdirectories.  */
  char * volatile * volatile subdir;
  size_t volatile subdir_count;
  size_t subdir_allocated;
  /* Absolute pathnames of files.  */
  char * volatile * volatile file;
  size_t volatile file_count;
  size_t file_allocated;
};

/* List of all temporary directories.  */
static struct
{
  struct tempdir * volatile * volatile tempdir_list;
  size_t volatile tempdir_count;
  size_t tempdir_allocated;
} cleanup_list /* = { NULL, 0, 0 } */;

/* The signal handler.  It gets called asynchronously.  */
static void
cleanup ()
{
  size_t i;

  for (i = 0; i < cleanup_list.tempdir_count; i++)
    {
      struct tempdir *dir = cleanup_list.tempdir_list[i];

      if (dir != NULL)
	{
	  size_t j;

	  /* First cleanup the files in the subdirectories.  */
	  for (j = dir->file_count; ; )
	    if (j > 0)
	      {
		const char *file = dir->file[--j];
		if (file != NULL)
		  unlink (file);
	      }
	    else
	      break;

	  /* Then cleanup the subdirectories.  */
	  for (j = dir->subdir_count; ; )
	    if (j > 0)
	      {
		const char *subdir = dir->subdir[--j];
		if (subdir != NULL)
		  rmdir (subdir);
	      }
	    else
	      break;

	  /* Then cleanup the temporary directory itself.  */
	  rmdir (dir->dirname);
	}
    }
}

/* Create a temporary directory.
   PREFIX is used as a prefix for the name of the temporary directory. It
   should be short and still give an indication about the program.
   Return a fresh 'struct temp_dir' on success.  Upon error, an error message
   is shown and NULL is returned.  */
struct temp_dir *
create_temp_dir (const char *prefix)
{
  struct tempdir * volatile *tmpdirp = NULL;
  struct tempdir *tmpdir;
  size_t i;
  char *template;
  char *tmpdirname;

  /* See whether it can take the slot of an earlier temporary directory
     already cleaned up.  */
  for (i = 0; i < cleanup_list.tempdir_count; i++)
    if (cleanup_list.tempdir_list[i] == NULL)
      {
	tmpdirp = &cleanup_list.tempdir_list[i];
	break;
      }
  if (tmpdirp == NULL)
    {
      /* See whether the array needs to be extended.  */
      if (cleanup_list.tempdir_count == cleanup_list.tempdir_allocated)
	{
	  /* Note that we cannot use xrealloc(), because then the cleanup()
	     function could access an already deallocated array.  */
	  struct tempdir * volatile *old_array = cleanup_list.tempdir_list;
	  size_t old_allocated = cleanup_list.tempdir_allocated;
	  size_t new_allocated = 2 * cleanup_list.tempdir_allocated + 1;
	  struct tempdir * volatile *new_array =
	    (struct tempdir * volatile *)
	    xmalloc (new_allocated * sizeof (struct tempdir * volatile));

	  if (old_allocated == 0)
	    /* First use of this facility.  Register the cleanup handler.  */
	    at_fatal_signal (&cleanup);
	  else
	    {
	      /* Don't use memcpy() here, because memcpy takes non-volatile
		 arguments and is therefore not guaranteed to complete all
		 memory stores before the next statement.  */
	      size_t k;

	      for (k = 0; k < old_allocated; k++)
		new_array[k] = old_array[k];
	    }

	  cleanup_list.tempdir_list = new_array;
	  cleanup_list.tempdir_allocated = new_allocated;

	  /* Now we can free the old array.  */
	  if (old_array != NULL)
	    free ((struct tempdir **) old_array);
	}

      tmpdirp = &cleanup_list.tempdir_list[cleanup_list.tempdir_count];
      /* Initialize *tmpdirp before incrementing tempdir_count, so that
	 cleanup() will skip this entry before it is fully initialized.  */
      *tmpdirp = NULL;
      cleanup_list.tempdir_count++;
    }

  /* Initialize a 'struct tmpdir'.  */
  tmpdir = (struct tempdir *) xmalloc (sizeof (struct tempdir));
  tmpdir->dirname = NULL;
  tmpdir->subdir = NULL;
  tmpdir->subdir_count = 0;
  tmpdir->subdir_allocated = 0;
  tmpdir->file = NULL;
  tmpdir->file_count = 0;
  tmpdir->file_allocated = 0;

  /* Create the temporary directory.  */
  template = (char *) xallocsa (PATH_MAX);
  if (path_search (template, PATH_MAX, NULL, prefix, true))
    {
      error (0, errno,
	     _("cannot find a temporary directory, try setting $TMPDIR"));
      goto quit;
    }
  block_fatal_signals ();
  tmpdirname = mkdtemp (template);
  if (tmpdirname != NULL)
    {
      tmpdir->dirname = tmpdirname;
      *tmpdirp = tmpdir;
    }
  unblock_fatal_signals ();
  if (tmpdirname == NULL)
    {
      error (0, errno,
	     _("cannot create a temporary directory using template \"%s\""),
	     template);
      goto quit;
    }
  /* Replace tmpdir->dirname with a copy that has indefinite extent.
     We cannot do this inside the block_fatal_signals/unblock_fatal_signals
     block because then the cleanup handler would not remove the directory
     if xstrdup fails.  */
  tmpdir->dirname = xstrdup (tmpdirname);
  freesa (template);
  return (struct temp_dir *) tmpdir;

 quit:
  freesa (template);
  return NULL;
}

/* Register the given ABSOLUTE_FILE_NAME as being a file inside DIR, that
   needs to be removed before DIR can be removed.
   Should be called before the file ABSOLUTE_FILE_NAME is created.  */
void
enqueue_temp_file (struct temp_dir *dir,
		   const char *absolute_file_name)
{
  struct tempdir *tmpdir = (struct tempdir *)dir;
  size_t j;

  /* See whether it can take the slot of an earlier file already dequeued.  */
  for (j = 0; j < tmpdir->file_count; j++)
    if (tmpdir->file[j] == NULL)
      {
	tmpdir->file[j] = xstrdup (absolute_file_name);
	return;
      }
  /* See whether the array needs to be extended.  */
  if (tmpdir->file_count == tmpdir->file_allocated)
    {
      /* Note that we cannot use xrealloc(), because then the cleanup()
	 function could access an already deallocated array.  */
      char * volatile * old_array = tmpdir->file;
      size_t old_allocated = tmpdir->file_allocated;
      size_t new_allocated = 2 * tmpdir->file_allocated + 1;
      char * volatile * new_array =
	(char * volatile *) xmalloc (new_allocated * sizeof (char * volatile));
      size_t k;

      /* Don't use memcpy() here, because memcpy takes non-volatile arguments
	 and is therefore not guaranteed to complete all memory stores before
	 the next statement.  */
      for (k = 0; k < old_allocated; k++)
	new_array[k] = old_array[k];

      tmpdir->file = new_array;
      tmpdir->file_allocated = new_allocated;

      /* Now we can free the old array.  */
      if (old_array != NULL)
	free ((char **) old_array);
    }

  /* Initialize the pointer before incrementing file_count, so that cleanup()
     will not see this entry before it is fully initialized.  */
  tmpdir->file[tmpdir->file_count] = xstrdup (absolute_file_name);
  tmpdir->file_count++;
}

/* Unregister the given ABSOLUTE_FILE_NAME as being a file inside DIR, that
   needs to be removed before DIR can be removed.
   Should be called when the file ABSOLUTE_FILE_NAME could not be created.  */
void
dequeue_temp_file (struct temp_dir *dir,
		   const char *absolute_file_name)
{
  struct tempdir *tmpdir = (struct tempdir *)dir;
  size_t j;

  for (j = 0; j < tmpdir->file_count; j++)
    if (tmpdir->file[j] != NULL
	&& strcmp (tmpdir->file[j], absolute_file_name) == 0)
      {
	/* Clear tmpdir->file[j].  */
	char *old_string = tmpdir->file[j];
	if (j + 1 == tmpdir->file_count)
	  {
	    while (j > 0 && tmpdir->file[j - 1] == NULL)
	      j--;
	    tmpdir->file_count = j;
	  }
	else
	  tmpdir->file[j] = NULL;
	/* Now only we can free the old tmpdir->file[j].  */
	free (old_string);
      }
}

/* Register the given ABSOLUTE_DIR_NAME as being a subdirectory inside DIR,
   that needs to be removed before DIR can be removed.
   Should be called before the subdirectory ABSOLUTE_DIR_NAME is created.  */
void
enqueue_temp_subdir (struct temp_dir *dir,
		     const char *absolute_dir_name)
{
  struct tempdir *tmpdir = (struct tempdir *)dir;

  /* Reusing the slot of an earlier subdirectory already dequeued is not
     possible here, because the order of the subdirectories matter.  */
  /* See whether the array needs to be extended.  */
  if (tmpdir->subdir_count == tmpdir->subdir_allocated)
    {
      /* Note that we cannot use xrealloc(), because then the cleanup()
	 function could access an already deallocated array.  */
      char * volatile * old_array = tmpdir->subdir;
      size_t old_allocated = tmpdir->subdir_allocated;
      size_t new_allocated = 2 * tmpdir->subdir_allocated + 1;
      char * volatile * new_array =
	(char * volatile *) xmalloc (new_allocated * sizeof (char * volatile));
      size_t k;

      /* Don't use memcpy() here, because memcpy takes non-volatile arguments
	 and is therefore not guaranteed to complete all memory stores before
	 the next statement.  */
      for (k = 0; k < old_allocated; k++)
	new_array[k] = old_array[k];

      tmpdir->subdir = new_array;
      tmpdir->subdir_allocated = new_allocated;

      /* Now we can free the old array.  */
      if (old_array != NULL)
	free ((char **) old_array);
    }

  /* Initialize the pointer before incrementing subdir_count, so that cleanup()
     will not see this entry before it is fully initialized.  */
  tmpdir->subdir[tmpdir->subdir_count] = xstrdup (absolute_dir_name);
  tmpdir->subdir_count++;
}

/* Unregister the given ABSOLUTE_DIR_NAME as being a subdirectory inside DIR,
   that needs to be removed before DIR can be removed.
   Should be called when the subdirectory ABSOLUTE_DIR_NAME could not be
   created.  */
void
dequeue_temp_subdir (struct temp_dir *dir,
		     const char *absolute_dir_name)
{
  struct tempdir *tmpdir = (struct tempdir *)dir;
  size_t j;

  for (j = 0; j < tmpdir->subdir_count; j++)
    if (tmpdir->subdir[j] != NULL
	&& strcmp (tmpdir->subdir[j], absolute_dir_name) == 0)
      {
	/* Clear tmpdir->subdir[j].  */
	char *old_string = tmpdir->subdir[j];
	bool anything_beyond_index_j = false;
	size_t k;

	for (k = j + 1; k < tmpdir->subdir_count; k++)
	  if (tmpdir->subdir[k] != NULL)
	    {
	      anything_beyond_index_j = true;
	      break;
	    }
	if (anything_beyond_index_j)
	  tmpdir->subdir[j] = NULL;
	else
	  tmpdir->subdir_count = j;
	/* Now only we can free the old tmpdir->subdir[j].  */
	free (old_string);
      }
}

/* Remove the given ABSOLUTE_FILE_NAME and unregister it.  */
void
cleanup_temp_file (struct temp_dir *dir,
		   const char *absolute_file_name)
{
  unlink (absolute_file_name);
  dequeue_temp_file (dir, absolute_file_name);
}

/* Remove the given ABSOLUTE_DIR_NAME and unregister it.  */
void
cleanup_temp_subdir (struct temp_dir *dir,
		     const char *absolute_dir_name)
{
  rmdir (absolute_dir_name);
  dequeue_temp_subdir (dir, absolute_dir_name);
}

/* Remove all registered files and subdirectories inside DIR.  */
void
cleanup_temp_dir_contents (struct temp_dir *dir)
{
  struct tempdir *tmpdir = (struct tempdir *)dir;
  size_t j;

  /* First cleanup the files in the subdirectories.  */
  for (j = tmpdir->file_count; ; )
    if (j > 0)
      {
	char *file = tmpdir->file[--j];
	if (file != NULL)
	  unlink (file);
	tmpdir->file_count = j;
	/* Now only we can free file.  */
	if (file != NULL)
	  free (file);
      }
    else
      break;

  /* Then cleanup the subdirectories.  */
  for (j = tmpdir->subdir_count; ; )
    if (j > 0)
      {
	char *subdir = tmpdir->subdir[--j];
	if (subdir != NULL)
	  rmdir (subdir);
	tmpdir->subdir_count = j;
	/* Now only we can free subdir.  */
	if (subdir != NULL)
	  free (subdir);
      }
    else
      break;
}

/* Remove all registered files and subdirectories inside DIR and DIR itself.
   DIR cannot be used any more after this call.  */
void
cleanup_temp_dir (struct temp_dir *dir)
{
  struct tempdir *tmpdir = (struct tempdir *)dir;
  size_t i;

  cleanup_temp_dir_contents (dir);
  rmdir (tmpdir->dirname);

  for (i = 0; i < cleanup_list.tempdir_count; i++)
    if (cleanup_list.tempdir_list[i] == tmpdir)
      {
	/* Remove cleanup_list.tempdir_list[i].  */
	if (i + 1 == cleanup_list.tempdir_count)
	  {
	    while (i > 0 && cleanup_list.tempdir_list[i - 1] == NULL)
	      i--;
	    cleanup_list.tempdir_count = i;
	  }
	else
	  cleanup_list.tempdir_list[i] = NULL;
	/* Now only we can free the tmpdir->dirname and tmpdir itself.  */
	free (tmpdir->dirname);
	free (tmpdir);
	return;
      }

  /* The user passed an invalid DIR argument.  */
  abort ();
}
