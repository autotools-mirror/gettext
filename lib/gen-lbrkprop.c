/* Generate a Unicode conforming Line Break Properties tables from a
   UnicodeData file.
   Written by Bruno Haible <haible@clisp.cons.org>, 2000-2001.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

/* Usage example:
     $ gen-lbrkprop /usr/local/share/Unidata/UnicodeData.txt \
		    /usr/local/share/Unidata/PropList.txt \
		    /usr/local/share/Unidata/EastAsianWidth.txt \
		    3.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* This structure represents one line in the UnicodeData.txt file.  */
struct unicode_attribute
{
  const char *name;           /* Character name */
  const char *category;       /* General category */
  const char *combining;      /* Canonical combining classes */
  const char *bidi;           /* Bidirectional category */
  const char *decomposition;  /* Character decomposition mapping */
  const char *decdigit;       /* Decimal digit value */
  const char *digit;          /* Digit value */
  const char *numeric;        /* Numeric value */
  int mirrored;               /* mirrored */
  const char *oldname;        /* Old Unicode 1.0 name */
  const char *comment;        /* Comment */
  unsigned int upper;         /* Uppercase mapping */
  unsigned int lower;         /* Lowercase mapping */
  unsigned int title;         /* Titlecase mapping */
};

/* Missing fields are represented with "" for strings, and NONE for
   characters.  */
#define NONE (~(unsigned int)0)

/* The entire contents of the UnicodeData.txt file.  */
struct unicode_attribute unicode_attributes [0x10000];

/* Stores in unicode_attributes[i] the values from the given fields.  */
static void
fill_attribute (unsigned int i,
		const char *field1, const char *field2,
		const char *field3, const char *field4,
		const char *field5, const char *field6,
		const char *field7, const char *field8,
		const char *field9, const char *field10,
		const char *field11, const char *field12,
		const char *field13, const char *field14)
{
  struct unicode_attribute * uni;

  if (i >= 0x10000)
    {
      fprintf (stderr, "index too large\n");
      exit (1);
    }
  uni = &unicode_attributes[i];
  /* Copy the strings.  */
  uni->name          = strdup (field1);
  uni->category      = (field2[0] == '\0' ? "" : strdup (field2));
  uni->combining     = (field3[0] == '\0' ? "" : strdup (field3));
  uni->bidi          = (field4[0] == '\0' ? "" : strdup (field4));
  uni->decomposition = (field5[0] == '\0' ? "" : strdup (field5));
  uni->decdigit      = (field6[0] == '\0' ? "" : strdup (field6));
  uni->digit         = (field7[0] == '\0' ? "" : strdup (field7));
  uni->numeric       = (field8[0] == '\0' ? "" : strdup (field8));
  uni->mirrored      = (field9[0] == 'Y');
  uni->oldname       = (field10[0] == '\0' ? "" : strdup (field10));
  uni->comment       = (field11[0] == '\0' ? "" : strdup (field11));
  uni->upper = (field12[0] =='\0' ? NONE : strtoul (field12, NULL, 16));
  uni->lower = (field13[0] =='\0' ? NONE : strtoul (field13, NULL, 16));
  uni->title = (field14[0] =='\0' ? NONE : strtoul (field14, NULL, 16));
}

/* Maximum length of a field in the UnicodeData.txt file.  */
#define FIELDLEN 120

/* Reads the next field from STREAM.  The buffer BUFFER has size FIELDLEN.
   Reads up to (but excluding) DELIM.
   Returns 1 when a field was successfully read, otherwise 0.  */
static int
getfield (FILE *stream, char *buffer, int delim)
{
  int count = 0;
  int c;

  for (; (c = getc (stream)), (c != EOF && c != delim); )
    {
      /* The original unicode.org UnicodeData.txt file happens to have
	 CR/LF line terminators.  Silently convert to LF.  */
      if (c == '\r')
	continue;

      /* Put c into the buffer.  */
      if (++count >= FIELDLEN - 1)
	{
	  fprintf (stderr, "field too long\n");
	  exit (1);
	}
      *buffer++ = c;
    }

  if (c == EOF)
    return 0;

  *buffer = '\0';
  return 1;
}

/* Stores in unicode_attributes[] the entire contents of the UnicodeData.txt
   file.  */
static void
fill_attributes (const char *unicodedata_filename)
{
  unsigned int i, j;
  FILE *stream;
  char field0[FIELDLEN];
  char field1[FIELDLEN];
  char field2[FIELDLEN];
  char field3[FIELDLEN];
  char field4[FIELDLEN];
  char field5[FIELDLEN];
  char field6[FIELDLEN];
  char field7[FIELDLEN];
  char field8[FIELDLEN];
  char field9[FIELDLEN];
  char field10[FIELDLEN];
  char field11[FIELDLEN];
  char field12[FIELDLEN];
  char field13[FIELDLEN];
  char field14[FIELDLEN];
  int lineno = 0;

  for (i = 0; i < 0x10000; i++)
    unicode_attributes[i].name = NULL;

  stream = fopen (unicodedata_filename, "r");
  if (stream == NULL)
    {
      fprintf (stderr, "error during fopen of '%s'\n", unicodedata_filename);
      exit (1);
    }

  for (;;)
    {
      int n;

      lineno++;
      n = getfield (stream, field0, ';');
      n += getfield (stream, field1, ';');
      n += getfield (stream, field2, ';');
      n += getfield (stream, field3, ';');
      n += getfield (stream, field4, ';');
      n += getfield (stream, field5, ';');
      n += getfield (stream, field6, ';');
      n += getfield (stream, field7, ';');
      n += getfield (stream, field8, ';');
      n += getfield (stream, field9, ';');
      n += getfield (stream, field10, ';');
      n += getfield (stream, field11, ';');
      n += getfield (stream, field12, ';');
      n += getfield (stream, field13, ';');
      n += getfield (stream, field14, '\n');
      if (n == 0)
	break;
      if (n != 15)
	{
	  fprintf (stderr, "short line in'%s':%d\n",
		   unicodedata_filename, lineno);
	  exit (1);
	}
      i = strtoul (field0, NULL, 16);
      if (field1[0] == '<'
	  && strlen (field1) >= 9
	  && !strcmp (field1 + strlen(field1) - 8, ", First>"))
	{
	  /* Deal with a range. */
	  lineno++;
	  n = getfield (stream, field0, ';');
	  n += getfield (stream, field1, ';');
	  n += getfield (stream, field2, ';');
	  n += getfield (stream, field3, ';');
	  n += getfield (stream, field4, ';');
	  n += getfield (stream, field5, ';');
	  n += getfield (stream, field6, ';');
	  n += getfield (stream, field7, ';');
	  n += getfield (stream, field8, ';');
	  n += getfield (stream, field9, ';');
	  n += getfield (stream, field10, ';');
	  n += getfield (stream, field11, ';');
	  n += getfield (stream, field12, ';');
	  n += getfield (stream, field13, ';');
	  n += getfield (stream, field14, '\n');
	  if (n != 15)
	    {
	      fprintf (stderr, "missing end range in '%s':%d\n",
		       unicodedata_filename, lineno);
	      exit (1);
	    }
	  if (!(field1[0] == '<'
		&& strlen (field1) >= 8
		&& !strcmp (field1 + strlen (field1) - 7, ", Last>")))
	    {
	      fprintf (stderr, "missing end range in '%s':%d\n",
		       unicodedata_filename, lineno);
	      exit (1);
	    }
	  field1[strlen (field1) - 7] = '\0';
	  j = strtoul (field0, NULL, 16);
	  for (; i <= j; i++)
	    fill_attribute (i, field1+1, field2, field3, field4, field5,
			       field6, field7, field8, field9, field10,
			       field11, field12, field13, field14);
	}
      else
	{
	  /* Single character line */
	  fill_attribute (i, field1, field2, field3, field4, field5,
			     field6, field7, field8, field9, field10,
			     field11, field12, field13, field14);
	}
    }
  if (ferror (stream) || fclose (stream))
    {
      fprintf (stderr, "error reading from '%s'\n", unicodedata_filename);
      exit (1);
    }
}

/* The combining property from the PropList.txt file.  */
char unicode_combining[0x10000];

/* Stores in unicode_combining[] the Combining property from the
   PropList.txt file.  */
static void
fill_combining (const char *proplist_filename)
{
  unsigned int i;
  FILE *stream;
  char buf[100+1];

  for (i = 0; i < 0x10000; i++)
    unicode_combining[i] = 0;

  stream = fopen (proplist_filename, "r");
  if (stream == NULL)
    {
      fprintf (stderr, "error during fopen of '%s'\n", proplist_filename);
      exit (1);
    }

  /* Search for the "Property dump for: 0x20000004 (Combining)" line.  */
  do
    {
      if (fscanf (stream, "%100[^\n]\n", buf) < 1)
	{
	  fprintf (stderr, "no combining property found in '%s'\n",
		   proplist_filename);
	  exit (1);
	}
    }
  while (strstr (buf, "(Combining)") == NULL);

  for (;;)
    {
      unsigned int i1, i2;

      if (fscanf (stream, "%100[^\n]\n", buf) < 1)
	{
	  fprintf (stderr, "premature end of combining property in '%s'\n",
		   proplist_filename);
	  exit (1);
	}
      if (buf[0] == '*')
	break;
      if (strlen (buf) >= 10 && buf[4] == '.' && buf[5] == '.')
	{
	  if (sscanf (buf, "%4X..%4X", &i1, &i2) < 2)
	    {
	      fprintf (stderr, "parse error in combining property in '%s'\n",
		       proplist_filename);
	      exit (1);
	    }
	}
      else if (strlen (buf) >= 4)
	{
	  if (sscanf (buf, "%4X", &i1) < 1)
	    {
	      fprintf (stderr, "parse error in combining property in '%s'\n",
		       proplist_filename);
	      exit (1);
	    }
	  i2 = i1;
	}
      else
	{
	  fprintf (stderr, "parse error in combining property in '%s'\n",
		   proplist_filename);
	  exit (1);
	}
      for (i = i1; i <= i2; i++)
	unicode_combining[i] = 1;
    }
  if (ferror (stream) || fclose (stream))
    {
      fprintf (stderr, "error reading from '%s'\n", proplist_filename);
      exit (1);
    }
}

/* The width property from the EastAsianWidth.txt file.
   Each is NULL (unassigned) or "N", "A", "H", "W", "F", "Na".  */
const char * unicode_width[0x10000];

/* Stores in unicode_width[] the width property from the PropList.txt
   file.  */
static void
fill_width (const char *width_filename)
{
  unsigned int i, j;
  FILE *stream;
  char field0[FIELDLEN];
  char field1[FIELDLEN];
  char field2[FIELDLEN];
  int lineno = 0;

  for (i = 0; i < 0x10000; i++)
    unicode_width[i] = (unicode_attributes[i].name != NULL ? "N" : NULL);

  stream = fopen (width_filename, "r");
  if (stream == NULL)
    {
      fprintf (stderr, "error during fopen of '%s'\n", width_filename);
      exit (1);
    }

  for (;;)
    {
      int n;
      int c;

      lineno++;
      c = getc (stream);
      if (c == EOF)
	break;
      if (c == '#')
	{
	  do c = getc (stream); while (c != EOF && c != '\n');
	  continue;
	}
      ungetc (c, stream);
      n = getfield (stream, field0, ';');
      n += getfield (stream, field1, ';');
      n += getfield (stream, field2, '\n');
      if (n == 0)
	break;
      if (n != 3)
	{
	  fprintf (stderr, "short line in '%s':%d\n", width_filename, lineno);
	  exit (1);
	}
      i = strtoul (field0, NULL, 16);
      if (field2[0] == '<'
	  && strlen (field2) >= 9
	  && !strcmp (field2 + strlen(field2) - 8, ", First>"))
	{
	  /* Deal with a range. */
	  lineno++;
	  n = getfield (stream, field0, ';');
	  n += getfield (stream, field1, ';');
	  n += getfield (stream, field2, '\n');
	  if (n != 3)
	    {
	      fprintf (stderr, "missing end range in '%s':%d\n",
		       width_filename, lineno);
	      exit (1);
	    }
	  if (!(field2[0] == '<'
		&& strlen (field2) >= 8
		&& !strcmp (field2 + strlen (field2) - 7, ", Last>")))
	    {
	      fprintf (stderr, "missing end range in '%s':%d\n",
		       width_filename, lineno);
	      exit (1);
	    }
	  field2[strlen (field2) - 7] = '\0';
	  j = strtoul (field0, NULL, 16);
	  for (; i <= j; i++)
	    unicode_width[i] = strdup (field1);
	}
      else
	{
	  /* Single character line */
	  unicode_width[i] = strdup (field1);
	}
    }
  if (ferror (stream) || fclose (stream))
    {
      fprintf (stderr, "error reading from '%s'\n", width_filename);
      exit (1);
    }
}

/* Line breaking classification.  */

enum
{
  /* Values >= 20 are resolved at run time. */
  LBP_BK =  0, /* mandatory break */
/*LBP_CR,         carriage return - not used here because it's a DOSism */
/*LBP_LF,         line feed - not used here because it's a DOSism */
  LBP_CM = 20, /* attached characters and combining marks */
/*LBP_SG,         surrogates - not used here because they are not characters */
  LBP_ZW =  1, /* zero width space */
  LBP_IN =  2, /* inseparable */
  LBP_GL =  3, /* non-breaking (glue) */
  LBP_CB = 22, /* contingent break opportunity */
  LBP_SP = 21, /* space */
  LBP_BA =  4, /* break opportunity after */
  LBP_BB =  5, /* break opportunity before */
  LBP_B2 =  6, /* break opportunity before and after */
  LBP_HY =  7, /* hyphen */
  LBP_NS =  8, /* non starter */
  LBP_OP =  9, /* opening punctuation */
  LBP_CL = 10, /* closing punctuation */
  LBP_QU = 11, /* ambiguous quotation */
  LBP_EX = 12, /* exclamation/interrogation */
  LBP_ID = 13, /* ideographic */
  LBP_NU = 14, /* numeric */
  LBP_IS = 15, /* infix separator (numeric) */
  LBP_SY = 16, /* symbols allowing breaks */
  LBP_AL = 17, /* ordinary alphabetic and symbol characters */
  LBP_PR = 18, /* prefix (numeric) */
  LBP_PO = 19, /* postfix (numeric) */
  LBP_SA = 23, /* complex context (South East Asian) */
  LBP_AI = 24, /* ambiguous (alphabetic or ideograph) */
  LBP_XX = 25  /* unknown */
};

/* Returns the line breaking classification for ch, as a bit mask.  */
static int
get_lbp (unsigned int ch)
{
  int attr = 0;

  if (unicode_attributes[ch].name != NULL)
    {
      /* mandatory break */
      if (ch == 0x000A || ch == 0x000D || ch == 0x0085 /* newline */
	  || ch == 0x000C /* form feed */
	  || ch == 0x2028 /* LINE SEPARATOR */
	  || ch == 0x2029 /* PARAGRAPH SEPARATOR */)
	attr |= 1 << LBP_BK;

      /* zero width space */
      if (ch == 0x200B /* ZERO WIDTH SPACE */)
	attr |= 1 << LBP_ZW;

      /* inseparable */
      if (ch == 0x2024 /* ONE DOT LEADER */
	  || ch == 0x2025 /* TWO DOT LEADER */
	  || ch == 0x2026 /* HORIZONTAL ELLIPSIS */)
	attr |= 1 << LBP_IN;

      /* non-breaking (glue) */
      if (ch == 0xFEFF /* ZERO WIDTH NO-BREAK SPACE */
	  || ch == 0x00A0 /* NO-BREAK SPACE */
	  || ch == 0x202F /* NARROW NO-BREAK SPACE */
	  || ch == 0x2007 /* FIGURE SPACE */
	  || ch == 0x2011 /* NON-BREAKING HYPHEN */
	  || ch == 0x0F0C /* TIBETAN MARK DELIMITER TSHEG BSTAR */)
	attr |= 1 << LBP_GL;

      /* contingent break opportunity */
      if (ch == 0xFFFC /* OBJECT REPLACEMENT CHARACTER */)
	attr |= 1 << LBP_CB;

      /* space */
      if (ch == 0x0020 /* SPACE */)
	attr |= 1 << LBP_SP;

      /* break opportunity after */
      if (ch == 0x2000 /* EN QUAD */
	  || ch == 0x2001 /* EM QUAD */
	  || ch == 0x2002 /* EN SPACE */
	  || ch == 0x2003 /* EM SPACE */
	  || ch == 0x2004 /* THREE-PER-EM SPACE */
	  || ch == 0x2005 /* FOUR-PER-EM SPACE */
	  || ch == 0x2006 /* SIX-PER-EM SPACE */
	  || ch == 0x2008 /* PUNCTUATION SPACE */
	  || ch == 0x2009 /* THIN SPACE */
	  || ch == 0x200A /* HAIR SPACE */
	  || ch == 0x0009 /* tab */
	  || ch == 0x2010 /* HYPHEN */
	  || ch == 0x058A /* ARMENIAN HYPHEN */
	  || ch == 0x00AD /* SOFT HYPHEN */
	  || ch == 0x0F0B /* TIBETAN MARK INTERSYLLABIC TSHEG */
	  || ch == 0x1361 /* ETHIOPIC WORDSPACE */
	  || ch == 0x1680 /* OGHAM SPACE MARK */
	  || ch == 0x17D5 /* KHMER SIGN BARIYOOSAN */
	  || ch == 0x2027 /* HYPHENATION POINT */
	  || ch == 0x007C /* VERTICAL LINE */)
	attr |= 1 << LBP_BA;

      /* break opportunity before */
      if (ch == 0x00B4 /* ACUTE ACCENT */
	  || ch == 0x02C8 /* MODIFIER LETTER VERTICAL LINE */
	  || ch == 0x02CC /* MODIFIER LETTER LOW VERTICAL LINE */
	  || ch == 0x1806 /* MONGOLIAN TODO SOFT HYPHEN */)
	attr |= 1 << LBP_BB;

      /* break opportunity before and after */
      if (ch == 0x2014 /* EM DASH */)
	attr |= 1 << LBP_B2;

      /* hyphen */
      if (ch == 0x002D /* HYPHEN-MINUS */)
	attr |= 1 << LBP_HY;

      /* exclamation/interrogation */
      if (ch == 0x0021 /* EXCLAMATION MARK */
	  || ch == 0x003F /* QUESTION MARK */
	  || ch == 0xFE56 /* SMALL QUESTION MARK */
	  || ch == 0xFE57 /* SMALL EXCLAMATION MARK */
	  || ch == 0xFF01 /* FULLWIDTH EXCLAMATION MARK */
	  || ch == 0xFF1F /* FULLWIDTH QUESTION MARK */)
	attr |= 1 << LBP_EX;

      /* opening punctuation */
      if (unicode_attributes[ch].category[0] == 'P'
	  && unicode_attributes[ch].category[1] == 's')
	attr |= 1 << LBP_OP;

      /* closing punctuation */
      if (ch == 0x3001 /* IDEOGRAPHIC COMMA */
	  || ch == 0x3002 /* IDEOGRAPHIC FULL STOP */
	  || ch == 0xFF0C /* FULLWIDTH COMMA */
	  || ch == 0xFF0E /* FULLWIDTH FULL STOP */
	  || ch == 0xFE50 /* SMALL COMMA */
	  || ch == 0xFE52 /* SMALL FULL STOP */
	  || ch == 0xFF61 /* HALFWIDTH IDEOGRAPHIC FULL STOP */
	  || ch == 0xFF64 /* HALFWIDTH IDEOGRAPHIC COMMA */
	  || (unicode_attributes[ch].category[0] == 'P'
	      && unicode_attributes[ch].category[1] == 'e'))
	attr |= 1 << LBP_CL;

      /* ambiguous quotation */
      if (ch == 0x0022 /* QUOTATION MARK */
	  || ch == 0x0027 /* APOSTROPHE */
	  || (unicode_attributes[ch].category[0] == 'P'
	      && (unicode_attributes[ch].category[1] == 'f'
		  || unicode_attributes[ch].category[1] == 'i')))
	attr |= 1 << LBP_QU;

      /* attached characters and combining marks */
      if ((unicode_attributes[ch].category[0] == 'M'
	   && (unicode_attributes[ch].category[1] == 'n'
	       || unicode_attributes[ch].category[1] == 'c'
	       || unicode_attributes[ch].category[1] == 'e'))
	  || (ch >= 0x1160 && ch <= 0x11F9)
	  || (unicode_attributes[ch].category[0] == 'C'
	      && (unicode_attributes[ch].category[1] == 'c'
		  || unicode_attributes[ch].category[1] == 'f')))
	if (!(attr & ((1 << LBP_BK) | (1 << LBP_BA) | (1 << LBP_GL))))
	  attr |= 1 << LBP_CM;

      /* non starter */
      if (ch == 0x0E5A /* THAI CHARACTER ANGKHANKHU */
	  || ch == 0x0E5B /* THAI CHARACTER KHOMUT */
	  || ch == 0x17D4 /* KHMER SIGN KHAN */
	  || ch == 0x17D6 /* KHMER SIGN CAMNUC PII KUUH */
	  || ch == 0x17D7 /* KHMER SIGN LEK TOO */
	  || ch == 0x17D8 /* KHMER SIGN BEYYAL */
	  || ch == 0x17D9 /* KHMER SIGN PHNAEK MUAN */
	  || ch == 0x17DA /* KHMER SIGN KOOMUUT */
	  || ch == 0x203C /* DOUBLE EXCLAMATION MARK */
	  || ch == 0x2044 /* FRACTION SLASH */
	  || ch == 0x301C /* WAVE DASH */
	  || ch == 0x30FB /* KATAKANA MIDDLE DOT */
	  || ch == 0x3005 /* IDEOGRAPHIC ITERATION MARK */
	  || ch == 0x309B /* KATAKANA-HIRAGANA VOICED SOUND MARK */
	  || ch == 0x309C /* KATAKANA-HIRAGANA SEMI-VOICED SOUND MARK */
	  || ch == 0x309D /* HIRAGANA ITERATION MARK */
	  || ch == 0x309E /* HIRAGANA VOICED ITERATION MARK */
	  || ch == 0x30FD /* KATAKANA ITERATION MARK */
	  || ch == 0xFE54 /* SMALL SEMICOLON */
	  || ch == 0xFE55 /* SMALL COLON */
	  || ch == 0xFF1A /* FULLWIDTH COLON */
	  || ch == 0xFF1B /* FULLWIDTH SEMICOLON */
	  || ch == 0xFF65 /* HALFWIDTH KATAKANA MIDDLE DOT */
	  || ch == 0xFF70 /* HALFWIDTH KATAKANA-HIRAGANA PROLONGED SOUND MARK */
	  || (unicode_attributes[ch].category[0] == 'L'
	      && unicode_attributes[ch].category[1] == 'm'
	      && (unicode_width[ch][0] == 'W'
		  || unicode_width[ch][0] == 'H'))
	  || (unicode_attributes[ch].category[0] == 'S'
	      && unicode_attributes[ch].category[1] == 'k'
	      && unicode_width[ch][0] == 'W')
	  || strstr (unicode_attributes[ch].name, "HIRAGANA LETTER SMALL ") != NULL
	  || strstr (unicode_attributes[ch].name, "KATAKANA LETTER SMALL ") != NULL)
	attr |= 1 << LBP_NS;

      /* numeric */
      if (unicode_attributes[ch].category[0] == 'N'
	  && unicode_attributes[ch].category[1] == 'd'
	  && strstr (unicode_attributes[ch].name, "FULLWIDTH") == NULL)
	attr |= 1 << LBP_NU;

      /* infix separator (numeric) */
      if (ch == 0x002C /* COMMA */
	  || ch == 0x002E /* FULL STOP */
	  || ch == 0x003A /* COLON */
	  || ch == 0x003B /* SEMICOLON */
	  || ch == 0x0589 /* ARMENIAN FULL STOP */)
	attr |= 1 << LBP_IS;

      /* symbols allowing breaks */
      if (ch == 0x002F /* SOLIDUS */)
	attr |= 1 << LBP_SY;

      /* postfix (numeric) */
      if (ch == 0x0025 /* PERCENT SIGN */
	  || ch == 0x00A2 /* CENT SIGN */
	  || ch == 0x00B0 /* DEGREE SIGN */
	  || ch == 0x2030 /* PER MILLE SIGN */
	  || ch == 0x2031 /* PER TEN THOUSAND SIGN */
	  || ch == 0x2032 /* PRIME */
	  || ch == 0x2033 /* DOUBLE PRIME */
	  || ch == 0x2034 /* TRIPLE PRIME */
	  || ch == 0x2035 /* REVERSED PRIME */
	  || ch == 0x20A7 /* PESETA SIGN */
	  || ch == 0x2103 /* DEGREE CELSIUS */
	  || ch == 0x2109 /* DEGREE FAHRENHEIT */
	  || ch == 0x2126 /* OHM SIGN */
	  || ch == 0xFE6A /* SMALL PERCENT SIGN */
	  || ch == 0xFF05 /* FULLWIDTH PERCENT SIGN */
	  || ch == 0xFFE0 /* FULLWIDTH DIGIT ZERO */)
	attr |= 1 << LBP_PO;

      /* prefix (numeric) */
      if (ch == 0x002B /* PLUS SIGN */
	  || ch == 0x005C /* REVERSE SOLIDUS */
	  || ch == 0x00B1 /* PLUS-MINUS SIGN */
	  || ch == 0x2212 /* MINUS SIGN */
	  || ch == 0x2116 /* NUMERO SIGN */
	  || ch == 0x2213 /* MINUS-OR-PLUS SIGN */
	  || (unicode_attributes[ch].category[0] == 'S'
	      && unicode_attributes[ch].category[1] == 'c'))
	if (!(attr & (1 << LBP_PO)))
	  attr |= 1 << LBP_PR;

      /* complex context (South East Asian) */
      if ((ch >= 0x0E00 && ch <= 0x0EFF)
	  || (ch >= 0x1000 && ch <= 0x109F)
	  || (ch >= 0x1780 && ch <= 0x17FF))
	if (!(attr & ((1 << LBP_CM) | (1 << LBP_NS) | (1 << LBP_NU) | (1 << LBP_BA) | (1 << LBP_PR))))
	  attr |= 1 << LBP_SA;

      /* ideographic */
      if ((ch >= 0x4E00 && ch <= 0x9FAF) /* CJK Ideograph */
	  || (ch >= 0x3400 && ch <= 0x4DBF) /* CJK Ideograph Extension A */
	  || (ch >= 0xF900 && ch <= 0xFAFF) /* CJK COMPATIBILITY IDEOGRAPH */
	  || ch == 0x3000 /* IDEOGRAPHIC SPACE */
	  || (ch >= 0xAC00 && ch <= 0xD7AF) /* HANGUL SYLLABLE */
	  || (ch >= 0x3130 && ch <= 0x318F) /* HANGUL LETTER */
	  || (ch >= 0x1100 && ch <= 0x115F) /* HANGUL CHOSEONG */
	  || (ch >= 0xA000 && ch <= 0xA48C) /* YI SYLLABLE */
	  || (ch >= 0xA490 && ch <= 0xACFF) /* YI RADICAL */
	  || (ch >= 0x2E80 && ch <= 0x2FFF) /* CJK RADICAL, KANGXI RADICAL, IDEOGRAPHIC DESCRIPTION */
	  || ch == 0xFE62 /* SMALL PLUS SIGN */
	  || ch == 0xFE63 /* SMALL HYPHEN-MINUS */
	  || ch == 0xFE64 /* SMALL LESS-THAN SIGN */
	  || ch == 0xFE65 /* SMALL GREATER-THAN SIGN */
	  || ch == 0xFE66 /* SMALL EQUALS SIGN */
	  || (ch >= 0xFF10 && ch <= 0xFF19) /* FULLWIDTH DIGIT */
	  || strstr (unicode_attributes[ch].name, "FULLWIDTH LATIN ") != NULL
	  || (ch >= 0x3000 && ch <= 0x33FF
	      && !(attr & ((1 << LBP_CM) | (1 << LBP_NS) | (1 << LBP_OP) | (1 << LBP_CL)))))
	{
	  /* ambiguous (ideograph) ? */
	  if (unicode_width[ch] != NULL
	      && unicode_width[ch][0] == 'A')
	    attr |= 1 << LBP_AI;
	  else
	    attr |= 1 << LBP_ID;
	}

      /* ordinary alphabetic and symbol characters */
      if ((unicode_attributes[ch].category[0] == 'L'
	   && (unicode_attributes[ch].category[1] == 'u'
	       || unicode_attributes[ch].category[1] == 'l'
	       || unicode_attributes[ch].category[1] == 't'
	       || unicode_attributes[ch].category[1] == 'm'
	       || unicode_attributes[ch].category[1] == 'o'))
	  || (unicode_attributes[ch].category[0] == 'S'
	      && (unicode_attributes[ch].category[1] == 'm'
		  || unicode_attributes[ch].category[1] == 'c'
		  || unicode_attributes[ch].category[1] == 'k'
		  || unicode_attributes[ch].category[1] == 'o')))
	if (!(attr & ((1 << LBP_CM) | (1 << LBP_NS) | (1 << LBP_ID) | (1 << LBP_BA) | (1 << LBP_BB) | (1 << LBP_PO) | (1 << LBP_PR) | (1 << LBP_SA) | (1 << LBP_CB))))
	  {
	    /* ambiguous (alphabetic) ? */
	    if (unicode_width[ch] != NULL
		&& unicode_width[ch][0] == 'A')
	      attr |= 1 << LBP_AI;
	    else
	      attr |= 1 << LBP_AL;
	  }
    }

  if (attr == 0)
    /* unknown */
    attr |= 1 << LBP_XX;

  return attr;
}

/* Output the line breaking properties in a human readable format.  */
static void
debug_output_lbp (FILE *stream)
{
  unsigned int i;

  for (i = 0; i < 0x10000; i++)
    {
      int attr = get_lbp (i);
      if (attr != 1 << LBP_XX)
	{
	  fprintf (stream, "0x%04X", i);
#define PRINT_BIT(attr,bit) \
  if (attr & (1 << bit)) fprintf (stream, " " ## #bit);
	  PRINT_BIT(attr,LBP_BK);
	  PRINT_BIT(attr,LBP_CM);
	  PRINT_BIT(attr,LBP_ZW);
	  PRINT_BIT(attr,LBP_IN);
	  PRINT_BIT(attr,LBP_GL);
	  PRINT_BIT(attr,LBP_CB);
	  PRINT_BIT(attr,LBP_SP);
	  PRINT_BIT(attr,LBP_BA);
	  PRINT_BIT(attr,LBP_BB);
	  PRINT_BIT(attr,LBP_B2);
	  PRINT_BIT(attr,LBP_HY);
	  PRINT_BIT(attr,LBP_NS);
	  PRINT_BIT(attr,LBP_OP);
	  PRINT_BIT(attr,LBP_CL);
	  PRINT_BIT(attr,LBP_QU);
	  PRINT_BIT(attr,LBP_EX);
	  PRINT_BIT(attr,LBP_ID);
	  PRINT_BIT(attr,LBP_NU);
	  PRINT_BIT(attr,LBP_IS);
	  PRINT_BIT(attr,LBP_SY);
	  PRINT_BIT(attr,LBP_AL);
	  PRINT_BIT(attr,LBP_PR);
	  PRINT_BIT(attr,LBP_PO);
	  PRINT_BIT(attr,LBP_SA);
	  PRINT_BIT(attr,LBP_XX);
	  PRINT_BIT(attr,LBP_AI);
#undef PRINT_BIT
	  fprintf (stream, "\n");
	}
    }
}

/* Construction of sparse 3-level tables.  */
#define TABLE lbp_table
#define ELEMENT unsigned char
#define DEFAULT LBP_XX
#define xmalloc malloc
#define xrealloc realloc
#include "3level.h"

static void
output_lbp (FILE *stream)
{
  unsigned int i;
  struct lbp_table t;
  unsigned int level1_offset, level2_offset, level3_offset;

  t.p = 7;
  t.q = 9;
  lbp_table_init (&t);

  for (i = 0; i < 0x10000; i++)
    {
      int attr = get_lbp (i);

      /* Now attr should contain exactly one bit.  */
      if (attr == 0 || ((attr & (attr - 1)) != 0))
	abort ();

      if (attr != 1 << LBP_XX)
	{
	  unsigned int log2_attr;
	  for (log2_attr = 0; attr > 1; attr >>= 1, log2_attr++);

	  lbp_table_add (&t, i, log2_attr);
	}
    }

  lbp_table_finalize (&t);

  level1_offset =
    5 * sizeof (uint32_t);
  level2_offset =
    5 * sizeof (uint32_t)
    + t.level1_size * sizeof (uint32_t);
  level3_offset =
    5 * sizeof (uint32_t)
    + t.level1_size * sizeof (uint32_t)
    + (t.level2_size << t.q) * sizeof (uint32_t);

  for (i = 0; i < 5; i++)
    fprintf (stream, "#define lbrkprop_header_%d %d\n", i,
	     ((uint32_t *) t.result)[i]);
  fprintf (stream, "static const\n");
  fprintf (stream, "struct\n");
  fprintf (stream, "  {\n");
  fprintf (stream, "    int level1[%d];\n", t.level1_size);
  fprintf (stream, "    int level2[%d << %d];\n", t.level2_size, t.q);
  fprintf (stream, "    unsigned char level3[%d << %d];\n", t.level3_size, t.p);
  fprintf (stream, "  }\n");
  fprintf (stream, "lbrkprop =\n");
  fprintf (stream, "{\n");
  fprintf (stream, "  { ");
  for (i = 0; i < t.level1_size; i++)
    fprintf (stream, "%d%s ",
	     (((uint32_t *) (t.result + level1_offset))[i] - level2_offset) / sizeof (uint32_t),
	     (i+1 < t.level1_size ? "," : ""));
  fprintf (stream, "},\n");
  fprintf (stream, "  {");
  if (t.level2_size << t.q > 8)
    fprintf (stream, "\n   ");
  for (i = 0; i < t.level2_size << t.q; i++)
    {
      if (i > 0 && (i % 8) == 0)
	fprintf (stream, "\n   ");
      fprintf (stream, " %5d%s",
	       (((uint32_t *) (t.result + level2_offset))[i] - level3_offset) / sizeof (uint8_t),
	       (i+1 < t.level2_size << t.q ? "," : ""));
    }
  if (t.level2_size << t.q > 8)
    fprintf (stream, "\n ");
  fprintf (stream, " },\n");
  fprintf (stream, "  {");
  if (t.level3_size << t.p > 8)
    fprintf (stream, "\n   ");
  for (i = 0; i < t.level3_size << t.p; i++)
    {
      unsigned char value = ((unsigned char *) (t.result + level3_offset))[i];
      const char *value_string;
      switch (value)
	{
#define CASE(x) case x: value_string = #x; break;
	  CASE(LBP_BK);
	  CASE(LBP_CM);
	  CASE(LBP_ZW);
	  CASE(LBP_IN);
	  CASE(LBP_GL);
	  CASE(LBP_CB);
	  CASE(LBP_SP);
	  CASE(LBP_BA);
	  CASE(LBP_BB);
	  CASE(LBP_B2);
	  CASE(LBP_HY);
	  CASE(LBP_NS);
	  CASE(LBP_OP);
	  CASE(LBP_CL);
	  CASE(LBP_QU);
	  CASE(LBP_EX);
	  CASE(LBP_ID);
	  CASE(LBP_NU);
	  CASE(LBP_IS);
	  CASE(LBP_SY);
	  CASE(LBP_AL);
	  CASE(LBP_PR);
	  CASE(LBP_PO);
	  CASE(LBP_SA);
	  CASE(LBP_XX);
	  CASE(LBP_AI);
#undef CASE
	  default:
	    abort ();
	}
      if (i > 0 && (i % 8) == 0)
	fprintf (stream, "\n   ");
      fprintf (stream, " %s%s", value_string,
	       (i+1 < t.level3_size << t.p ? "," : ""));
    }
  if (t.level3_size << t.p > 8)
    fprintf (stream, "\n ");
  fprintf (stream, " }\n");
  fprintf (stream, "};\n");
}

static void
debug_output_tables (const char *filename)
{
  FILE *stream;

  stream = fopen (filename, "w");
  if (stream == NULL)
    {
      fprintf (stderr, "cannot open '%s' for writing\n", filename);
      exit (1);
    }

  debug_output_lbp (stream);

  if (ferror (stream) || fclose (stream))
    {
      fprintf (stderr, "error writing to '%s'\n", filename);
      exit (1);
    }
}

static void
output_tables (const char *filename, const char *version)
{
  FILE *stream;

  stream = fopen (filename, "w");
  if (stream == NULL)
    {
      fprintf (stderr, "cannot open '%s' for writing\n", filename);
      exit (1);
    }

  fprintf (stream, "/* Line breaking properties of Unicode characters.  */\n");
  fprintf (stream, "/* Generated automatically by gen-lbrkprop for Unicode %s.  */\n",
	   version);
  fprintf (stream, "\n");

  output_lbp (stream);

  if (ferror (stream) || fclose (stream))
    {
      fprintf (stderr, "error writing to '%s'\n", filename);
      exit (1);
    }
}

int
main (int argc, char * argv[])
{
  if (argc != 5)
    {
      fprintf (stderr, "Usage: %s UnicodeData.txt PropList.txt EastAsianWidth.txt version\n",
	       argv[0]);
      exit (1);
    }

  fill_attributes (argv[1]);
  fill_combining (argv[2]);
  fill_width (argv[3]);

  debug_output_tables ("lbrkprop.txt");

  output_tables ("lbrkprop.h", argv[4]);

  return 0;
}
