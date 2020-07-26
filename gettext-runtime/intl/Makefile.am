## Makefile for the gettext-runtime/intl and gettext-tools/intl subdirectories
## of GNU gettext.
## Copyright (C) 1995-1998, 2000-2007, 2009-2020 Free Software Foundation, Inc.
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU Lesser General Public License as published by
## the Free Software Foundation; either version 2.1 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU Lesser General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public License
## along with this program.  If not, see <https://www.gnu.org/licenses/>.

## Process this file with automake to produce Makefile.in.

AUTOMAKE_OPTIONS = 1.10 gnu no-dependencies
EXTRA_DIST =
BUILT_SOURCES =
MOSTLYCLEANFILES = core *.stackdump
CLEANFILES =
DISTCLEANFILES =
MAINTAINERCLEANFILES =
SUFFIXES =

# The VPATH variable allows builds with $builddir != $srcdir, assuming a
# 'make' program that supports VPATH (such as GNU make). The VPATH definition
# is removed by autoconf automatically when "$(srcdir)" = ".".
# In this directory, the VPATH handling is particular:
# 1. The .c -> .lo rule carefully uses $(srcdir), so that VPATH can be omitted.
# 2. If PACKAGE = gettext-tools, VPATH _must_ be omitted, because otherwise
#    'make' does the wrong thing if GNU gettext was configured with
#    "./configure --srcdir=`pwd`", namely it gets confused by the .lo and .la
#    files it finds in srcdir = ../../gettext-runtime/intl.
origsrcdir = @srcdir@
# In gettext-tools/intl/, distribute the right Makefile.in.
dist-hook:
if PACKAGE_IS_GETTEXT_TOOLS
	for file in Makefile.am Makefile.in; do \
	  cp -p $(origsrcdir)/$$file $(distdir)/$$file || exit 1; \
	done
else
	@:
endif

# -DBUILDING_LIBINTL: Change expansion of LIBINTL_DLL_EXPORTED macro.
# -DBUILDING_DLL: Change expansion of RELOCATABLE_DLL_EXPORTED macro.
AM_CPPFLAGS = \
  -DLOCALEDIR=\"$(localedir)\" -DLOCALE_ALIAS_PATH=\"$(aliaspath)\" \
  -DLIBDIR=\"$(libdir)\" -DBUILDING_LIBINTL -DBUILDING_DLL -DIN_LIBINTL

if WOE32
# On mingw, disable the declarations of *printf functions as aliases to the
# corresponding __mingw_*printf functions, because
# - these functions are useless for i18n purposes (not POSIX/XSI compliant),
# - they pull in a dependency to the libgcc_s_sjlj DLL (through the symbols
#   __udivdi3, __umoddi3).
AM_CPPFLAGS += -D__USE_MINGW_ANSI_STDIO=0
endif

# Parametrization of the 'relocatable-lib-lgpl' module.
AM_CPPFLAGS += \
  -DENABLE_RELOCATABLE=1 -DIN_LIBRARY -DINSTALLDIR=\"$(libdir)\" -DNO_XMALLOC \
  -Dset_relocation_prefix=libintl_set_relocation_prefix \
  -Drelocate=libintl_relocate \
  -Drelocate2=libintl_relocate2 \
  -DDEPENDS_ON_LIBICONV=1

BISON = @INTLBISON@
YACC = $(BISON) -d
BISONFLAGS = --name-prefix=__gettext

# Tell the ELF linker which symbols to export.
AM_CFLAGS = @CFLAG_VISIBILITY@
# Tell the mingw or Cygwin linker which symbols to export.
if WOE32DLL
AM_LDFLAGS = -Wl,--export-all-symbols
endif


# Library include file and code.

EXTRA_DIST += \
  gmo.h \
  gettextP.h \
  hash-string.h \
  loadinfo.h \
  plural-exp.h \
  eval-plural.h \
  localcharset.h \
  lock.h windows-mutex.h windows-rwlock.h windows-recmutex.h windows-once.h windows-initguard.h \
  relocatable.h \
  arg-nonnull.h \
  attribute.h \
  filename.h \
  flexmember.h \
  localename-table.in.h \
  setlocale_null.h \
  tsearch.h tsearch.c \
  verify.h \
  xsize.h \
  printf-args.h printf-args.c \
  printf-parse.h wprintf-parse.h printf-parse.c \
  vasnprintf.h vasnwprintf.h vasnprintf.c \
  intl-exports.c os2compat.h os2compat.c \
  libgnuintl.in.h

# Overview of the code
# --------------------
#
# The following files implement public API, declared in libgnuintl.in.h:
#   version.c:          libintl_version
#   gettext.c:          libintl_gettext
#   dgettext.c:         libintl_dgettext
#   dcgettext.c:        libintl_dcgettext
#   ngettext.c:         libintl_ngettext
#   dngettext.c:        libintl_dngettext
#   dcngettext.c:       libintl_dcngettext
#   textdomain.c:       libintl_textdomain
#   bindtextdom.c:      libintl_bindtextdomain
#                       libintl_bind_textdomain_codeset
#   printf.c:           libintl_fprintf
#                       libintl_vfprintf
#                       libintl_printf
#                       libintl_vprintf
#                       libintl_sprintf
#                       libintl_vsprintf
#                       libintl_snprintf
#                       libintl_vsnprintf
#                       libintl_asprintf
#                       libintl_vasprintf
#                       libintl_fwprintf
#                       libintl_vfwprintf
#                       libintl_wprintf
#                       libintl_vwprintf
#                       libintl_swprintf
#                       libintl_vswprintf
#   localename.c:       libintl_newlocale
#                       libintl_duplocale
#                       libintl_freelocale
#   setlocale.c:        libintl_newlocale
#                       libintl_setlocale
#   relocatable.c:      libintl_set_relocation_prefix
#
# The following files implement private API.
#   gettextP.h:     dcigettext.c:       libintl_dcigettext
#                                       _nl_find_msg
#                                       libintl_nl_default_dirname
#                                       libintl_nl_domain_bindings
#                                       libintl_nl_default_default_domain
#                                       libintl_nl_current_default_domain
#                   localename.c:       _nl_locale_name_canonicalize
#                                       _nl_locale_name_from_win32_LANGID
#                                       _nl_locale_name_from_win32_LCID
#                                       _nl_locale_name_thread_unsafe
#                                       _nl_locale_name_thread
#                                       _nl_locale_name_posix
#                                       _nl_locale_name_environ
#                                       _nl_locale_name_default
#                                       _nl_locale_name
#                   langprefs.c:        _nl_language_preferences_default
#                   finddomain.c:       _nl_find_domain
#                   loadmsgcat.c:       _nl_load_domain
#                                       _nl_msg_cat_cntr
#   loadinfo.h:     l10nflist.c:        _nl_make_l10nflist
#                                       _nl_normalize_codeset
#                   localealias.c:      _nl_expand_alias
#                   explodename.c:      _nl_explode_name
#   plural-exp.h:   plural.y:           libintl_gettext_free_exp
#                                       libintl_gettextparse
#                   plural-exp.c:       libintl_gettext_germanic_plural
#                                       libintl_gettext_extract_plural
#                   eval-plural.h:      plural_eval
#
# Other files:
#   gmo.h:          Describes the GNU MO file format.
#   hash-string.h:  hash-string.c:      libintl_hash_string
#
# Some other files are borrowed from gnulib, some with small modifications.
#
# Overview of invocation hierarchy
# --------------------------------
#
# The main general translation lookup function is libintl_dcigettext.
# libintl_dcigettext
#   \-> guess_category_value
#         \-> _nl_locale_name_thread_unsafe
#         \-> _nl_locale_name_posix
#         \-> _nl_locale_name_default
#         \-> _nl_language_preferences_default
#   \-> _nl_find_domain
#         \-> _nl_make_l10nflist
#         \-> _nl_load_domain
#         \-> _nl_expand_alias
#         \-> _nl_explode_name
#               \-> _nl_normalize_codeset
#   \-> _nl_find_msg
#         \-> _nl_load_domain
#         \-> get_output_charset
#   \-> plural_lookup
#         \-> plural_eval
#
# _nl_load_domain loads a MO file into memory.
# _nl_load_domain
#   \-> _nl_find_msg (for the header entry only)
#   \-> libintl_gettext_extract_plural

LIBINTLSOURCES = \
  bindtextdom.c \
  dcgettext.c \
  dgettext.c \
  gettext.c \
  finddomain.c \
  hash-string.c \
  loadmsgcat.c \
  localealias.c \
  textdomain.c \
  l10nflist.c \
  explodename.c \
  dcigettext.c \
  dcngettext.c \
  dngettext.c \
  ngettext.c \
  plural.y \
  plural-exp.c \
  localcharset.c \
  threadlib.c \
  lock.c \
  relocatable.c \
  langprefs.c \
  localename.c \
  localename-table.c \
  log.c \
  printf.c \
  setlocale.c \
  setlocale-lock.c \
  setlocale_null.c \
  version.c \
  xsize.c \
  osdep.c \
  intl-compat.c
if WINDOWS_NATIVE
LIBINTLSOURCES += \
  windows-mutex.c \
  windows-rwlock.c \
  windows-recmutex.c \
  windows-once.c
endif

# We must not install the libintl.h/libintl.la files if we are on a
# system which has the GNU gettext() function in its C library or in a
# separate library.
if USE_INCLUDED_LIBINTL
nodist_include_HEADERS = libintl.h
# Note: libintl.la actually gets installed in $(libdir). See below.
noinst_LTLIBRARIES = libintl.la
libintl_la_SOURCES = $(LIBINTLSOURCES)
else
if BUILD_INCLUDED_LIBINTL
noinst_LTLIBRARIES = libgnuintl.la
libgnuintl_la_SOURCES = $(LIBINTLSOURCES)
endif
endif

# export.h is an auxiliary file used in constructing the .h files.
EXTRA_DIST += export.h

libgnuintl.h: $(srcdir)/libgnuintl.in.h $(srcdir)/export.h
	sed -e '/IN_LIBGLOCALE/d' \
	    -e 's,@''HAVE_POSIX_PRINTF''@,@HAVE_POSIX_PRINTF@,g' \
	    -e 's,@''HAVE_ASPRINTF''@,@HAVE_ASPRINTF@,g' \
	    -e 's,@''HAVE_SNPRINTF''@,@HAVE_SNPRINTF@,g' \
	    -e 's,@''HAVE_WPRINTF''@,@HAVE_WPRINTF@,g' \
	    -e 's,@''HAVE_NAMELESS_LOCALES''@,@HAVE_NAMELESS_LOCALES@,g' \
	    -e 's,@''HAVE_NEWLOCALE''@,@HAVE_NEWLOCALE@,g' \
	  < $(srcdir)/libgnuintl.in.h \
	| if test '@WOE32DLL@' = yes; then \
	    sed -e 's/extern \([^()]*\);/extern __declspec (dllimport) \1;/'; \
	  else \
	    cat; \
	  fi \
	| sed -e 's/extern \([^"]\)/extern LIBINTL_DLL_EXPORTED \1/' \
	      -e "/#define _LIBINTL_H/r $(srcdir)/export.h" \
	| sed -e 's,@''HAVE_VISIBILITY''@,@HAVE_VISIBILITY@,g' \
	  > libgnuintl.h
MOSTLYCLEANFILES += libgnuintl.h

libintl.h: $(srcdir)/libgnuintl.in.h
	sed -e '/IN_LIBGLOCALE/d' \
	    -e 's,@''HAVE_POSIX_PRINTF''@,@HAVE_POSIX_PRINTF@,g' \
	    -e 's,@''HAVE_ASPRINTF''@,@HAVE_ASPRINTF@,g' \
	    -e 's,@''HAVE_SNPRINTF''@,@HAVE_SNPRINTF@,g' \
	    -e 's,@''HAVE_WPRINTF''@,@HAVE_WPRINTF@,g' \
	    -e 's,@''HAVE_NAMELESS_LOCALES''@,@HAVE_NAMELESS_LOCALES@,g' \
	    -e 's,@''HAVE_NEWLOCALE''@,@HAVE_NEWLOCALE@,g' \
	  < $(srcdir)/libgnuintl.in.h > libintl.h
MOSTLYCLEANFILES += libintl.h

localename-table.h: $(srcdir)/localename-table.in.h $(srcdir)/export.h
	sed -e 's/extern \([^"]\)/extern LIBINTL_DLL_EXPORTED \1/' \
	    -e 's/extern,/extern LIBINTL_DLL_EXPORTED,/' \
	    -e "/#if HAVE_WORKING_USELOCALE/r $(srcdir)/export.h" \
	  < $(srcdir)/localename-table.in.h \
	| sed -e 's,@''HAVE_VISIBILITY''@,@HAVE_VISIBILITY@,g' \
	  > localename-table.h
MOSTLYCLEANFILES += localename-table.h

# The Automake generated .y.c rule is broken: When executed in a VPATH build,
#   - The .c file gets generated in the build directory. But since it requires
#     special tools to rebuild it, we need to distribute it in the tarballs,
#     and by the GNU Coding Standards
#     <https://www.gnu.org/prep/standards/html_node/Makefile-Basics.html>
#     the file should be generated in the source directory.
#   - The #line numbers in the .c file refer to a nonexistent file once it
#     has been moved from the build directory to the source directory. This
#     leads to error if 'lcov' is used later.
# Additionally, here we assume GNU Bison and therefore don't need the ylwrap
# script.
# Additionally, here we need a rule that works even without a VPATH variable.
# Therefore we override this rule.

$(srcdir)/plural.c: $(srcdir)/plural.y
	$(AM_V_YACC)$(YACC) $(BISONFLAGS) --output plural.c $(srcdir)/plural.y \
	&& test ':' = '$(BISON)' || { \
	  sed -e 's|".*/plural.y"|"plural.y"|' < plural.c > plural.c-tmp \
	  && rm -f plural.c plural.h \
	  && mv plural.c-tmp $(srcdir)/plural.c; \
	}
# Don't put plural.c into BUILT_SOURCES. Since plural.c is in the source
# directory, 'make' does not find it without a VPATH variable.
MOSTLYCLEANFILES     += plural.c-tmp
MAINTAINERCLEANFILES += plural.c
EXTRA_DIST           += plural.c

# Rules for compiling a .c file, that work even without a VPATH variable.
bindtextdom.lo: $(srcdir)/bindtextdom.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/bindtextdom.c
dcgettext.lo: $(srcdir)/dcgettext.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/dcgettext.c
dgettext.lo: $(srcdir)/dgettext.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/dgettext.c
gettext.lo: $(srcdir)/gettext.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/gettext.c
finddomain.lo: $(srcdir)/finddomain.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/finddomain.c
hash-string.lo: $(srcdir)/hash-string.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/hash-string.c
loadmsgcat.lo: $(srcdir)/loadmsgcat.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/loadmsgcat.c
localealias.lo: $(srcdir)/localealias.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/localealias.c
textdomain.lo: $(srcdir)/textdomain.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/textdomain.c
l10nflist.lo: $(srcdir)/l10nflist.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/l10nflist.c
explodename.lo: $(srcdir)/explodename.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/explodename.c
dcigettext.lo: $(srcdir)/dcigettext.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/dcigettext.c
dcngettext.lo: $(srcdir)/dcngettext.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/dcngettext.c
dngettext.lo: $(srcdir)/dngettext.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/dngettext.c
ngettext.lo: $(srcdir)/ngettext.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/ngettext.c
plural.lo: $(srcdir)/plural.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/plural.c
plural-exp.lo: $(srcdir)/plural-exp.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/plural-exp.c
localcharset.lo: $(srcdir)/localcharset.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/localcharset.c
threadlib.lo: $(srcdir)/threadlib.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/threadlib.c
lock.lo: $(srcdir)/lock.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/lock.c
relocatable.lo: $(srcdir)/relocatable.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/relocatable.c
langprefs.lo: $(srcdir)/langprefs.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/langprefs.c
localename.lo: $(srcdir)/localename.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/localename.c
localename-table.lo: $(srcdir)/localename-table.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/localename-table.c
log.lo: $(srcdir)/log.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/log.c
printf.lo: $(srcdir)/printf.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/printf.c
setlocale.lo: $(srcdir)/setlocale.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/setlocale.c
setlocale-lock.lo: $(srcdir)/setlocale-lock.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/setlocale-lock.c
setlocale_null.lo: $(srcdir)/setlocale_null.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/setlocale_null.c
version.lo: $(srcdir)/version.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/version.c
xsize.lo: $(srcdir)/xsize.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/xsize.c
osdep.lo: $(srcdir)/osdep.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/osdep.c
intl-compat.lo: $(srcdir)/intl-compat.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/intl-compat.c
windows-mutex.lo: $(srcdir)/windows-mutex.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/windows-mutex.c
windows-rwlock.lo: $(srcdir)/windows-rwlock.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/windows-rwlock.c
windows-recmutex.lo: $(srcdir)/windows-recmutex.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/windows-recmutex.c
windows-once.lo: $(srcdir)/windows-once.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/windows-once.c

# Dependencies.

PLURAL_DEPS =
if USE_INCLUDED_LIBINTL
# A bison-2.1 generated plural.c includes <libintl.h> if ENABLE_NLS.
PLURAL_DEPS += libintl.h
endif

bindtextdom.lo:      ../config.h $(srcdir)/gettextP.h libgnuintl.h $(srcdir)/gmo.h $(srcdir)/loadinfo.h $(srcdir)/lock.h $(srcdir)/windows-mutex.h $(srcdir)/windows-rwlock.h $(srcdir)/windows-recmutex.h $(srcdir)/windows-once.h $(srcdir)/windows-initguard.h
dcgettext.lo:        ../config.h $(srcdir)/gettextP.h libgnuintl.h $(srcdir)/gmo.h $(srcdir)/loadinfo.h $(srcdir)/lock.h $(srcdir)/windows-mutex.h $(srcdir)/windows-rwlock.h $(srcdir)/windows-recmutex.h $(srcdir)/windows-once.h $(srcdir)/windows-initguard.h
dgettext.lo:         ../config.h $(srcdir)/gettextP.h libgnuintl.h $(srcdir)/gmo.h $(srcdir)/loadinfo.h $(srcdir)/lock.h $(srcdir)/windows-mutex.h $(srcdir)/windows-rwlock.h $(srcdir)/windows-recmutex.h $(srcdir)/windows-once.h $(srcdir)/windows-initguard.h
gettext.lo:          ../config.h $(srcdir)/gettextP.h libgnuintl.h $(srcdir)/gmo.h $(srcdir)/loadinfo.h $(srcdir)/lock.h $(srcdir)/windows-mutex.h $(srcdir)/windows-rwlock.h $(srcdir)/windows-recmutex.h $(srcdir)/windows-once.h $(srcdir)/windows-initguard.h
finddomain.lo:       ../config.h $(srcdir)/gettextP.h libgnuintl.h $(srcdir)/gmo.h $(srcdir)/loadinfo.h $(srcdir)/lock.h $(srcdir)/windows-mutex.h $(srcdir)/windows-rwlock.h $(srcdir)/windows-recmutex.h $(srcdir)/windows-once.h $(srcdir)/windows-initguard.h
hash-string.lo:      ../config.h $(srcdir)/hash-string.h
loadmsgcat.lo:       ../config.h $(srcdir)/gettextP.h libgnuintl.h $(srcdir)/gmo.h $(srcdir)/loadinfo.h $(srcdir)/lock.h $(srcdir)/windows-mutex.h $(srcdir)/windows-rwlock.h $(srcdir)/windows-recmutex.h $(srcdir)/windows-once.h $(srcdir)/windows-initguard.h $(srcdir)/hash-string.h $(srcdir)/plural-exp.h
localealias.lo:      ../config.h $(srcdir)/gettextP.h libgnuintl.h $(srcdir)/gmo.h $(srcdir)/loadinfo.h $(srcdir)/lock.h $(srcdir)/windows-mutex.h $(srcdir)/windows-rwlock.h $(srcdir)/windows-recmutex.h $(srcdir)/windows-once.h $(srcdir)/windows-initguard.h $(srcdir)/relocatable.h
textdomain.lo:       ../config.h $(srcdir)/gettextP.h libgnuintl.h $(srcdir)/gmo.h $(srcdir)/loadinfo.h $(srcdir)/lock.h $(srcdir)/windows-mutex.h $(srcdir)/windows-rwlock.h $(srcdir)/windows-recmutex.h $(srcdir)/windows-once.h $(srcdir)/windows-initguard.h
l10nflist.lo:        ../config.h $(srcdir)/loadinfo.h $(srcdir)/filename.h
explodename.lo:      ../config.h $(srcdir)/loadinfo.h
dcigettext.lo:       ../config.h $(srcdir)/gettextP.h libgnuintl.h $(srcdir)/gmo.h $(srcdir)/loadinfo.h $(srcdir)/lock.h $(srcdir)/windows-mutex.h $(srcdir)/windows-rwlock.h $(srcdir)/windows-recmutex.h $(srcdir)/windows-once.h $(srcdir)/windows-initguard.h $(srcdir)/localcharset.h $(srcdir)/plural-exp.h $(srcdir)/hash-string.h $(srcdir)/tsearch.h $(srcdir)/tsearch.c $(srcdir)/eval-plural.h $(srcdir)/filename.h
dcngettext.lo:       ../config.h $(srcdir)/gettextP.h libgnuintl.h $(srcdir)/gmo.h $(srcdir)/loadinfo.h $(srcdir)/lock.h $(srcdir)/windows-mutex.h $(srcdir)/windows-rwlock.h $(srcdir)/windows-recmutex.h $(srcdir)/windows-once.h $(srcdir)/windows-initguard.h
dngettext.lo:        ../config.h $(srcdir)/gettextP.h libgnuintl.h $(srcdir)/gmo.h $(srcdir)/loadinfo.h $(srcdir)/lock.h $(srcdir)/windows-mutex.h $(srcdir)/windows-rwlock.h $(srcdir)/windows-recmutex.h $(srcdir)/windows-once.h $(srcdir)/windows-initguard.h
ngettext.lo:         ../config.h $(srcdir)/gettextP.h libgnuintl.h $(srcdir)/gmo.h $(srcdir)/loadinfo.h $(srcdir)/lock.h $(srcdir)/windows-mutex.h $(srcdir)/windows-rwlock.h $(srcdir)/windows-recmutex.h $(srcdir)/windows-once.h $(srcdir)/windows-initguard.h
plural.lo:           ../config.h $(srcdir)/plural-exp.h $(PLURAL_DEPS)
plural-exp.lo:       ../config.h $(srcdir)/plural-exp.h
localcharset.lo:     ../config.h $(srcdir)/localcharset.h
threadlib.lo:        ../config.h
lock.lo:             ../config.h $(srcdir)/lock.h $(srcdir)/windows-mutex.h $(srcdir)/windows-rwlock.h $(srcdir)/windows-recmutex.h $(srcdir)/windows-once.h $(srcdir)/windows-initguard.h
relocatable.lo:      ../config.h $(srcdir)/relocatable.h
langprefs.lo:        ../config.h
localename.lo:       ../config.h $(srcdir)/gettextP.h libgnuintl.h $(srcdir)/gmo.h $(srcdir)/loadinfo.h $(srcdir)/lock.h $(srcdir)/windows-mutex.h $(srcdir)/windows-rwlock.h $(srcdir)/windows-recmutex.h $(srcdir)/windows-once.h $(srcdir)/windows-initguard.h $(srcdir)/flexmember.h $(srcdir)/setlocale_null.h $(srcdir)/arg-nonnull.h localename-table.h
localename-table.lo: ../config.h localename-table.h $(srcdir)/lock.h $(srcdir)/windows-mutex.h $(srcdir)/windows-rwlock.h $(srcdir)/windows-recmutex.h $(srcdir)/windows-once.h $(srcdir)/windows-initguard.h
log.lo:              ../config.h $(srcdir)/lock.h $(srcdir)/windows-mutex.h $(srcdir)/windows-rwlock.h $(srcdir)/windows-recmutex.h $(srcdir)/windows-once.h $(srcdir)/windows-initguard.h
printf.lo:           ../config.h $(srcdir)/printf-args.c $(srcdir)/printf-args.h $(srcdir)/printf-parse.c $(srcdir)/printf-parse.h $(srcdir)/wprintf-parse.h $(srcdir)/xsize.h $(srcdir)/vasnprintf.c $(srcdir)/vasnprintf.h $(srcdir)/vasnwprintf.h $(srcdir)/verify.h
setlocale.lo:        ../config.h $(srcdir)/gettextP.h libgnuintl.h $(srcdir)/gmo.h $(srcdir)/loadinfo.h $(srcdir)/lock.h $(srcdir)/windows-mutex.h $(srcdir)/windows-rwlock.h $(srcdir)/windows-recmutex.h $(srcdir)/windows-once.h $(srcdir)/windows-initguard.h
setlocale-lock.lo:   ../config.h $(srcdir)/windows-initguard.h
setlocale_null.lo:   ../config.h $(srcdir)/setlocale_null.h $(srcdir)/arg-nonnull.h
version.lo:          ../config.h libgnuintl.h
xsize.lo:            ../config.h $(srcdir)/xsize.h
osdep.lo:            ../config.h $(srcdir)/intl-exports.c $(srcdir)/os2compat.c
intl-compat.lo:      ../config.h $(srcdir)/gettextP.h libgnuintl.h $(srcdir)/gmo.h $(srcdir)/loadinfo.h $(srcdir)/lock.h $(srcdir)/windows-mutex.h $(srcdir)/windows-rwlock.h $(srcdir)/windows-recmutex.h $(srcdir)/windows-once.h $(srcdir)/windows-initguard.h
windows-mutex.lo:    ../config.h $(srcdir)/windows-mutex.h $(srcdir)/windows-initguard.h
windows-rwlock.lo:   ../config.h $(srcdir)/windows-rwlock.h $(srcdir)/windows-initguard.h
windows-recmutex.lo: ../config.h $(srcdir)/windows-recmutex.h $(srcdir)/windows-initguard.h
windows-once.lo:     ../config.h $(srcdir)/windows-once.h

# Version information according to Woe32 conventions.
EXTRA_DIST += libintl.rc
if WOE32
WOE32_LIBADD = libintl.res.lo
# This rule is executed only on Woe32 systems.
# Use $(RC) with libtool, $(WINDRES) when not using libtool.
# The following sed expressions come from the windres-options script. They are
# inlined here, so that they can be written in a Makefile without requiring a
# temporary file. They must contain literal newlines rather than semicolons,
# so that they work with the sed-3.02 that is shipped with MSYS.
libintl.res.lo: $(srcdir)/libintl.rc
	nlinit=`echo 'nl="'; echo '"'`; eval "$$nlinit"; \
	sed_extract_major='/^[0-9]/{'$${nl}'s/^\([0-9]*\).*/\1/p'$${nl}q$${nl}'}'$${nl}'c\'$${nl}0$${nl}q; \
	sed_extract_minor='/^[0-9][0-9]*[.][0-9]/{'$${nl}'s/^[0-9]*[.]\([0-9]*\).*/\1/p'$${nl}q$${nl}'}'$${nl}'c\'$${nl}0$${nl}q; \
	sed_extract_subminor='/^[0-9][0-9]*[.][0-9][0-9]*[.][0-9]/{'$${nl}'s/^[0-9]*[.][0-9]*[.]\([0-9]*\).*/\1/p'$${nl}q$${nl}'}'$${nl}'c\'$${nl}0$${nl}q; \
	$(LIBTOOL) --tag=RC $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=compile $(RC) \
	  "-DPACKAGE_VERSION_STRING=\\\"$(VERSION)\\\"" \
	  "-DPACKAGE_VERSION_MAJOR="`echo '$(VERSION)' | sed -n -e "$$sed_extract_major"` \
	  "-DPACKAGE_VERSION_MINOR="`echo '$(VERSION)' | sed -n -e "$$sed_extract_minor"` \
	  "-DPACKAGE_VERSION_SUBMINOR="`echo '$(VERSION)' | sed -n -e "$$sed_extract_subminor"` \
	  -i $(srcdir)/libintl.rc -o libintl.res.lo --output-format=coff
MOSTLYCLEANFILES += libintl.res.lo
else
WOE32_LIBADD =
endif
libintl_la_LIBADD          = $(WOE32_LIBADD)
libintl_la_DEPENDENCIES    = $(WOE32_LIBADD)
libgnuintl_la_LIBADD       = $(WOE32_LIBADD)
libgnuintl_la_DEPENDENCIES = $(WOE32_LIBADD)

# langprefs.c (_nl_language_preferences_win32_95) uses functions from
# advapi32.dll.
if WINDOWS_NATIVE
INTL_WINDOWS_LIBS = -ladvapi32
else
INTL_WINDOWS_LIBS =
endif

# Libtool's library version information for libintl.
# Before making a gettext release, you must change this according to the
# libtool documentation, section "Library interface versions".
LTV_CURRENT=10
LTV_REVISION=0
LTV_AGE=2

# How to build libintl.la and libgnuintl.la.
# Limit the exported symbols: Don't export glwthread* (from gnulib modules).
OTHER_LDFLAGS = \
  @LTLIBICONV@ @INTL_MACOSX_LIBS@ $(INTL_WINDOWS_LIBS) @LTLIBTHREAD@ \
  -no-undefined \
  -export-symbols-regex '^([^g]|g[^l]|gl[^w]|glw[^t]|glwt[^h]|glwth[^r]|glwthr[^e]|glwthre[^a]|glwthrea[^d]).*' \
  -version-info $(LTV_CURRENT):$(LTV_REVISION):$(LTV_AGE) \
  -rpath $(libdir)
libintl_la_LDFLAGS    = $(AM_LDFLAGS) $(OTHER_LDFLAGS)
libgnuintl_la_LDFLAGS = $(AM_LDFLAGS) $(OTHER_LDFLAGS)


# Installation of libintl.la.

if USE_INCLUDED_LIBINTL

install-exec-local: install-exec-libintl
install-exec-libintl: libintl.la
	$(mkdir_p) $(DESTDIR)$(libdir)
	$(LIBTOOL) --mode=install \
	  $(INSTALL_DATA) libintl.la $(DESTDIR)$(libdir)/libintl.la; \
	if test "@RELOCATABLE@" = yes; then \
	  dependencies=`sed -n -e 's,^dependency_libs=\(.*\),\1,p' < $(DESTDIR)$(libdir)/libintl.la | sed -e "s,^',," -e "s,'\$$,,"`; \
	  if test -n "$$dependencies"; then \
	    rm -f $(DESTDIR)$(libdir)/libintl.la; \
	  fi; \
	fi

installdirs-local: installdirs-libintl
installdirs-libintl:
	$(mkdir_p) $(DESTDIR)$(libdir)

uninstall-local: uninstall-libintl
uninstall-libintl:
	$(LIBTOOL) --mode=uninstall \
	  rm -f $(DESTDIR)$(libdir)/libintl.la

endif


# Installation of preloadable_libintl.so.

if PRELOADABLE_LIBINTL

install-exec-local: install-exec-preloadable
install-exec-preloadable: libgnuintl.la
	$(mkdir_p) $(DESTDIR)$(libdir)
	$(LIBTOOL) --mode=install \
	  $(INSTALL_DATA) libgnuintl.la $(DESTDIR)$(libdir)/libgnuintl.la
	rm -f $(DESTDIR)$(libdir)/preloadable_libintl.so
	$(INSTALL_DATA) $(DESTDIR)$(libdir)/libgnuintl.so $(DESTDIR)$(libdir)/preloadable_libintl.so
	$(LIBTOOL) --mode=uninstall \
	  rm -f $(DESTDIR)$(libdir)/libgnuintl.la

installdirs-local: installdirs-preloadable
installdirs-preloadable:
	$(mkdir_p) $(DESTDIR)$(libdir)

uninstall-local: uninstall-preloadable
uninstall-preloadable:
	rm -f $(DESTDIR)$(libdir)/preloadable_libintl.so

endif


# Installation of locale.alias.

EXTRA_DIST += locale.alias ref-add.sin ref-del.sin

if USE_INCLUDED_LIBINTL

all-local: ref-add.sed ref-del.sed

ref-add.sed: $(srcdir)/ref-add.sin
	sed -e '/^#/d' -e 's/@''PACKAGE''@/@PACKAGE@/g' $(srcdir)/ref-add.sin > t-ref-add.sed
	mv t-ref-add.sed ref-add.sed
MOSTLYCLEANFILES += ref-add.sed

ref-del.sed: $(srcdir)/ref-del.sin
	sed -e '/^#/d' -e 's/@''PACKAGE''@/@PACKAGE@/g' $(srcdir)/ref-del.sin > t-ref-del.sed
	mv t-ref-del.sed ref-del.sed
MOSTLYCLEANFILES += ref-del.sed

install-data-local: install-data-aliasfile
install-data-aliasfile: ref-add.sed
	$(mkdir_p) $(DESTDIR)$(localedir)
	test -f $(DESTDIR)$(localedir)/locale.alias \
	  && orig=$(DESTDIR)$(localedir)/locale.alias \
	  || orig=$(srcdir)/locale.alias; \
	temp=$(DESTDIR)$(localedir)/t-locale.alias; \
	dest=$(DESTDIR)$(localedir)/locale.alias; \
	sed -f ref-add.sed $$orig > $$temp; \
	$(INSTALL_DATA) $$temp $$dest; \
	rm -f $$temp

installdirs-local: installdirs-aliasfile
installdirs-aliasfile:
	$(mkdir_p) $(DESTDIR)$(localedir)

uninstall-local: uninstall-aliasfile
uninstall-aliasfile: ref-del.sed
	if test -f $(DESTDIR)$(localedir)/locale.alias; then \
	  temp=$(DESTDIR)$(localedir)/t-locale.alias; \
	  dest=$(DESTDIR)$(localedir)/locale.alias; \
	  sed -f ref-del.sed $$dest > $$temp; \
	  if grep '^# Packages using this file: $$' $$temp > /dev/null; then \
	    rm -f $$dest; \
	  else \
	    $(INSTALL_DATA) $$temp $$dest; \
	  fi; \
	  rm -f $$temp; \
	fi

endif


# We don't install the source code any more.

if PACKAGE_IS_GETTEXT_TOOLS

install-data-local: uninstall-sources
uninstall-local: uninstall-sources
uninstall-sources:
	rm -rf $(DESTDIR)$(datadir)/gettext/intl

endif


# Clean up after Solaris cc.
clean-local:
	rm -rf SunWS_cache


# Windows support.

EXTRA_DIST += INSTALL.windows


# Miscellaneous files.

EXTRA_DIST += COPYING.LIB libintl.glibc
