## Process this file with automake to produce Makefile.in -*-Makefile-*-

# Files installed for use by gettextize.

aclocaldir = @aclocaldir@
aclocal_DATA = \
  ../../gettext-runtime/gnulib-m4/host-cpu-c-abi.m4 \
  ../../gettext-runtime/gnulib-m4/lib-ld.m4 \
  ../../gettext-runtime/gnulib-m4/lib-link.m4 \
  ../../gettext-runtime/gnulib-m4/lib-prefix.m4 \
  ../../gettext-runtime/m4/gettext.m4 \
  ../../gettext-runtime/m4/iconv.m4 \
  ../../gettext-runtime/m4/intlmacosx.m4 \
  ../../gettext-runtime/m4/nls.m4 \
  ../../gettext-runtime/m4/po.m4 \
  ../../gettext-runtime/m4/progtest.m4

# Files installed for the examples.

examplesbuildauxdir = $(docdir)/examples/build-aux
examplesbuildaux_DATA = \
  ../gnulib-m4/javacomp.m4 ../gnulib-m4/javaexec.m4 \
  ../gnulib-m4/csharpcomp.m4 ../gnulib-m4/csharpexec.m4 csharpexec-test.exe

# Generate this list with
# find . -type f -name '*.m4' -printf '%f\n' | sort | tr '\012' @ | sed 's/@$/%/;s/@/ \\@/g' | tr @% '\012\012'
EXTRA_DIST = README csharpexec-test.exe \
exported.m4 \
hostname.m4 \
locale-de.m4 \
setlocale.m4 \
siginfo.m4
