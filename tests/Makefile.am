## Makefile for the check subdirectory of the GNU NLS Utilities
## Copyright (C) 1995-1997, 2001-2002 Free Software Foundation, Inc.
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

## Process this file with automake to produce Makefile.in.

AUTOMAKE_OPTIONS = 1.5 gnits no-dependencies

TESTS = gettext-1 gettext-2 \
	msgattrib-1 msgattrib-2 msgattrib-3 msgattrib-4 msgattrib-5 \
	msgattrib-6 msgattrib-7 msgattrib-8 msgattrib-9 msgattrib-10 \
	msgattrib-11 msgattrib-12 msgattrib-13 msgattrib-14 \
	msgcat-1 msgcat-2 msgcat-3 msgcat-4 msgcat-5 msgcat-6 msgcat-7 \
	msgcmp-1 msgcmp-2 \
	msgcomm-1 msgcomm-2 msgcomm-3 msgcomm-4 msgcomm-5 msgcomm-6 msgcomm-7 \
	msgcomm-8 msgcomm-9 msgcomm-10 msgcomm-11 msgcomm-12 msgcomm-13 \
	msgcomm-14 msgcomm-15 msgcomm-16 msgcomm-17 msgcomm-18 msgcomm-19 \
	msgcomm-20 msgcomm-21 msgcomm-22 msgcomm-23 \
	msgconv-1 msgconv-2 msgconv-3 \
	msgen-1 \
	msgexec-1 msgexec-2 \
	msgfilter-1 msgfilter-2 \
	msgfmt-1 msgfmt-2 msgfmt-3 msgfmt-4 msgfmt-5 msgfmt-6 msgfmt-7 \
	msgfmt-8 msgfmt-9 msgfmt-10 msgfmt-11 msgfmt-12 \
	msggrep-1 msggrep-2 msggrep-3 msggrep-4 msggrep-5 \
	msgmerge-1 msgmerge-2 msgmerge-3 msgmerge-4 msgmerge-5 msgmerge-6 \
	msgmerge-7 msgmerge-8 msgmerge-9 msgmerge-10 msgmerge-11 msgmerge-12 \
	msgmerge-13 msgmerge-14 msgmerge-15 msgmerge-16 msgmerge-17 \
	msgmerge-18 msgmerge-19 msgmerge-20 \
	msgunfmt-1 msgunfmt-2 msgunfmt-3 \
	msguniq-1 msguniq-2 msguniq-3 \
	xgettext-1 xgettext-2 xgettext-3 xgettext-4 xgettext-5 xgettext-6 \
	xgettext-7 xgettext-8 xgettext-9 xgettext-10 xgettext-11 xgettext-12 \
	xgettext-13 xgettext-14 xgettext-15 xgettext-16 xgettext-17 \
	xgettext-18 xgettext-19 xgettext-20 xgettext-21 xgettext-22 \
	format-awk-1 format-awk-2 \
	format-c-1 format-c-2 format-c-3 format-c-4 \
	format-elisp-1 format-elisp-2 \
	format-java-1 format-java-2 \
	format-librep-1 format-librep-2 \
	format-lisp-1 format-lisp-2 \
	format-python-1 format-python-2 \
	format-pascal-1 format-pascal-2 \
	format-tcl-1 format-tcl-2 \
	format-ycp-1 format-ycp-2 \
	plural-1 plural-2 \
	lang-c lang-c++ lang-objc lang-python lang-clisp lang-elisp lang-librep lang-java lang-gawk lang-pascal lang-ycp lang-tcl lang-po lang-rst \
	rpath-1a rpath-1b \
	rpath-2aaa rpath-2aab rpath-2aac rpath-2aad \
	rpath-2aba rpath-2abb rpath-2abc rpath-2abd \
	rpath-2baa rpath-2bab rpath-2bac rpath-2bad \
	rpath-2bba rpath-2bbb rpath-2bbc rpath-2bbd \
	rpath-3aaa rpath-3aab rpath-3aac rpath-3aad \
	           rpath-3aaf            rpath-3aah \
	rpath-3aba rpath-3abb rpath-3abc rpath-3abd \
	           rpath-3abf            rpath-3abh \
	rpath-3baa rpath-3bab rpath-3bac rpath-3bad \
	rpath-3bae rpath-3baf rpath-3bag rpath-3bah \
	rpath-3bba rpath-3bbb rpath-3bbc rpath-3bbd \
	rpath-3bbe rpath-3bbf rpath-3bbg rpath-3bbh

EXTRA_DIST = $(TESTS) \
	test.mo xg-test1.ok.po mex-test2.ok msguniq-a.in msguniq-a.out \
	rpathx/configure.in rpathx/aclocal.m4 rpathx/configure \
	rpathx/Makefile.am rpathx/Makefile.in rpathx/rpathx.c \
	rpathy/configure.in rpathy/aclocal.m4 rpathy/configure \
	rpathy/Makefile.am rpathy/Makefile.in rpathy/rpathy.c \
	rpathz/configure.in rpathz/aclocal.m4 rpathz/configure \
	rpathz/Makefile.am rpathz/Makefile.in rpathz/rpathz.c \
	rpathlx/configure.in rpathlx/aclocal.m4 rpathlx/configure \
	rpathlx/Makefile.am rpathlx/Makefile.in rpathlx/usex.c \
	rpathly/configure.in rpathly/aclocal.m4 rpathly/configure \
	rpathly/Makefile.am rpathly/Makefile.in rpathly/usey.c \
	rpathlyx/configure.in rpathlyx/aclocal.m4 rpathlyx/configure \
	rpathlyx/Makefile.am rpathlyx/Makefile.in rpathlyx/usey.c \
	rpathlz/configure.in rpathlz/aclocal.m4 rpathlz/configure \
	rpathlz/Makefile.am rpathlz/Makefile.in rpathlz/usez.c \
	rpathlzyx/configure.in rpathlzyx/aclocal.m4 rpathlzyx/configure \
	rpathlzyx/Makefile.am rpathlzyx/Makefile.in rpathlzyx/usez.c \
	rpath-1 rpath-2_a rpath-2_b rpath-3_a rpath-3_b rpath.README \
	rpath-3aae rpath-3aag rpath-3abe rpath-3abg \
	rpathcfg.sh

XGETTEXT = ../src/xgettext

TESTS_ENVIRONMENT = top_srcdir=$(top_srcdir) PATH=.:../src:$$PATH \
		    GETTEXT=tstgettext \
		    NGETTEXT=tstngettext \
		    XGETTEXT=xgettext \
		    MSGATTRIB=msgattrib \
		    MSGCAT=msgcat \
		    MSGCMP=msgcmp \
		    MSGCOMM=msgcomm \
		    MSGCONV=msgconv \
		    MSGEN=msgen \
		    MSGEXEC=msgexec \
		    MSGFILTER=msgfilter \
		    MSGFMT=msgfmt \
		    MSGGREP=msggrep \
		    MSGMERGE=msgmerge \
		    MSGUNFMT=msgunfmt \
		    MSGUNIQ=msguniq \
		    OBJEXT='@OBJEXT@' EXEEXT='@EXEEXT@' \
		    LIBTOOL='@LIBTOOL@' \
		    CC='@CC@' CFLAGS='@CFLAGS@' \
		    CXX='@CXX@' CXXFLAGS='@CXXFLAGS@' \
		    CPPFLAGS='@CPPFLAGS@' LDFLAGS='@LDFLAGS@' \
		    LTLIBINTL='@LTLIBINTL@' \
		    TESTJAVA='@TESTJAVA@' \
		    CONFIG_SHELL='$(SHELL)' \
		    $(SHELL)

xg-test1.ok.po: $(top_srcdir)/src/xgettext.c $(top_srcdir)/src/msgfmt.c \
		$(top_srcdir)/src/gettext.c
	$(XGETTEXT) -d xg-test1.ok -p $(srcdir) \
	  -k_ --omit-header --no-location \
	  $(top_srcdir)/src/xgettext.c $(top_srcdir)/src/msgfmt.c \
	  $(top_srcdir)/src/gettext.c

# Three auxiliary programs used by the tests.
INCLUDES = -I.. -I../lib -I$(top_srcdir)/lib -I$(top_srcdir)/intl
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@
LDADD = $(LDADD_@USE_INCLUDED_LIBINTL@)
LDADD_yes = ../intl/libintl.la
LDADD_no = ../intl/libgnuintl.la @LTLIBINTL@
noinst_PROGRAMS = tstgettext tstngettext cake fc3 fc4
tstgettext_SOURCES = tstgettext.c setlocale.c
tstgettext_LDADD = ../lib/libgettextlib.la $(LDADD)
tstngettext_SOURCES = tstngettext.c setlocale.c
tstngettext_LDADD = ../lib/libgettextlib.la $(LDADD)
cake_SOURCES = plural-1-prg.c setlocale.c
cake_LDADD = ../lib/libgettextlib.la $(LDADD)
fc3_SOURCES = format-c-3-prg.c setlocale.c
fc3_LDADD = ../lib/libgettextlib.la $(LDADD)
fc4_SOURCES = format-c-4-prg.c setlocale.c
fc4_LDADD = ../lib/libgettextlib.la $(LDADD)

# Help maintaining config.rpath.
rpathcfg: rpathcfg.sh
	top_srcdir=$(top_srcdir) \
	CONFIG_SHELL='$(SHELL)' \
	CC='@CC@' GCC='@GCC@' \
	LDFLAGS='@LDFLAGS@' \
	LD='@LD@' with_gnu_ld='@with_gnu_ld@' \
	host='@host@' \
	$(SHELL) $(srcdir)/rpathcfg.sh > rpathcfg 2> rpathcfg.log
MOSTLYCLEANFILES = rpathcfg rpathcfg.log
