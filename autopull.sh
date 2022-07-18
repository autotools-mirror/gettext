#!/bin/sh
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

# Convenience script for fetching auxiliary files that are omitted from
# the version control repository of this package.
#
# This script requires:
#   - Wget
#   - XZ Utils
#
# In addition, it fetches the archive.dir.tar.gz file, which contains
# data files used by the autopoint program.  If you already have the
# file, place it under gettext-tools/misc, before running this script.

# Usage: ./autopull.sh

# Nuisances.
(unset CDPATH) >/dev/null 2>&1 && unset CDPATH

./gitsub.sh pull || exit 1

# Fetch gettext-tools/misc/archive.dir.tar.
if ! test -f gettext-tools/misc/archive.dir.tar; then
  if ! test -f gettext-tools/misc/archive.dir.tar.xz; then
    echo "$0: getting gettext-tools/misc/archive.dir.tar..."
    wget -q --timeout=5 -O gettext-tools/misc/archive.dir.tar.xz-t "https://alpha.gnu.org/gnu/gettext/archive.dir-latest.tar.xz" \
      && mv gettext-tools/misc/archive.dir.tar.xz-t gettext-tools/misc/archive.dir.tar.xz
    retval=$?
    rm -f gettext-tools/misc/archive.dir.tar.xz-t
    test $retval -eq 0 || exit $retval
  fi
  xz -d -c < gettext-tools/misc/archive.dir.tar.xz > gettext-tools/misc/archive.dir.tar-t \
    && mv gettext-tools/misc/archive.dir.tar-t gettext-tools/misc/archive.dir.tar
  retval=$?
  rm -f gettext-tools/misc/archive.dir.tar-t
  test $retval -eq 0 || exit $retval
fi

dir0=`pwd`

echo "$0: generating files in libtextstyle..."
cd libtextstyle
./autopull.sh || exit $?
cd "$dir0"

echo "$0: done.  Now you can run './autogen.sh'."
