## Makefile for the gettext-tools/examples subdirectory of GNU gettext
## Copyright (C) 2001-2003 Free Software Foundation, Inc.
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

AUTOMAKE_OPTIONS = 1.2 gnits
SUBDIRS = po
EXTRA_DIST =

docdir = @docdir@
examplesdir = $(docdir)/examples

examples_DATA = installpaths

EXAMPLESFILES = \
  \
  hello-c/INSTALL \
  hello-c/autogen.sh \
  hello-c/autoclean.sh \
  hello-c/hello.c \
  hello-c/Makefile.am \
  hello-c/configure.ac \
  hello-c/m4/Makefile.am \
  hello-c/po/Makevars \
  hello-c/po/POTFILES.in \
  hello-c/po/LINGUAS \
  hello-c/po/de.po \
  \
  hello-c-gnome/INSTALL \
  hello-c-gnome/autogen.sh \
  hello-c-gnome/autoclean.sh \
  hello-c-gnome/hello.c \
  hello-c-gnome/Makefile.am \
  hello-c-gnome/configure.ac \
  hello-c-gnome/m4/Makefile.am \
  hello-c-gnome/m4/gnome.m4 \
  hello-c-gnome/m4/gnome-gnorba-check.m4 \
  hello-c-gnome/m4/gnome-orbit-check.m4 \
  hello-c-gnome/po/Makevars \
  hello-c-gnome/po/POTFILES.in \
  hello-c-gnome/po/LINGUAS \
  hello-c-gnome/po/de.po \
  \
  hello-c++/INSTALL \
  hello-c++/autogen.sh \
  hello-c++/autoclean.sh \
  hello-c++/hello.cc \
  hello-c++/Makefile.am \
  hello-c++/configure.ac \
  hello-c++/m4/Makefile.am \
  hello-c++/po/Makevars \
  hello-c++/po/POTFILES.in \
  hello-c++/po/LINGUAS \
  hello-c++/po/de.po \
  \
  hello-c++-qt/INSTALL \
  hello-c++-qt/autogen.sh \
  hello-c++-qt/autoclean.sh \
  hello-c++-qt/hello.cc \
  hello-c++-qt/Makefile.am \
  hello-c++-qt/configure.ac \
  hello-c++-qt/m4/Makefile.am \
  hello-c++-qt/m4/qt.m4 \
  hello-c++-qt/po/Makefile.am \
  hello-c++-qt/po/LINGUAS \
  hello-c++-qt/po/de.po \
  \
  hello-c++-kde/INSTALL \
  hello-c++-kde/autogen.sh \
  hello-c++-kde/autoclean.sh \
  hello-c++-kde/hello.cc \
  hello-c++-kde/hellowindow.h \
  hello-c++-kde/hellowindow.cc \
  hello-c++-kde/Makefile.am \
  hello-c++-kde/configure.in.in \
  hello-c++-kde/m4/Makefile.am \
  hello-c++-kde/po/Makevars \
  hello-c++-kde/po/POTFILES.in \
  hello-c++-kde/po/LINGUAS \
  hello-c++-kde/po/de.po \
  \
  hello-c++-gnome/INSTALL \
  hello-c++-gnome/autogen.sh \
  hello-c++-gnome/autoclean.sh \
  hello-c++-gnome/hello.cc \
  hello-c++-gnome/Makefile.am \
  hello-c++-gnome/configure.ac \
  hello-c++-gnome/m4/Makefile.am \
  hello-c++-gnome/m4/gnome.m4 \
  hello-c++-gnome/m4/gnome-gnorba-check.m4 \
  hello-c++-gnome/m4/gnome-orbit-check.m4 \
  hello-c++-gnome/m4/gtk.m4 \
  hello-c++-gnome/m4/gtk--.m4 \
  hello-c++-gnome/po/Makevars \
  hello-c++-gnome/po/POTFILES.in \
  hello-c++-gnome/po/LINGUAS \
  hello-c++-gnome/po/de.po \
  \
  hello-objc/INSTALL \
  hello-objc/autogen.sh \
  hello-objc/autoclean.sh \
  hello-objc/hello.m \
  hello-objc/Makefile.am \
  hello-objc/configure.ac \
  hello-objc/m4/Makefile.am \
  hello-objc/po/Makevars \
  hello-objc/po/POTFILES.in \
  hello-objc/po/LINGUAS \
  hello-objc/po/de.po \
  \
  hello-objc-gnustep/INSTALL \
  hello-objc-gnustep/autogen.sh \
  hello-objc-gnustep/autoclean.sh \
  hello-objc-gnustep/main.m \
  hello-objc-gnustep/AppController.h \
  hello-objc-gnustep/AppController.m \
  hello-objc-gnustep/Hello.h \
  hello-objc-gnustep/Hello.m \
  hello-objc-gnustep/GNUmakefile \
  hello-objc-gnustep/po/GNUmakefile \
  hello-objc-gnustep/po/LocaleAliases \
  hello-objc-gnustep/po/LINGUAS \
  hello-objc-gnustep/po/de.po \
  \
  hello-objc-gnome/INSTALL \
  hello-objc-gnome/autogen.sh \
  hello-objc-gnome/autoclean.sh \
  hello-objc-gnome/hello.m \
  hello-objc-gnome/Makefile.am \
  hello-objc-gnome/configure.ac \
  hello-objc-gnome/m4/Makefile.am \
  hello-objc-gnome/m4/gnome.m4 \
  hello-objc-gnome/m4/gnome-gnorba-check.m4 \
  hello-objc-gnome/m4/gnome-orbit-check.m4 \
  hello-objc-gnome/po/Makevars \
  hello-objc-gnome/po/POTFILES.in \
  hello-objc-gnome/po/LINGUAS \
  hello-objc-gnome/po/de.po \
  \
  hello-sh/INSTALL \
  hello-sh/autogen.sh \
  hello-sh/autoclean.sh \
  hello-sh/hello.sh \
  hello-sh/Makefile.am \
  hello-sh/configure.ac \
  hello-sh/m4/Makefile.am \
  hello-sh/po/Makefile.am \
  hello-sh/po/LINGUAS \
  hello-sh/po/de.po \
  \
  hello-python/INSTALL \
  hello-python/autogen.sh \
  hello-python/autoclean.sh \
  hello-python/hello.py.in \
  hello-python/Makefile.am \
  hello-python/configure.ac \
  hello-python/m4/Makefile.am \
  hello-python/po/Makefile.am \
  hello-python/po/LINGUAS \
  hello-python/po/de.po \
  \
  hello-clisp/INSTALL \
  hello-clisp/autogen.sh \
  hello-clisp/autoclean.sh \
  hello-clisp/hello.lisp.in \
  hello-clisp/Makefile.am \
  hello-clisp/configure.ac \
  hello-clisp/m4/Makefile.am \
  hello-clisp/po/Makefile.am \
  hello-clisp/po/LINGUAS \
  hello-clisp/po/de.po \
  \
  hello-librep/INSTALL \
  hello-librep/autogen.sh \
  hello-librep/autoclean.sh \
  hello-librep/hello.jl.in \
  hello-librep/Makefile.am \
  hello-librep/configure.ac \
  hello-librep/m4/Makefile.am \
  hello-librep/po/Makefile.am \
  hello-librep/po/LINGUAS \
  hello-librep/po/de.po \
  \
  hello-smalltalk/INSTALL \
  hello-smalltalk/autogen.sh \
  hello-smalltalk/autoclean.sh \
  hello-smalltalk/hello.st.in \
  hello-smalltalk/Makefile.am \
  hello-smalltalk/configure.ac \
  hello-smalltalk/m4/Makefile.am \
  hello-smalltalk/po/Makefile.am \
  hello-smalltalk/po/LINGUAS \
  hello-smalltalk/po/de.po \
  \
  hello-java/INSTALL \
  hello-java/autogen.sh \
  hello-java/autoclean.sh \
  hello-java/Hello.java \
  hello-java/Makefile.am \
  hello-java/configure.ac \
  hello-java/m4/Makefile.am \
  hello-java/po/Makefile.am \
  hello-java/po/LINGUAS \
  hello-java/po/de.po \
  \
  hello-java-awt/INSTALL \
  hello-java-awt/autogen.sh \
  hello-java-awt/autoclean.sh \
  hello-java-awt/Hello.java \
  hello-java-awt/Makefile.am \
  hello-java-awt/configure.ac \
  hello-java-awt/m4/Makefile.am \
  hello-java-awt/m4/TestAWT.java \
  hello-java-awt/m4/TestAWT.class \
  hello-java-awt/po/Makefile.am \
  hello-java-awt/po/LINGUAS \
  hello-java-awt/po/de.po \
  \
  hello-java-swing/INSTALL \
  hello-java-swing/autogen.sh \
  hello-java-swing/autoclean.sh \
  hello-java-swing/Hello.java \
  hello-java-swing/Makefile.am \
  hello-java-swing/configure.ac \
  hello-java-swing/m4/Makefile.am \
  hello-java-swing/m4/TestAWT.java \
  hello-java-swing/m4/TestAWT.class \
  hello-java-swing/po/Makefile.am \
  hello-java-swing/po/LINGUAS \
  hello-java-swing/po/de.po \
  \
  hello-gawk/INSTALL \
  hello-gawk/autogen.sh \
  hello-gawk/autoclean.sh \
  hello-gawk/hello.awk \
  hello-gawk/Makefile.am \
  hello-gawk/configure.ac \
  hello-gawk/m4/Makefile.am \
  hello-gawk/po/Makefile.am \
  hello-gawk/po/LINGUAS \
  hello-gawk/po/de.po \
  \
  hello-pascal/INSTALL \
  hello-pascal/autogen.sh \
  hello-pascal/autoclean.sh \
  hello-pascal/hello.pas \
  hello-pascal/Makefile.am \
  hello-pascal/configure.ac \
  hello-pascal/m4/Makefile.am \
  hello-pascal/po/Makefile.am \
  hello-pascal/po/LINGUAS \
  hello-pascal/po/de.po \
  \
  hello-ycp/INSTALL \
  hello-ycp/autogen.sh \
  hello-ycp/autoclean.sh \
  hello-ycp/hello.ycp \
  hello-ycp/Makefile.am \
  hello-ycp/configure.ac \
  hello-ycp/m4/Makefile.am \
  hello-ycp/po/Makefile.am \
  hello-ycp/po/LINGUAS \
  hello-ycp/po/de.po \
  \
  hello-tcl/INSTALL \
  hello-tcl/autogen.sh \
  hello-tcl/autoclean.sh \
  hello-tcl/hello.tcl \
  hello-tcl/Makefile.am \
  hello-tcl/configure.ac \
  hello-tcl/m4/Makefile.am \
  hello-tcl/po/Makefile.am \
  hello-tcl/po/LINGUAS \
  hello-tcl/po/de.po \
  \
  hello-tcl-tk/INSTALL \
  hello-tcl-tk/autogen.sh \
  hello-tcl-tk/autoclean.sh \
  hello-tcl-tk/hello.tcl \
  hello-tcl-tk/Makefile.am \
  hello-tcl-tk/configure.ac \
  hello-tcl-tk/m4/Makefile.am \
  hello-tcl-tk/po/Makefile.am \
  hello-tcl-tk/po/LINGUAS \
  hello-tcl-tk/po/de.po \
  \
  hello-perl/INSTALL \
  hello-perl/autogen.sh \
  hello-perl/autoclean.sh \
  hello-perl/hello.pl \
  hello-perl/Makefile.am \
  hello-perl/configure.ac \
  hello-perl/m4/Makefile.am \
  hello-perl/po/Makefile.am \
  hello-perl/po/LINGUAS \
  hello-perl/po/de.po \
  \
  hello-php/INSTALL \
  hello-php/README \
  hello-php/autogen.sh \
  hello-php/autoclean.sh \
  hello-php/hello.php \
  hello-php/Makefile.am \
  hello-php/configure.ac \
  hello-php/m4/Makefile.am \
  hello-php/po/Makefile.am \
  hello-php/po/LINGUAS \
  hello-php/po/de.po \
  \
  README

install-data-local: $(EXAMPLESFILES)
	$(mkinstalldirs) $(DESTDIR)$(examplesdir)
	for f in $(EXAMPLESFILES); do \
	  d=`echo /$$f | sed -e 's,/[^/]*$$,,'`; \
	  test -z "$$d" || $(mkinstalldirs) $(DESTDIR)$(examplesdir)$$d || exit 1; \
	  b=`echo $$f | sed -e 's,^.*/,,'`; \
	  if test $$b = autogen.sh || test $$b = autoclean.sh; then \
	    echo "$(INSTALL_SCRIPT) $(srcdir)/$$f $(DESTDIR)$(examplesdir)/$$f"; \
	    $(INSTALL_SCRIPT) $(srcdir)/$$f $(DESTDIR)$(examplesdir)/$$f; \
	  else \
	    echo "$(INSTALL_DATA) $(srcdir)/$$f $(DESTDIR)$(examplesdir)/$$f"; \
	    $(INSTALL_DATA) $(srcdir)/$$f $(DESTDIR)$(examplesdir)/$$f; \
	  fi; \
	done

installdirs-local:
	$(mkinstalldirs) $(DESTDIR)$(examplesdir)
	for f in $(EXAMPLESFILES); do \
	  d=`echo /$$f | sed -e 's,/[^/]*$$,,'`; \
	  test -z "$$d" || $(mkinstalldirs) $(DESTDIR)$(examplesdir)$$d || exit 1; \
	done

uninstall-local:
	for f in $(EXAMPLESFILES); do \
	  rm -f $(DESTDIR)$(examplesdir)/$$f; \
	done

EXTRA_DIST += $(EXAMPLESFILES)

# Temporary directories created by po/Makefile.
CLEANFILES = tmp-hello-*


# VMS support.

EXTRA_DIST += Makefile.vms


# Woe32 support.

EXTRA_DIST += Makefile.msvc
