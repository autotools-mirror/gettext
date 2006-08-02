/* Searching in a string.
   Copyright (C) 2001-2003, 2006 Free Software Foundation, Inc.

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


/* The functions defined in this file assume the "C" locale and a character
   set without diacritics (ASCII-US or EBCDIC-US or something like that).
   Even if the "C" locale on a particular system is an extension of the ASCII
   character set (like on BeOS, where it is UTF-8, or on AmigaOS, where it
   is ISO-8859-1), the functions in this file recognize only the ASCII
   characters.  More precisely, one of the string arguments must be an ASCII
   string with additional restrictions.  */


#ifdef __cplusplus
extern "C" {
#endif

/* Find the first occurrence of NEEDLE in HAYSTACK.
   This function is safe to be called, even in a multibyte locale, if NEEDLE
     1. consists solely of printable ASCII characters excluding '\\' and '~'
        [this restriction is needed because of Shift_JIS and JOHAB]
        or of the control ASCII characters '\a' '\b' '\f' '\n' '\r' '\t' '\v'
        [this restriction is needed because of VISCII], and
     2. has at least length 2
        [this restriction is needed because of BIG5, BIG5-HKSCS, GBK, GB18030,
         Shift_JIS, JOHAB], and
     3. does not consist entirely of decimal digits, or has at least length 4
        [this restricion is needed because of GB18030].  */
extern char *c_strstr (const char *haystack, const char *needle);

#ifdef __cplusplus
}
#endif
