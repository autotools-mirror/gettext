/* Creation of subprocesses, communicating via pipes.
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

/* Specification.  */
#include "pipe.h"

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef HAVE_POSIX_SPAWN
# include <spawn.h>
#else
# ifdef HAVE_VFORK_H
#  include <vfork.h>
# endif
#endif

#include "error.h"
#include "libgettext.h"

#ifndef STDIN_FILENO
# define STDIN_FILENO 0
#endif
#ifndef STDOUT_FILENO
# define STDOUT_FILENO 1
#endif

#define _(str) gettext (str)


/* Prototypes for local functions.  Needed to ensure compiler checking of
   function argument counts despite of K&R C function definition syntax.  */
#ifdef EINTR
static inline int nonintr_close PARAMS ((int fd));
static inline int nonintr_open PARAMS ((const char *pathname, int oflag,
					mode_t mode));
#endif


#ifdef EINTR

/* EINTR handling for close(), open().
   These functions can return -1/EINTR even though we don't have any
   signal handlers set up, namely when we get interrupted via SIGSTOP.  */

static inline int
nonintr_close (fd)
     int fd;
{
  int retval;

  do
    retval = close (fd);
  while (retval < 0 && errno == EINTR);

  return retval;
}
#define close nonintr_close

static inline int
nonintr_open (pathname, oflag, mode)
     const char *pathname;
     int oflag;
     mode_t mode;
{
  int retval;

  do
    retval = open (pathname, oflag, mode);
  while (retval < 0 && errno == EINTR);

  return retval;
}
#define open nonintr_open

#endif


/* Open a pipe for input from a child process.
 * The child's stdin comes to a file.
 *
 *           read        system                write
 *    parent  <-   fd[0]   <-   STDOUT_FILENO   <-   child
 *
 */
pid_t
create_pipe_in (progname, prog_path, prog_argv, prog_stdin, fd)
     const char *progname;
     const char *prog_path;
     char **prog_argv;
     const char *prog_stdin;
     int fd[1];
{
  int ifd[2];
#if HAVE_POSIX_SPAWN
  posix_spawn_file_actions_t actions;
  int err;
  pid_t child;
#else
  int child;
#endif

  if (pipe (ifd) < 0)
    error (EXIT_FAILURE, errno, _("cannot create pipe"));
/* Data flow diagram:
 *
 *           read        system         write
 *    parent  <-  ifd[0]   <-   ifd[1]   <-   child
 */

#if HAVE_POSIX_SPAWN
  if ((err = posix_spawn_file_actions_init (&actions)) != 0
      || (err = posix_spawn_file_actions_adddup2 (&actions,
						  ifd[1], STDOUT_FILENO)) != 0
      || (err = posix_spawn_file_actions_addclose (&actions, ifd[1])) != 0
      || (err = posix_spawn_file_actions_addclose (&actions, ifd[0])) != 0
      || (err = posix_spawn_file_actions_addopen (&actions, STDIN_FILENO,
						  prog_stdin, O_RDONLY,
						  0)) != 0
      || (err = posix_spawnp (&child, prog_path, &actions, NULL, prog_argv,
			      environ)) != 0)
    error (EXIT_FAILURE, err, _("%s subprocess failed"), progname);
  posix_spawn_file_actions_destroy (&actions);
#else
  /* Use vfork() instead of fork() for efficiency.  */
  if ((child = vfork ()) == 0)
    {
      /* Child process code.  */
      int stdinfd;

      if (dup2 (ifd[1], STDOUT_FILENO) >= 0
	  && close (ifd[1]) >= 0
	  && close (ifd[0]) >= 0
	  && (stdinfd = open (prog_stdin, O_RDONLY, 0)) >= 0
	  && (stdinfd == STDIN_FILENO
	      || (dup2 (stdinfd, STDIN_FILENO) >= 0
		  && close (stdinfd) >= 0)))
	execvp (prog_path, prog_argv);
      _exit (-1);
    }
  if (child == -1)
    error (EXIT_FAILURE, errno, _("%s subprocess failed"), progname);
#endif
  close (ifd[1]);

  fd[0] = ifd[0];
  return child;
}
