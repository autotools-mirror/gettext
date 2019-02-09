/* Output stream with no-op styling, referring to a file descriptor.
   Copyright (C) 2006, 2019 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2019.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#ifndef _FD_STYLED_OSTREAM_H
#define _FD_STYLED_OSTREAM_H

#include "styled-ostream.h"


struct fd_styled_ostream : struct styled_ostream
{
methods:
};


#ifdef __cplusplus
extern "C" {
#endif


/* Create an output stream referring to the file descriptor FD, that supports
   the styling operations as no-ops.
   FILENAME is used only for error messages.
   Note that the resulting stream must be closed before FD can be closed.  */
extern fd_styled_ostream_t
       fd_styled_ostream_create (int fd, const char *filename);


#ifdef __cplusplus
}
#endif

#endif /* _FD_STYLED_OSTREAM_H */
