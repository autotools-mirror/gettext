/* Output stream for attributed text, producing ANSI escape sequences.
   Copyright (C) 2006-2008, 2017, 2019 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2006.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

/* Specification.  */
#include "term-ostream.h"

/* Set to 1 to get debugging output regarding signals.  */
#define DEBUG_SIGNALS 0

#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#if DEBUG_SIGNALS
# include <stdio.h>
#endif
#if HAVE_TCGETATTR || HAVE_TCDRAIN
# include <termios.h>
# if !defined NOFLSH            /* QNX */
#  define NOFLSH 0
# endif
#endif
#if HAVE_TCGETATTR
# include <sys/stat.h>
#endif

#include "error.h"
#include "fatal-signal.h"
#include "sig-handler.h"
#include "full-write.h"
#include "terminfo.h"
#include "same-inode.h"
#include "xalloc.h"
#include "xsize.h"
#include "gettext.h"

#define _(str) gettext (str)

#if HAVE_TPARAM
/* GNU termcap's tparam() function requires a buffer argument.  Make it so
   large that there is no risk that tparam() needs to call malloc().  */
static char tparambuf[100];
/* Define tparm in terms of tparam.  In the scope of this file, it is called
   with at most one argument after the string.  */
# define tparm(str, arg1) \
  tparam (str, tparambuf, sizeof (tparambuf), arg1)
#endif

#define SIZEOF(a) (sizeof(a) / sizeof(a[0]))


/* =========================== Color primitives =========================== */

/* A color in RGB format.  */
typedef struct
{
  unsigned int red   : 8; /* range 0..255 */
  unsigned int green : 8; /* range 0..255 */
  unsigned int blue  : 8; /* range 0..255 */
} rgb_t;

/* A color in HSV (a.k.a. HSB) format.  */
typedef struct
{
  float hue;        /* normalized to interval [0,6) */
  float saturation; /* normalized to interval [0,1] */
  float brightness; /* a.k.a. value, normalized to interval [0,1] */
} hsv_t;

/* Conversion of a color in RGB to HSV format.  */
static void
rgb_to_hsv (rgb_t c, hsv_t *result)
{
  unsigned int r = c.red;
  unsigned int g = c.green;
  unsigned int b = c.blue;

  if (r > g)
    {
      if (b > r)
        {
          /* b > r > g, so max = b, min = g */
          result->hue = 4.0f + (float) (r - g) / (float) (b - g);
          result->saturation = 1.0f - (float) g / (float) b;
          result->brightness = (float) b / 255.0f;
        }
      else if (b <= g)
        {
          /* r > g >= b, so max = r, min = b */
          result->hue = 0.0f + (float) (g - b) / (float) (r - b);
          result->saturation = 1.0f - (float) b / (float) r;
          result->brightness = (float) r / 255.0f;
        }
      else
        {
          /* r >= b > g, so max = r, min = g */
          result->hue = 6.0f - (float) (b - g) / (float) (r - g);
          result->saturation = 1.0f - (float) g / (float) r;
          result->brightness = (float) r / 255.0f;
        }
    }
  else
    {
      if (b > g)
        {
          /* b > g >= r, so max = b, min = r */
          result->hue = 4.0f - (float) (g - r) / (float) (b - r);
          result->saturation = 1.0f - (float) r / (float) b;
          result->brightness = (float) b / 255.0f;
        }
      else if (b < r)
        {
          /* g >= r > b, so max = g, min = b */
          result->hue = 2.0f - (float) (r - b) / (float) (g - b);
          result->saturation = 1.0f - (float) b / (float) g;
          result->brightness = (float) g / 255.0f;
        }
      else if (g > r)
        {
          /* g >= b >= r, g > r, so max = g, min = r */
          result->hue = 2.0f + (float) (b - r) / (float) (g - r);
          result->saturation = 1.0f - (float) r / (float) g;
          result->brightness = (float) g / 255.0f;
        }
      else
        {
          /* r = g = b.  A grey color.  */
          result->hue = 0; /* arbitrary */
          result->saturation = 0;
          result->brightness = (float) r / 255.0f;
        }
    }
}

/* Square of distance of two colors.  */
static float
color_distance (const hsv_t *color1, const hsv_t *color2)
{
#if 0
  /* Formula taken from "John Smith: Color Similarity",
       http://www.ctr.columbia.edu/~jrsmith/html/pubs/acmmm96/node8.html.  */
  float angle1 = color1->hue * 1.04719755f; /* normalize to [0,2π] */
  float angle2 = color2->hue * 1.04719755f; /* normalize to [0,2π] */
  float delta_x = color1->saturation * cosf (angle1)
                  - color2->saturation * cosf (angle2);
  float delta_y = color1->saturation * sinf (angle1)
                  - color2->saturation * sinf (angle2);
  float delta_v = color1->brightness
                  - color2->brightness;

  return delta_x * delta_x + delta_y * delta_y + delta_v * delta_v;
#else
  /* Formula that considers hue differences with more weight than saturation
     or brightness differences, like the human eye does.  */
  float delta_hue =
    (color1->hue >= color2->hue
     ? (color1->hue - color2->hue >= 3.0f
        ? 6.0f + color2->hue - color1->hue
        : color1->hue - color2->hue)
     : (color2->hue - color1->hue >= 3.0f
        ? 6.0f + color1->hue - color2->hue
        : color2->hue - color1->hue));
  float min_saturation =
    (color1->saturation < color2->saturation
     ? color1->saturation
     : color2->saturation);
  float delta_saturation = color1->saturation - color2->saturation;
  float delta_brightness = color1->brightness - color2->brightness;

  return delta_hue * delta_hue * min_saturation
         + delta_saturation * delta_saturation * 0.2f
         + delta_brightness * delta_brightness * 0.8f;
#endif
}

/* Return the index of the color in a color table that is nearest to a given
   color.  */
static unsigned int
nearest_color (rgb_t given, const rgb_t *table, unsigned int table_size)
{
  hsv_t given_hsv;
  unsigned int best_index;
  float best_distance;
  unsigned int i;

  assert (table_size > 0);

  rgb_to_hsv (given, &given_hsv);

  best_index = 0;
  best_distance = 1000000.0f;
  for (i = 0; i < table_size; i++)
    {
      hsv_t i_hsv;

      rgb_to_hsv (table[i], &i_hsv);

      /* Avoid converting a color to grey, or fading out a color too much.  */
      if (i_hsv.saturation > given_hsv.saturation * 0.5f)
        {
          float distance = color_distance (&given_hsv, &i_hsv);
          if (distance < best_distance)
            {
              best_index = i;
              best_distance = distance;
            }
        }
    }

#if 0 /* Debugging code */
  hsv_t best_hsv;
  rgb_to_hsv (table[best_index], &best_hsv);
  fprintf (stderr, "nearest: (%d,%d,%d) = (%f,%f,%f)\n    -> (%f,%f,%f) = (%d,%d,%d)\n",
                   given.red, given.green, given.blue,
                   (double)given_hsv.hue, (double)given_hsv.saturation, (double)given_hsv.brightness,
                   (double)best_hsv.hue, (double)best_hsv.saturation, (double)best_hsv.brightness,
                   table[best_index].red, table[best_index].green, table[best_index].blue);
#endif

  return best_index;
}

/* The luminance of a color.  This is the brightness of the color, as it
   appears to the human eye.  This must be used in color to grey conversion.  */
static float
color_luminance (int r, int g, int b)
{
  /* Use the luminance model used by NTSC and JPEG.
     Taken from http://www.fho-emden.de/~hoffmann/gray10012001.pdf .
     No need to care about rounding errors leading to luminance > 1;
     this cannot happen.  */
  return (0.299f * r + 0.587f * g + 0.114f * b) / 255.0f;
}


/* ============================= Color models ============================= */

/* The color model used by the terminal.  */
typedef enum
{
  cm_monochrome,        /* No colors.  */
  cm_common8,           /* Usual terminal with at least 8 colors.  */
  cm_xterm8,            /* TERM=xterm, with 8 colors.  */
  cm_xterm16,           /* TERM=xterm-16color, with 16 colors.  */
  cm_xterm88,           /* TERM=xterm-88color, with 88 colors.  */
  cm_xterm256           /* TERM=xterm-256color, with 256 colors.  */
} colormodel_t;

/* ----------------------- cm_monochrome color model ----------------------- */

/* A non-default color index doesn't exist in this color model.  */
static inline term_color_t
rgb_to_color_monochrome ()
{
  return COLOR_DEFAULT;
}

/* ------------------------ cm_common8 color model ------------------------ */

/* A non-default color index is in the range 0..7.
                       RGB components
   COLOR_BLACK         000
   COLOR_BLUE          001
   COLOR_GREEN         010
   COLOR_CYAN          011
   COLOR_RED           100
   COLOR_MAGENTA       101
   COLOR_YELLOW        110
   COLOR_WHITE         111 */
static const rgb_t colors_of_common8[8] =
{
  /* R    G    B        grey  index */
  {   0,   0,   0 }, /* 0.000   0 */
  {   0,   0, 255 },
  {   0, 255,   0 },
  {   0, 255, 255 },
  { 255,   0,   0 },
  { 255,   0, 255 },
  { 255, 255,   0 },
  { 255, 255, 255 }  /* 1.000   7 */
};

static inline term_color_t
rgb_to_color_common8 (int r, int g, int b)
{
  rgb_t color;
  hsv_t hsv;

  color.red = r; color.green = g; color.blue = b;
  rgb_to_hsv (color, &hsv);

  if (hsv.saturation < 0.065f)
    {
      /* Greyscale approximation.  */
      float luminance = color_luminance (r, g, b);
      if (luminance < 0.500f)
        return 0;
      else
        return 7;
    }
  else
    /* Color approximation.  */
    return nearest_color (color, colors_of_common8, 8);
}

/* Convert a cm_common8 color in RGB encoding to BGR encoding.
   See the ncurses terminfo(5) manual page, section "Color Handling", for an
   explanation why this is needed.  */
static _GL_ASYNC_SAFE inline int
color_bgr (term_color_t color)
{
  return ((color & 4) >> 2) | (color & 2) | ((color & 1) << 2);
}

/* ------------------------- cm_xterm8 color model ------------------------- */

/* A non-default color index is in the range 0..7.
                       BGR components
   COLOR_BLACK         000
   COLOR_RED           001
   COLOR_GREEN         010
   COLOR_YELLOW        011
   COLOR_BLUE          100
   COLOR_MAGENTA       101
   COLOR_CYAN          110
   COLOR_WHITE         111 */
static const rgb_t colors_of_xterm8[8] =
{
  /* The real xterm's colors are dimmed; assume full-brightness instead.  */
  /* R    G    B        grey  index */
  {   0,   0,   0 }, /* 0.000   0 */
  { 255,   0,   0 },
  {   0, 255,   0 },
  { 255, 255,   0 },
  {   0,   0, 255 },
  { 255,   0, 255 },
  {   0, 255, 255 },
  { 255, 255, 255 }  /* 1.000   7 */
};

static inline term_color_t
rgb_to_color_xterm8 (int r, int g, int b)
{
  rgb_t color;
  hsv_t hsv;

  color.red = r; color.green = g; color.blue = b;
  rgb_to_hsv (color, &hsv);

  if (hsv.saturation < 0.065f)
    {
      /* Greyscale approximation.  */
      float luminance = color_luminance (r, g, b);
      if (luminance < 0.500f)
        return 0;
      else
        return 7;
    }
  else
    /* Color approximation.  */
    return nearest_color (color, colors_of_xterm8, 8);
}

/* ------------------------ cm_xterm16 color model ------------------------ */

/* A non-default color index is in the range 0..15.
   The RGB values come from xterm's XTerm-col.ad.  */
static const rgb_t colors_of_xterm16[16] =
{
  /* R    G    B        grey  index */
  {   0,   0,   0 }, /* 0.000   0 */
  { 205,   0,   0 },
  {   0, 205,   0 },
  { 205, 205,   0 },
  {   0,   0, 205 },
  { 205,   0, 205 },
  {   0, 205, 205 },
  { 229, 229, 229 }, /* 0.898   7 */
  {  77,  77,  77 }, /* 0.302   8 */
  { 255,   0,   0 },
  {   0, 255,   0 },
  { 255, 255,   0 },
  {   0,   0, 255 },
  { 255,   0, 255 },
  {   0, 255, 255 },
  { 255, 255, 255 }  /* 1.000  15 */
};

static inline term_color_t
rgb_to_color_xterm16 (int r, int g, int b)
{
  rgb_t color;
  hsv_t hsv;

  color.red = r; color.green = g; color.blue = b;
  rgb_to_hsv (color, &hsv);

  if (hsv.saturation < 0.065f)
    {
      /* Greyscale approximation.  */
      float luminance = color_luminance (r, g, b);
      if (luminance < 0.151f)
        return 0;
      else if (luminance < 0.600f)
        return 8;
      else if (luminance < 0.949f)
        return 7;
      else
        return 15;
    }
  else
    /* Color approximation.  */
    return nearest_color (color, colors_of_xterm16, 16);
}

/* ------------------------ cm_xterm88 color model ------------------------ */

/* A non-default color index is in the range 0..87.
   Colors 0..15 are the same as in the cm_xterm16 color model.
   Colors 16..87 are defined in xterm's 88colres.h.  */

static const rgb_t colors_of_xterm88[88] =
{
  /* R    G    B        grey  index */
  {   0,   0,   0 }, /* 0.000   0 */
  { 205,   0,   0 },
  {   0, 205,   0 },
  { 205, 205,   0 },
  {   0,   0, 205 },
  { 205,   0, 205 },
  {   0, 205, 205 },
  { 229, 229, 229 }, /* 0.898   7 */
  {  77,  77,  77 }, /* 0.302   8 */
  { 255,   0,   0 },
  {   0, 255,   0 },
  { 255, 255,   0 },
  {   0,   0, 255 },
  { 255,   0, 255 },
  {   0, 255, 255 },
  { 255, 255, 255 }, /* 1.000  15 */
  {   0,   0,   0 }, /* 0.000  16 */
  {   0,   0, 139 },
  {   0,   0, 205 },
  {   0,   0, 255 },
  {   0, 139,   0 },
  {   0, 139, 139 },
  {   0, 139, 205 },
  {   0, 139, 255 },
  {   0, 205,   0 },
  {   0, 205, 139 },
  {   0, 205, 205 },
  {   0, 205, 255 },
  {   0, 255,   0 },
  {   0, 255, 139 },
  {   0, 255, 205 },
  {   0, 255, 255 },
  { 139,   0,   0 },
  { 139,   0, 139 },
  { 139,   0, 205 },
  { 139,   0, 255 },
  { 139, 139,   0 },
  { 139, 139, 139 }, /* 0.545  37 */
  { 139, 139, 205 },
  { 139, 139, 255 },
  { 139, 205,   0 },
  { 139, 205, 139 },
  { 139, 205, 205 },
  { 139, 205, 255 },
  { 139, 255,   0 },
  { 139, 255, 139 },
  { 139, 255, 205 },
  { 139, 255, 255 },
  { 205,   0,   0 },
  { 205,   0, 139 },
  { 205,   0, 205 },
  { 205,   0, 255 },
  { 205, 139,   0 },
  { 205, 139, 139 },
  { 205, 139, 205 },
  { 205, 139, 255 },
  { 205, 205,   0 },
  { 205, 205, 139 },
  { 205, 205, 205 }, /* 0.804  58 */
  { 205, 205, 255 },
  { 205, 255,   0 },
  { 205, 255, 139 },
  { 205, 255, 205 },
  { 205, 255, 255 },
  { 255,   0,   0 },
  { 255,   0, 139 },
  { 255,   0, 205 },
  { 255,   0, 255 },
  { 255, 139,   0 },
  { 255, 139, 139 },
  { 255, 139, 205 },
  { 255, 139, 255 },
  { 255, 205,   0 },
  { 255, 205, 139 },
  { 255, 205, 205 },
  { 255, 205, 255 },
  { 255, 255,   0 },
  { 255, 255, 139 },
  { 255, 255, 205 },
  { 255, 255, 255 }, /* 1.000  79 */
  {  46,  46,  46 }, /* 0.180  80 */
  {  92,  92,  92 }, /* 0.361  81 */
  { 115, 115, 115 }, /* 0.451  82 */
  { 139, 139, 139 }, /* 0.545  83 */
  { 162, 162, 162 }, /* 0.635  84 */
  { 185, 185, 185 }, /* 0.725  85 */
  { 208, 208, 208 }, /* 0.816  86 */
  { 231, 231, 231 }  /* 0.906  87 */
};

static inline term_color_t
rgb_to_color_xterm88 (int r, int g, int b)
{
  rgb_t color;
  hsv_t hsv;

  color.red = r; color.green = g; color.blue = b;
  rgb_to_hsv (color, &hsv);

  if (hsv.saturation < 0.065f)
    {
      /* Greyscale approximation.  */
      float luminance = color_luminance (r, g, b);
      if (luminance < 0.090f)
        return 0;
      else if (luminance < 0.241f)
        return 80;
      else if (luminance < 0.331f)
        return 8;
      else if (luminance < 0.406f)
        return 81;
      else if (luminance < 0.498f)
        return 82;
      else if (luminance < 0.585f)
        return 37;
      else if (luminance < 0.680f)
        return 84;
      else if (luminance < 0.764f)
        return 85;
      else if (luminance < 0.810f)
        return 58;
      else if (luminance < 0.857f)
        return 86;
      else if (luminance < 0.902f)
        return 7;
      else if (luminance < 0.953f)
        return 87;
      else
        return 15;
    }
  else
    /* Color approximation.  */
    return nearest_color (color, colors_of_xterm88, 88);
}

/* ------------------------ cm_xterm256 color model ------------------------ */

/* A non-default color index is in the range 0..255.
   Colors 0..15 are the same as in the cm_xterm16 color model.
   Colors 16..255 are defined in xterm's 256colres.h.  */

static const rgb_t colors_of_xterm256[256] =
{
  /* R    G    B        grey  index */
  {   0,   0,   0 }, /* 0.000   0 */
  { 205,   0,   0 },
  {   0, 205,   0 },
  { 205, 205,   0 },
  {   0,   0, 205 },
  { 205,   0, 205 },
  {   0, 205, 205 },
  { 229, 229, 229 }, /* 0.898   7 */
  {  77,  77,  77 }, /* 0.302   8 */
  { 255,   0,   0 },
  {   0, 255,   0 },
  { 255, 255,   0 },
  {   0,   0, 255 },
  { 255,   0, 255 },
  {   0, 255, 255 },
  { 255, 255, 255 }, /* 1.000  15 */
  {   0,   0,   0 }, /* 0.000  16 */
  {   0,   0,  42 },
  {   0,   0,  85 },
  {   0,   0, 127 },
  {   0,   0, 170 },
  {   0,   0, 212 },
  {   0,  42,   0 },
  {   0,  42,  42 },
  {   0,  42,  85 },
  {   0,  42, 127 },
  {   0,  42, 170 },
  {   0,  42, 212 },
  {   0,  85,   0 },
  {   0,  85,  42 },
  {   0,  85,  85 },
  {   0,  85, 127 },
  {   0,  85, 170 },
  {   0,  85, 212 },
  {   0, 127,   0 },
  {   0, 127,  42 },
  {   0, 127,  85 },
  {   0, 127, 127 },
  {   0, 127, 170 },
  {   0, 127, 212 },
  {   0, 170,   0 },
  {   0, 170,  42 },
  {   0, 170,  85 },
  {   0, 170, 127 },
  {   0, 170, 170 },
  {   0, 170, 212 },
  {   0, 212,   0 },
  {   0, 212,  42 },
  {   0, 212,  85 },
  {   0, 212, 127 },
  {   0, 212, 170 },
  {   0, 212, 212 },
  {  42,   0,   0 },
  {  42,   0,  42 },
  {  42,   0,  85 },
  {  42,   0, 127 },
  {  42,   0, 170 },
  {  42,   0, 212 },
  {  42,  42,   0 },
  {  42,  42,  42 }, /* 0.165  59 */
  {  42,  42,  85 },
  {  42,  42, 127 },
  {  42,  42, 170 },
  {  42,  42, 212 },
  {  42,  85,   0 },
  {  42,  85,  42 },
  {  42,  85,  85 },
  {  42,  85, 127 },
  {  42,  85, 170 },
  {  42,  85, 212 },
  {  42, 127,   0 },
  {  42, 127,  42 },
  {  42, 127,  85 },
  {  42, 127, 127 },
  {  42, 127, 170 },
  {  42, 127, 212 },
  {  42, 170,   0 },
  {  42, 170,  42 },
  {  42, 170,  85 },
  {  42, 170, 127 },
  {  42, 170, 170 },
  {  42, 170, 212 },
  {  42, 212,   0 },
  {  42, 212,  42 },
  {  42, 212,  85 },
  {  42, 212, 127 },
  {  42, 212, 170 },
  {  42, 212, 212 },
  {  85,   0,   0 },
  {  85,   0,  42 },
  {  85,   0,  85 },
  {  85,   0, 127 },
  {  85,   0, 170 },
  {  85,   0, 212 },
  {  85,  42,   0 },
  {  85,  42,  42 },
  {  85,  42,  85 },
  {  85,  42, 127 },
  {  85,  42, 170 },
  {  85,  42, 212 },
  {  85,  85,   0 },
  {  85,  85,  42 },
  {  85,  85,  85 }, /* 0.333 102 */
  {  85,  85, 127 },
  {  85,  85, 170 },
  {  85,  85, 212 },
  {  85, 127,   0 },
  {  85, 127,  42 },
  {  85, 127,  85 },
  {  85, 127, 127 },
  {  85, 127, 170 },
  {  85, 127, 212 },
  {  85, 170,   0 },
  {  85, 170,  42 },
  {  85, 170,  85 },
  {  85, 170, 127 },
  {  85, 170, 170 },
  {  85, 170, 212 },
  {  85, 212,   0 },
  {  85, 212,  42 },
  {  85, 212,  85 },
  {  85, 212, 127 },
  {  85, 212, 170 },
  {  85, 212, 212 },
  { 127,   0,   0 },
  { 127,   0,  42 },
  { 127,   0,  85 },
  { 127,   0, 127 },
  { 127,   0, 170 },
  { 127,   0, 212 },
  { 127,  42,   0 },
  { 127,  42,  42 },
  { 127,  42,  85 },
  { 127,  42, 127 },
  { 127,  42, 170 },
  { 127,  42, 212 },
  { 127,  85,   0 },
  { 127,  85,  42 },
  { 127,  85,  85 },
  { 127,  85, 127 },
  { 127,  85, 170 },
  { 127,  85, 212 },
  { 127, 127,   0 },
  { 127, 127,  42 },
  { 127, 127,  85 },
  { 127, 127, 127 }, /* 0.498 145 */
  { 127, 127, 170 },
  { 127, 127, 212 },
  { 127, 170,   0 },
  { 127, 170,  42 },
  { 127, 170,  85 },
  { 127, 170, 127 },
  { 127, 170, 170 },
  { 127, 170, 212 },
  { 127, 212,   0 },
  { 127, 212,  42 },
  { 127, 212,  85 },
  { 127, 212, 127 },
  { 127, 212, 170 },
  { 127, 212, 212 },
  { 170,   0,   0 },
  { 170,   0,  42 },
  { 170,   0,  85 },
  { 170,   0, 127 },
  { 170,   0, 170 },
  { 170,   0, 212 },
  { 170,  42,   0 },
  { 170,  42,  42 },
  { 170,  42,  85 },
  { 170,  42, 127 },
  { 170,  42, 170 },
  { 170,  42, 212 },
  { 170,  85,   0 },
  { 170,  85,  42 },
  { 170,  85,  85 },
  { 170,  85, 127 },
  { 170,  85, 170 },
  { 170,  85, 212 },
  { 170, 127,   0 },
  { 170, 127,  42 },
  { 170, 127,  85 },
  { 170, 127, 127 },
  { 170, 127, 170 },
  { 170, 127, 212 },
  { 170, 170,   0 },
  { 170, 170,  42 },
  { 170, 170,  85 },
  { 170, 170, 127 },
  { 170, 170, 170 }, /* 0.667 188 */
  { 170, 170, 212 },
  { 170, 212,   0 },
  { 170, 212,  42 },
  { 170, 212,  85 },
  { 170, 212, 127 },
  { 170, 212, 170 },
  { 170, 212, 212 },
  { 212,   0,   0 },
  { 212,   0,  42 },
  { 212,   0,  85 },
  { 212,   0, 127 },
  { 212,   0, 170 },
  { 212,   0, 212 },
  { 212,  42,   0 },
  { 212,  42,  42 },
  { 212,  42,  85 },
  { 212,  42, 127 },
  { 212,  42, 170 },
  { 212,  42, 212 },
  { 212,  85,   0 },
  { 212,  85,  42 },
  { 212,  85,  85 },
  { 212,  85, 127 },
  { 212,  85, 170 },
  { 212,  85, 212 },
  { 212, 127,   0 },
  { 212, 127,  42 },
  { 212, 127,  85 },
  { 212, 127, 127 },
  { 212, 127, 170 },
  { 212, 127, 212 },
  { 212, 170,   0 },
  { 212, 170,  42 },
  { 212, 170,  85 },
  { 212, 170, 127 },
  { 212, 170, 170 },
  { 212, 170, 212 },
  { 212, 212,   0 },
  { 212, 212,  42 },
  { 212, 212,  85 },
  { 212, 212, 127 },
  { 212, 212, 170 },
  { 212, 212, 212 }, /* 0.831 231 */
  {   8,   8,   8 }, /* 0.031 232 */
  {  18,  18,  18 }, /* 0.071 233 */
  {  28,  28,  28 }, /* 0.110 234 */
  {  38,  38,  38 }, /* 0.149 235 */
  {  48,  48,  48 }, /* 0.188 236 */
  {  58,  58,  58 }, /* 0.227 237 */
  {  68,  68,  68 }, /* 0.267 238 */
  {  78,  78,  78 }, /* 0.306 239 */
  {  88,  88,  88 }, /* 0.345 240 */
  {  98,  98,  98 }, /* 0.384 241 */
  { 108, 108, 108 }, /* 0.424 242 */
  { 118, 118, 118 }, /* 0.463 243 */
  { 128, 128, 128 }, /* 0.502 244 */
  { 138, 138, 138 }, /* 0.541 245 */
  { 148, 148, 148 }, /* 0.580 246 */
  { 158, 158, 158 }, /* 0.620 247 */
  { 168, 168, 168 }, /* 0.659 248 */
  { 178, 178, 178 }, /* 0.698 249 */
  { 188, 188, 188 }, /* 0.737 250 */
  { 198, 198, 198 }, /* 0.776 251 */
  { 208, 208, 208 }, /* 0.816 252 */
  { 218, 218, 218 }, /* 0.855 253 */
  { 228, 228, 228 }, /* 0.894 254 */
  { 238, 238, 238 }  /* 0.933 255 */
};

static inline term_color_t
rgb_to_color_xterm256 (int r, int g, int b)
{
  rgb_t color;
  hsv_t hsv;

  color.red = r; color.green = g; color.blue = b;
  rgb_to_hsv (color, &hsv);

  if (hsv.saturation < 0.065f)
    {
      /* Greyscale approximation.  */
      float luminance = color_luminance (r, g, b);
      if (luminance < 0.015f)
        return 0;
      else if (luminance < 0.051f)
        return 232;
      else if (luminance < 0.090f)
        return 233;
      else if (luminance < 0.129f)
        return 234;
      else if (luminance < 0.157f)
        return 235;
      else if (luminance < 0.177f)
        return 59;
      else if (luminance < 0.207f)
        return 236;
      else if (luminance < 0.247f)
        return 237;
      else if (luminance < 0.284f)
        return 238;
      else if (luminance < 0.304f)
        return 8;
      else if (luminance < 0.319f)
        return 239;
      else if (luminance < 0.339f)
        return 102;
      else if (luminance < 0.364f)
        return 240;
      else if (luminance < 0.404f)
        return 241;
      else if (luminance < 0.443f)
        return 242;
      else if (luminance < 0.480f)
        return 243;
      else if (luminance < 0.500f)
        return 145;
      else if (luminance < 0.521f)
        return 244;
      else if (luminance < 0.560f)
        return 245;
      else if (luminance < 0.600f)
        return 246;
      else if (luminance < 0.639f)
        return 247;
      else if (luminance < 0.663f)
        return 248;
      else if (luminance < 0.682f)
        return 188;
      else if (luminance < 0.717f)
        return 249;
      else if (luminance < 0.756f)
        return 250;
      else if (luminance < 0.796f)
        return 251;
      else if (luminance < 0.823f)
        return 252;
      else if (luminance < 0.843f)
        return 231;
      else if (luminance < 0.874f)
        return 253;
      else if (luminance < 0.896f)
        return 254;
      else if (luminance < 0.915f)
        return 7;
      else if (luminance < 0.966f)
        return 255;
      else
        return 15;
    }
  else
    /* Color approximation.  */
    return nearest_color (color, colors_of_xterm256, 256);
}


/* ============================= attributes_t ============================= */

/* ANSI C and ISO C99 6.7.2.1.(4) forbid use of bit fields for types other
   than 'int' or 'unsigned int'.
   On the other hand, C++ forbids conversion between enum types and integer
   types without an explicit cast.  */
#ifdef __cplusplus
# define BITFIELD_TYPE(orig_type,integer_type) orig_type
#else
# define BITFIELD_TYPE(orig_type,integer_type) integer_type
#endif

/* Attributes that can be set on a character.  */
typedef struct
{
  BITFIELD_TYPE(term_color_t,     signed int)   color     : 9;
  BITFIELD_TYPE(term_color_t,     signed int)   bgcolor   : 9;
  BITFIELD_TYPE(term_weight_t,    unsigned int) weight    : 1;
  BITFIELD_TYPE(term_posture_t,   unsigned int) posture   : 1;
  BITFIELD_TYPE(term_underline_t, unsigned int) underline : 1;
} attributes_t;

/* Compare two sets of attributes for equality.  */
static inline bool
equal_attributes (attributes_t attr1, attributes_t attr2)
{
  return (attr1.color == attr2.color
          && attr1.bgcolor == attr2.bgcolor
          && attr1.weight == attr2.weight
          && attr1.posture == attr2.posture
          && attr1.underline == attr2.underline);
}


/* ============================ EINTR handling ============================ */

/* EINTR handling for tcgetattr(), tcsetattr(), tcdrain().
   These functions can return -1/EINTR even when we don't have any
   signal handlers set up, namely when we get interrupted via SIGSTOP.  */

#if HAVE_TCGETATTR

static inline int
nonintr_tcgetattr (int fd, struct termios *tcp)
{
  int retval;

  do
    retval = tcgetattr (fd, tcp);
  while (retval < 0 && errno == EINTR);

  return retval;
}

static inline int
nonintr_tcsetattr (int fd, int flush_mode, const struct termios *tcp)
{
  int retval;

  do
    retval = tcsetattr (fd, flush_mode, tcp);
  while (retval < 0 && errno == EINTR);

  return retval;
}

#endif

#if HAVE_TCDRAIN

static inline int
nonintr_tcdrain (int fd)
{
  int retval;

  do
    retval = tcdrain (fd);
  while (retval < 0 && errno == EINTR);

  return retval;
}

#endif


/* ========================== Logging primitives ========================== */

/* We need logging, especially for the signal handling, because
     - Debugging through gdb is hardly possible, because gdb produces output
       by itself and interferes with the process states.
     - strace is buggy when it comes to SIGTSTP handling:  By default, it
       sends the process a SIGSTOP signal instead of SIGTSTP.  It supports
       an option '-D -I4' to mitigate this, though.  Also, race conditions
       appear with different probability with and without strace.
   fprintf(stderr) is not possible within async-safe code, because fprintf()
   may invoke malloc().  */

#if DEBUG_SIGNALS

/* Log a simple message.  */
static _GL_ASYNC_SAFE void
log_message (const char *message)
{
  full_write (STDERR_FILENO, message, strlen (message));
}

#else

# define log_message(message)

#endif

#if HAVE_TCGETATTR || DEBUG_SIGNALS

/* Async-safe implementation of sprintf (str, "%d", n).  */
static _GL_ASYNC_SAFE void
sprintf_integer (char *str, int x)
{
  unsigned int y;
  char buf[20];
  char *p;
  size_t n;

  if (x < 0)
    {
      *str++ = '-';
      y = (unsigned int) (-1 - x) + 1;
    }
  else
    y = x;

  p = buf + sizeof (buf);
  do
    {
      *--p = '0' + (y % 10);
      y = y / 10;
    }
  while (y > 0);
  n = buf + sizeof (buf) - p;
  memcpy (str, p, n);
  str[n] = '\0';
}

#endif

#if HAVE_TCGETATTR

/* Async-safe conversion of errno value to string.  */
static _GL_ASYNC_SAFE void
simple_errno_string (char *str, int errnum)
{
  switch (errnum)
    {
    case EBADF:  strcpy (str, "EBADF"); break;
    case EINTR:  strcpy (str, "EINTR"); break;
    case EINVAL: strcpy (str, "EINVAL"); break;
    case EIO:    strcpy (str, "EIO"); break;
    case ENOTTY: strcpy (str, "ENOTTY"); break;
    default: sprintf_integer (str, errnum); break;
    }
}

#endif

#if DEBUG_SIGNALS

/* Async-safe conversion of signal number to name.  */
static _GL_ASYNC_SAFE void
simple_signal_string (char *str, int sig)
{
  switch (sig)
    {
    /* Fatal signals (see fatal-signal.c).  */
    #ifdef SIGINT
    case SIGINT:   strcpy (str, "SIGINT"); break;
    #endif
    #ifdef SIGTERM
    case SIGTERM:  strcpy (str, "SIGTERM"); break;
    #endif
    #ifdef SIGHUP
    case SIGHUP:   strcpy (str, "SIGHUP"); break;
    #endif
    #ifdef SIGPIPE
    case SIGPIPE:  strcpy (str, "SIGPIPE"); break;
    #endif
    #ifdef SIGXCPU
    case SIGXCPU:  strcpy (str, "SIGXCPU"); break;
    #endif
    #ifdef SIGXFSZ
    case SIGXFSZ:  strcpy (str, "SIGXFSZ"); break;
    #endif
    #ifdef SIGBREAK
    case SIGBREAK: strcpy (str, "SIGBREAK"); break;
    #endif
    /* Stopping signals.  */
    #ifdef SIGTSTP
    case SIGTSTP:  strcpy (str, "SIGTSTP"); break;
    #endif
    #ifdef SIGTTIN
    case SIGTTIN:  strcpy (str, "SIGTTIN"); break;
    #endif
    #ifdef SIGTTOU
    case SIGTTOU:  strcpy (str, "SIGTTOU"); break;
    #endif
    /* Continuing signals.  */
    #ifdef SIGCONT
    case SIGCONT:  strcpy (str, "SIGCONT"); break;
    #endif
    default: sprintf_integer (str, sig); break;
    }
}

/* Emit a message that a given signal handler is being run.  */
static _GL_ASYNC_SAFE void
log_signal_handler_called (int sig)
{
  char message[100];
  strcpy (message, "Signal handler for signal ");
  simple_signal_string (message + strlen (message), sig);
  strcat (message, " called.\n");
  log_message (message);
}

#else

# define log_signal_handler_called(sig)

#endif


/* ============================ term_ostream_t ============================ */

struct term_ostream : struct ostream
{
fields:
  /* The file descriptor used for output.  Note that ncurses termcap emulation
     uses the baud rate information from file descriptor 1 (stdout) if it is
     a tty, or from file descriptor 2 (stderr) otherwise.  */
  int fd;
  char *filename;
  /* Values from the terminal type's terminfo/termcap description.
     See terminfo(5) for details.  */
                                         /* terminfo  termcap */
  int max_colors;                        /* colors    Co */
  int no_color_video;                    /* ncv       NC */
  char * volatile set_a_foreground;      /* setaf     AF */
  char * volatile set_foreground;        /* setf      Sf */
  char * volatile set_a_background;      /* setab     AB */
  char * volatile set_background;        /* setb      Sb */
  char *orig_pair;                       /* op        op */
  char * volatile enter_bold_mode;       /* bold      md */
  char * volatile enter_italics_mode;    /* sitm      ZH */
  char *exit_italics_mode;               /* ritm      ZR */
  char * volatile enter_underline_mode;  /* smul      us */
  char *exit_underline_mode;             /* rmul      ue */
  char *exit_attribute_mode;             /* sgr0      me */
  /* Inferred values.  */
  bool volatile supports_foreground;
  bool volatile supports_background;
  colormodel_t volatile colormodel;
  bool volatile supports_weight;
  bool volatile supports_posture;
  bool volatile supports_underline;
  /* Inferred values for the exit handler and the signal handlers.  */
  const char * volatile restore_colors;
  const char * volatile restore_weight;
  const char * volatile restore_posture;
  const char * volatile restore_underline;
  /* Signal handling and tty control.  */
  ttyctl_t volatile tty_control;
  #if HAVE_TCGETATTR
  bool volatile same_as_stderr;
  #endif
  /* Variable state, representing past output.  */
  attributes_t default_attr;         /* Default simplified attributes of the
                                        terminal.  */
  attributes_t volatile active_attr; /* Simplified attributes that we have set
                                        on the terminal.  */
  bool non_default_active;           /* True if activate_non_default_attr()
                                        is in effect.
                                        active_attr != default_attr implies
                                        non_default_active == true,
                                        but not the opposite!  */
  /* Variable state, representing future output.  */
  char *buffer;                      /* Buffer for the current line.  */
  attributes_t *attrbuffer;          /* Buffer for the simplified attributes;
                                        same length as buffer.  */
  size_t buflen;                     /* Number of bytes stored so far.  */
  size_t allocated;                  /* Allocated size of the buffer.  */
  attributes_t curr_attr;            /* Current attributes.  */
  attributes_t simp_attr;            /* Simplified current attributes.  */
};

/* Simplify attributes, according to the terminal's capabilities.  */
static attributes_t
simplify_attributes (term_ostream_t stream, attributes_t attr)
{
  if ((attr.color != COLOR_DEFAULT || attr.bgcolor != COLOR_DEFAULT)
      && stream->no_color_video > 0)
    {
      /* When colors and attributes can not be represented simultaneously,
         we give preference to the color.  */
      if (stream->no_color_video & 2)
        /* Colors conflict with underlining.  */
        attr.underline = UNDERLINE_OFF;
      if (stream->no_color_video & 32)
        /* Colors conflict with bold weight.  */
        attr.weight = WEIGHT_NORMAL;
    }
  if (!stream->supports_foreground)
    attr.color = COLOR_DEFAULT;
  if (!stream->supports_background)
    attr.bgcolor = COLOR_DEFAULT;
  if (!stream->supports_weight)
    attr.weight = WEIGHT_DEFAULT;
  if (!stream->supports_posture)
    attr.posture = POSTURE_DEFAULT;
  if (!stream->supports_underline)
    attr.underline = UNDERLINE_DEFAULT;
  return attr;
}

/* There are several situations which can cause garbled output on the terminal's
   screen:
   (1) When the program calls exit() after calling flush_to_current_style,
       the program would terminate and leave the terminal in a non-default
       state.
   (2) When the program is interrupted through a fatal signal, the terminal
       would be left in a non-default state.
   (3) When the program is stopped through a stopping signal, the terminal
       would be left (for temporary use by other programs) in a non-default
       state.
   (4) When a foreground process receives a SIGINT, the kernel(!) prints '^C'.
       On Linux, the place where this happens is
         linux-5.0/drivers/tty/n_tty.c:713..730
       within a call sequence
         n_tty_receive_signal_char (n_tty.c:1245..1246)
         -> commit_echoes (n_tty.c:792)
         -> __process_echoes (n_tty.c:713..730).
   (5) When a signal is sent, the output buffer is cleared.
       On Linux, this output buffer consists of the "echo buffer" in the tty
       and the "output buffer" in the driver.  The place where this happens is
         linux-5.0/drivers/tty/n_tty.c:1133..1140
       within a call
         isig (n_tty.c:1133..1140).

   How do we mitigate these problems?
   (1) We install an exit handler that restores the terminal to the default
       state.
   (2) If tty_control is TTYCTL_PARTIAL or TTYCTL_FULL:
       For some of the fatal signals (see gnulib's 'fatal-signal' module for
       the precise list), we install a handler that attempts to restore the
       terminal to the default state.  Since the terminal may be in the middle
       of outputting an escape sequence at this point, the first escape
       sequence emitted from this handler may have no effect and produce
       garbled characters instead.  Therefore the handler outputs the cleanup
       sequence twice.
       For the other fatal signals, we don't do anything.
   (3) If tty_control is TTYCTL_PARTIAL or TTYCTL_FULL:
       For some of the stopping signals (SIGTSTP, SIGTTIN, SIGTTOU), we install
       a handler that attempts to restore the terminal to the default state.
       For SIGCONT, we install a handler that does the opposite: it puts the
       terminal into the desired state again.
       For SIGSTOP, we cannot do anything.
   (4) If tty_control is TTYCTL_FULL:
       The kernel's action depends on L_ECHO(tty) and L_ISIG(tty), that is, on
       the local modes of the tty (see
       <https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap11.html>
       section 11.2.5).  We don't want to change L_ISIG; hence we change L_ECHO.
       So, we disable the ECHO local flag of the tty; the equivalent command is
       'stty -echo'.
   (5) If tty_control is TTYCTL_FULL:
       The kernel's action depends on !L_NOFLSH(tty), that is, again on the
       local modes of the tty (see
       <https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap11.html>
       section 11.2.5).  So, we enable the NOFLSH local flag of the tty; the
       equivalent command is 'stty noflsh'.
       For terminals with a baud rate < 9600 this is suboptimal.  For this case
       - where the traditional flushing behaviour makes sense - we would use a
       technique that involves tcdrain(), TIOCOUTQ, and usleep() when it is OK
       to disable NOFLSH.

   Regarding (4) and (5), there is a complication: Changing the local modes is
   done through tcsetattr().  However, when the process is put into the
   background, tcsetattr() does not operate the same way as when the process is
   running in the foreground.
   To test this kind of behaviour, use the 'color-filter' example like this:
     $ yes | ./filter '.*'
     <Ctrl-Z>
     $ bg 1
   We have three possible implementation options:
     * If we don't ignore the signal SIGTTOU:
       If the TOSTOP bit in the terminal's local mode is clear (command
       equivalent: 'stty -tostop') and the process is put into the background,
       normal output would continue (per POSIX
       <https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap11.html>
       section 11.2.5) but tcsetattr() calls would cause it to stop due to
       a SIGTTOU signal (per POSIX
       <https://pubs.opengroup.org/onlinepubs/9699919799/functions/tcsetattr.html>).
       Thus, the program would behave differently with term-ostream than
       without.
     * If we ignore the signal SIGTTOU when the TOSTOP bit in the terminal's
       local mode is clear (i.e. when (tc.c_lflag & TOSTOP) == 0):
       The tcsetattr() calls do not stop the process, but they don't have the
       desired effect.
       On Linux, when I put the process into the background and then kill it with
       signal SIGINT, I can see that the last operation on the terminal settings
       (as shown by 'strace') is
         ioctl(1, TCSETSW, {B38400 opost isig icanon echo ...}) = 0
       and yet, once the process is terminated, the terminal settings contain
       '-echo', not 'echo'.
     * Don't call tcsetattr() if the process is not in the foreground.
       This approach produces reliable results.

   Blocking some signals while a non-default style is active is *not* useful:
     - It does not help against (1), since exit() is not a signal.
     - Signal handlers are the better approach against (2) and (3).
     - It does not help against (4) and (5), because the kernel's actions happen
       outside the process.  */
#define BLOCK_SIGNALS_DURING_NON_DEFAULT_STYLE_OUTPUT 0

/* File descriptor of the currently open term_ostream_t.  */
static int volatile term_fd = -1;

#if HAVE_TCGETATTR

/* Status of the process group of term_fd.  */
typedef enum
{
  PGRP_UNKNOWN = 0,     /* term_fd < 0.  Unknown status.  */
  PGRP_NO_TTY,          /* term_fd >= 0 but is not connected to a tty.  */
  PGRP_IN_FOREGROUND,   /* term_fd >= 0 is a tty.  This process is running in
                           the foreground.  */
  PGRP_IN_BACKGROUND    /* term_fd >= 0 is a tty.  This process is running in
                           the background.  */
} pgrp_status_t;
static pgrp_status_t volatile pgrp_status = PGRP_UNKNOWN;

/* Update pgrp_status, depending on term_fd.  */
static _GL_ASYNC_SAFE void
update_pgrp_status (void)
{
  int fd = term_fd;
  if (fd < 0)
    {
      pgrp_status = PGRP_UNKNOWN;
      log_message ("pgrp_status = PGRP_UNKNOWN\n");
    }
  else
    {
      pid_t p = tcgetpgrp (fd);
      if (p < 0)
        {
          pgrp_status = PGRP_NO_TTY;
          log_message ("pgrp_status = PGRP_NO_TTY\n");
        }
      else
        {
          /* getpgrp () changes when the process gets put into the background
             by a shell that implements job control.  */
          if (p == getpgrp ())
            {
              pgrp_status = PGRP_IN_FOREGROUND;
              log_message ("pgrp_status = PGRP_IN_FOREGROUND\n");
            }
          else
            {
              pgrp_status = PGRP_IN_BACKGROUND;
              log_message ("pgrp_status = PGRP_IN_BACKGROUND\n");
            }
        }
    }
}

#else

# define update_pgrp_status()

#endif

/* Stream that contains information about how the various out_* functions shall
   do output.  */
static term_ostream_t volatile out_stream;

/* File descriptor to which out_char and out_char_unchecked shall output escape
   sequences.
   Same as (out_stream != NULL ? out_stream->fd : -1).  */
static int volatile out_fd = -1;

/* Filename of out_fd.
   Same as (out_stream != NULL ? out_stream->filename : NULL).  */
static const char * volatile out_filename;

/* Signal error after full_write failed.  */
static void
out_error ()
{
  error (EXIT_FAILURE, errno, _("error writing to %s"), out_filename);
}

/* Output a single char to out_fd.  */
static int
out_char (int c)
{
  char bytes[1];

  bytes[0] = (char)c;
  /* We have to write directly to the file descriptor, not to a buffer with
     the same destination, because of the padding and sleeping that tputs()
     does.  */
  if (full_write (out_fd, bytes, 1) < 1)
    out_error ();
  return 0;
}

/* Output a single char to out_fd.  Ignore errors.  */
static _GL_ASYNC_SAFE int
out_char_unchecked (int c)
{
  char bytes[1];

  bytes[0] = (char)c;
  full_write (out_fd, bytes, 1);
  return 0;
}

/* Output escape sequences to switch the foreground color to NEW_COLOR.  */
static _GL_ASYNC_SAFE void
out_color_change (term_ostream_t stream, term_color_t new_color,
                  bool async_safe)
{
  assert (stream->supports_foreground);
  assert (new_color != COLOR_DEFAULT);
  switch (stream->colormodel)
    {
    case cm_common8:
      assert (new_color >= 0 && new_color < 8);
      if (stream->set_a_foreground != NULL)
        tputs (tparm (stream->set_a_foreground, color_bgr (new_color)),
               1, async_safe ? out_char_unchecked : out_char);
      else
        tputs (tparm (stream->set_foreground, new_color),
               1, async_safe ? out_char_unchecked : out_char);
      break;
    /* When we are dealing with an xterm, there is no need to go through
       tputs() because we know there is no padding and sleeping.  */
    case cm_xterm8:
      assert (new_color >= 0 && new_color < 8);
      {
        char bytes[5];
        bytes[0] = 0x1B; bytes[1] = '[';
        bytes[2] = '3'; bytes[3] = '0' + new_color;
        bytes[4] = 'm';
        if (full_write (out_fd, bytes, 5) < 5)
          if (!async_safe)
            out_error ();
      }
      break;
    case cm_xterm16:
      assert (new_color >= 0 && new_color < 16);
      {
        char bytes[5];
        bytes[0] = 0x1B; bytes[1] = '[';
        if (new_color < 8)
          {
            bytes[2] = '3'; bytes[3] = '0' + new_color;
          }
        else
          {
            bytes[2] = '9'; bytes[3] = '0' + (new_color - 8);
          }
        bytes[4] = 'm';
        if (full_write (out_fd, bytes, 5) < 5)
          if (!async_safe)
            out_error ();
      }
      break;
    case cm_xterm88:
      assert (new_color >= 0 && new_color < 88);
      {
        char bytes[10];
        char *p;
        bytes[0] = 0x1B; bytes[1] = '[';
        bytes[2] = '3'; bytes[3] = '8'; bytes[4] = ';';
        bytes[5] = '5'; bytes[6] = ';';
        p = bytes + 7;
        if (new_color >= 10)
          *p++ = '0' + (new_color / 10);
        *p++ = '0' + (new_color % 10);
        *p++ = 'm';
        if (full_write (out_fd, bytes, p - bytes) < p - bytes)
          if (!async_safe)
            out_error ();
      }
      break;
    case cm_xterm256:
      assert (new_color >= 0 && new_color < 256);
      {
        char bytes[11];
        char *p;
        bytes[0] = 0x1B; bytes[1] = '[';
        bytes[2] = '3'; bytes[3] = '8'; bytes[4] = ';';
        bytes[5] = '5'; bytes[6] = ';';
        p = bytes + 7;
        if (new_color >= 100)
          *p++ = '0' + (new_color / 100);
        if (new_color >= 10)
          *p++ = '0' + ((new_color % 100) / 10);
        *p++ = '0' + (new_color % 10);
        *p++ = 'm';
        if (full_write (out_fd, bytes, p - bytes) < p - bytes)
          if (!async_safe)
            out_error ();
      }
      break;
    default:
      abort ();
    }
}

/* Output escape sequences to switch the background color to NEW_BGCOLOR.  */
static _GL_ASYNC_SAFE void
out_bgcolor_change (term_ostream_t stream, term_color_t new_bgcolor,
                    bool async_safe)
{
  assert (stream->supports_background);
  assert (new_bgcolor != COLOR_DEFAULT);
  switch (stream->colormodel)
    {
    case cm_common8:
      assert (new_bgcolor >= 0 && new_bgcolor < 8);
      if (stream->set_a_background != NULL)
        tputs (tparm (stream->set_a_background, color_bgr (new_bgcolor)),
               1, async_safe ? out_char_unchecked : out_char);
      else
        tputs (tparm (stream->set_background, new_bgcolor),
               1, async_safe ? out_char_unchecked : out_char);
      break;
    /* When we are dealing with an xterm, there is no need to go through
       tputs() because we know there is no padding and sleeping.  */
    case cm_xterm8:
      assert (new_bgcolor >= 0 && new_bgcolor < 8);
      {
        char bytes[5];
        bytes[0] = 0x1B; bytes[1] = '[';
        bytes[2] = '4'; bytes[3] = '0' + new_bgcolor;
        bytes[4] = 'm';
        if (full_write (out_fd, bytes, 5) < 5)
          if (!async_safe)
            out_error ();
      }
      break;
    case cm_xterm16:
      assert (new_bgcolor >= 0 && new_bgcolor < 16);
      {
        char bytes[6];
        bytes[0] = 0x1B; bytes[1] = '[';
        if (new_bgcolor < 8)
          {
            bytes[2] = '4'; bytes[3] = '0' + new_bgcolor;
            bytes[4] = 'm';
            if (full_write (out_fd, bytes, 5) < 5)
              if (!async_safe)
                out_error ();
          }
        else
          {
            bytes[2] = '1'; bytes[3] = '0';
            bytes[4] = '0' + (new_bgcolor - 8); bytes[5] = 'm';
            if (full_write (out_fd, bytes, 6) < 6)
              if (!async_safe)
                out_error ();
          }
      }
      break;
    case cm_xterm88:
      assert (new_bgcolor >= 0 && new_bgcolor < 88);
      {
        char bytes[10];
        char *p;
        bytes[0] = 0x1B; bytes[1] = '[';
        bytes[2] = '4'; bytes[3] = '8'; bytes[4] = ';';
        bytes[5] = '5'; bytes[6] = ';';
        p = bytes + 7;
        if (new_bgcolor >= 10)
          *p++ = '0' + (new_bgcolor / 10);
        *p++ = '0' + (new_bgcolor % 10);
        *p++ = 'm';
        if (full_write (out_fd, bytes, p - bytes) < p - bytes)
          if (!async_safe)
            out_error ();
      }
      break;
    case cm_xterm256:
      assert (new_bgcolor >= 0 && new_bgcolor < 256);
      {
        char bytes[11];
        char *p;
        bytes[0] = 0x1B; bytes[1] = '[';
        bytes[2] = '4'; bytes[3] = '8'; bytes[4] = ';';
        bytes[5] = '5'; bytes[6] = ';';
        p = bytes + 7;
        if (new_bgcolor >= 100)
          *p++ = '0' + (new_bgcolor / 100);
        if (new_bgcolor >= 10)
          *p++ = '0' + ((new_bgcolor % 100) / 10);
        *p++ = '0' + (new_bgcolor % 10);
        *p++ = 'm';
        if (full_write (out_fd, bytes, p - bytes) < p - bytes)
          if (!async_safe)
            out_error ();
      }
      break;
    default:
      abort ();
    }
}

/* Output escape sequences to switch the weight to NEW_WEIGHT.  */
static _GL_ASYNC_SAFE void
out_weight_change (term_ostream_t stream, term_weight_t new_weight,
                   bool async_safe)
{
  assert (stream->supports_weight);
  assert (new_weight != WEIGHT_DEFAULT);
  /* This implies:  */
  assert (new_weight == WEIGHT_BOLD);
  tputs (stream->enter_bold_mode,
         1, async_safe ? out_char_unchecked : out_char);
}

/* Output escape sequences to switch the posture to NEW_POSTURE.  */
static _GL_ASYNC_SAFE void
out_posture_change (term_ostream_t stream, term_posture_t new_posture,
                    bool async_safe)
{
  assert (stream->supports_posture);
  assert (new_posture != POSTURE_DEFAULT);
  /* This implies:  */
  assert (new_posture == POSTURE_ITALIC);
  tputs (stream->enter_italics_mode,
         1, async_safe ? out_char_unchecked : out_char);
}

/* Output escape sequences to switch the underline to NEW_UNDERLINE.  */
static _GL_ASYNC_SAFE void
out_underline_change (term_ostream_t stream, term_underline_t new_underline,
                      bool async_safe)
{
  assert (stream->supports_underline);
  assert (new_underline != UNDERLINE_DEFAULT);
  /* This implies:  */
  assert (new_underline == UNDERLINE_ON);
  tputs (stream->enter_underline_mode,
         1, async_safe ? out_char_unchecked : out_char);
}

/* The exit handler.  */
static void
restore (void)
{
  /* Only do something while some output was started but not completed.  */
  if (out_stream != NULL)
    {
      if (out_stream->restore_colors != NULL)
        tputs (out_stream->restore_colors, 1, out_char_unchecked);
      if (out_stream->restore_weight != NULL)
        tputs (out_stream->restore_weight, 1, out_char_unchecked);
      if (out_stream->restore_posture != NULL)
        tputs (out_stream->restore_posture, 1, out_char_unchecked);
      if (out_stream->restore_underline != NULL)
        tputs (out_stream->restore_underline, 1, out_char_unchecked);
    }
}

#if HAVE_TCGETATTR

/* Return a failure message after tcsetattr() failed.  */
static _GL_ASYNC_SAFE void
tcsetattr_failed (char message[100], const char *caller)
{
  int errnum = errno;
  strcpy (message, caller);
  strcat (message, ": tcsetattr(fd=");
  sprintf_integer (message + strlen (message), out_fd);
  strcat (message, ") failed, errno=");
  simple_errno_string (message + strlen (message), errnum);
  strcat (message, "\n");
}

/* True when orig_lflag represents the original tc.c_lflag.  */
static bool volatile orig_lflag_set;
static tcflag_t volatile orig_lflag;

/* Modifies the tty's local mode, preparing for non-default terminal state.
   Used only when the out_stream's tty_control is TTYCTL_FULL.  */
static _GL_ASYNC_SAFE void
clobber_local_mode (void)
{
  /* Here, out_fd == term_fd.  */
  struct termios tc;
  if (pgrp_status == PGRP_IN_FOREGROUND
      && nonintr_tcgetattr (out_fd, &tc) >= 0)
    {
      if (!orig_lflag_set)
        orig_lflag = tc.c_lflag;
      /* Set orig_lflag_set to true before actually modifying the tty's local
         mode, because restore_local_mode does nothing if orig_lflag_set is
         false.  */
      orig_lflag_set = true;
      tc.c_lflag &= ~ECHO;
      tc.c_lflag |= NOFLSH;
      if (nonintr_tcsetattr (out_fd, TCSANOW, &tc) < 0)
        {
          /* Since tcsetattr failed, restore_local_mode does not need to restore
             anything.  Set orig_lflag_set to false to indicate this.  */
          orig_lflag_set = false;
          {
            char message[100];
            tcsetattr_failed (message, "term_ostream:"":clobber_local_mode");
            full_write (STDERR_FILENO, message, strlen (message));
          }
        }
    }
}

/* Modifies the tty's local mode, once the terminal is back to the default state.
   Returns true if ECHO was turned off.
   Used only when the out_stream's tty_control is TTYCTL_FULL.  */
static _GL_ASYNC_SAFE bool
restore_local_mode (void)
{
  /* Here, out_fd == term_fd.  */
  bool echo_was_off = false;
  /* Nothing to do if !orig_lflag_set.  */
  if (orig_lflag_set)
    {
      struct termios tc;
      if (nonintr_tcgetattr (out_fd, &tc) >= 0)
        {
          echo_was_off = (tc.c_lflag & ECHO) == 0;
          tc.c_lflag = orig_lflag;
          if (nonintr_tcsetattr (out_fd, TCSADRAIN, &tc) < 0)
            {
              char message[100];
              tcsetattr_failed (message, "term_ostream:"":restore_local_mode");
              full_write (STDERR_FILENO, message, strlen (message));
            }
        }
      orig_lflag_set = false;
    }
  return echo_was_off;
}

#endif

#if defined SIGCONT

/* The list of signals whose default behaviour is to stop or continue the
   program.  */
static int const job_control_signals[] =
  {
    #ifdef SIGTSTP
    SIGTSTP,
    #endif
    #ifdef SIGTTIN
    SIGTTIN,
    #endif
    #ifdef SIGTTOU
    SIGTTOU,
    #endif
    #ifdef SIGCONT
    SIGCONT,
    #endif
    0
  };

# define num_job_control_signals (SIZEOF (job_control_signals) - 1)

#endif

/* The following signals are relevant because they do tputs invocations:
     - fatal signals,
     - stopping signals,
     - continuing signals (SIGCONT).  */

static sigset_t relevant_signal_set;
static bool relevant_signal_set_initialized = false;

static void
init_relevant_signal_set ()
{
  if (!relevant_signal_set_initialized)
    {
      int fatal_signals[64];
      size_t num_fatal_signals;
      size_t i;

      num_fatal_signals = get_fatal_signals (fatal_signals);

      sigemptyset (&relevant_signal_set);
      for (i = 0; i < num_fatal_signals; i++)
        sigaddset (&relevant_signal_set, fatal_signals[i]);
      #if defined SIGCONT
      for (i = 0; i < num_job_control_signals; i++)
        sigaddset (&relevant_signal_set, job_control_signals[i]);
      #endif

      relevant_signal_set_initialized = true;
    }
}

/* Temporarily delay the relevant signals.  */
static _GL_ASYNC_SAFE inline void
block_relevant_signals ()
{
  /* The caller must ensure that init_relevant_signal_set () was already
     called.  */
  if (!relevant_signal_set_initialized)
    abort ();

  sigprocmask (SIG_BLOCK, &relevant_signal_set, NULL);
}

/* Stop delaying the relevant signals.  */
static _GL_ASYNC_SAFE inline void
unblock_relevant_signals ()
{
  sigprocmask (SIG_UNBLOCK, &relevant_signal_set, NULL);
}

/* Determines whether a signal is ignored.  */
static _GL_ASYNC_SAFE bool
is_ignored (int sig)
{
  struct sigaction action;

  return (sigaction (sig, NULL, &action) >= 0
          && get_handler (&action) == SIG_IGN);
}

#if HAVE_TCGETATTR

/* Write the same signal marker that the kernel would have printed if ECHO had
   been turned on.  See (4) above.
   This is a makeshift and is not perfect:
     - When stderr refers to a different target than out_stream->fd, it is too
       hairy to write the signal marker.
     - In some cases, when the signal was generated right before and delivered
       right after a clobber_local_mode invocation, the result is that the
       marker appears twice, e.g. ^C^C.  This occurs only with a small
       probability.
     - In some cases, when the signal was generated right before and delivered
       right after a restore_local_mode invocation, the result is that the
       marker does not appear at all.  This occurs only with a small
       probability.
   To test this kind of behaviour, use the 'color-filter' example like this:
     $ yes | ./filter '.*'
 */
static _GL_ASYNC_SAFE void
show_signal_marker (int sig)
{
  /* Write to stderr, not to out_stream->fd, because out_stream->fd is often
     logged or used with 'less -R'.  */
  if (out_stream != NULL && out_stream->same_as_stderr)
    switch (sig)
      {
      /* The kernel's action when the user presses the INTR key.  */
      case SIGINT:
        full_write (STDERR_FILENO, "^C", 2); break;
      /* The kernel's action when the user presses the SUSP key.  */
      case SIGTSTP:
        full_write (STDERR_FILENO, "^Z", 2); break;
      /* The kernel's action when the user presses the QUIT key.  */
      case SIGQUIT:
        full_write (STDERR_FILENO, "^\\", 2); break;
      default: break;
      }
}

#endif

/* The main code of the signal handler for fatal signals and stopping signals.
   It is reentrant.  */
static _GL_ASYNC_SAFE void
fatal_or_stopping_signal_handler (int sig)
{
  bool echo_was_off = false;
  /* Only do something while some output was interrupted.  */
  if (out_stream != NULL && out_stream->tty_control != TTYCTL_NONE)
    {
      unsigned int i;

      /* Block the relevant signals.  This is needed, because the tputs
         invocations below are not reentrant.  */
      block_relevant_signals ();

      /* Restore the terminal to the default state.  */
      for (i = 0; i < 2; i++)
        {
          if (out_stream->restore_colors != NULL)
            tputs (out_stream->restore_colors, 1, out_char_unchecked);
          if (out_stream->restore_weight != NULL)
            tputs (out_stream->restore_weight, 1, out_char_unchecked);
          if (out_stream->restore_posture != NULL)
            tputs (out_stream->restore_posture, 1, out_char_unchecked);
          if (out_stream->restore_underline != NULL)
            tputs (out_stream->restore_underline, 1, out_char_unchecked);
        }
      #if HAVE_TCGETATTR
      if (out_stream->tty_control == TTYCTL_FULL)
        {
          /* Restore the local mode, once the tputs calls above have reached
             their destination.  */
          echo_was_off = restore_local_mode ();
        }
      #endif

      /* Unblock the relevant signals.  */
      unblock_relevant_signals ();
    }

  #if HAVE_TCGETATTR
  if (echo_was_off)
    show_signal_marker (sig);
  #endif
}

/* The signal handler for fatal signals.
   It is reentrant.  */
static _GL_ASYNC_SAFE void
fatal_signal_handler (int sig)
{
  log_signal_handler_called (sig);
  fatal_or_stopping_signal_handler (sig);
}

#if defined SIGCONT

/* The signal handler for stopping signals.
   It is reentrant.  */
static _GL_ASYNC_SAFE void
stopping_signal_handler (int sig)
{
  log_signal_handler_called (sig);
  fatal_or_stopping_signal_handler (sig);

  /* Now execute the signal's default action.
     We reinstall the handler later, during the SIGCONT handler.  */
  {
    struct sigaction action;
    action.sa_handler = SIG_DFL;
    action.sa_flags = SA_NODEFER;
    sigemptyset (&action.sa_mask);
    sigaction (sig, &action, NULL);
  }
  raise (sig);
}

/* The signal handler for SIGCONT.
   It is reentrant.  */
static _GL_ASYNC_SAFE void
continuing_signal_handler (int sig)
{
  log_signal_handler_called (sig);
  update_pgrp_status ();
  /* Only do something while some output was interrupted.  */
  if (out_stream != NULL && out_stream->tty_control != TTYCTL_NONE)
    {
      /* Reinstall the signals handlers removed in stopping_signal_handler.  */
      {
        unsigned int i;

        for (i = 0; i < num_job_control_signals; i++)
          {
            int sig = job_control_signals[i];

            if (sig != SIGCONT && !is_ignored (sig))
              {
                struct sigaction action;
                action.sa_handler = &stopping_signal_handler;
                /* If we get a stopping or continuing signal while executing
                   stopping_signal_handler or continuing_signal_handler, enter
                   it recursively, since it is reentrant.
                   Hence no SA_RESETHAND.  */
                action.sa_flags = SA_NODEFER;
                sigemptyset (&action.sa_mask);
                sigaction (sig, &action, NULL);
              }
          }
      }

      /* Block the relevant signals.  This is needed, because the tputs
         invocations done inside the out_* calls below are not reentrant.  */
      block_relevant_signals ();

      #if HAVE_TCGETATTR
      if (out_stream->tty_control == TTYCTL_FULL)
        {
          /* Modify the local mode.  */
          clobber_local_mode ();
        }
      #endif
      /* Set the terminal attributes.  */
      attributes_t new_attr = out_stream->active_attr;
      if (new_attr.color != COLOR_DEFAULT)
        out_color_change (out_stream, new_attr.color, true);
      if (new_attr.bgcolor != COLOR_DEFAULT)
        out_bgcolor_change (out_stream, new_attr.bgcolor, true);
      if (new_attr.weight != WEIGHT_DEFAULT)
        out_weight_change (out_stream, new_attr.weight, true);
      if (new_attr.posture != POSTURE_DEFAULT)
        out_posture_change (out_stream, new_attr.posture, true);
      if (new_attr.underline != UNDERLINE_DEFAULT)
        out_underline_change (out_stream, new_attr.underline, true);

      /* Unblock the relevant signals.  */
      unblock_relevant_signals ();
    }
}

/* Ensure the signal handlers are installed.
   Once they are installed, we leave them installed.  It's not worth
   installing and uninstalling them each time we switch the terminal to a
   non-default state and back; instead we set out_stream to tell the signal
   handler whether it has something to do or not.  */

static void
ensure_continuing_signal_handler (void)
{
  static bool signal_handler_installed = false;

  if (!signal_handler_installed)
    {
      int sig = SIGCONT;
      struct sigaction action;
      action.sa_handler = &continuing_signal_handler;
      /* If we get a stopping or continuing signal while executing
         continuing_signal_handler, enter it recursively, since it is
         reentrant.  Hence no SA_RESETHAND.  */
      action.sa_flags = SA_NODEFER;
      sigemptyset (&action.sa_mask);
      sigaction (sig, &action, NULL);

      signal_handler_installed = true;
    }
}

#endif

static void
ensure_other_signal_handlers (void)
{
  static bool signal_handlers_installed = false;

  if (!signal_handlers_installed)
    {
      unsigned int i;

      /* Install the handlers for the fatal signals.  */
      at_fatal_signal (fatal_signal_handler);

      #if defined SIGCONT

      /* Install the handlers for the stopping and continuing signals.  */
      for (i = 0; i < num_job_control_signals; i++)
        {
          int sig = job_control_signals[i];

          if (sig == SIGCONT)
            /* Already handled in ensure_continuing_signal_handler.  */
            ;
          else if (!is_ignored (sig))
            {
              struct sigaction action;
              action.sa_handler = &stopping_signal_handler;
              /* If we get a stopping or continuing signal while executing
                 stopping_signal_handler, enter it recursively, since it is
                 reentrant.  Hence no SA_RESETHAND.  */
              action.sa_flags = SA_NODEFER;
              sigemptyset (&action.sa_mask);
              sigaction (sig, &action, NULL);
            }
          #if DEBUG_SIGNALS
          else
            {
              fprintf (stderr, "Signal %d is ignored. Not installing a handler!\n",
                       sig);
              fflush (stderr);
            }
          #endif
        }

      #endif

      signal_handlers_installed = true;
    }
}

/* Output escape sequences to switch from STREAM->ACTIVE_ATTR to NEW_ATTR,
   and update STREAM->ACTIVE_ATTR.  */
static void
out_attr_change (term_ostream_t stream, attributes_t new_attr)
{
  attributes_t old_attr = stream->active_attr;
  bool cleared_attributes;

  /* We don't know the default colors of the terminal.  The only way to switch
     back to a default color is to use stream->orig_pair.  */
  if ((new_attr.color == COLOR_DEFAULT && old_attr.color != COLOR_DEFAULT)
      || (new_attr.bgcolor == COLOR_DEFAULT && old_attr.bgcolor != COLOR_DEFAULT))
    {
      assert (stream->supports_foreground || stream->supports_background);
      tputs (stream->orig_pair, 1, out_char);
      old_attr.color = COLOR_DEFAULT;
      old_attr.bgcolor = COLOR_DEFAULT;
    }

  /* To turn off WEIGHT_BOLD, the only way is to output the exit_attribute_mode
     sequence.  (With xterm, you can also do it with "Esc [ 0 m", but this
     escape sequence is not contained in the terminfo description.)  It may
     also clear the colors; this is the case e.g. when TERM="xterm" or
     TERM="ansi".
     To turn off UNDERLINE_ON, we can use the exit_underline_mode or the
     exit_attribute_mode sequence.  In the latter case, it will not only
     turn off UNDERLINE_ON, but also the other attributes, and possibly also
     the colors.
     To turn off POSTURE_ITALIC, we can use the exit_italics_mode or the
     exit_attribute_mode sequence.  Again, in the latter case, it will not
     only turn off POSTURE_ITALIC, but also the other attributes, and possibly
     also the colors.
     There is no point in setting an attribute just before emitting an
     escape sequence that may again turn off the attribute.  Therefore we
     proceed in two steps: First, clear the attributes that need to be
     cleared; then - taking into account that this may have cleared all
     attributes and all colors - set the colors and the attributes.
     The variable 'cleared_attributes' tells whether an escape sequence
     has been output that may have cleared all attributes and all color
     settings.  */
  cleared_attributes = false;
  if (old_attr.posture != POSTURE_NORMAL
      && new_attr.posture == POSTURE_NORMAL
      && stream->exit_italics_mode != NULL)
    {
      tputs (stream->exit_italics_mode, 1, out_char);
      old_attr.posture = POSTURE_NORMAL;
      cleared_attributes = true;
    }
  if (old_attr.underline != UNDERLINE_OFF
      && new_attr.underline == UNDERLINE_OFF
      && stream->exit_underline_mode != NULL)
    {
      tputs (stream->exit_underline_mode, 1, out_char);
      old_attr.underline = UNDERLINE_OFF;
      cleared_attributes = true;
    }
  if ((old_attr.weight != WEIGHT_NORMAL
       && new_attr.weight == WEIGHT_NORMAL)
      || (old_attr.posture != POSTURE_NORMAL
          && new_attr.posture == POSTURE_NORMAL
          /* implies stream->exit_italics_mode == NULL */)
      || (old_attr.underline != UNDERLINE_OFF
          && new_attr.underline == UNDERLINE_OFF
          /* implies stream->exit_underline_mode == NULL */))
    {
      tputs (stream->exit_attribute_mode, 1, out_char);
      /* We don't know exactly what effects exit_attribute_mode has, but
         this is the minimum effect:  */
      old_attr.weight = WEIGHT_NORMAL;
      if (stream->exit_italics_mode == NULL)
        old_attr.posture = POSTURE_NORMAL;
      if (stream->exit_underline_mode == NULL)
        old_attr.underline = UNDERLINE_OFF;
      cleared_attributes = true;
    }

  /* Turn on the colors.  */
  if (new_attr.color != old_attr.color
      || (cleared_attributes && new_attr.color != COLOR_DEFAULT))
    {
      out_color_change (stream, new_attr.color, false);
    }
  if (new_attr.bgcolor != old_attr.bgcolor
      || (cleared_attributes && new_attr.bgcolor != COLOR_DEFAULT))
    {
      out_bgcolor_change (stream, new_attr.bgcolor, false);
    }
  if (new_attr.weight != old_attr.weight
      || (cleared_attributes && new_attr.weight != WEIGHT_DEFAULT))
    {
      out_weight_change (stream, new_attr.weight, false);
    }
  if (new_attr.posture != old_attr.posture
      || (cleared_attributes && new_attr.posture != POSTURE_DEFAULT))
    {
      out_posture_change (stream, new_attr.posture, false);
    }
  if (new_attr.underline != old_attr.underline
      || (cleared_attributes && new_attr.underline != UNDERLINE_DEFAULT))
    {
      out_underline_change (stream, new_attr.underline, false);
    }

  /* Keep track of the active attributes.  */
  stream->active_attr = new_attr;
}

/* Prepare for activating some non-default attributes.
   Note: This may block some signals.  */
static void
activate_non_default_attr (term_ostream_t stream)
{
  if (!stream->non_default_active)
    {
      if (stream->tty_control != TTYCTL_NONE)
        ensure_other_signal_handlers ();

      #if BLOCK_SIGNALS_DURING_NON_DEFAULT_STYLE_OUTPUT
      /* Block fatal signals, so that a SIGINT or similar doesn't interrupt
         us without the possibility of restoring the terminal's state.
         Likewise for SIGTSTP etc.  */
      block_relevant_signals ();
      #endif

      /* Enable the exit handler for restoring the terminal's state,
         and make the signal handlers effective.  */
      if (out_stream != NULL)
        {
          /* We can't support two term_ostream_t active with non-default
             attributes at the same time.  */
          abort ();
        }
      /* The uses of 'volatile' (and ISO C 99 section 5.1.2.3.(5)) ensure that
         we set out_stream to a non-NULL value only after the memory locations
         out_fd and out_filename have been filled.  */
      out_fd = stream->fd;
      out_filename = stream->filename;
      out_stream = stream;

      #if HAVE_TCGETATTR
      /* Now that the signal handlers are effective, modify the tty.  */
      if (stream->tty_control == TTYCTL_FULL)
        {
          /* Modify the local mode.  */
          clobber_local_mode ();
        }
      #endif

      stream->non_default_active = true;
    }
}

/* Deactivate the non-default attributes mode.
   Note: This may unblock some signals.  */
static void
deactivate_non_default_attr (term_ostream_t stream)
{
  if (stream->non_default_active)
    {
      #if HAVE_TCGETATTR
      /* Before we make the signal handlers ineffective, modify the tty.  */
      if (out_stream->tty_control == TTYCTL_FULL)
        {
          /* Restore the local mode, once the tputs calls from out_attr_change
             have reached their destination.  */
          restore_local_mode ();
        }
      #endif

      /* Disable the exit handler, and make the signal handlers ineffective.  */
      /* The uses of 'volatile' (and ISO C 99 section 5.1.2.3.(5)) ensure that
         we reset out_fd and out_filename only after the memory location
         out_stream has been cleared.  */
      out_stream = NULL;
      out_fd = -1;
      out_filename = NULL;

      #if BLOCK_SIGNALS_DURING_NON_DEFAULT_STYLE_OUTPUT
      /* Unblock the relevant signals.  */
      unblock_relevant_signals ();
      #endif

      stream->non_default_active = false;
    }
}

/* Activate the default attributes.  */
static void
activate_default_attr (term_ostream_t stream)
{
  /* Switch back to the default attributes.  */
  out_attr_change (stream, stream->default_attr);

  deactivate_non_default_attr (stream);
}

/* Output the buffered line atomically.
   The terminal is left in the the state (regarding colors and attributes)
   represented by the simplified attributes goal_attr.  */
static void
output_buffer (term_ostream_t stream, attributes_t goal_attr)
{
  const char *cp;
  const attributes_t *ap;
  size_t len;
  size_t n;

  cp = stream->buffer;
  ap = stream->attrbuffer;
  len = stream->buflen;

  /* See how much we can output without blocking signals.  */
  for (n = 0; n < len && equal_attributes (ap[n], stream->active_attr); n++)
    ;
  if (n > 0)
    {
      if (full_write (stream->fd, cp, n) < n)
        {
          int error_code = errno;
          /* Do output to stderr only after we have switched back to the
             default attributes.  Otherwise this output may come out with
             the wrong text attributes.  */
          if (!equal_attributes (stream->active_attr, stream->default_attr))
            activate_default_attr (stream);
          error (EXIT_FAILURE, error_code, _("error writing to %s"),
                 stream->filename);
        }
      cp += n;
      ap += n;
      len -= n;
    }
  if (len > 0)
    {
      if (!equal_attributes (*ap, stream->default_attr))
        activate_non_default_attr (stream);

      do
        {
          /* Activate the attributes in *ap.  */
          out_attr_change (stream, *ap);
          /* See how many characters we can output without further attribute
             changes.  */
          for (n = 1; n < len && equal_attributes (ap[n], stream->active_attr); n++)
            ;
          if (full_write (stream->fd, cp, n) < n)
            {
              int error_code = errno;
              /* Do output to stderr only after we have switched back to the
                 default attributes.  Otherwise this output may come out with
                 the wrong text attributes.  */
              if (!equal_attributes (stream->active_attr, stream->default_attr))
                activate_default_attr (stream);
              error (EXIT_FAILURE, error_code, _("error writing to %s"),
                     stream->filename);
            }
          cp += n;
          ap += n;
          len -= n;
        }
      while (len > 0);
    }
  stream->buflen = 0;

  /* Before changing to goal_attr, we may need to enable the non-default
     attributes mode.  */
  if (!equal_attributes (goal_attr, stream->default_attr))
    activate_non_default_attr (stream);
  /* Change to goal_attr.  */
  if (!equal_attributes (goal_attr, stream->active_attr))
    out_attr_change (stream, goal_attr);
  /* When we can deactivate the non-default attributes mode, do so.  */
  if (equal_attributes (goal_attr, stream->default_attr))
    deactivate_non_default_attr (stream);
}

/* Implementation of ostream_t methods.  */

static term_color_t
term_ostream::rgb_to_color (term_ostream_t stream, int red, int green, int blue)
{
  switch (stream->colormodel)
    {
    case cm_monochrome:
      return rgb_to_color_monochrome ();
    case cm_common8:
      return rgb_to_color_common8 (red, green, blue);
    case cm_xterm8:
      return rgb_to_color_xterm8 (red, green, blue);
    case cm_xterm16:
      return rgb_to_color_xterm16 (red, green, blue);
    case cm_xterm88:
      return rgb_to_color_xterm88 (red, green, blue);
    case cm_xterm256:
      return rgb_to_color_xterm256 (red, green, blue);
    default:
      abort ();
    }
}

static void
term_ostream::write_mem (term_ostream_t stream, const void *data, size_t len)
{
  const char *cp = (const char *) data;
  while (len > 0)
    {
      /* Look for the next newline.  */
      const char *newline = (const char *) memchr (cp, '\n', len);
      size_t n = (newline != NULL ? newline - cp : len);

      /* Copy n bytes into the buffer.  */
      if (n > stream->allocated - stream->buflen)
        {
          size_t new_allocated =
            xmax (xsum (stream->buflen, n),
                  xsum (stream->allocated, stream->allocated));
          if (size_overflow_p (new_allocated))
            error (EXIT_FAILURE, 0,
                   _("%s: too much output, buffer size overflow"),
                   "term_ostream");
          stream->buffer = (char *) xrealloc (stream->buffer, new_allocated);
          stream->attrbuffer =
            (attributes_t *)
            xrealloc (stream->attrbuffer,
                      new_allocated * sizeof (attributes_t));
          stream->allocated = new_allocated;
        }
      memcpy (stream->buffer + stream->buflen, cp, n);
      {
        attributes_t attr = stream->simp_attr;
        attributes_t *ap = stream->attrbuffer + stream->buflen;
        attributes_t *ap_end = ap + n;
        for (; ap < ap_end; ap++)
          *ap = attr;
      }
      stream->buflen += n;

      if (newline != NULL)
        {
          output_buffer (stream, stream->default_attr);
          if (full_write (stream->fd, "\n", 1) < 1)
            error (EXIT_FAILURE, errno, _("error writing to %s"),
                   stream->filename);
          cp += n + 1; /* cp = newline + 1; */
          len -= n + 1;
        }
      else
        break;
    }
}

static void
term_ostream::flush (term_ostream_t stream, ostream_flush_scope_t scope)
{
  output_buffer (stream, stream->default_attr);
  if (scope == FLUSH_ALL)
    {
      /* For streams connected to a disk file:  */
      fsync (stream->fd);
      #if HAVE_TCDRAIN
      /* For streams connected to a terminal:  */
      nonintr_tcdrain (stream->fd);
      #endif
    }
}

static void
term_ostream::free (term_ostream_t stream)
{
  term_ostream_flush (stream, FLUSH_THIS_STREAM);
  /* Verify that the non-default attributes mode is turned off.  */
  if (stream->non_default_active)
    abort ();
  term_fd = -1;
  update_pgrp_status ();
  free (stream->filename);
  if (stream->set_a_foreground != NULL)
    free (stream->set_a_foreground);
  if (stream->set_foreground != NULL)
    free (stream->set_foreground);
  if (stream->set_a_background != NULL)
    free (stream->set_a_background);
  if (stream->set_background != NULL)
    free (stream->set_background);
  if (stream->orig_pair != NULL)
    free (stream->orig_pair);
  if (stream->enter_bold_mode != NULL)
    free (stream->enter_bold_mode);
  if (stream->enter_italics_mode != NULL)
    free (stream->enter_italics_mode);
  if (stream->exit_italics_mode != NULL)
    free (stream->exit_italics_mode);
  if (stream->enter_underline_mode != NULL)
    free (stream->enter_underline_mode);
  if (stream->exit_underline_mode != NULL)
    free (stream->exit_underline_mode);
  if (stream->exit_attribute_mode != NULL)
    free (stream->exit_attribute_mode);
  free (stream->buffer);
  free (stream);
}

/* Implementation of term_ostream_t methods.  */

static term_color_t
term_ostream::get_color (term_ostream_t stream)
{
  return stream->curr_attr.color;
}

static void
term_ostream::set_color (term_ostream_t stream, term_color_t color)
{
  stream->curr_attr.color = color;
  stream->simp_attr = simplify_attributes (stream, stream->curr_attr);
}

static term_color_t
term_ostream::get_bgcolor (term_ostream_t stream)
{
  return stream->curr_attr.bgcolor;
}

static void
term_ostream::set_bgcolor (term_ostream_t stream, term_color_t color)
{
  stream->curr_attr.bgcolor = color;
  stream->simp_attr = simplify_attributes (stream, stream->curr_attr);
}

static term_weight_t
term_ostream::get_weight (term_ostream_t stream)
{
  return stream->curr_attr.weight;
}

static void
term_ostream::set_weight (term_ostream_t stream, term_weight_t weight)
{
  stream->curr_attr.weight = weight;
  stream->simp_attr = simplify_attributes (stream, stream->curr_attr);
}

static term_posture_t
term_ostream::get_posture (term_ostream_t stream)
{
  return stream->curr_attr.posture;
}

static void
term_ostream::set_posture (term_ostream_t stream, term_posture_t posture)
{
  stream->curr_attr.posture = posture;
  stream->simp_attr = simplify_attributes (stream, stream->curr_attr);
}

static term_underline_t
term_ostream::get_underline (term_ostream_t stream)
{
  return stream->curr_attr.underline;
}

static void
term_ostream::set_underline (term_ostream_t stream, term_underline_t underline)
{
  stream->curr_attr.underline = underline;
  stream->simp_attr = simplify_attributes (stream, stream->curr_attr);
}

static void
term_ostream::flush_to_current_style (term_ostream_t stream)
{
  output_buffer (stream, stream->simp_attr);
}

/* Constructor.  */

static inline char *
xstrdup0 (const char *str)
{
  if (str == NULL)
    return NULL;
#if HAVE_TERMINFO
  if (str == (const char *)(-1))
    return NULL;
#endif
  return xstrdup (str);
}

term_ostream_t
term_ostream_create (int fd, const char *filename, ttyctl_t tty_control)
{
  term_ostream_t stream = XMALLOC (struct term_ostream_representation);
  const char *term;

  stream->base.vtable = &term_ostream_vtable;
  stream->fd = fd;
  stream->filename = xstrdup (filename);

  /* Defaults.  */
  stream->max_colors = -1;
  stream->no_color_video = -1;
  stream->set_a_foreground = NULL;
  stream->set_foreground = NULL;
  stream->set_a_background = NULL;
  stream->set_background = NULL;
  stream->orig_pair = NULL;
  stream->enter_bold_mode = NULL;
  stream->enter_italics_mode = NULL;
  stream->exit_italics_mode = NULL;
  stream->enter_underline_mode = NULL;
  stream->exit_underline_mode = NULL;
  stream->exit_attribute_mode = NULL;

  /* Retrieve the terminal type.  */
  term = getenv ("TERM");
  if (term != NULL && term[0] != '\0')
    {
      /* When the terminfo function are available, we prefer them over the
         termcap functions because
           1. they don't risk a buffer overflow,
           2. on OSF/1, for TERM=xterm, the tiget* functions provide access
              to the number of colors and the color escape sequences, whereas
              the tget* functions don't provide them.  */
#if HAVE_TERMINFO
      int err = 1;

      if (setupterm (term, fd, &err) || err == 1)
        {
          /* Retrieve particular values depending on the terminal type.  */
          stream->max_colors = tigetnum ("colors");
          stream->no_color_video = tigetnum ("ncv");
          stream->set_a_foreground = xstrdup0 (tigetstr ("setaf"));
          stream->set_foreground = xstrdup0 (tigetstr ("setf"));
          stream->set_a_background = xstrdup0 (tigetstr ("setab"));
          stream->set_background = xstrdup0 (tigetstr ("setb"));
          stream->orig_pair = xstrdup0 (tigetstr ("op"));
          stream->enter_bold_mode = xstrdup0 (tigetstr ("bold"));
          stream->enter_italics_mode = xstrdup0 (tigetstr ("sitm"));
          stream->exit_italics_mode = xstrdup0 (tigetstr ("ritm"));
          stream->enter_underline_mode = xstrdup0 (tigetstr ("smul"));
          stream->exit_underline_mode = xstrdup0 (tigetstr ("rmul"));
          stream->exit_attribute_mode = xstrdup0 (tigetstr ("sgr0"));
        }
#elif HAVE_TERMCAP
      struct { char buf[1024]; char canary[4]; } termcapbuf;
      int retval;

      /* Call tgetent, being defensive against buffer overflow.  */
      memcpy (termcapbuf.canary, "CnRy", 4);
      retval = tgetent (termcapbuf.buf, term);
      if (memcmp (termcapbuf.canary, "CnRy", 4) != 0)
        /* Buffer overflow!  */
        abort ();

      if (retval > 0)
        {
          struct { char buf[1024]; char canary[4]; } termentrybuf;
          char *termentryptr;

          /* Prepare for calling tgetstr, being defensive against buffer
             overflow.  ncurses' tgetstr() supports a second argument NULL,
             but NetBSD's tgetstr() doesn't.  */
          memcpy (termentrybuf.canary, "CnRz", 4);
          #define TEBP ((termentryptr = termentrybuf.buf), &termentryptr)

          /* Retrieve particular values depending on the terminal type.  */
          stream->max_colors = tgetnum ("Co");
          stream->no_color_video = tgetnum ("NC");
          stream->set_a_foreground = xstrdup0 (tgetstr ("AF", TEBP));
          stream->set_foreground = xstrdup0 (tgetstr ("Sf", TEBP));
          stream->set_a_background = xstrdup0 (tgetstr ("AB", TEBP));
          stream->set_background = xstrdup0 (tgetstr ("Sb", TEBP));
          stream->orig_pair = xstrdup0 (tgetstr ("op", TEBP));
          stream->enter_bold_mode = xstrdup0 (tgetstr ("md", TEBP));
          stream->enter_italics_mode = xstrdup0 (tgetstr ("ZH", TEBP));
          stream->exit_italics_mode = xstrdup0 (tgetstr ("ZR", TEBP));
          stream->enter_underline_mode = xstrdup0 (tgetstr ("us", TEBP));
          stream->exit_underline_mode = xstrdup0 (tgetstr ("ue", TEBP));
          stream->exit_attribute_mode = xstrdup0 (tgetstr ("me", TEBP));

# ifdef __BEOS__
          /* The BeOS termcap entry for "beterm" is broken: For "AF" and "AB"
             it contains balues in terminfo syntax but the system's tparam()
             function understands only the termcap syntax.  */
          if (stream->set_a_foreground != NULL
              && strcmp (stream->set_a_foreground, "\033[3%p1%dm") == 0)
            {
              free (stream->set_a_foreground);
              stream->set_a_foreground = xstrdup ("\033[3%dm");
            }
          if (stream->set_a_background != NULL
              && strcmp (stream->set_a_background, "\033[4%p1%dm") == 0)
            {
              free (stream->set_a_background);
              stream->set_a_background = xstrdup ("\033[4%dm");
            }
# endif

          /* The termcap entry for cygwin is broken: It has no "ncv" value,
             but bold and underline are actually rendered through colors.  */
          if (strcmp (term, "cygwin") == 0)
            stream->no_color_video |= 2 | 32;

          /* Done with tgetstr.  Detect possible buffer overflow.  */
          #undef TEBP
          if (memcmp (termentrybuf.canary, "CnRz", 4) != 0)
            /* Buffer overflow!  */
            abort ();
        }
#else
    /* Fallback code for platforms with neither the terminfo nor the termcap
       functions, such as mingw.
       Assume the ANSI escape sequences.  Extracted through
       "TERM=ansi infocmp", replacing \E with \033.  */
      stream->max_colors = 8;
      stream->no_color_video = 3;
      stream->set_a_foreground = xstrdup ("\033[3%p1%dm");
      stream->set_a_background = xstrdup ("\033[4%p1%dm");
      stream->orig_pair = xstrdup ("\033[39;49m");
      stream->enter_bold_mode = xstrdup ("\033[1m");
      stream->enter_underline_mode = xstrdup ("\033[4m");
      stream->exit_underline_mode = xstrdup ("\033[m");
      stream->exit_attribute_mode = xstrdup ("\033[0;10m");
#endif

      /* AIX 4.3.2, IRIX 6.5, HP-UX 11, Solaris 7..10 all lack the
         description of color capabilities of "xterm" and "xterms"
         in their terminfo database.  But it is important to have
         color in xterm.  So we provide the color capabilities here.  */
      if (stream->max_colors <= 1
          && (strcmp (term, "xterm") == 0 || strcmp (term, "xterms") == 0))
        {
          stream->max_colors = 8;
          stream->set_a_foreground = xstrdup ("\033[3%p1%dm");
          stream->set_a_background = xstrdup ("\033[4%p1%dm");
          stream->orig_pair = xstrdup ("\033[39;49m");
        }
    }

  /* Infer the capabilities.  */
  stream->supports_foreground =
    (stream->max_colors >= 8
     && (stream->set_a_foreground != NULL || stream->set_foreground != NULL)
     && stream->orig_pair != NULL);
  stream->supports_background =
    (stream->max_colors >= 8
     && (stream->set_a_background != NULL || stream->set_background != NULL)
     && stream->orig_pair != NULL);
  stream->colormodel =
    (stream->supports_foreground || stream->supports_background
     ? (term != NULL
        && (/* Recognize xterm-16color, xterm-88color, xterm-256color.  */
            (strlen (term) >= 5 && memcmp (term, "xterm", 5) == 0)
            || /* Recognize rxvt-16color.  */
               (strlen (term) >= 4 && memcmp (term, "rxvt", 4) == 0)
            || /* Recognize konsole-16color.  */
               (strlen (term) >= 7 && memcmp (term, "konsole", 7) == 0))
        ? (stream->max_colors == 256 ? cm_xterm256 :
           stream->max_colors == 88 ? cm_xterm88 :
           stream->max_colors == 16 ? cm_xterm16 :
           cm_xterm8)
        : cm_common8)
     : cm_monochrome);
  stream->supports_weight =
    (stream->enter_bold_mode != NULL && stream->exit_attribute_mode != NULL);
  stream->supports_posture =
    (stream->enter_italics_mode != NULL
     && (stream->exit_italics_mode != NULL
         || stream->exit_attribute_mode != NULL));
  stream->supports_underline =
    (stream->enter_underline_mode != NULL
     && (stream->exit_underline_mode != NULL
         || stream->exit_attribute_mode != NULL));

  /* Infer the restore strings.  */
  stream->restore_colors =
    (stream->supports_foreground || stream->supports_background
     ? stream->orig_pair
     : NULL);
  stream->restore_weight =
    (stream->supports_weight ? stream->exit_attribute_mode : NULL);
  stream->restore_posture =
    (stream->supports_posture
     ? (stream->exit_italics_mode != NULL
        ? stream->exit_italics_mode
        : stream->exit_attribute_mode)
     : NULL);
  stream->restore_underline =
    (stream->supports_underline
     ? (stream->exit_underline_mode != NULL
        ? stream->exit_underline_mode
        : stream->exit_attribute_mode)
     : NULL);

  /* Prepare tty control.  */
  if (tty_control == TTYCTL_AUTO)
    tty_control = TTYCTL_FULL;
  stream->tty_control = tty_control;
  if (stream->tty_control != TTYCTL_NONE)
    init_relevant_signal_set ();
  #if HAVE_TCGETATTR
  if (stream->tty_control == TTYCTL_FULL)
    {
      struct stat statbuf1;
      struct stat statbuf2;
      if (fd == STDERR_FILENO
          || (fstat (fd, &statbuf1) >= 0
              && fstat (STDERR_FILENO, &statbuf2) >= 0
              && SAME_INODE (statbuf1, statbuf2)))
        stream->same_as_stderr = true;
      else
        stream->same_as_stderr = false;
    }
  else
    /* This value is actually not used.  */
    stream->same_as_stderr = false;
  #endif

  /* Start keeping track of the process group status.  */
  term_fd = fd;
  #if defined SIGCONT
  ensure_continuing_signal_handler ();
  #endif
  update_pgrp_status ();

  /* Initialize the buffer.  */
  stream->allocated = 120;
  stream->buffer = XNMALLOC (stream->allocated, char);
  stream->attrbuffer = XNMALLOC (stream->allocated, attributes_t);
  stream->buflen = 0;

  /* Initialize the current attributes.  */
  {
    attributes_t assumed_default;
    attributes_t simplified_default;

    assumed_default.color = COLOR_DEFAULT;
    assumed_default.bgcolor = COLOR_DEFAULT;
    assumed_default.weight = WEIGHT_DEFAULT;
    assumed_default.posture = POSTURE_DEFAULT;
    assumed_default.underline = UNDERLINE_DEFAULT;

    simplified_default = simplify_attributes (stream, assumed_default);

    stream->default_attr = simplified_default;
    stream->active_attr = simplified_default;
    stream->non_default_active = false;
    stream->curr_attr = assumed_default;
    stream->simp_attr = simplified_default;
  }

  /* Register an exit handler.  */
  {
    static bool registered = false;
    if (!registered)
      {
        atexit (restore);
        registered = true;
      }
  }

  return stream;
}
