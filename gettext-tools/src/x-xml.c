/* xgettext XML backend.
   Copyright (C) 2015-2026 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Daiki Ueno.  */

#include <config.h>

/* Specification.  */
#include "x-xml.h"

#include <stdlib.h>

#include "string-desc.h"
#include "xg-encoding.h"
#include "xg-arglist-context.h"
#include "xg-message.h"
#include "xgettext.h"
#include "xvasprintf.h"
#include "message.h"
#include "pos.h"
#include "po-charset.h"
#include "its.h"


static message_ty *
xgettext_its_extract_callback (message_list_ty *mlp,
                               const char *msgctxt,
                               const char *msgid,
                               lex_pos_ty *pos,
                               string_desc_t extracted_comment,
                               const char *marker,
                               enum its_whitespace_type_ty whitespace)
{
  message_ty *message =
    remember_a_message (mlp,
                        (msgctxt == NULL
                         ? NO_MSGCTXT
                         : (string_desc_opt) SOME (sd_from_c (msgctxt))),
                        sd_from_c (msgid),
                        false, false,
                        null_context_region (), pos,
                        extracted_comment, NULL, false);

  if (add_itstool_comments)
    {
      char *dot = xasprintf ("(itstool) path: %s", marker);
      message_comment_dot_append (message, sd_from_c (dot));
      free (dot);

      if (whitespace == ITS_WHITESPACE_PRESERVE)
        message->do_wrap = no;
    }

  return message;
}

void
extract_xml (FILE *fp, const char *real_filename,
             const char *logical_filename,
             its_rule_list_ty *rules,
             msgdomain_list_ty *mdlp)
{
  /* The default encoding for XML is UTF-8.  It can be overridden by
     an XML declaration in the XML file itself, not through the
     --from-code option.  */
  xgettext_current_source_encoding = po_charset_utf8;

#if HAVE_ICONV
  xgettext_current_source_iconv = xgettext_global_source_iconv;
#endif

  its_rule_list_extract (rules, fp, real_filename, logical_filename,
                         mdlp,
                         xgettext_its_extract_callback);
}
