/* Special definitions, processed by autoheader.
   Copyright (C) 1995-1998, 2001 Free Software Foundation.
   Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1995.  */

/* Default value for alignment of strings in .mo file.  */
#define DEFAULT_OUTPUT_ALIGNMENT 1

#ifndef PARAMS
# if __STDC__
#  define PARAMS(args) args
# else
#  define PARAMS(args) ()
# endif
#endif

@TOP@

/* Define to the name of the distribution.  */
#undef PACKAGE

/* Define to the version of the distribution.  */
#undef VERSION

@BOTTOM@

/* We don't test for the basename function but still want to use the
   version in the libc when compiling for a system using glibc.  */
#ifdef __GNU_LIBRARY__
# define HAVE_BASENAME	1
#endif


/* A file name cannot consist of any character possible.  INVALID_PATH_CHAR
   contains the characters not allowed.  */
#ifndef MSDOS
# define	INVALID_PATH_CHAR "\1\2\3\4\5\6\7\10\11\12\13\14\15\16\17\20\21\22\23\24\25\26\27\30\31\32\33\34\35\36\37 \177/"
#else
/* Something like this for MSDOG.  */
# define	INVALID_PATH_CHAR "\1\2\3\4\5\6\7\10\11\12\13\14\15\16\17\20\21\22\23\24\25\26\27\30\31\32\33\34\35\36\37 \177\\:."
#endif

/* This is the page width for the message_print function.  It should
   not be set to more than 79 characters (Emacs users will appreciate
   it).  It is used to wrap the msgid and msgstr strings, and also to
   wrap the file position (#:) comments.  */
#define PAGE_WIDTH 79
