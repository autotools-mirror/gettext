# OS/2 GNU Makefile for building gettext with GNU Make and GNU C compiler
#
# OS/2 still supports the regular configure/make building mechanism, but its
# way more clumsy, complicated and error prone. It is highly recommended to
# use this makefile instead, because :
# - this makefile builds an optimized static and dynamic version of the
#   library
# - it is able to build both optimized and debug versions of the library
#   without any reconfiguring
# - it is able to generate a complete OS/2 binary distribution (make distr)
# - besides its simply alot faster than configure generated makefiles
#
# The makefile is designed to be more or less gettext version independent,
# so it is likely to work with future versions of gettext as well. However,
# please inspect the pre-built config.h file (if you haven't built one with
# configure) so that VERSION macro contains the correct value.
#

# Use CMD.EXE as shell since its way faster
SHELL = $(COMSPEC)
# An Unix-like shell (needed for running config.charset)
UNIXSHELL = sh.exe

# Debug mode (1) or optimize mode (0)
DEBUG = 0

# The version of INTL.DLL (the name suffix)
INTLDLLVER =

# Pack the DLL and executables with lxlite
LXLITE = 1

# Output directory
OUT = out/$(OUT.SUFFIX)/
# Root package directory
ROOT = ../
# The base directory for distribution archive (emx/ or usr/)
INST = emx/
# A shortcut for emx/src/gettext-version/
INSTSRC = $(INST)src/gettext-$(VERSION)/

# Tools
CC = gcc -c
CFLAGS = -Wall -Zmt $(INCLUDE) $(DEFS)
INCLUDE = -I. -I$(ROOT) -I$(ROOT)intl -I$(ROOT)src -I$(ROOT)lib
DEFS = -DHAVE_CONFIG_H -DLIBDIR=\"/usr/lib\" \
  -DLOCALEDIR=\"/usr/share/locale\" -DLOCALE_ALIAS_PATH=\"/usr/share/locale\"

LD = gcc
LDFLAGS = -Zmt -Zcrtdll
LDFLAGS.SHARED = -Zmt -Zcrtdll -Zdll
LIBS = -liberty -lgcc

AR = ar
ARFLAGS = crs

MKDIR = mkdir.exe -p
COPY = cp -p

LINKINTL = $(OUT)intl.a

ifeq ($(DEBUG),0)
  CFLAGS += -s -O2
  LDFLAGS += -s -Zexe
  LDFLAGS.SHARED += -s
  OUT.SUFFIX = release
else
  CFLAGS += -g -Zomf
  LDFLAGS += -g -Zexe -Zomf
  LDFLAGS.SHARED += -g -Zomf
  OUT.SUFFIX = debug
  LXLITE := 0
  LINKINTL = $(OUT)intl_s.a
endif

# The list of languages to be included in binary distribution
LINGUAS = cs da de el es fr gl id it nl nn no pl pt pt_BR ru sl sv tr
# The following languages don't work with OS/2 iconv:
#en@boldquot en@quot et ja ko zh

VERSION = $(shell sed config.h -ne "/VERSION/{" -e "s/.*VERSION[ 	]*//" -e 's/"//g' -e "p" -e "}")

# Fetch the list of source files for libintl from intl/Makefile.in
INTL.SOURCES = $(addprefix $(ROOT)intl/,\
  $(subst $$lo,c,\
  $(subst @INTLOS2OBJS@,os2compat.c,\
  $(subst @INTLOBJS@,intl-compat.c,\
  $(subst OBJECTS = ,,\
  $(subst \,,\
  $(shell sed $(ROOT)intl/Makefile.in -ne "/^OBJECTS =/,/[^\]$$/p")))))))
INTL.OBJECTS = $(addprefix $(OUT),$(subst $(ROOT),,$(INTL.SOURCES:.c=.o)))

PROGRAMS = gettext ngettext msgcmp msgfmt msgmerge msgunfmt xgettext msgcomm
PROGRAMS.EXE = $(addprefix $(OUT),$(addsuffix .exe,$(PROGRAMS)))
SRC.SOURCES = $(filter-out $(addprefix $(ROOT)src/,$(addsuffix .c,$(PROGRAMS))),\
  $(wildcard $(ROOT)src/*.c))
SRC.OBJECTS = $(addprefix $(OUT),$(subst $(ROOT),,$(SRC.SOURCES:.c=.o)))
LIB.SOURCES = $(addprefix $(ROOT)lib/,basename.c error.c stpcpy.c stpncpy.c \
  concatpath.c mbswidth.c fstrcmp.c hash.c printf-prs.c c-ctype.c linebreak.c \
  getline.c)
LIB.OBJECTS = $(addprefix $(OUT),$(subst $(ROOT),,$(LIB.SOURCES:.c=.o)))

OUTDIRS = $(OUT) $(sort $(dir $(INTL.OBJECTS) $(SRC.OBJECTS) $(LIB.OBJECTS) $(INSTALL.FILES)))

INSTALL.FILES = $(addprefix $(INST)bin/,$(addsuffix .exe,$(PROGRAMS))) \
  $(INST)lib/intl.a $(INST)lib/intl_s.a $(INST)dll/intl.dll $(INST)include/libintl.h \
  $(INST)share/locale/charset.alias $(INST)share/locale/locale.alias \
  $(INST)doc/gettext-$(VERSION)/README.OS2 \
  $(INST)doc/gettext-$(VERSION)/COPYING $(INST)doc/gettext-$(VERSION)/README \
  $(INST)include/iconv.h $(INST)include/langinfo.h \
  $(addsuffix /LC_MESSAGES/gettext.mo,$(addprefix $(INST)share/locale/,$(LINGUAS))) \
  $(INSTALL.DIFF)

# The diff and source files which we want in binary distribution (do we?)
INSTALL.DIFF = $(INSTSRC)gettext-$(VERSION).diff \
  $(addprefix $(INSTSRC)os2/,$(wildcard *.c *.h *.def) Makefile README.OS2) \
  $(INSTSRC)intl/os2compat.c

.SUFFIXES:
.SUFFIXES: .o .a .def .exe .dll .po .mo
.PRECIOUS: $(OUT)%.o

.PHONY: all depend clean distr rmzip

$(OUT)%.o: $(ROOT)%.c
	$(CC) $(CFLAGS) -o $@ $<

# To avoid playing with object file lists for every program we will build
# instead a library containing all the object files from src directory, and
# then link the library against the main program module, so that linker can
# pull all the required functions from there
$(OUT)%.exe: $(OUT)src/%.o $(OUT)util.a $(LINKINTL)
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)
ifeq ($(LXLITE),1)
	lxlite $@
endif

all: $(OUTDIRS) $(ROOT)src/po-gram-gen2.h $(ROOT)intl/libintl.h \
  $(OUT)intl.a $(OUT)intl_s.a $(OUT)intl$(INTLDLLVER).dll $(PROGRAMS.EXE)

depend: $(INTL.SOURCES) $(SRC.SOURCES)
	makedep $(INCLUDE) $(DEFS) -p $$(OUT) -r $^

clean:
	rm -rf out emx

distr: all rmzip gettext-os2-$(VERSION)-bin.zip

rmzip:
	rm -f gettext-os2-$(VERSION)-bin.zip

$(OUTDIRS):
	$(MKDIR) $(@:/=)

$(OUT)intl_s.a: $(INTL.OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

$(OUT)intl.def: $(INTL.OBJECTS)
	@echo LIBRARY INTL$(INTLDLLVER) INITINSTANCE TERMINSTANCE>$@
	@echo DESCRIPTION "GNU gettext internationalization library version $(VERSION)">>$@
	@echo DATA MULTIPLE NONSHARED>>$@
	@echo EXPORTS>>$@
	type backward.def>>$@
	emxexp $^ >>$@

$(OUT)intl$(INTLDLLVER).dll: $(INTL.OBJECTS) $(OUT)intl.def
	@echo *********************************************************
	@echo *** YOU CAN SAFELY IGNORE WARNINGS FROM EMXBIND BELOW ***
	@echo *********************************************************
	$(LD) $(LDFLAGS.SHARED) -o $@ $^ $(LIBS)
ifeq ($(LXLITE),1)
	lxlite $@
endif

# Remove the ordinals from exports so that newer programs import by name
$(OUT)intl.a: $(OUT)intl.def
	emximp -o $@ $<

$(OUT)util.a: $(SRC.OBJECTS) $(LIB.OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

$(ROOT)src/po-gram-gen2.h: $(ROOT)src/po-gram-gen.h
	sed -e "s/[yY][yY]/po_gram_/g" $< > $@

$(ROOT)intl/libintl.h: $(ROOT)intl/libgnuintl.h 
	$(COPY) $< $@

gettext-os2-$(VERSION)-bin.zip: $(INSTALL.FILES)
	@rm -f $@
	zip -9XD $@ $^

# The following rules are for `make distr' target only

$(INST)share/locale/charset.alias: $(ROOT)lib/config.charset
	$(UNIXSHELL) $< i386-pc-os2-emx >$@
$(INST)share/locale/locale.alias: $(ROOT)intl/locale.alias
	$(COPY) $< $@
$(INST)bin/% $(INST)lib/% $(INST)dll/%: $(OUT)%
	$(COPY) $< $@
$(INST)include/%: $(ROOT)intl/%
	$(COPY) $< $@
$(INST)doc/gettext-$(VERSION)/%: $(ROOT)%
	$(COPY) $< $@
$(INST)doc/gettext-$(VERSION)/% $(INSTSRC)% $(INSTSRC)os2/% $(INST)include/%: %
	$(COPY) $< $@
$(INSTSRC)%: $(ROOT)%
	$(COPY) $< $@
$(INST)share/locale/%/LC_MESSAGES/gettext.mo: $(ROOT)po/%.po
	$(MKDIR) $(dir $@)
	$(COMSPEC) /c "$(subst /,\\,set BEGINLIBPATH=$(OUT:/=) && \
	  $(OUT)msgfmt.exe) --statistics --verbose -o $@ $<"

# DO NOT DELETE this line -- makedep depends on it

$(OUT)intl-compat.o: ../intl/libgnuintl.h ../intl/gettextP.h \
  ../intl/loadinfo.h ../intl/gettext.h
$(OUT)bindtextdom.o: ../intl/libgnuintl.h ../intl/gettextP.h \
  ../intl/loadinfo.h ../intl/gettext.h
$(OUT)dcgettext.o: ../intl/gettextP.h ../intl/loadinfo.h \
  ../intl/gettext.h ../intl/libgnuintl.h
$(OUT)dgettext.o: ../intl/gettextP.h ../intl/loadinfo.h \
  ../intl/gettext.h ../intl/libgnuintl.h
$(OUT)gettext.o: ../intl/gettextP.h ../intl/loadinfo.h \
  ../intl/gettext.h ../intl/libgnuintl.h
$(OUT)finddomain.o: ../intl/gettextP.h ../intl/loadinfo.h \
  ../intl/gettext.h ../intl/libgnuintl.h
$(OUT)loadmsgcat.o: ../intl/gettext.h ../intl/gettextP.h \
  ../intl/loadinfo.h
$(OUT)localealias.o: ../intl/gettextP.h ../intl/loadinfo.h \
  ../intl/gettext.h
$(OUT)textdomain.o: ../intl/libgnuintl.h ../intl/gettextP.h \
  ../intl/loadinfo.h ../intl/gettext.h
$(OUT)l10nflist.o: ../intl/loadinfo.h
$(OUT)explodename.o: ../intl/loadinfo.h
$(OUT)dcigettext.o: ../intl/gettextP.h ../intl/loadinfo.h \
  ../intl/gettext.h ../intl/libgnuintl.h ../intl/hash-string.h
$(OUT)dcngettext.o: ../intl/gettextP.h ../intl/loadinfo.h \
  ../intl/gettext.h ../intl/libgnuintl.h
$(OUT)dngettext.o: ../intl/gettextP.h ../intl/loadinfo.h \
  ../intl/gettext.h ../intl/libgnuintl.h
$(OUT)ngettext.o: ../intl/gettextP.h ../intl/loadinfo.h \
  ../intl/gettext.h ../intl/libgnuintl.h
$(OUT)plural.o: ../intl/gettextP.h ../intl/loadinfo.h \
  ../intl/gettext.h
$(OUT)os2compat.o: ../os2/iconv.c
$(OUT)dir-list.o: ../lib/system.h ../src/dir-list.h \
  ../src/str-list.h
$(OUT)message.o: ../lib/fstrcmp.h ../src/message.h ../src/str-list.h \
  ../src/pos.h ../lib/system.h
$(OUT)open-po.o: ../src/open-po.h ../src/dir-list.h ../lib/error.h \
  ../lib/system.h ../intl/libgettext.h
$(OUT)po-gram-gen.o: ../src/po-lex.h ../lib/error.h ../src/pos.h \
  ../src/po-gram.h ../lib/system.h ../intl/libgettext.h \
  ../src/po.h
$(OUT)po-hash-gen.o: ../src/po-hash.h ../src/po.h ../src/po-lex.h \
  ../lib/error.h ../src/pos.h
$(OUT)po-lex.o: ../intl/libgettext.h ../src/po-lex.h ../lib/error.h \
  ../src/pos.h ../src/open-po.h ../src/po-gram-gen2.h
$(OUT)po.o: ../src/po.h ../src/po-lex.h ../lib/error.h \
  ../src/pos.h ../src/po-hash.h ../lib/mbswidth.h \
  ../intl/libgettext.h
$(OUT)str-list.o: ../src/str-list.h
$(OUT)write-po.o: ../src/write-po.h ../src/message.h \
  ../src/str-list.h ../src/pos.h ../lib/c-ctype.h \
  ../lib/linebreak.h ../lib/error.h ../intl/libgettext.h
$(OUT)xget-lex.o: ../src/dir-list.h ../lib/error.h \
  ../intl/libgettext.h ../lib/hash.h ../src/str-list.h \
  ../src/xget-lex.h
