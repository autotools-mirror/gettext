/* Internationalization Tag Set (ITS) handling
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

#ifndef _ITS_H_
#define _ITS_H_

#include "message.h"
#include "xgettext.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct its_rule_list_ty its_rule_list_ty;

typedef message_ty * (*its_extract_callback_ty) (message_list_ty *mlp,
                                                 const char *msgctxt,
                                                 const char *msgid,
                                                 lex_pos_ty *pos,
                                                 const char *extracted_comment,
                                                 const char *marker);

/* Creates a fresh its_rule_list_ty holding global ITS rules.  */
extern its_rule_list_ty *its_rule_list_alloc (void);

/* Releases memory allocated for RULES.  */
extern void its_rule_list_free (its_rule_list_ty *rules);

/* Loads global ITS rules from FILENAME.  */
extern bool its_rule_list_add_file (its_rule_list_ty *rules,
                                    const char *filename);

/* Extracts messages from FP, accoding to the loaded ITS rules.  */
extern void its_rule_list_extract (its_rule_list_ty *rules,
                                   FILE *fp, const char *real_filename,
                                   const char *logical_filename,
                                   flag_context_list_table_ty *flag_table,
                                   msgdomain_list_ty *mdlp,
                                   its_extract_callback_ty callback);

#ifdef __cplusplus
}
#endif

#endif	/* _ITS_H_ */
