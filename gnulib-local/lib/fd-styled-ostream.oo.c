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

#include <config.h>

/* Specification.  */
#include "fd-styled-ostream.h"

#include "fd-ostream.h"

#include "xalloc.h"


struct fd_styled_ostream : struct styled_ostream
{
fields:
  /* The destination stream.  */
  fd_ostream_t destination;
};

/* Implementation of ostream_t methods.  */

static void
fd_styled_ostream::write_mem (fd_styled_ostream_t stream,
                              const void *data, size_t len)
{
  fd_ostream_write_mem (stream->destination, data, len);
}

static void
fd_styled_ostream::flush (fd_styled_ostream_t stream, ostream_flush_scope_t scope)
{
  fd_ostream_flush (stream->destination, scope);
}

static void
fd_styled_ostream::free (fd_styled_ostream_t stream)
{
  fd_ostream_free (stream->destination);
  free (stream);
}

/* Implementation of styled_ostream_t methods.  */

static void
fd_styled_ostream::begin_use_class (fd_styled_ostream_t stream,
                                    const char *classname)
{
}

static void
fd_styled_ostream::end_use_class (fd_styled_ostream_t stream,
                                  const char *classname)
{
}

static void
fd_styled_ostream::flush_to_current_style (fd_styled_ostream_t stream)
{
  fd_ostream_flush (stream->destination, FLUSH_THIS_STREAM);
}

/* Constructor.  */

fd_styled_ostream_t
fd_styled_ostream_create (int fd, const char *filename)
{
  fd_styled_ostream_t stream =
    XMALLOC (struct fd_styled_ostream_representation);

  stream->base.base.vtable = &fd_styled_ostream_vtable;
  stream->destination = fd_ostream_create (fd, filename, true);

  return stream;
}
