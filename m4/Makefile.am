## Process this file with automake to produce Makefile.in -*-Makefile-*-

aclocaldir = @aclocaldir@
aclocal_DATA = codeset.m4 gettext.m4 glibc21.m4 iconv.m4 isc-posix.m4 lcmessage.m4 progtest.m4

# Generate this list with
# find . -type f -name '*.m4' -printf '%f\n'|sort |fmt |tr '\012' @ \
#   |sed 's/@$/%/;s/@/ \\@/g' |tr @% '\012\012'
EXTRA_DIST = README \
c-bs-a.m4 codeset.m4 getline.m4 gettext.m4 glibc21.m4 iconv.m4 \
inttypes_h.m4 isc-posix.m4 lcmessage.m4 libtool.m4 mbrtowc.m4 mbstate_t.m4 \
mbswidth.m4 progtest.m4 setlocale.m4 signed.m4 uintmax_t.m4 ulonglong.m4
