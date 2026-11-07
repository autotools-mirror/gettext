/* GNU gettext - internationalization aids
   Copyright (C) 1995-2026 Free Software Foundation, Inc.

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

/* Written by Peter Miller, Ulrich Drepper, and Bruno Haible.  */

%{
#include <config.h>

/* Specification.  */
#include "read-po-internal.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <error.h>
#include "string-desc.h"
#include "xstring-desc.h"
#include "string-list.h"
#include "read-po-lex.h"
#include "po-charset.h"
#include "xalloc.h"
#include "gettext.h"
#include "read-catalog-abstract.h"

#define _(str) gettext (str)

#define check_obsolete(value1,value2) \
  if ((value1).obsolete != (value2).obsolete) \
    po_gram_error_at_line (ps->catr, &(value2).pos, \
                           _("inconsistent use of #~"));

#define warn_about_nul(pretty_part) \
  po_gram_error_at_line (ps->catr, &ps->gram_pos, \
                         _("'%s' string contains a NUL character"), \
                         pretty_part)
#define check_string_against_nuls(value,pretty_part) \
  if (sd_index (sd_substring ((value).string, 0, sd_length ((value).string) - 1), '\0') >= 0) \
    warn_about_nul (pretty_part)

static inline void
do_callback_message (struct po_parser_state *ps,
                     char *msgctxt,
                     char *msgid, size_t msgid_len, lex_pos_ty *msgid_pos,
                     char *msgid_plural, size_t msgid_plural_len,
                     char *msgstr, size_t msgstr_len, lex_pos_ty *msgstr_pos,
                     char *prev_msgctxt,
                     char *prev_msgid, char *prev_msgid_plural,
                     bool obsolete)
{
  /* Test for header entry.  Ignore fuzziness of the header entry.  */
  if (msgctxt == NULL && msgid_len == 0 + 1 && !obsolete)
    po_lex_charset_set (ps, msgstr, ps->gram_pos.file_name, ps->gram_pot_role);

  catalog_reader_seen_message (ps->catr,
                               msgctxt,
                               msgid, msgid_len, msgid_pos,
                               msgid_plural, msgid_plural_len,
                               msgstr, msgstr_len, msgstr_pos,
                               prev_msgctxt, prev_msgid, prev_msgid_plural,
                               false, obsolete);
}

#define free_message_intro(value) \
  if ((value).prev_ctxt != NULL)        \
    free ((value).prev_ctxt);           \
  if ((value).prev_id != NULL)          \
    free ((value).prev_id);             \
  if ((value).prev_id_plural != NULL)   \
    free ((value).prev_id_plural);      \
  if ((value).ctxt != NULL)             \
    free ((value).ctxt);

%}

%require "3.0"
%define api.pure full

/* Remap parser interface names, so we can have multiple Bison
   generated parsers in the same program.  */
%define api.prefix {po_gram_}

%parse-param {struct po_parser_state *ps}
%lex-param {struct po_parser_state *ps}

%token COMMENT
%token DOMAIN
%token JUNK
%token PREV_MSGCTXT
%token PREV_MSGID
%token PREV_MSGID_PLURAL
%token PREV_STRING
%token MSGCTXT
%token MSGID
%token MSGID_PLURAL
%token MSGSTR
%token OTHER_IDENTIFIER
%token '[' ']'
%token NUMBER
%token STRING

%union
{
  struct { string_desc_t string; lex_pos_ty pos; bool obsolete; } comment;
  struct { char *string; lex_pos_ty pos; bool obsolete; } string;
  struct { rw_string_desc_t string; lex_pos_ty pos; bool obsolete; } stringdesc;
  struct { string_list_ty list; lex_pos_ty pos; bool obsolete; } stringlist;
  struct { string_desc_list_ty list; lex_pos_ty pos; bool obsolete; } stringdesclist;
  struct { char *ctxt; lex_pos_ty pos; bool obsolete; } context;
  struct { long number; lex_pos_ty pos; bool obsolete; } number;
  struct { char *string; lex_pos_ty pos; bool obsolete; } identifier;
  struct { lex_pos_ty pos; bool obsolete; } pos;
  struct { char *ctxt; char *id; char *id_plural; lex_pos_ty pos; bool obsolete; } prev;
  struct { char *prev_ctxt; char *prev_id; char *prev_id_plural; char *ctxt; lex_pos_ty pos; bool obsolete; } message_intro;
  struct { char *prev_ctxt; char *prev_id; char *prev_id_plural; char *ctxt; rw_string_desc_t id; lex_pos_ty pos; bool obsolete; } message_upto_msgid;
  struct { struct msgstr_def rhs; lex_pos_ty pos; bool obsolete; } rhs;
}

%type <comment> COMMENT
%type <string> prev_msgid_pluralform
%type <stringdesc> STRING PREV_STRING
                   msgid_pluralform
%type <stringlist> stringwithoutnuls_list prev_stringwithoutnuls_list
%type <stringdesclist> stringwithnuls_list
%type <context> msg_intro prev_msg_intro
%type <number> NUMBER
%type <identifier> OTHER_IDENTIFIER
%type <pos> DOMAIN
            PREV_MSGCTXT PREV_MSGID PREV_MSGID_PLURAL
            MSGCTXT MSGID MSGID_PLURAL MSGSTR '[' ']'
%type <prev> prev prev_upto_msgid
%type <message_intro> message_intro
%type <message_upto_msgid> message_upto_msgid
%type <rhs> pluralform pluralform_list

%right MSGSTR

%%

po_file
        : /* empty */
        | po_file comment
        | po_file domain
        | po_file message
        | po_file error
        ;


comment
        : COMMENT
                {
                  catalog_reader_seen_generic_comment (ps->catr, $1.string);
                }
        ;


domain
        : DOMAIN STRING
                {
                  check_string_against_nuls ($2, "domain");
                  catalog_reader_seen_domain (ps->catr, sd_data ($2.string),
                                              &ps->gram_pos);
                }
        ;


message
        : message_upto_msgid MSGSTR
                {
                  strcpy (ps->pretty_part, "msgstr");
                }
          stringwithoutnuls_list
                {
                  char *string4 = string_list_concat_destroy (&$4.list);

                  check_obsolete ($1, $2);
                  check_obsolete ($1, $4);
                  if (!$1.obsolete || ps->catr->pass_obsolete_entries)
                    {
                      char *msgid = xsd_c ($1.id);
                      size_t msgid_len = sd_length ($1.id) + 1;
                      do_callback_message (ps, $1.ctxt, msgid, msgid_len, &$1.pos,
                                           NULL, 0,
                                           string4, strlen (string4) + 1, &$2.pos,
                                           $1.prev_ctxt,
                                           $1.prev_id, $1.prev_id_plural,
                                           $1.obsolete);
                    }
                  else
                    {
                      free_message_intro ($1);
                      free (string4);
                    }
                  sd_free ($1.id);
                }
        | message_upto_msgid msgid_pluralform pluralform_list
                {
                  check_obsolete ($1, $2);
                  check_obsolete ($1, $3);
                  if (!$1.obsolete || ps->catr->pass_obsolete_entries)
                    {
                      char *msgid = xsd_c ($1.id);
                      size_t msgid_len = sd_length ($1.id) + 1;
                      char *msgid_plural = xsd_c ($2.string);
                      size_t msgid_plural_len = sd_length ($2.string) + 1;
                      do_callback_message (ps, $1.ctxt, msgid, msgid_len, &$1.pos,
                                           msgid_plural, msgid_plural_len,
                                           $3.rhs.msgstr, $3.rhs.msgstr_len, &$3.pos,
                                           $1.prev_ctxt,
                                           $1.prev_id, $1.prev_id_plural,
                                           $1.obsolete);
                    }
                  else
                    {
                      free_message_intro ($1);
                      free ($3.rhs.msgstr);
                    }
                  sd_free ($1.id);
                  sd_free ($2.string);
                }
        | message_upto_msgid msgid_pluralform
                {
                  check_obsolete ($1, $2);
                  po_gram_error_at_line (ps->catr, &$1.pos, _("missing 'msgstr[]' section"));
                  free_message_intro ($1);
                  sd_free ($2.string);
                }
        | message_upto_msgid pluralform_list
                {
                  check_obsolete ($1, $2);
                  po_gram_error_at_line (ps->catr, &$1.pos, _("missing 'msgid_plural' section"));
                  free_message_intro ($1);
                  free ($2.rhs.msgstr);
                }
        | message_upto_msgid
                {
                  po_gram_error_at_line (ps->catr, &$1.pos, _("missing 'msgstr' section"));
                  free_message_intro ($1);
                }
        ;


message_upto_msgid
        : message_intro
                {
                  strcpy (ps->pretty_part, "msgid");
                }
          stringwithnuls_list
                {
                  check_obsolete ($1, $3);
                  $$.prev_ctxt = $1.prev_ctxt;
                  $$.prev_id = $1.prev_id;
                  $$.prev_id_plural = $1.prev_id_plural;
                  $$.ctxt = $1.ctxt;
                  $$.id = string_desc_list_concat_destroy (&$3.list);
                  $$.pos = $1.pos;
                  $$.obsolete = $1.obsolete;
                }
        ;


message_intro
        : msg_intro
                {
                  $$.prev_ctxt = NULL;
                  $$.prev_id = NULL;
                  $$.prev_id_plural = NULL;
                  $$.ctxt = $1.ctxt;
                  $$.pos = $1.pos;
                  $$.obsolete = $1.obsolete;
                }
        | prev msg_intro
                {
                  check_obsolete ($1, $2);
                  $$.prev_ctxt = $1.ctxt;
                  $$.prev_id = $1.id;
                  $$.prev_id_plural = $1.id_plural;
                  $$.ctxt = $2.ctxt;
                  $$.pos = $2.pos;
                  $$.obsolete = $2.obsolete;
                }
        ;


prev
        : prev_upto_msgid
                {
                  $$.ctxt = $1.ctxt;
                  $$.id = $1.id;
                  $$.id_plural = $1.id_plural;
                  $$.pos = $1.pos;
                  $$.obsolete = $1.obsolete;
                }
        | prev_upto_msgid prev_msgid_pluralform
                {
                  check_obsolete ($1, $2);
                  $$.ctxt = $1.ctxt;
                  $$.id = $1.id;
                  $$.id_plural = $2.string;
                  $$.pos = $1.pos;
                  $$.obsolete = $1.obsolete;
                }
        ;


prev_upto_msgid
        : prev_msg_intro
                {
                  strcpy (ps->pretty_part, "msgid");
                }
          prev_stringwithoutnuls_list
                {
                  check_obsolete ($1, $3);
                  $$.ctxt = $1.ctxt;
                  $$.id = string_list_concat_destroy (&$3.list);
                  $$.id_plural = NULL;
                  $$.pos = $1.pos;
                  $$.obsolete = $1.obsolete;
                }
        ;


msg_intro
        : MSGID
                {
                  $$.ctxt = NULL;
                  $$.pos = $1.pos;
                  $$.obsolete = $1.obsolete;
                }
        | MSGCTXT
                {
                  strcpy (ps->pretty_part, "msgctxt");
                }
          stringwithoutnuls_list MSGID
                {
                  check_obsolete ($1, $3);
                  check_obsolete ($1, $4);
                  $$.ctxt = string_list_concat_destroy (&$3.list);
                  $$.pos = $4.pos;
                  $$.obsolete = $4.obsolete;
                }
        ;

prev_msg_intro
        : PREV_MSGID
                {
                  $$.ctxt = NULL;
                  $$.pos = $1.pos;
                  $$.obsolete = $1.obsolete;
                }
        | PREV_MSGCTXT
                {
                  strcpy (ps->pretty_part, "msgctxt");
                }
          prev_stringwithoutnuls_list PREV_MSGID
                {
                  check_obsolete ($1, $3);
                  check_obsolete ($1, $4);
                  $$.ctxt = string_list_concat_destroy (&$3.list);
                  $$.pos = $4.pos;
                  $$.obsolete = $4.obsolete;
                }
        ;


msgid_pluralform
        : MSGID_PLURAL
                {
                  strcpy (ps->pretty_part, "msgid_plural");
                }
          stringwithnuls_list
                {
                  check_obsolete ($1, $3);
                  ps->plural_counter = 0;
                  $$.string = string_desc_list_concat_destroy (&$3.list);
                  $$.pos = $1.pos;
                  $$.obsolete = $1.obsolete;
                }
        ;

prev_msgid_pluralform
        : PREV_MSGID_PLURAL
                {
                  strcpy (ps->pretty_part, "msgid_plural");
                }
          prev_stringwithoutnuls_list
                {
                  check_obsolete ($1, $3);
                  $$.string = string_list_concat_destroy (&$3.list);
                  $$.pos = $1.pos;
                  $$.obsolete = $1.obsolete;
                }
        ;


pluralform_list
        : pluralform
                {
                  $$ = $1;
                }
        | pluralform_list pluralform
                {
                  check_obsolete ($1, $2);
                  $$.rhs.msgstr = XNMALLOC ($1.rhs.msgstr_len + $2.rhs.msgstr_len, char);
                  memcpy ($$.rhs.msgstr, $1.rhs.msgstr, $1.rhs.msgstr_len);
                  memcpy ($$.rhs.msgstr + $1.rhs.msgstr_len, $2.rhs.msgstr, $2.rhs.msgstr_len);
                  $$.rhs.msgstr_len = $1.rhs.msgstr_len + $2.rhs.msgstr_len;
                  free ($1.rhs.msgstr);
                  free ($2.rhs.msgstr);
                  $$.pos = $1.pos;
                  $$.obsolete = $1.obsolete;
                }
        ;

pluralform
        : MSGSTR '[' NUMBER ']'
                {
                  sprintf (ps->pretty_part, "msgstr[%ld]", $3.number);
                }
          stringwithoutnuls_list
                {
                  check_obsolete ($1, $2);
                  check_obsolete ($1, $3);
                  check_obsolete ($1, $4);
                  check_obsolete ($1, $6);
                  if ($3.number != ps->plural_counter)
                    {
                      if (ps->plural_counter == 0)
                        po_gram_error_at_line (ps->catr, &$1.pos, _("first plural form has nonzero index"));
                      else
                        po_gram_error_at_line (ps->catr, &$1.pos, _("plural form has wrong index"));
                    }
                  ps->plural_counter++;
                  $$.rhs.msgstr = string_list_concat_destroy (&$6.list);
                  $$.rhs.msgstr_len = strlen ($$.rhs.msgstr) + 1;
                  $$.pos = $1.pos;
                  $$.obsolete = $1.obsolete;
                }
        ;


stringwithoutnuls_list          // depends on ps->pretty_part being set
        : STRING
                {
                  check_string_against_nuls ($1, ps->pretty_part);
                  string_list_init (&$$.list);
                  string_list_append (&$$.list, sd_data ($1.string));
                  sd_free ($1.string);
                  $$.pos = $1.pos;
                  $$.obsolete = $1.obsolete;
                }
        | stringwithoutnuls_list STRING
                {
                  check_obsolete ($1, $2);
                  check_string_against_nuls ($2, ps->pretty_part);
                  $$.list = $1.list;
                  string_list_append (&$$.list, sd_data ($2.string));
                  sd_free ($2.string);
                  $$.pos = $1.pos;
                  $$.obsolete = $1.obsolete;
                }
        ;

prev_stringwithoutnuls_list     // depends on ps->pretty_part being set
        : PREV_STRING
                {
                  check_string_against_nuls ($1, ps->pretty_part);
                  string_list_init (&$$.list);
                  string_list_append (&$$.list, sd_data ($1.string));
                  sd_free ($1.string);
                  $$.pos = $1.pos;
                  $$.obsolete = $1.obsolete;
                }
        | prev_stringwithoutnuls_list PREV_STRING
                {
                  check_obsolete ($1, $2);
                  check_string_against_nuls ($2, ps->pretty_part);
                  $$.list = $1.list;
                  string_list_append (&$$.list, sd_data ($2.string));
                  sd_free ($2.string);
                  $$.pos = $1.pos;
                  $$.obsolete = $1.obsolete;
                }
        ;

stringwithnuls_list             // depends on ps->pretty_part being set
        : STRING
                {
                  if (!ps->catr->allow_nul_in_msgid)
                    check_string_against_nuls ($1, ps->pretty_part);
                  string_desc_list_init (&$$.list);
                  string_desc_list_append (&$$.list, sd_readonly (sd_substring ($1.string, 0, sd_length ($1.string) - 1)));
                  sd_free ($1.string);
                  $$.pos = $1.pos;
                  $$.obsolete = $1.obsolete;
                }
        | stringwithnuls_list STRING
                {
                  check_obsolete ($1, $2);
                  if (!ps->catr->allow_nul_in_msgid)
                    check_string_against_nuls ($2, ps->pretty_part);
                  $$.list = $1.list;
                  string_desc_list_append (&$$.list, sd_readonly (sd_substring ($2.string, 0, sd_length ($2.string) - 1)));
                  sd_free ($2.string);
                  $$.pos = $1.pos;
                  $$.obsolete = $1.obsolete;
                }
        ;
