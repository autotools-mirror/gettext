## Makefile for intl-java directory in GNU NLS utilities package.
## Copyright (C) 2001 Free Software Foundation, Inc.
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

jardir = $(datadir)/gettext

JAR = @JAR@
JAVACOMP = $(SHELL) ../lib/javacomp.sh


# Special rules for Java compilation.

all-local: all-java-@BUILDJAVA@
all-java-no:
all-java-yes: libintl.jar

gnu/gettext/GettextResource.class: $(srcdir)/gnu/gettext/GettextResource.java
	$(JAVACOMP) -d . $(srcdir)/gnu/gettext/GettextResource.java

libintl.jar: gnu/gettext/GettextResource.class
	$(JAR) cf $@ gnu/gettext/GettextResource*.class

CLEANFILES = libintl.jar gnu/gettext/*.class

install-data-local: install-java-@BUILDJAVA@
install-java-no:
install-java-yes: all-java-yes
	$(INSTALL_DATA) libintl.jar $(DESTDIR)$(jardir)/libintl.jar

installdirs-local:
	$(mkinstalldirs) $(DESTDIR)$(jardir)

uninstall-local:
	$(RM) $(DESTDIR)$(jardir)/libintl.jar


# One more automake bug.
installdirs: installdirs-local
