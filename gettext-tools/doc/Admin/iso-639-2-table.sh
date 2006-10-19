#!/bin/sh
# Extracts the ISO_639-2 file from http://www.loc.gov/standards/iso639-2/code_list.html
# Usage: iso-639-2-table.sh < code_list.html
LC_ALL=C
export LC_ALL
tr '\012' ' ' |
sed -e 's,<tr ,\
<tr ,g' |
sed -n -e 's,^<tr [^>]*>[^<>]*<td[^>]*>\([^<>]*\)</td>[^<>]*<td>&nbsp;</td>[^<>]*<td>\([^<>]*\)</td>.*$,\1   \2,p' |
iconv -f ISO-8859-1 -t UTF-8
