#!@GUILE@ -s
!#
;;; Example for use of GNU gettext.
;;; Copyright (C) 2004-2005 Free Software Foundation, Inc.
;;; This file is in the public domain.

;;; Source code of the GNU guile program.

(use-modules (ice-9 format))

(textdomain "hello-guile")
(bindtextdomaindir "hello-guile" "@localedir@")
(define _ gettext)

(display (_ "Hello, world!"))
(newline)
(format #t (_ "This program is running as process number ~D.") (getpid))
(newline)
