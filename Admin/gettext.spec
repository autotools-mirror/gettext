Description: Library and tools for message translation.
Name: gettext
Version: 0.10.23
Release: 1
Copyright: GPL
Group: Development/Libraries
Source: ftp://alpha.gnu.org/gnu/gettext-0.10.23.tar.gz
Packager: Ulrich Drepper <drepper@cygnus.com>
Buildprefix: /tmp

%prep
%setup

%build
CFLAGS="$RPM_OPT_FLAGS" LDFLAGS=-s configure --with-included-gettext --prefix=/usr/local
make

%install
make install prefix=$RPM_BUILDPREFIX

%files
%doc ABOUT-NLS
%doc NEWS

/usr/local/bin/gettext
/usr/local/bin/msgcmp
/usr/local/bin/msgfmt
/usr/local/bin/msgmerge
/usr/local/bin/msgunfmt
/usr/local/bin/xgettext
/usr/local/include/libintl.h
/usr/local/lib/libintl.a
/usr/local/info/gettext*
/usr/local/share/locale/*/LC_MESSAGES/gettext.mo
/usr/local/share/gettext
/usr/local/share/emacs/site-lisp/po-mode.elc
