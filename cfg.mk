SED = sed

# autoreconf is unsupported in this package! Use autogen.sh instead.
_autoreconf = ./autogen.sh

# Avoid line breaks in Copyright lines.
update-copyright-env = \
  UPDATE_COPYRIGHT_USE_INTERVALS=1 \
  UPDATE_COPYRIGHT_MAX_LINE_LENGTH=999

# Disable a nonsensical maint.mk rule.
# See <https://lists.gnu.org/archive/html/bug-gnulib/2022-09/msg00074.html>.
gl_public_submodule_commit =
