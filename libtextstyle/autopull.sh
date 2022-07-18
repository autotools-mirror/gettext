#!/bin/sh
# Convenience script for fetching auxiliary files that are omitted from
# the version control repository of this package.

# Copyright (C) 2003-2022 Free Software Foundation, Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# Usage: ./autopull.sh

TEXINFO_VERSION=6.5

mkdir -p build-aux
# texinfo.tex
# The most recent snapshot of it is available in the gnulib repository.
# But this is a snapshot, with all possible dangers.
# A stable release of it is available through "automake --add-missing --copy",
# but that depends on the Automake version. So take the version which matches
# the latest stable texinfo release.
if test ! -f build-aux/texinfo.tex; then
  { wget -q --timeout=5 -O build-aux/texinfo.tex.tmp 'https://git.savannah.gnu.org/gitweb/?p=texinfo.git;a=blob_plain;f=doc/texinfo.tex;hb=refs/tags/texinfo-'"$TEXINFO_VERSION" \
      && mv build-aux/texinfo.tex.tmp build-aux/texinfo.tex; \
  } || rm -f build-aux/texinfo.tex.tmp
fi
