#!/bin/sh
# Copyright (C) 2003-2014 Free Software Foundation, Inc.
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
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# This script regenerates materials included in the released tarball,
# such as PO files and manual pages.
#
# Before running this script, you need to have both gettext-runtime
# and gettext-tools built in the source tree.  Parallel build trees
# are not supported.
#
# Usage: ./update-po.sh

# Nuisances.
(unset CDPATH) >/dev/null 2>&1 && unset CDPATH

test -f gettext-runtime/config.status \
  && test -f gettext-tools/config.status \
  && test -f gettext-tools/examples/config.status \
  || { echo "$0: *** build the source before running this script"; exit 1; }

# Adjust the gettext_datadir and PATH envvars and run config.status
# --recheck to prefer the included version of gettext-tools to the
# installed version.
prepend_path=
case ${gettext_builddir--} in
  -)
    gettext_builddir=$PWD/gettext-tools/src
    prepend_path="$gettext_builddir:$prepend_path"
    ;;
esac

case ${gettext_datadir--} in
  -)
    gettext_datadir=$PWD/gettext-tools/misc
    export gettext_datadir
    prepend_path="$gettext_datadir:$prepend_path"
    ;;
esac

test -n "$prepend_path" && PATH="$prepend_path:$PATH"
export PATH

echo "$0: updating PO files in gettext-runtime..."
(cd gettext-runtime \
 && ./config.status --recheck \
 && ./config.status po/Makefile.in po-directories \
 && (cd po && make update-po)) || exit $?

echo "$0: updating PO files in gettext-tools..."
(cd gettext-tools \
 && ./config.status --recheck \
 && ./config.status po/Makefile.in po-directories \
 && (cd po && make update-po)) || exit $?

echo "$0: updating manual pages in gettext-tools..."
(cd gettext-tools/man && make update-man1) || exit $?

echo "$0: updating PO files in gettext-tools/examples..."
(cd gettext-tools/examples \
 && ./config.status --recheck \
 && ./config.status po/Makefile \
 && (cd po && make update-po)) || exit $?
