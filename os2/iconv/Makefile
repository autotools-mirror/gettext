#
# OS/2 GNU Makefile for building libiconv with GNU Make and GNU C compiler
#
# This makefile will build iconv.dll, iconv.a (the import library for ICONV.DLL)
# and iconv_s.a (static library).
#
# You will need the Unicode API add-on for EMX, which is included with latest
# distributions of gcc for OS/2 (gcc 3.0 and later).
#

# Use CMD.EXE as shell since its way faster
SHELL = $(COMSPEC)

# Pack the DLL and executables with lxlite
LXLITE = 1

# Tools
CC = gcc -c
CFLAGS = -s -O2 -Wall -Zmt $(INCLUDE) $(DEFS)
INCLUDE = -I.

LD = gcc
LDFLAGS.SHARED = -s -Zmt -Zcrtdll -Zdll
LIBS = -lgcc

AR = ar
ARFLAGS = crs

.SUFFIXES:
.SUFFIXES: .o .a .def .dll

.PHONY: all clean

ICONV.VERSION = 0.0.1
ICONV.OBJECTS = iconv.o

# How to compile a .c file
$(OUT)%.o: %.c
	$(CC) $(CFLAGS) -o $@ $<

# How to build an import library from a .DEF file
$(OUT)%.a: $(OUT)%.def
	emximp -o $@ $<

all: iconv.dll iconv.a iconv_s.a

clean:
	rm -rf *.o iconv.dll iconv*.a iconv.def

iconv_s.a: $(ICONV.OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

$(OUT)iconv.def: $(ICONV.OBJECTS)
	@echo LIBRARY ICONV INITINSTANCE TERMINSTANCE>$@
	@echo DESCRIPTION "iconv API library version $(ICONV.VERSION)">>$@
	@echo DATA MULTIPLE NONSHARED>>$@
	@echo EXPORTS>>$@
	emxexp $^ >>$@

$(OUT)iconv.dll: $(ICONV.OBJECTS) $(OUT)iconv.def
	$(LD) $(LDFLAGS.SHARED) -o $@ $^ $(LIBS)
ifeq ($(LXLITE),1)
	lxlite $@
endif
