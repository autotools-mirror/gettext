/* Program name management.
   Copyright (C) 2001-2003 Free Software Foundation, Inc.
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

/* Specification.  */
#include "progname.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#if HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <sys/stat.h>

#if defined _WIN32 || defined __WIN32__
# undef WIN32   /* avoid warning on mingw32 */
# define WIN32
#endif

#ifdef WIN32
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif

#include "xreadlink.h"
#include "canonicalize.h"
#include "relocatable.h"

#ifdef NO_XMALLOC
# define xmalloc malloc
#else
# include "xmalloc.h"
#endif

#undef set_program_name


/* String containing name the program is called with.
   To be initialized by main().  */
const char *program_name;

/* Set program_name, based on argv[0].  */
void
set_program_name (const char *argv0)
{
  /* libtool creates a temporary executable whose name is prefixed with
     "lt-".  Remove this prefix here.  */
#ifdef __BEOS__
  /* BeOS also makes argv[0] absolute.  Remove a leading "<dirname>/lt-".  */
  const char *slash;
  const char *base;

  slash = strrchr (argv0, '/');
  base = (slash != NULL ? slash + 1 : argv0);
  if (strncmp (base, "lt-", 3) == 0)
    argv0 = base + 3;
#else
  if (strncmp (argv0, "lt-", 3) == 0)
    argv0 += 3;
#endif
  program_name = argv0;
}


#if ENABLE_RELOCATABLE

#ifdef __linux__
/* File descriptor of the executable.
   (Only used to verify that we find the correct executable.)  */
static int executable_fd = -1;
#endif

/* Tests whether a given pathname may belong to the executable.  */
static bool
maybe_executable (const char *filename)
{
  if (access (filename, X_OK) < 0)
    return false;

#ifdef __linux__
  if (executable_fd >= 0)
    {
      /* If we already have an executable_fd, check that filename points to
	 the same inode.  */
      struct stat statexe;
      struct stat statfile;

      if (fstat (executable_fd, &statexe) >= 0)
	{
	  if (stat (filename, &statfile) < 0)
	    return false;
	  if (!(statfile.st_dev
		&& statfile.st_dev == statexe.st_dev
		&& statfile.st_ino == statexe.st_ino))
	    return false;
	}
    }
#endif

  return true;
}

/* Determine the full pathname of the current executable, freshly allocated.
   Return NULL if unknown.
   Guaranteed to work on Linux and Woe32.  Likely to work on the other
   Unixes (maybe except BeOS), under most conditions.  */
static char *
find_executable (const char *argv0)
{
#ifdef WIN32
  char buf[1024];
  int length = GetModuleFileName (NULL, buf, sizeof (buf));
  if (length < 0)
    return NULL;
  if (!IS_PATH_WITH_DIR (location))
    /* Shouldn't happen.  */
    return NULL;
  return xstrdup (buf);
#else /* Unix */
#ifdef __linux__
  /* The executable is accessible as /proc/<pid>/exe.  In newer Linux
     versions, also as /proc/self/exe.  Linux >= 2.1 provides a symlink
     to the true pathname; older Linux versions give only device and ino,
     enclosed in brackets, which we cannot use here.  */
  {
    char *link;

    link = xreadlink ("/proc/self/exe");
    if (link != NULL && link[0] != '[')
      return link;
    if (executable_fd < 0)
      executable_fd = open ("/proc/self/exe", O_RDONLY, 0);

    {
      char buf[6+10+5];
      sprintf (buf, "/proc/%d/exe", getpid ());
      link = xreadlink (buf);
      if (link != NULL && link[0] != '[')
	return link;
      if (executable_fd < 0)
	executable_fd = open (buf, O_RDONLY, 0);
    }
  }
#endif
  /* Guess the executable's full path.  We assume the executable has been
     called via execlp() or execvp() with properly set up argv[0].  The
     login(1) convention to add a '-' prefix to argv[0] is not supported.  */
  {
    bool has_slash = false;
    {
      const char *p;
      for (p = argv0; *p; p++)
	if (*p == '/')
	  {
	    has_slash = true;
	    break;
	  }
    }
    if (!has_slash)
      {
	/* exec searches paths without slashes in the directory list given
	   by $PATH.  */
	const char *path = getenv ("PATH");

	if (path != NULL)
	  {
	    const char *p;
	    const char *p_next;

	    for (p = path; *p; p = p_next)
	      {
		const char *q;
		size_t p_len;
		char *concat_name;

		for (q = p; *q; q++)
		  if (*q == ':')
		    break;
		p_len = q - p;
		p_next = (*q == '\0' ? q : q + 1);

		/* We have a path item at p, of length p_len.
		   Now concatenate the path item and argv0.  */
		concat_name = (char *) xmalloc (p_len + strlen (argv0) + 2);
#ifdef NO_XMALLOC
		if (concat_name == NULL)
		  return NULL;
#endif
		if (p_len == 0)
		  /* An empty PATH element designates the current directory.  */
		  strcpy (concat_name, argv0);
		else
		  {
		    memcpy (concat_name, p, p_len);
		    concat_name[p_len] = '/';
		    strcpy (concat_name + p_len + 1, argv0);
		  }
		if (maybe_executable (concat_name))
		  return canonicalize_file_name (concat_name);
		free (concat_name);
	      }
	  }
	/* Not found in the PATH, assume the current directory.  */
      }
    /* exec treats paths containing slashes as relative to the current
       directory.  */
    if (maybe_executable (argv0))
      return canonicalize_file_name (argv0);
  }
  /* No way to find the executable.  */
  return NULL;
#endif
}

/* Full pathname of executable, or NULL.  */
static char *executable_fullname;

static void
prepare_relocate (const char *orig_installprefix, const char *orig_installdir,
		  const char *argv0)
{
  const char *curr_prefix;

  /* Determine the full pathname of the current executable.  */
  executable_fullname = find_executable (argv0);

  /* Determine the current installation prefix from it.  */
  curr_prefix = compute_curr_prefix (orig_installprefix, orig_installdir,
				     executable_fullname);
  if (curr_prefix != NULL)
    /* Now pass this prefix to all copies of the relocate.c source file.  */
    set_relocation_prefix (orig_installprefix, curr_prefix);
}

/* Set program_name, based on argv[0], and original installation prefix and
   directory, for relocatability.  */
void
set_program_name_and_installdir (const char *argv0,
				 const char *orig_installprefix,
				 const char *orig_installdir)
{
  const char *argv0_stripped = argv0;

  /* Relocatable programs are renamed to .bin by install-reloc.  Remove
     this suffix here.  */
  {
    size_t argv0_len = strlen (argv0);
    if (argv0_len > 4 && memcmp (argv0 + argv0_len - 4, ".bin", 4) == 0)
      {
	char *shorter = (char *) xmalloc (argv0_len - 4 + 1);
#ifdef NO_XMALLOC
	if (shorter != NULL)
#endif
	  {
	    memcpy (shorter, argv0, argv0_len - 4);
	    shorter[argv0_len - 4] = '\0';
	    argv0_stripped = shorter;
	  }
      }
  }

  set_program_name (argv0_stripped);

  prepare_relocate (orig_installprefix, orig_installdir, argv0);
}

/* Return the full pathname of the current executable, based on the earlier
   call to set_program_name_and_installdir.  Return NULL if unknown.  */
char *
get_full_program_name ()
{
  return executable_fullname;
}

#endif


/* Indicates whether errors and warnings get prefixed with program_name.
   Default is true.  */
bool error_with_progname = true;

/* Print program_name prefix on stderr if and only if error_with_progname
   is true.  */
void
maybe_print_progname ()
{
  if (error_with_progname)
    fprintf (stderr, "%s: ", program_name);
}
