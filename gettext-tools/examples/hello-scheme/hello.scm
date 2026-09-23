#! /bin/sh
:; exec @SCM@ -l"$0" "$@"

;;; Example for use of GNU gettext with GNU slib.
;;; This file is in the public domain.

;;; Source code of the GNU scm program.

(require 'i18n)
(require 'format)

(setlocale LC_ALL "")
(textdomain "hello-scheme")
(bindtextdomain "hello-scheme" "@localedir@")
(define _ gettext)

(display (_ "Hello, world!"))
(newline)
(format #t (_ "This program is running as process number ~D.")
           ; In GNU scm: (getpid)
           ; In Chez Scheme: (get-process-id)
           (getpid))
(newline)
