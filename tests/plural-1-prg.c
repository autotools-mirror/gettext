/* Test program, used by the plural-1 test.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <locale.h>

/* Make sure we use the included libintl, not the system's one. */
#define textdomain textdomain__
#define bindtextdomain bindtextdomain__
#define ngettext ngettext__
#undef _LIBINTL_H
#include "libgnuintl.h"

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
