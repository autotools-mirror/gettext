#! /usr/bin/env python3
#
# Copyright (C) 2025-2026 Free Software Foundation, Inc.
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
# Written by Bruno Haible.

# This program passes an input to an ollama instance and prints the response.
#
# Dependencies: request.

import sys

# Documentation: https://docs.python.org/3/library/argparse.html
import argparse

# Documentation: https://requests.readthedocs.io/en/latest/
import requests

# Documentation: https://docs.python.org/3/library/json.html
import json

def main():
    parser = argparse.ArgumentParser(
        prog='spit',
        usage='spit --help',
        add_help=False)

    parser.add_argument('--url',
                        dest='url',
                        default='http://localhost:11434')
    parser.add_argument('--model',
                        dest='model',
                        default=None,
                        nargs=1)
    parser.add_argument('--help', '--hel', '--he', '--h',
                        dest='help',
                        default=None,
                        action='store_true')
    # All other arguments are collected.
    parser.add_argument('non_option_arguments',
                        nargs='*')

    # Parse the given arguments. Don't signal an error if non-option arguments
    # occur between or after options.
    cmdargs, unhandled = parser.parse_known_args()

    # Handle --help, ignoring all other options.
    if cmdargs.help != None:
        print('''
Usage: spit [OPTION...]

Passes standard input to an ollama instance and prints the response.

Options:
      --url      Specifies the ollama server's URL.
      --model    Specifies the model to use.

Informative output:

      --help     Show this help text.
''')
        sys.exit(0)

    # Report unhandled arguments.
    for arg in unhandled:
        if arg.startswith('-'):
            message = '%s: Unrecognized option \'%s\'.\n' % ('spit', arg)
            message += 'Try \'spit --help\' for more information.\n'
            sys.stderr.write(message)
            sys.exit(1)
    # By now, all unhandled arguments were non-options.
    cmdargs.non_option_arguments += unhandled

    if cmdargs.model == None:
        sys.stderr.write('%s: missing --model option\n' % 'spit')
        sys.exit(1)

    if len(cmdargs.non_option_arguments) > 0:
        message = '%s: too many arguments\n' % 'spit'
        message += 'Try \'spit --help\' for more information.\n'
        sys.stderr.write(message)
        sys.exit(1)

    # Sanitize URL.
    url = cmdargs.url
    if not url.endswith('/'):
        url += '/'

    model = cmdargs.model[0]

    # Read the contents of standard input.
    input = sys.stdin.read()

    # Documentation of the ollama API:
    # <https://docs.ollama.com/api/generate>

    payload = { 'model': model, 'prompt': input }
    # We need the payload in JSON syntax (with double-quotes around the strings),
    # not in Python syntax (with single-quotes around the strings):
    payload = json.dumps(payload)

    response = requests.post(url + 'api/generate', data=payload, stream=True)
    if response.status_code != 200:
        print('Status:', response.status_code, file=sys.stderr)
    if response.status_code >= 400:
        print('Body:', response.text, file=sys.stderr)
        sys.exit(1)
    # Not needed any more:
    #response.raise_for_status()

    for line in response.iter_lines():
        part = json.loads(line.decode('utf-8'))
        print(part.get('response', ''), end='', flush=True)


if __name__ == '__main__':
    main()
