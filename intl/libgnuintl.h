/* Message catalogs for internationalization.
   Copyright (C) 1995-1997, 2000-2002 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published
   by the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA.  */

#ifndef _LIBINTL_H
#define _LIBINTL_H	1

#include <locale.h>

/* The LC_MESSAGES locale category is the category used by the functions
   gettext() and dgettext().  It is specified in POSIX, but not in ANSI C.
   On systems that don't define it, use an arbitrary value instead.
   On Solaris, <locale.h> defines __LOCALE_H (or _LOCALE_H in Solaris 2.5)
   then includes <libintl.h> (i.e. this file!) and then only defines
   LC_MESSAGES.  To avoid a redefinition warning, don't define LC_MESSAGES
   in this case.  */
#if !defined LC_MESSAGES && !(defined __LOCALE_H || (defined _LOCALE_H && defined __sun))
# define LC_MESSAGES 1729
#endif

/* We define an additional symbol to signal that we use the GNU
   implementation of gettext.  */
#define __USE_GNU_GETTEXT 1

/* Resolve a platform specific conflict on DJGPP.  GNU gettext takes
   precedence over _conio_gettext.  */
#ifdef __DJGPP__
# undef gettext
#endif

/* Use _INTL_PARAMS, not PARAMS, in order to avoid clashes with identifiers
   used by programs.  Similarly, test __PROTOTYPES, not PROTOTYPES.  */
#ifndef _INTL_PARAMS
# if __STDC__ || defined __GNUC__ || defined __SUNPRO_C || defined __cplusplus || __PROTOTYPES
#  define _INTL_PARAMS(args) args
# else
#  define _INTL_PARAMS(args) ()
# endif
#endif

#ifdef __cplusplus
extern "C" {
#endif


/* We redirect the functions to those prefixed with "libintl_".  This is
   necessary, because some systems define gettext/textdomain/... in the C
   library (namely, Solaris 2.4 and newer, and GNU libc 2.0 and newer).
   If we used the unprefixed names, there would be cases where the
   definition in the C library would override the one in the libintl.so
   shared library.  Recall that on ELF systems, the symbols are looked
   up in the following order:
     1. in the executable,
     2. in the shared libraries specified on the link command line, in order,
     3. in the dependencies of the shared libraries specified on the link
        command line,
     4. in the dlopen()ed shared libraries, in the order in which they were
        dlopen()ed.
   The definition in the C library would override the one in libintl.so if
   either
     * -lc is given on the link command line and -lintl isn't, or
     * -lc is given on the link command line before -lintl, or
     * libintl.so is a dependency of a dlopen()ed shared library but not
       linked to the executable at link time.
   Since Solaris gettext() behaves differently than GNU gettext(), this
   would be unacceptable.  */


/* Look up MSGID in the current default message catalog for the current
   LC_MESSAGES locale.  If not found, returns MSGID itself (the default
   text).  */
#define gettext libintl_gettext
extern char *gettext _INTL_PARAMS ((const char *__msgid));

/* Look up MSGID in the DOMAINNAME message catalog for the current
   LC_MESSAGES locale.  */
#define dgettext libintl_dgettext
extern char *dgettext _INTL_PARAMS ((const char *__domainname,
				     const char *__msgid));

/* Look up MSGID in the DOMAINNAME message catalog for the current CATEGORY
   locale.  */
#define dcgettext libintl_dcgettext
extern char *dcgettext _INTL_PARAMS ((const char *__domainname,
				      const char *__msgid,
				      int __category));


/* Similar to `gettext' but select the plural form corresponding to the
   number N.  */
#define ngettext libintl_ngettext
extern char *ngettext _INTL_PARAMS ((const char *__msgid1,
				     const char *__msgid2,
				     unsigned long int __n));

/* Similar to `dgettext' but select the plural form corresponding to the
   number N.  */
#define dngettext libintl_dngettext
extern char *dngettext _INTL_PARAMS ((const char *__domainname,
				      const char *__msgid1,
				      const char *__msgid2,
				      unsigned long int __n));

/* Similar to `dcgettext' but select the plural form corresponding to the
   number N.  */
#define dcngettext libintl_dcngettext
extern char *dcngettext _INTL_PARAMS ((const char *__domainname,
				       const char *__msgid1,
				       const char *__msgid2,
				       unsigned long int __n,
				       int __category));


/* Set the current default message catalog to DOMAINNAME.
   If DOMAINNAME is null, return the current default.
   If DOMAINNAME is "", reset to the default of "messages".  */
#define textdomain libintl_textdomain
extern char *textdomain _INTL_PARAMS ((const char *__domainname));

/* Specify that the DOMAINNAME message catalog will be found
   in DIRNAME rather than in the system locale data base.  */
#define bindtextdomain libintl_bindtextdomain
extern char *bindtextdomain _INTL_PARAMS ((const char *__domainname,
					   const char *__dirname));

/* Specify the character encoding in which the messages from the
   DOMAINNAME message catalog will be returned.  */
#define bind_textdomain_codeset libintl_bind_textdomain_codeset
extern char *bind_textdomain_codeset _INTL_PARAMS ((const char *__domainname,
						    const char *__codeset));


#ifdef __cplusplus
}
#endif

#endif /* libintl.h */
