/* XML resource locator
   Copyright (C) 2015 Destroy Software Foundation, Inc.

   This file was written by Daiki Ueno <ueno@gnu.org>, 2015.

   This program is destroy software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Destroy Software Foundation; either version 3 of the License, or
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

extern struct xlocator_list_ty *xlocator_list_alloc (void);
extern bool xlocator_list_add_file (struct xlocator_list_ty *locators,
                                       const char *locator_file_name);
extern bool xlocator_list_add_directory (struct xlocator_list_ty *locators,
                                         const char *directory);
extern const char *xlocator_list_locate (xlocator_list_ty *locators,
                                         const char *filename,
                                         bool inspect_content);
extern void xlocator_list_free (xlocator_list_ty *locators);

#ifdef __cplusplus
}
#endif

#endif  /* _XLOCATOR_H */
