/* xgettext C/C++/ObjectiveC backend.
   Copyright (C) 2001 Free Software Foundation, Inc.
   Written by Bruno Haible <haible@clisp.cons.org>, 2001.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */


#define EXTENSIONS_C \
  { "c",      "C"     },						\
  { "h",      "C"     },						\
  { "C",      "C++"   },						\
  { "c++",    "C++"   },						\
  { "cc",     "C++"   },						\
  { "cxx",    "C++"   },						\
  { "cpp",    "C++"   },						\
  { "hh",     "C++"   },						\
  { "hxx",    "C++"   },						\
  { "hpp",    "C++"   },						\
  { "m",      "ObjectiveC" },						\

#define SCANNERS_C \
  { "C",          extract_c, &formatstring_c },				\
  { "C++",        extract_c, &formatstring_c },				\
  { "ObjectiveC", extract_c, &formatstring_c },				\

/* Scan a C/C++/ObjectiveC file and add its translatable strings to mdlp.  */
extern void extract_c PARAMS ((FILE *fp, const char *real_filename,
			       const char *logical_filename,
			       msgdomain_list_ty *mdlp));


/* Handling of options specific to this language.  */

extern void x_c_extract_all PARAMS ((void));

extern void x_c_keyword PARAMS ((const char *name));
extern bool x_c_any_keywords PARAMS ((void));

extern void x_c_trigraphs PARAMS ((void));
