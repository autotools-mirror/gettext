AUTOMAKE_OPTIONS = 1.5 foreign no-dependencies
mkinstalldirs = $(SHELL) @global_top_srcdir@/mkinstalldirs
bin_PROGRAMS = usex
usex_SOURCES = usex.c
LDADD = @LIBRPATHX@

check: all
	./usex
