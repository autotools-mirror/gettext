AUTOMAKE_OPTIONS = 1.5 foreign no-dependencies
mkinstalldirs = $(SHELL) @global_top_srcdir@/mkinstalldirs
bin_PROGRAMS = usez
usez_SOURCES = usez.c
LDADD = @LIBRPATHZ@

check: all
	./usez
