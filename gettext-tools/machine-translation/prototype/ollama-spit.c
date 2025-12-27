/*
 * Copyright (C) 2025 Free Software Foundation, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Written by Bruno Haible <bruno@clisp.org>, 2025.
 */

/*
 * This program passes an input to an ollama instance and prints the response.
 */

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* We use JSON-C.
   It is multithread-safe, as long as we don't use any of the json_global_*
   API functions, nor the json_util_get_last_err function.
   Documentation:
   <http://json-c.github.io/json-c/json-c-0.18/doc/html/index.html#using>  */
#include <json.h>

/* We use libcurl.
   Documentation:
   <https://curl.se/libcurl/c/>  */
#include <curl/curl.h>


static void
xalloc_die ()
{
  fprintf (stderr, "spit: out of memory\n");
  exit (EXIT_FAILURE);
}

static void
curl_die ()
{
  fprintf (stderr, "spit: curl error\n");
  exit (EXIT_FAILURE);
}

static void
process_response_line (const char *line)
{
  /* Note: As of json-c version 0.15, the jerrno value is unreliable.
     See <https://github.com/json-c/json-c/issues/853>
     and <https://github.com/json-c/json-c/issues/854>.
     json-c version 0.17 introduces json_tokener_error_memory, but its value
     changes in version 0.18.  */
  struct json_object *j;
#if JSON_C_MAJOR_VERSION > 0 || (JSON_C_MAJOR_VERSION == 0 && JSON_C_MINOR_VERSION >= 18)
  enum json_tokener_error jerrno = json_tokener_error_memory;
  j = json_tokener_parse_verbose (line, &jerrno);
  if (j == NULL && jerrno == json_tokener_error_memory)
    xalloc_die ();
#else
  j = json_tokener_parse (line);
#endif
  /* Ignore an empty line.  */
  if (j != NULL)
    {
      /* We expect a JSON object.  */
      if (json_object_is_type (j, json_type_object))
        {
          /* Output its "response" property.  */
          const char *prop =
            json_object_get_string(json_object_object_get (j, "response"));
          if (prop != NULL)
            {
              fputs (prop, stdout);
              fflush (stdout);
            }
        }
    }
}

/* A libcurl header callback that determines, during curl_easy_perform,
   whether the HTTP request returned an error.  */
static size_t
my_header_callback (char *buffer, size_t one, size_t n, void *userdata)
{
  bool *is_error_p = (bool *) userdata;
#if DEBUG
  fprintf (stderr, "in my_header_callback: buffer = %.*s\n", (int) n, buffer);
#endif
  if (n >= 5 && memcmp (buffer, "HTTP/", 5) == 0)
    {
      /* buffer contains a line of the form "HTTP/1.1 code description".
         Extract the code.  */
      char old = buffer[n - 1];
      buffer[n - 1] = '\0';
      int code;
      if (sscanf (buffer, "%*s %d\n", &code) == 1)
        {
          if (code >= 400)
            *is_error_p = true;
        }
      buffer[n - 1] = old;
    }
  return n;
}

struct my_write_locals
{
  bool is_error;
  char *body;
  size_t body_allocated;
  size_t body_start;
  size_t body_end;
};

/* Makes room for n more bytes in l->body.  */
static void
my_write_grow (struct my_write_locals *l, size_t n)
{
  if ((l->body_end - l->body_start) + n > l->body_allocated)
    {
      /* Grow the buffer.  */
      size_t new_allocated = (l->body_end - l->body_start) + n;
      if (new_allocated < 2 * l->body_allocated)
        new_allocated = 2 * l->body_allocated;
      if (new_allocated < 1024)
        new_allocated = 1024;
      char *new_body;
      if (l->body_start == 0 && l->body_end > 0)
        {
          new_body = (char *) realloc (l->body, new_allocated);
          if (new_body == NULL)
            xalloc_die ();
        }
      else
        {
          new_body = (char *) malloc (new_allocated);
          if (new_body == NULL)
            xalloc_die ();
          memcpy (new_body, l->body + l->body_start, l->body_end - l->body_start);
          free (l->body);
          l->body_end = l->body_end - l->body_start;
          l->body_start = 0;
        }
      l->body = new_body;
      l->body_allocated = new_allocated;
    }
  else
    {
      /* We can keep the buffer, but may need to move the contents to the
         front.  */
      if (l->body_end + n > l->body_allocated)
        {
          memmove (l->body, l->body + l->body_start, l->body_end - l->body_start);
          l->body_end = l->body_end - l->body_start;
          l->body_start = 0;
        }
    }
  /* Here l->body_end + n <= l->body_allocated.  */
}

/* A libcurl write callback that processes a piece of response body,
   depending on whether the HTTP request returned an error.  */
static size_t
my_write_callback (char *buffer, size_t one, size_t n, void *userdata)
{
  struct my_write_locals *l = (struct my_write_locals *) userdata;
#if DEBUG
  fprintf (stderr, "in my_write_callback: buffer = %.*s\n", (int) n, buffer);
#endif

  /* Append the buffer's contents to the body.  */
  my_write_grow (l, n);
  memcpy (l->body + l->body_end, buffer, n);
  l->body_end += n;

  if (!l->is_error)
    {
      /* Process entire lines that are in the buffer.  */
      char *newline = (char *) memchr (l->body + l->body_end - n, '\n', n);
      while (newline != NULL)
        {
          /* We have an entire line.  */
          *newline = '\0';
          process_response_line (l->body + l->body_start);
          l->body_start = (newline + 1) - l->body;

          newline = (char *) memchr (l->body + l->body_start, '\n',
                                     l->body_end - l->body_start);
        }
    }

  return n;
}

static void
usage ()
{
  printf ("%s", "Usage: spit [OPTION...]\n\
\n\
Passes standard input to an ollama instance and prints the response.\n\
\n\
Options:\n\
      --url      Specifies the ollama server's URL.\n\
      --model    Specifies the model to use.\n\
\n\
Informative output:\n\
\n\
      --help     Show this help text.\n");
}

int
main (int argc, char *argv[])
{
  /* Command-line option processing.  */
  const char *url = "http://localhost:11434";
  const char *model = NULL;
  bool do_help = false;
  static const struct option long_options[] =
  {
    { "url",   required_argument, NULL, CHAR_MAX + 2 },
    { "model", required_argument, NULL, CHAR_MAX + 3 },
    { "help",  no_argument,       NULL, CHAR_MAX + 1 },
    { NULL, 0, NULL, 0 }
  };
  {
    int optc;
    while ((optc = getopt_long (argc, argv, "", long_options, NULL)) != EOF)
      switch (optc)
      {
      case '\0':          /* Long option.  */
        break;
      case CHAR_MAX + 1: /* --help */
        do_help = true;
        break;
      case CHAR_MAX + 2: /* --url */
        url = optarg;
        break;
      case CHAR_MAX + 3: /* --model */
        model = optarg;
        break;
      default:
        fprintf (stderr, "Try 'spit --help' for more information.\n");
        exit (EXIT_FAILURE);
      }
  }
  if (do_help)
    {
      usage ();
      exit (EXIT_SUCCESS);
    }
  if (argc > optind)
    {
      fprintf (stderr, "spit: too many arguments\n");
      fprintf (stderr, "Try 'spit --help' for more information.\n");
      exit (EXIT_FAILURE);
    }
  if (model == NULL)
    {
      fprintf (stderr, "spit: missing --model option\n");
      exit (EXIT_FAILURE);
    }

  /* Sanitize URL.  */
  if (!(strlen (url) > 0 && url[strlen (url) - 1] == '/'))
    {
      char *new_url = malloc (strlen (url) + 1 + 1);
      if (new_url == NULL)
        xalloc_die ();
      sprintf (new_url, "%s/", url);
      url = new_url;
    }

  /* Read the contents of standard input.  */
  char *input = NULL;
  size_t input_allocated = 0;
  size_t input_length = 0;
  for (;;)
    {
      int c = fgetc (stdin);
      if (c == EOF)
        break;

      if (input_length >= input_allocated)
        {
          size_t new_allocated = 2 * input_allocated + 1;
          char *new_input = (char *) realloc (input, new_allocated);
          if (new_input == NULL)
            xalloc_die ();
          input = new_input;
          input_allocated = new_allocated;
        }
      input[input_length++] = c;
    }

  /* Documentation of the ollama API:
     <https://docs.ollama.com/api/generate>  */

  /* Compose the payload.  */
  struct json_object *payload = json_object_new_object ();
  if (payload == NULL)
    xalloc_die ();
  {
    struct json_object *value = json_object_new_string (model);
    if (value == NULL)
      xalloc_die ();
    if (json_object_object_add (payload, "model", value))
      xalloc_die ();
  }
  {
    struct json_object *value = json_object_new_string (input);
    if (value == NULL)
      xalloc_die ();
    if (json_object_object_add (payload, "prompt", value))
      xalloc_die ();
  }
  const char *payload_as_string =
    json_object_to_json_string_ext (payload, JSON_C_TO_STRING_PLAIN
                                             | JSON_C_TO_STRING_NOSLASHESCAPE);
  if (payload_as_string == NULL)
    xalloc_die ();

  /* Make the request to the ollama server.  */
  if (curl_global_init (CURL_GLOBAL_DEFAULT))
    curl_die ();
  CURL *curl = curl_easy_init ();
  if (!curl)
    curl_die ();
  {
    char *target_url = malloc (strlen (url) + 12 + 1);
    if (target_url == NULL)
      xalloc_die ();
    sprintf (target_url, "%sapi/generate", url);
    /* Documentation: <https://curl.se/libcurl/c/CURLOPT_URL.html>  */
    curl_easy_setopt (curl, CURLOPT_URL, target_url);
  }

  /* Documentation: <https://curl.se/libcurl/c/CURLOPT_POST.html>  */
  curl_easy_setopt (curl, CURLOPT_POST, 1L);

  {
    struct curl_slist *headers = NULL;
    /* Override the Content-Type header set by CURLOPT_POST.  */
    headers = curl_slist_append(headers, "Content-Type: " "application/json");
    /* Documentation: <https://curl.se/libcurl/c/CURLOPT_HTTPHEADER.html>  */
    curl_easy_setopt (curl, CURLOPT_HTTPHEADER, headers);
  }

  /* Set the payload.
     Documentation: <https://curl.se/libcurl/c/CURLOPT_POSTFIELDS.html>  */
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload_as_string);

  /* Documentation: <https://curl.se/libcurl/c/CURLOPT_NOPROGRESS.html>  */
  curl_easy_setopt (curl, CURLOPT_NOPROGRESS, 1L);

#if DEBUG > 1
  /* For debugging:  */
  /* Documentation: <https://curl.se/libcurl/c/CURLOPT_VERBOSE.html>  */
  curl_easy_setopt (curl, CURLOPT_VERBOSE, 1L);
#endif

#if 0
  /* Not reliable, see <https://curl.se/libcurl/c/CURLOPT_FAILONERROR.html>.  */
  curl_easy_setopt (curl, CURLOPT_FAILONERROR, 1L);
#endif

  struct my_write_locals locals;
  locals.is_error = false;
  locals.body = NULL;
  locals.body_allocated = 0;
  locals.body_start = 0;
  locals.body_end = 0;

  /* Documentation:
     <https://curl.se/libcurl/c/CURLOPT_HEADERFUNCTION.html>
     <https://curl.se/libcurl/c/CURLOPT_HEADERDATA.html>  */
  curl_easy_setopt (curl, CURLOPT_HEADERFUNCTION, my_header_callback);
  curl_easy_setopt (curl, CURLOPT_HEADERDATA, &locals.is_error);

  /* Documentation:
     <https://curl.se/libcurl/c/CURLOPT_WRITEFUNCTION.html>
     <https://curl.se/libcurl/c/CURLOPT_WRITEDATA.html>  */
  curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, my_write_callback);
  curl_easy_setopt (curl, CURLOPT_WRITEDATA, &locals);

  /* Documentation: <https://curl.se/libcurl/c/curl_easy_perform.html>  */
  CURLcode ret = curl_easy_perform (curl);
  if (ret != CURLE_OK)
    {
      fprintf (stderr, "spit: curl error %u: %s\n", ret,
               /* Documentation: <https://curl.se/libcurl/c/curl_easy_strerror.html>  */
               curl_easy_strerror (ret));
      exit (EXIT_FAILURE);
    }
  /* Documentation: <https://curl.se/libcurl/c/CURLINFO_RESPONSE_CODE.html>  */
  long status_code;
  curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &status_code);
  if (status_code != 200)
    fprintf (stderr, "Status: %ld\n", status_code);
  if (locals.is_error != (status_code >= 400))
    /* The my_header_callback did not work right.  */
    abort ();
  if (status_code >= 400)
    {
      fprintf (stderr, "Body: ");
      fwrite (locals.body + locals.body_start,
              1, locals.body_end - locals.body_start,
              stderr);
      fprintf (stderr, "\n");
      exit (1);
    }
  /* Most lines have already been processed through my_write_callback.
     Now process the last line (without terminating newline).  */
  if (locals.body_end > locals.body_start)
    {
      my_write_grow (&locals, 1);
      *(locals.body + locals.body_end) = '\0';
      process_response_line (locals.body + locals.body_start);
    }

  return 0;
}

/*
 * Local Variables:
 * compile-command: "gcc -Wall -I/usr/include/json-c -O2 -o ollama-spit ollama-spit.c -lcurl -ljson-c"
 * run-command: "echo 'Translate into German: "Welcome to the GNU project!"' | ./ollama-spit --model=ministral-3:14b"
 * End:
 */
