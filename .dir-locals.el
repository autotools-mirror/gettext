;;; This file contains customizations for Emacs, that apply when editing
;;; files in this directory and its subdirectories.
;;; See https://www.gnu.org/software/emacs/manual/html_node/emacs/Directory-Variables.html

;; Force indentation without tabs.
((c-mode . ((indent-tabs-mode . nil)))
 (sh-mode . ((indent-tabs-mode . nil) (sh-basic-offset . 2))))
