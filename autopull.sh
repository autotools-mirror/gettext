#!/bin/sh
# Copyright (C) 2003-2025 Free Software Foundation, Inc.
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

# func_git_clone_shallow SUBDIR URL REVISION
func_git_clone_shallow ()
{
  # Only want a shallow checkout of REVISION, but git does not
  # support cloning by commit hash. So attempt a shallow fetch by
  # commit hash to minimize the amount of data downloaded and changes
  # needed to be processed, which can drastically reduce download and
  # processing time for checkout. If the fetch by commit fails, a
  # shallow fetch cannot be performed because we do not know what the
  # depth of the commit is without fetching all commits. So fall back
  # to fetching all commits.
  # REVISION can be a commit id, a tag name, or a branch name.
  mkdir -p "$1"
  # Use a -c option to silence an annoying message
  # "hint: Using 'master' as the name for the initial branch."
  # (cf. <https://stackoverflow.com/questions/65524512/>).
  git -C "$1" -c init.defaultBranch=master init
  git -C "$1" remote add origin "$2"
  if git -C "$1" fetch --depth 1 origin "$3"; then
    # "git fetch" of the specific commit succeeded.
    git -C "$1" reset --hard FETCH_HEAD || { rm -rf "$1"; exit 1; }
    # "git fetch" does not fetch tags (at least in git version 2.43).
    # If REVISION is a tag (not a commit id or branch name),
    # add the tag explicitly.
    revision=`git -C "$1" log -1 --pretty=format:%H`
    branch=`LC_ALL=C git -C "$1" remote show origin \
            | sed -n -e 's/^    \([^ ]*\) * tracked$/\1/p'`
    test "$revision" = "$3" || test "$branch" = "$3" || git -C "$1" tag "$3"
  else
    # Fetch the entire repository.
    git -C "$1" fetch origin || { rm -rf "$1"; exit 1; }
    git -C "$1" checkout "$3" || { rm -rf "$1"; exit 1; }
  fi
}

# Fetch the compilable (mostly generated) tree-sitter source code.
TREE_SITTER_VERSION=0.23.2
TREE_SITTER_GO_VERSION=0.23.4
TREE_SITTER_RUST_VERSION=0.23.2
TREE_SITTER_TYPESCRIPT_VERSION=0.23.2
TREE_SITTER_D_VERSION=0.8.2
# Cache the relevant source code. Erase the rest of the tree-sitter projects.
test -d gettext-tools/tree-sitter-$TREE_SITTER_VERSION || {
  func_git_clone_shallow tree-sitter https://github.com/tree-sitter/tree-sitter.git v$TREE_SITTER_VERSION
  (cd tree-sitter && patch -p1) < gettext-tools/build-aux/tree-sitter-portability.diff
  mkdir gettext-tools/tree-sitter-$TREE_SITTER_VERSION
  mv tree-sitter/LICENSE gettext-tools/tree-sitter-$TREE_SITTER_VERSION/LICENSE
  mv tree-sitter/lib gettext-tools/tree-sitter-$TREE_SITTER_VERSION/lib
  rm -rf tree-sitter
}
test -d gettext-tools/tree-sitter-go-$TREE_SITTER_GO_VERSION || {
  func_git_clone_shallow tree-sitter-go https://github.com/tree-sitter/tree-sitter-go.git v$TREE_SITTER_GO_VERSION
  (cd tree-sitter-go && patch -p1) < gettext-tools/build-aux/tree-sitter-go-portability.diff
  mkdir gettext-tools/tree-sitter-go-$TREE_SITTER_GO_VERSION
  mv tree-sitter-go/LICENSE gettext-tools/tree-sitter-go-$TREE_SITTER_GO_VERSION/LICENSE
  mv tree-sitter-go/src gettext-tools/tree-sitter-go-$TREE_SITTER_GO_VERSION/src
  mv gettext-tools/tree-sitter-go-$TREE_SITTER_GO_VERSION/src/parser.c gettext-tools/tree-sitter-go-$TREE_SITTER_GO_VERSION/src/go-parser.c
  rm -rf tree-sitter-go
}
test -d gettext-tools/tree-sitter-rust-$TREE_SITTER_RUST_VERSION || {
  func_git_clone_shallow tree-sitter-rust https://github.com/tree-sitter/tree-sitter-rust.git v$TREE_SITTER_RUST_VERSION
  (cd tree-sitter-rust && patch -p1) < gettext-tools/build-aux/tree-sitter-rust-portability.diff
  mkdir gettext-tools/tree-sitter-rust-$TREE_SITTER_RUST_VERSION
  mv tree-sitter-rust/LICENSE gettext-tools/tree-sitter-rust-$TREE_SITTER_RUST_VERSION/LICENSE
  mv tree-sitter-rust/src gettext-tools/tree-sitter-rust-$TREE_SITTER_RUST_VERSION/src
  mv gettext-tools/tree-sitter-rust-$TREE_SITTER_RUST_VERSION/src/parser.c gettext-tools/tree-sitter-rust-$TREE_SITTER_RUST_VERSION/src/rust-parser.c
  mv gettext-tools/tree-sitter-rust-$TREE_SITTER_RUST_VERSION/src/scanner.c gettext-tools/tree-sitter-rust-$TREE_SITTER_RUST_VERSION/src/rust-scanner.c
  rm -rf tree-sitter-rust
}
test -d gettext-tools/tree-sitter-typescript-$TREE_SITTER_TYPESCRIPT_VERSION || {
  func_git_clone_shallow tree-sitter-typescript https://github.com/tree-sitter/tree-sitter-typescript.git v$TREE_SITTER_TYPESCRIPT_VERSION
  (cd tree-sitter-typescript && patch -p1) < gettext-tools/build-aux/tree-sitter-typescript-portability.diff
  mkdir gettext-tools/tree-sitter-typescript-$TREE_SITTER_TYPESCRIPT_VERSION
  mkdir gettext-tools/tree-sitter-typescript-$TREE_SITTER_TYPESCRIPT_VERSION/common
  mkdir gettext-tools/tree-sitter-typescript-$TREE_SITTER_TYPESCRIPT_VERSION/typescript
  mkdir gettext-tools/tree-sitter-typescript-$TREE_SITTER_TYPESCRIPT_VERSION/tsx
  mv tree-sitter-typescript/LICENSE gettext-tools/tree-sitter-typescript-$TREE_SITTER_TYPESCRIPT_VERSION/LICENSE
  mv tree-sitter-typescript/common/scanner.h gettext-tools/tree-sitter-typescript-$TREE_SITTER_TYPESCRIPT_VERSION/common/scanner.h
  mv tree-sitter-typescript/typescript/src gettext-tools/tree-sitter-typescript-$TREE_SITTER_TYPESCRIPT_VERSION/typescript/src
  mv tree-sitter-typescript/tsx/src gettext-tools/tree-sitter-typescript-$TREE_SITTER_TYPESCRIPT_VERSION/tsx/src
  mv gettext-tools/tree-sitter-typescript-$TREE_SITTER_TYPESCRIPT_VERSION/typescript/src/parser.c gettext-tools/tree-sitter-typescript-$TREE_SITTER_TYPESCRIPT_VERSION/typescript/src/ts-parser.c
  mv gettext-tools/tree-sitter-typescript-$TREE_SITTER_TYPESCRIPT_VERSION/typescript/src/scanner.c gettext-tools/tree-sitter-typescript-$TREE_SITTER_TYPESCRIPT_VERSION/typescript/src/ts-scanner.c
  mv gettext-tools/tree-sitter-typescript-$TREE_SITTER_TYPESCRIPT_VERSION/tsx/src/parser.c gettext-tools/tree-sitter-typescript-$TREE_SITTER_TYPESCRIPT_VERSION/tsx/src/tsx-parser.c
  mv gettext-tools/tree-sitter-typescript-$TREE_SITTER_TYPESCRIPT_VERSION/tsx/src/scanner.c gettext-tools/tree-sitter-typescript-$TREE_SITTER_TYPESCRIPT_VERSION/tsx/src/tsx-scanner.c
  rm -rf tree-sitter-typescript
}
test -d gettext-tools/tree-sitter-d-$TREE_SITTER_D_VERSION || {
  func_git_clone_shallow tree-sitter-d https://github.com/gdamore/tree-sitter-d.git v$TREE_SITTER_D_VERSION
  (cd tree-sitter-d && patch -p1) < gettext-tools/build-aux/tree-sitter-d-portability.diff
  (cd tree-sitter-d && patch -p1) < gettext-tools/build-aux/tree-sitter-d-optimization-bug.diff
  mkdir gettext-tools/tree-sitter-d-$TREE_SITTER_D_VERSION
  mv tree-sitter-d/LICENSE.txt gettext-tools/tree-sitter-d-$TREE_SITTER_D_VERSION/LICENSE
  mv tree-sitter-d/src gettext-tools/tree-sitter-d-$TREE_SITTER_D_VERSION/src
  mv gettext-tools/tree-sitter-d-$TREE_SITTER_D_VERSION/src/parser.c gettext-tools/tree-sitter-d-$TREE_SITTER_D_VERSION/src/d-parser.c
  mv gettext-tools/tree-sitter-d-$TREE_SITTER_D_VERSION/src/scanner.c gettext-tools/tree-sitter-d-$TREE_SITTER_D_VERSION/src/d-scanner.c
  rm -rf tree-sitter-d
}
cat > gettext-tools/tree-sitter.cfg <<EOF
TREE_SITTER_VERSION=$TREE_SITTER_VERSION
TREE_SITTER_GO_VERSION=$TREE_SITTER_GO_VERSION
TREE_SITTER_RUST_VERSION=$TREE_SITTER_RUST_VERSION
TREE_SITTER_TYPESCRIPT_VERSION=$TREE_SITTER_TYPESCRIPT_VERSION
TREE_SITTER_D_VERSION=$TREE_SITTER_D_VERSION
EOF

dir0=`pwd`

echo "$0: generating files in libtextstyle..."
cd libtextstyle
./autopull.sh || exit $?
cd "$dir0"

echo "$0: done.  Now you can run './autogen.sh'."
