/* Information about terminal capabilities.
   Copyright (C) 2006 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2006.

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

#ifndef _TERMCAP_H
#define _TERMCAP_H

/* Including <curses.h> or <term.h> is dangerous, because it also declares
   a lot of junk, such as variables PC, UP, and other.  */

#ifdef __cplusplus
extern "C" {
#endif

/* Gets the capability information for terminal type TYPE.
   Returns 1 if successful, 0 if TYPE is unknown, -1 on other error.  */
extern int tgetent (char *bp, const char *type);

/* Retrieves the value of a numerical capability.
   Returns -1 if it is not available.  */
extern int tgetnum (const char *id);

/* Retrieves the value of a boolean capability.
   Returns 1 if it available, 0 otherwise.  */
extern int tgetflag (const char *id);

/* Retrieves the value of a string capability.
   Returns NULL if it is not available.
   Also, if AREA != NULL, stores it at *AREA and advances *AREA.  */
extern const char * tgetstr (const char *id, char **area);

/* Instantiates a string capability with format strings.
   The return value is statically allocated and must not be freed.  */
extern char * tparm (const char *str, ...);

/* Retrieves a string that causes cursor positioning to (column, row).
   This function is necessary because the string returned by tgetstr ("cm")
   is in a special format.  */
extern const char * tgoto (const char *cm, int column, int row);

/* Retrieves the value of a string capability.
   OUTCHARFUN is called in turn for each 'char' of the result.
   This function is necessary because string capabilities can contain
   padding commands.  */
extern void tputs (const char *cp, int affcnt, int (*outcharfun) (int));

/* The ncurses functions for color handling (see ncurses/base/lib_color.c)
   are overkill: Most terminal emulators support only a fixed, small number
   of colors.  */

#ifdef __cplusplus
}
#endif

#endif /* _TERMCAP_H */
