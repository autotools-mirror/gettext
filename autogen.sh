# This script requires autoconf-2.58..2.59 and automake-1.8.2 in the PATH.

aclocal
autoconf
automake

(cd autoconf-lib-link
 aclocal -I m4 -I ../config/m4
 autoconf
 automake
)

(cd gettext-runtime
 aclocal -I m4 -I ../gettext-tools/m4 -I ../autoconf-lib-link/m4 -I ../config/m4
 autoconf
 autoheader && touch config.h.in
 automake
)

(cd gettext-runtime/libasprintf
 aclocal -I ../../config/m4 -I ../m4
 autoconf
 autoheader && touch config.h.in
 automake
)

cp -p gettext-runtime/ABOUT-NLS gettext-tools/ABOUT-NLS

(cd gettext-tools
 aclocal -I m4 -I ../gettext-runtime/m4 -I ../autoconf-lib-link/m4 -I ../config/m4
 autoconf
 autoheader && touch config.h.in
 automake
)

cp -p autoconf-lib-link/config.rpath config/config.rpath
