/* Checking of messages in POT files: so-called "syntax checks".
   Copyright (C) 2015-2023 Free Software Foundation, Inc.

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

/* Written by Daiki Ueno <ueno@gnu.org>.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* Specification.  */
#include "xg-check.h"

#include <stdlib.h>
#include <string.h>

#include "xalloc.h"
#include "xvasprintf.h"
#include "message.h"
#include "po-xerror.h"
#include "sentence.h"
#include "c-ctype.h"
#include "unictype.h"
#include "unistr.h"
#include "quote.h"
#include "gettext.h"

#define _(str) gettext (str)


/* Function that implements a single syntax check.
   MP is a message.
   MSGID is either MP->msgid or MP->msgid_plural.
   Returns the number of errors that were seen and reported.  */
typedef int (* syntax_check_function) (const message_ty *mp, const char *msgid);


/* Implementation of the sc_ellipsis_unicode syntax check.  */

static int
syntax_check_ellipsis_unicode (const message_ty *mp, const char *msgid)
{
  const char *str = msgid;
  const char *str_limit = str + strlen (msgid);
  int seen_errors = 0;

  while (str < str_limit)
    {
      const char *end, *cp;
      ucs4_t ending_char;

      end = sentence_end (str, &ending_char);

      /* sentence_end doesn't treat '...' specially.  */
      cp = end - (ending_char == '.' ? 2 : 3);
      if (cp >= str && memcmp (cp, "...", 3) == 0)
        {
          po_xerror (PO_SEVERITY_ERROR, mp, NULL, 0, 0, false,
                     _("ASCII ellipsis ('...') instead of Unicode"));
          seen_errors++;
        }

      str = end + 1;
    }

  return seen_errors;
}


/* Implementation of the sc_space_ellipsis syntax check.  */

static int
syntax_check_space_ellipsis (const message_ty *mp, const char *msgid)
{
  const char *str = msgid;
  const char *str_limit = str + strlen (msgid);
  int seen_errors = 0;

  while (str < str_limit)
    {
      const char *end, *ellipsis = NULL;
      ucs4_t ending_char;

      end = sentence_end (str, &ending_char);

      if (ending_char == 0x2026)
        ellipsis = end;
      else if (ending_char == '.')
        {
          /* sentence_end doesn't treat '...' specially.  */
          const char *cp = end - 2;
          if (cp >= str && memcmp (cp, "...", 3) == 0)
            ellipsis = cp;
        }
      else
        {
          /* Look for a '...'.  */
          const char *cp = end - 3;
          if (cp >= str && memcmp (cp, "...", 3) == 0)
            ellipsis = cp;
          else
            {
              ucs4_t uc = 0xfffd;

              /* Look for a U+2026.  */
              for (cp = end - 1; cp >= str; cp--)
                {
                  u8_mbtouc (&uc, (const unsigned char *) cp, end - cp);
                  if (uc != 0xfffd)
                    break;
                }

              if (uc == 0x2026)
                ellipsis = cp;
            }
        }

      if (ellipsis)
        {
          const char *cp;
          ucs4_t uc = 0xfffd;

          /* Look at the character before ellipsis.  */
          for (cp = ellipsis - 1; cp >= str; cp--)
            {
              u8_mbtouc (&uc, (const unsigned char *) cp, ellipsis - cp);
              if (uc != 0xfffd)
                break;
            }

          if (uc != 0xfffd && uc_is_space (uc))
            {
              po_xerror (PO_SEVERITY_ERROR, mp, NULL, 0, 0, false,
                         _("space before ellipsis found in user visible strings"));
              seen_errors++;
            }
        }

      str = end + 1;
    }

  return seen_errors;
}


/* Implementation of the sc_quote_unicode syntax check.  */

struct callback_arg
{
  const message_ty *mp;
  int seen_errors;
};

static void
syntax_check_quote_unicode_callback (char quote, const char *quoted,
                                     size_t quoted_length, void *data)
{
  struct callback_arg *arg = data;

  switch (quote)
    {
    case '"':
      po_xerror (PO_SEVERITY_ERROR, arg->mp, NULL, 0, 0, false,
                 _("ASCII double quote used instead of Unicode"));
      arg->seen_errors++;
      break;

    case '\'':
      po_xerror (PO_SEVERITY_ERROR, arg->mp, NULL, 0, 0, false,
                 _("ASCII single quote used instead of Unicode"));
      arg->seen_errors++;
      break;

    default:
      break;
    }
}

static int
syntax_check_quote_unicode (const message_ty *mp, const char *msgid)
{
  struct callback_arg arg;

  arg.mp = mp;
  arg.seen_errors = 0;

  scan_quoted (msgid, strlen (msgid),
               syntax_check_quote_unicode_callback, &arg);

  return arg.seen_errors;
}


/* Implementation of the sc_bullet_unicode syntax check.  */

struct bullet_ty
{
  int c;
  size_t depth;
};

struct bullet_stack_ty
{
  struct bullet_ty *items;
  size_t nitems;
  size_t nitems_max;
};

static struct bullet_stack_ty bullet_stack;

static int
syntax_check_bullet_unicode (const message_ty *mp, const char *msgid)
{
  const char *str = msgid;
  const char *str_limit = str + strlen (msgid);
  struct bullet_ty *last_bullet = NULL;
  bool seen_error = false;

  bullet_stack.nitems = 0;

  while (str < str_limit)
    {
      const char *p = str, *end;

      while (p < str_limit && c_isspace (*p))
        p++;

      if ((*p == '*' || *p == '-') && *(p + 1) == ' ')
        {
          size_t depth = p - str;
          if (last_bullet == NULL || depth > last_bullet->depth)
            {
              struct bullet_ty bullet;

              bullet.c = *p;
              bullet.depth = depth;

              if (bullet_stack.nitems >= bullet_stack.nitems_max)
                {
                  bullet_stack.nitems_max = 2 * bullet_stack.nitems_max + 4;
                  bullet_stack.items = xrealloc (bullet_stack.items,
                                                 bullet_stack.nitems_max
                                                 * sizeof (struct bullet_ty));
                }

              last_bullet = &bullet_stack.items[bullet_stack.nitems++];
              memcpy (last_bullet, &bullet, sizeof (struct bullet_ty));
            }
          else
            {
              if (depth < last_bullet->depth)
                {
                  if (bullet_stack.nitems > 1)
                    {
                      bullet_stack.nitems--;
                      last_bullet =
                        &bullet_stack.items[bullet_stack.nitems - 1];
                    }
                  else
                    last_bullet = NULL;
                }

              if (last_bullet && depth == last_bullet->depth)
                {
                  if (last_bullet->c != *p)
                    last_bullet->c = *p;
                  else
                    {
                      seen_error = true;
                      break;
                    }
                }
            }
        }
      else
        {
          bullet_stack.nitems = 0;
          last_bullet = NULL;
        }

      end = strchrnul (str, '\n');
      str = end + 1;
    }

  if (seen_error)
    {
      char *msg;
      msg = xasprintf (_("ASCII bullet ('%c') instead of Unicode"),
                       last_bullet->c);
      po_xerror (PO_SEVERITY_ERROR, mp, NULL, 0, 0, false, msg);
      free (msg);
      return 1;
    }

  return 0;
}


/* List of all syntax checks.  */
static const syntax_check_function sc_funcs[NSYNTAXCHECKS] =
{
  syntax_check_ellipsis_unicode,
  syntax_check_space_ellipsis,
  syntax_check_quote_unicode,
  syntax_check_bullet_unicode
};


/* Perform all syntax checks on a non-obsolete message.
   Return the number of errors that were seen.  */
static int
syntax_check_message (const message_ty *mp)
{
  int seen_errors = 0;
  int i;

  for (i = 0; i < NSYNTAXCHECKS; i++)
    {
      if (mp->do_syntax_check[i] == yes)
        {
          seen_errors += sc_funcs[i] (mp, mp->msgid);
          if (mp->msgid_plural)
            seen_errors += sc_funcs[i] (mp, mp->msgid_plural);
        }
    }

  return seen_errors;
}


/* Perform all syntax checks on a message list.
   Return the number of errors that were seen.  */
int
syntax_check_message_list (message_list_ty *mlp)
{
  int seen_errors = 0;
  size_t j;

  for (j = 0; j < mlp->nitems; j++)
    {
      message_ty *mp = mlp->item[j];

      if (!is_header (mp))
        seen_errors += syntax_check_message (mp);
    }

  return seen_errors;
}
