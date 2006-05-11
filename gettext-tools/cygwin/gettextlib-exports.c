/* List of exported symbols of libgettextlib on Cygwin.
   Copyright (C) 2006 Free Software Foundation, Inc.
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

#include "cygwin/export.h"

VARIABLE(argmatch_die)
VARIABLE(error_message_count)
VARIABLE(error_one_per_line)
VARIABLE(error_print_progname)
VARIABLE(error_with_progname)
VARIABLE(exit_failure)
VARIABLE(program_name)
VARIABLE(rpl_optarg)
VARIABLE(rpl_optind)
VARIABLE(simple_backup_suffix)

#if 0 /* not needed - we use --export-all-symbols */
FUNCTION(_obstack_begin)
FUNCTION(_obstack_newchunk)
FUNCTION(concatenated_pathname)
FUNCTION(c_strcasecmp)
FUNCTION(c_strncasecmp)
FUNCTION(cleanup_temp_dir)
FUNCTION(close_stdout)
FUNCTION(compile_csharp_class)
FUNCTION(compile_java_class)
FUNCTION(copy_file_preserving)
FUNCTION(create_pipe_bidi)
FUNCTION(create_pipe_in)
FUNCTION(create_pipe_out)
FUNCTION(create_temp_dir)
FUNCTION(dequeue_temp_file)
FUNCTION(dequeue_temp_subdir)
FUNCTION(enqueue_temp_file)
FUNCTION(enqueue_temp_subdir)
FUNCTION(error)
FUNCTION(error_at_line)
FUNCTION(execute)
FUNCTION(execute_csharp_program)
FUNCTION(execute_java_class)
FUNCTION(find_backup_file_name)
FUNCTION(find_in_path)
FUNCTION(freesa)
FUNCTION(fstrcmp)
FUNCTION(full_write)
FUNCTION(fwriteerror)
FUNCTION(gcd)
FUNCTION(gnu_basename)
FUNCTION(gnu_getline)
FUNCTION(gnu_stpncpy)
FUNCTION(hash_destroy)
FUNCTION(hash_find_entry)
FUNCTION(hash_init)
FUNCTION(hash_insert_entry)
FUNCTION(hash_iterate)
FUNCTION(hash_iterate_modify)
FUNCTION(hash_set_value)
FUNCTION(iconv_string)
FUNCTION(maybe_print_progname)
FUNCTION(mbs_width_linebreaks)
FUNCTION(multiline_error)
FUNCTION(multiline_warning)
FUNCTION(next_prime)
FUNCTION(obstack_free)
FUNCTION(proper_name)
FUNCTION(proper_name_utf8)
FUNCTION(rpl_getopt_long)
FUNCTION(set_program_name)
FUNCTION(shell_quote_argv)
FUNCTION(stpcpy)
FUNCTION(uc_width)
FUNCTION(wait_subprocess)
FUNCTION(xalloc_die)
FUNCTION(xasprintf)
FUNCTION(xcalloc)
FUNCTION(xget_version)
FUNCTION(xmalloc)
FUNCTION(xmallocsa)
FUNCTION(xrealloc)
FUNCTION(xsetenv)
FUNCTION(xstrdup)
#endif
