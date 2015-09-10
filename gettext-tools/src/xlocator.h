/* XML resource locator
   Copyright (C) 2015 Free Software Foundation, Inc.

   This file was written by Daiki Ueno <ueno@gnu.org>, 2015.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef _XLOCATOR_H
#define _XLOCATOR_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct xlocator_list_ty xlocator_list_ty;

/* Creates a fresh xlocator_list_ty with the base URI BASE, and loads
   the locating rules from the files in DIRECTORY.  */
extern struct xlocator_list_ty *xlocator_list_alloc (const char *base,
                                                     const char *directory);

/* Determines the location of resource associated with PATH, accoding
   to the loaded locating rules.  If INSPECT_CONTENT is true, it also
   checks the content of the file pointed by PATH.  */
extern char *xlocator_list_locate (xlocator_list_ty *locators,
                                   const char *path,
                                   bool inspect_content);

/* Releases memory allocated for LOCATORS.  */
extern void xlocator_list_free (xlocator_list_ty *locators);

#ifdef __cplusplus
}
#endif

#endif  /* _XLOCATOR_H */
