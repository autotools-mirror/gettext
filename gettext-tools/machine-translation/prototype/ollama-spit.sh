#! /bin/sh
#
# Copyright (C) 2025 Free Software Foundation, Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
# Written by Bruno Haible <bruno@clisp.org>, 2025.

# This program passes an input to an ollama instance and prints the response.
#
# Dependencies: curl, jq.

progname=$0

# func_exit STATUS
# exits with a given status.
func_exit ()
{
  exit $1
}

# func_usage
# outputs to stdout the --help usage message.
func_usage ()
{
  echo "\
Usage: spit [OPTION...]

Passes standard input to an ollama instance and prints the response.

Options:
      --url      Specifies the ollama server's URL.
      --model    Specifies the model to use.

Informative output:

      --help     Show this help text."
}

# Command-line option processing.
# Removes the OPTIONS from the arguments. Sets the variables:
# - url
# - model
{
  url='http://localhost:11434'
  model=

  while test $# -gt 0; do
    case "$1" in
      --url | --ur | --u )
        shift
        if test $# = 0; then
          echo "$progname: missing argument for --url" 1>&2
          func_exit 1
        fi
        url="$1"
        shift
        ;;
      --url=* )
        url=`echo "X$1" | sed -e 's/^X--url=//'`
        shift
        ;;
      --model | --mode | --mod | --mo | --m )
        shift
        if test $# = 0; then
          echo "$progname: missing argument for --model" 1>&2
          func_exit 1
        fi
        model="$1"
        shift
        ;;
      --model=* )
        model=`echo "X$1" | sed -e 's/^X--model=//'`
        shift
        ;;
      --help | --hel | --he | --h )
        func_usage
        func_exit $? ;;
      -- )
        # Stop option processing
        shift
        break ;;
      -* )
        echo "$progname: unknown option $1" 1>&2
        echo "Try '$progname --help' for more information." 1>&2
        func_exit 1 ;;
      * )
        break ;;
    esac
  done

  if test -z "$model"; then
    echo "$progname: missing --model option" 1>&2
    func_exit 1
  fi

  # Sanitize URL.
  case "$url" in
    */) ;;
    *) url="$url/" ;;
  esac

  # Read the contents of standard input.
  input=`cat`
  input_quoted=`echo "$input" | sed -e 's|"|\\\"|g'`

  # Documentation of the ollama API:
  # <https://docs.ollama.com/api/generate>

  curl --no-progress-meter --no-buffer \
       -X POST "$url"api/generate \
       -d '{ "model": "'"$model"'", "prompt": "'"$input_quoted"'" }' \
    | jq --join-output --unbuffered '.response'

  # Note: Error handling is not good here. For example, when passing a
  # nonexistent model name, the server answers with HTTP status 404
  # and a response body {"error":"<error message>"}, that gets output
  # to stdout (not stderr!) and then swallowed by 'jq'.
}
