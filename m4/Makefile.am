## Process this file with automake to produce Makefile.in -*-Makefile-*-

aclocaldir = @aclocaldir@
aclocal_DATA = gettext.m4 isc-posix.m4 lcmessage.m4 progtest.m4

# Generate this list with
# find . -type f -name '*.m4' -printf '%f\n'|sort |fmt |tr '\012' @ \
#   |sed 's/@$/%/;s/@/ \\@/g' |tr @% '\012\012'
EXTRA_DIST = README \
c-bs-a.m4 codeset.m4 gettext.m4 iconv.m4 inttypes_h.m4 isc-posix.m4 \
lcmessage.m4 progtest.m4 setlocale.m4 uintmax_t.m4 ulonglong.m4
