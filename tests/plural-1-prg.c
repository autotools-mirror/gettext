/* Test program, used by the plural-1 test.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <locale.h>

/* Make sure we use the included libintl, not the system's one. */
#if 0
#include <libintl.h>
#else
#define ENABLE_NLS 1
#include "libgettext.h"
#undef textdomain
#define textdomain textdomain__
#undef bindtextdomain
#define bindtextdomain bindtextdomain__
#undef ngettext
#define ngettext ngettext__
#endif

int main (argc, argv)
  int argc;
  char *argv[];
{
  int n = atoi (argv[1]);

  if (setlocale (LC_ALL, "") == NULL)
    return 1;

  textdomain ("cake");
  bindtextdomain ("cake", ".");
  printf (ngettext ("a piece of cake", "%d pieces of cake", n), n);
  printf ("\n");
  return 0;
}
