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

#ifndef _PIPE_H
#define _PIPE_H

/* Get pid_t.  */
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/types.h>

/* Open a pipe for output to a child process.
 * The child's stdout goes to a file.
 *
 *           write       system                read
 *    parent  ->   fd[0]   ->   STDIN_FILENO    ->   child
 *
 */
extern pid_t create_pipe_out PARAMS ((const char *progname,
				      const char *prog_path, char **prog_argv,
				      const char *prog_stdout,
				      int fd[1]));

/* Open a pipe for input from a child process.
 * The child's stdin comes to a file.
 *
 *           read        system                write
 *    parent  <-   fd[0]   <-   STDOUT_FILENO   <-   child
 *
 */
extern pid_t create_pipe_in PARAMS ((const char *progname,
				     const char *prog_path, char **prog_argv,
				     const char *prog_stdin,
				     int fd[1]));

/* Open a bidirectional pipe.
 *
 *           write       system                read
 *    parent  ->   fd[1]   ->   STDIN_FILENO    ->   child
 *    parent  <-   fd[0]   <-   STDOUT_FILENO   <-   child
 *           read        system                write
 *
 */
extern pid_t create_pipe_bidi PARAMS ((const char *progname,
				       const char *prog_path, char **prog_argv,
				       int fd[2]));

#endif /* _PIPE_H */
