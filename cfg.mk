SED ?= sed

update-copyright-env = \
  UPDATE_COPYRIGHT_USE_INTERVALS=1 \
  UPDATE_COPYRIGHT_MAX_LINE_LENGTH=79

# Disable a nonsensical maint.mk rule.
# See <https://lists.gnu.org/archive/html/bug-gnulib/2022-09/msg00074.html>.
gl_public_submodule_commit =
