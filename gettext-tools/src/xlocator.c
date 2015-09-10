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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "concat-filename.h"

#if HAVE_DIRENT_H
# include <dirent.h>
#endif

#if HAVE_DIRENT_H
# define HAVE_DIR 1
#else
# define HAVE_DIR 0
#endif

#include <errno.h>
#include "error.h"
#include <fnmatch.h>
#include "gettext.h"
#include "hash.h"
#include <libxml/parser.h>
#include <libxml/uri.h>
#include "xalloc.h"

#include "xlocator.h"

#define _(str) gettext (str)

/* The schema is the same as the one used in nXML-mode (in Emacs):
   http://www.gnu.org/software/emacs/manual/html_node/nxml-mode/Schema-locating-file-syntax-basics.html#Schema-locating-file-syntax-basics
 */

#define LOCATING_RULES_NS "http://thaiopensource.com/ns/locating-rules/1.0"

enum xlocator_type
{
  XLOCATOR_URI,
  XLOCATOR_URI_PATTERN,
  XLOCATOR_NAMESPACE,
  XLOCATOR_DOCUMENT_ELEMENT
};

struct xlocator_target_ty
{
  bool is_indirection;
  char *uri;
};

struct xlocator_ty
{
  enum xlocator_type type;

  union {
    char *uri;
    char *pattern;
    char *ns;
    struct {
      char *prefix;
      char *local_name;
    } d;
  } matcher;

  bool is_transform;
  struct xlocator_target_ty target;
};

struct xlocator_list_ty
{
  char *base;

  hash_table indirections;

  struct xlocator_ty *items;
  size_t nitems;
  size_t nitems_max;
};

static char *
_xlocator_get_attribute (xmlNode *node, const char *attr)
{
  xmlChar *value;
  char *result;

  value = xmlGetProp (node, BAD_CAST attr);
  result = xstrdup ((const char *) value);
  xmlFree (value);

  return result;
}

static bool
xlocator_match (struct xlocator_ty *locator, const char *path,
                bool inspect_content)
{
  switch (locator->type)
    {
    case XLOCATOR_URI:
      return strcmp (locator->matcher.uri, path) == 0;

    case XLOCATOR_URI_PATTERN:
      /* FIXME: We should not use fnmatch() here, since PATTERN is a
         URI, with a wildcard.  */
      return fnmatch (locator->matcher.pattern, path, FNM_PATHNAME) == 0;

    case XLOCATOR_NAMESPACE:
    case XLOCATOR_DOCUMENT_ELEMENT:
      if (!inspect_content)
        return false;
      else
        {
          xmlDoc *doc;
          xmlNode *root;
          bool result;

          doc = xmlReadFile (path, "utf-8",
                             XML_PARSE_NONET
                             | XML_PARSE_NOWARNING
                             | XML_PARSE_NOBLANKS
                             | XML_PARSE_NOERROR);
          if (doc == NULL)
            return false;


          root = xmlDocGetRootElement (doc);
          if (locator->type == XLOCATOR_NAMESPACE)
            result = xmlStrEqual (root->ns->href, BAD_CAST locator->matcher.ns);
          else
            result =
              ((!locator->matcher.d.prefix
                || !root->ns
                || xmlStrEqual (root->ns->prefix,
                                BAD_CAST locator->matcher.d.prefix))
               && (!locator->matcher.d.local_name
                   || xmlStrEqual (root->name,
                                   BAD_CAST locator->matcher.d.local_name)));
          xmlFreeDoc (doc);
          return result;
        }

    default:
      error (0, 0, _("unsupported locator type: %d"), locator->type);
      return false;
    }
}

static char *
xlocator_list_resolve_target (struct xlocator_list_ty *locators,
                              struct xlocator_target_ty *target)
{
  const char *target_uri = NULL;
  char *result = NULL;

  if (!target->is_indirection)
    target_uri = target->uri;
  else
    {
      void *value;

      if (hash_find_entry (&locators->indirections,
                           target->uri, strlen (target->uri),
                           &value) == 0)
        {
          struct xlocator_target_ty *next_target =
            (struct xlocator_target_ty *) value;
          target_uri = xlocator_list_resolve_target (locators, next_target);
        }
      else
        error (0, 0, _("cannot resolve \"typeId\" %s"), target->uri);
    }

  if (target_uri != NULL)
    {
      char *path;
      xmlChar *absolute_uri;
      xmlURI *uri;

      /* Use a dummy file name under the locators->base directory, so
         that xmlBuildURI() resolve a URI relative to the file, not
         the parent directory.  */
      path = xconcatenated_filename (locators->base, ".", NULL);
      absolute_uri = xmlBuildURI (BAD_CAST target_uri, BAD_CAST path);
      free (path);

      uri = xmlParseURI ((const char *) absolute_uri);
      if (uri != NULL)
        result = xstrdup (uri->path);
      xmlFreeURI (uri);
    }

  return result;
}

char *
xlocator_list_locate (struct xlocator_list_ty *locators,
                      const char *path,
                      bool inspect_content)
{
  struct xlocator_ty *locator;
  size_t i;

  for (i = 0; i < locators->nitems; i++)
    {
      locator = &locators->items[i];
      if (xlocator_match (locator, path, inspect_content))
        break;
    }

  if (i == locators->nitems)
    return NULL;

  return xlocator_list_resolve_target (locators, &locator->target);
}

static bool
xlocator_target_init (struct xlocator_target_ty *target, xmlNode *node)
{
  if (!(xmlHasProp (node, BAD_CAST "uri")
        || xmlHasProp (node, BAD_CAST "typeId")))
    {
      error (0, 0, _("node does not have \"uri\" nor \"typeId\""));
      return false;
    }

  if (xmlHasProp (node, BAD_CAST "uri"))
    {
      target->uri = _xlocator_get_attribute (node, "uri");
      target->is_indirection = false;
    }
  else if (xmlHasProp (node, BAD_CAST "typeId"))
    {
      target->uri = _xlocator_get_attribute (node, "typeId");
      target->is_indirection = true;
    }

  return true;
}

static bool
xlocator_init (struct xlocator_ty *locator, xmlNode *node)
{
  memset (locator, 0, sizeof (struct xlocator_ty));

  if (xmlStrEqual (node->name, BAD_CAST "uri"))
    {
      if (!(xmlHasProp (node, BAD_CAST "resource")
            || xmlHasProp (node, BAD_CAST "pattern")))
        {
          error (0, 0,
                 _("\"uri\" node does not have \"resource\" nor \"pattern\""));
          return false;
        }

      if (xmlHasProp (node, BAD_CAST "resource"))
        {
          locator->type = XLOCATOR_URI;
          locator->matcher.uri = _xlocator_get_attribute (node, "resource");
        }
      else
        {
          locator->type = XLOCATOR_URI_PATTERN;
          locator->matcher.uri = _xlocator_get_attribute (node, "pattern");
        }

      return xlocator_target_init (&locator->target, node);
    }
  else if (xmlStrEqual (node->name, BAD_CAST "transformURI"))
    {
      if (!(xmlHasProp (node, BAD_CAST "fromPattern")
            && xmlHasProp (node, BAD_CAST "toPattern")))
        {
          error (0, 0,
                 _("\"transformURI\" node does not have \"fromPattern\""
                   " and \"toPattern\""));
          return false;
        }

      locator->type = XLOCATOR_URI_PATTERN;
      locator->matcher.uri = _xlocator_get_attribute (node, "fromPattern");
      locator->target.uri = _xlocator_get_attribute (node, "toPattern");
      locator->is_transform = true;

      return true;
    }
  else if (xmlStrEqual (node->name, BAD_CAST "namespace"))
    {
      if (!xmlHasProp (node, BAD_CAST "ns"))
        {
          error (0, 0,
                 _("\"namespace\" node does not have \"ns\""));
          return false;
        }

      locator->type = XLOCATOR_NAMESPACE;
      locator->matcher.ns = _xlocator_get_attribute (node, "ns");

      return xlocator_target_init (&locator->target, node);
    }
  else if (xmlStrEqual (node->name, BAD_CAST "documentElement"))
    {
      if (!(xmlHasProp (node, BAD_CAST "prefix")
            || xmlHasProp (node, BAD_CAST "localName")))
        {
          error (0, 0,
                 _("\"documentElement\" node does not have \"prefix\""
                   " and \"localName\""));
          return false;
        }

      locator->type = XLOCATOR_DOCUMENT_ELEMENT;
      locator->matcher.d.prefix =
        _xlocator_get_attribute (node, "prefix");
      locator->matcher.d.local_name =
        _xlocator_get_attribute (node, "localName");

      return xlocator_target_init (&locator->target, node);
    }

  return false;
}

static bool
xlocator_list_add_file (struct xlocator_list_ty *locators,
                        const char *locator_file_name)
{
  xmlDoc *doc;
  xmlNode *root, *node;

  doc = xmlReadFile (locator_file_name, "utf-8",
                     XML_PARSE_NONET
                     | XML_PARSE_NOWARNING
                     | XML_PARSE_NOBLANKS
                     | XML_PARSE_NOERROR);
  if (doc == NULL)
    return false;

  root = xmlDocGetRootElement (doc);
  if (!(xmlStrEqual (root->name, BAD_CAST "locatingRules")
        && xmlStrEqual (root->ns->href, BAD_CAST LOCATING_RULES_NS)))
    {
      error (0, 0, _("the root element is not \"locatingRules\""
                     " under namespace %s"),
             LOCATING_RULES_NS);
      xmlFreeDoc (doc);
      return false;
    }

  for (node = root->children; node; node = node->next)
    {
      if (xmlStrEqual (node->name, BAD_CAST "typeId"))
        {
          struct xlocator_target_ty *target;
          char *id;

          if (!(xmlHasProp (node, BAD_CAST "id")
                && (xmlHasProp (node, BAD_CAST "typeId")
                    || xmlHasProp (node, BAD_CAST "uri"))))
            {
              xmlFreeDoc (doc);
              return false;
            }

          id = _xlocator_get_attribute (node, "id");
          target = XMALLOC (struct xlocator_target_ty);
          if (xmlHasProp (node, BAD_CAST "typeId"))
            {
              target->is_indirection = true;
              target->uri = _xlocator_get_attribute (node, "typeId");
            }
          else
            {
              target->is_indirection = false;
              target->uri = _xlocator_get_attribute (node, "uri");
            }
          hash_insert_entry (&locators->indirections, id, strlen (id),
                             target);
          free (id);
        }
      else
        {
          struct xlocator_ty locator;

          if (!xlocator_init (&locator, node))
            {
              xmlFreeDoc (doc);
              return false;
            }

          if (locators->nitems == locators->nitems_max)
            {
              locators->nitems_max = 2 * locators->nitems_max + 1;
              locators->items =
                xrealloc (locators->items,
                          sizeof (struct xlocator_ty) * locators->nitems_max);
            }
          memcpy (&locators->items[locators->nitems++], &locator,
                  sizeof (struct xlocator_ty));
        }
    }

  return true;
}

static bool
xlocator_list_add_directory (struct xlocator_list_ty *locators,
                             const char *directory)
{
#if HAVE_DIR
  DIR *dirp;

  dirp = opendir (directory);
  if (dirp == NULL)
    return false;

  for (;;)
    {
      struct dirent *dp;

      errno = 0;
      dp = readdir (dirp);
      if (dp != NULL)
        {
          const char *name = dp->d_name;
          size_t namlen = strlen (name);

          if (namlen > 4 && memcmp (name + namlen - 4, ".loc", 4) == 0)
            {
              char *locator_file_name =
                xconcatenated_filename (directory, name, NULL);
              xlocator_list_add_file (locators, locator_file_name);
              free (locator_file_name);
            }
        }
      else if (errno != 0)
        return false;
      else
        break;
    }
  if (closedir (dirp))
    return false;

#endif
  return true;
}

struct xlocator_list_ty *
xlocator_list_alloc (const char *base, const char *directory)
{
  struct xlocator_list_ty *result;

  xmlCheckVersion (LIBXML_VERSION);

  result = XCALLOC (1, struct xlocator_list_ty);
  hash_init (&result->indirections, 10);
  result->base = xstrdup (base);

  xlocator_list_add_directory (result, directory);

  return result;
}

static void
xlocator_destroy (struct xlocator_ty *locator)
{
  switch (locator->type)
    {
    case XLOCATOR_URI:
      free (locator->matcher.uri);
      break;

    case XLOCATOR_URI_PATTERN:
      free (locator->matcher.pattern);
      break;

    case XLOCATOR_NAMESPACE:
      free (locator->matcher.ns);
      break;

    case XLOCATOR_DOCUMENT_ELEMENT:
      free (locator->matcher.d.prefix);
      free (locator->matcher.d.local_name);
      break;
    }

  free (locator->target.uri);
}

void
xlocator_list_destroy (struct xlocator_list_ty *locators)
{
  hash_destroy (&locators->indirections);
  while (locators->nitems-- > 0)
    xlocator_destroy (&locators->items[locators->nitems]);
}

void
xlocator_list_free (struct xlocator_list_ty *locators)
{
  xlocator_list_destroy (locators);
  free (locators);
}
