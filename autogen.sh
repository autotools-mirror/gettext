# This script requires autoconf-2.57 and automake-1.7.2 in the PATH.

aclocal
autoconf
automake

(cd autoconf-lib-link
 aclocal -I m4
 autoconf
 automake
)

(cd gettext-runtime
 aclocal -I m4 -I ../gettext-tools/m4 -I ../autoconf-lib-link/m4 -I ../config/m4
 autoconf
 autoheader
 automake
)

(cd gettext-tools
 aclocal -I m4 -I ../gettext-runtime/m4 -I ../autoconf-lib-link/m4 -I ../config/m4
 autoconf
 autoheader
 automake
)

cp -p autoconf-lib-link/config.rpath config/config.rpath
