# Additional editing of Makefiles
/@GMSGFMT@/ s,\$GMSGFMT,msgfmt,
/@MSGFMT@/ s,\$MSGFMT,msgfmt,
/@XGETTEXT@/ s,\$XGETTEXT,xgettext,
/ac_given_INSTALL=/,/^CEOF/ {
  /^CEOF$/ i\
# DJGPP specific Makefile changes.\
  /^aliaspath *	*=/s,:,";",g\
  /^lispdir *	*=/ c\\\\\
lispdir = \\$(prefix)/gnu/emacs/site-lisp\
  /TEXINPUTS=/s,:,";",g\
  /PATH=/s,:,";",g\
  s,\\.new\\.,_new.,g\
  s,\\.old\\.,_old.,g\
  s,\\.tab\\.c,_tab.c,g\
  s,\\.tab\\.h,_tab.h,g\
  s,config\\.h\\.in,config.h-in,g\
  s,Makefile\\.in\\.in,Makefile.in-in,g\
  s,gettext-1,gettext.1,g\
  s,gettext-2,gettext.2,g\
  s,msgcmp-1,msgcmp.1,g\
  s,msgcmp-2,msgcmp.2,g\
  s,msgfmt-1,msgfmt.1,g\
  s,msgfmt-2,msgfmt.2,g\
  s,msgfmt-3,msgfmt.3,g\
  s,msgfmt-4,msgfmt.4,g\
  s,msgmerge-1,msgmerge.1,g\
  s,msgmerge-2,msgmerge.2,g\
  s,msgmerge-3,msgmerge.3,g\
  s,msgmerge-4,msgmerge.4,g\
  s,msgmerge-5,msgmerge.5,g\
  s,msgunfmt-1,msgunfmt.1,g\
  s,xgettext-1,xgettext.1,g\
  s,xgettext-2,xgettext.2,g\
  s,xgettext-3,xgettext.3,g\
  s,xgettext-4,xgettext.4,g\
  s,xgettext-5,xgettext.5,g\
  s,xgettext-6,xgettext.6,g\
  s,xgettext-7,xgettext.7,g\
  s,xgettext-8,xgettext.8,g\
  s,xgettext-9,xgettext.9,g\
  s,xg-test1.ok.po,xg-test1.ok-po,g\
  /^TESTS[ 	]*=/ s,plural-\\([1-9]\\+\\),plural.\\1,g\
  /^install-info-am:/,/^$/ {\
    /@list=/ s,\\\$(INFO_DEPS),& gettext.i,\
    s,file-\\[0-9\\]\\[0-9\\],& \\$\\$file[0-9] \\$\\$file[0-9][0-9],\
  }\
  /^iso-apdx\\.texi *	*:.*$/ {\
    s,iso-apdx,\\$(srcdir)/&,g\
    s,ISO_639,\\$(srcdir)/&,\
  }\
  /^# Some rules for yacc handling\\./,$ {\
    /\\\$(YACC)/ a\\\\\
	-@test -f y.tab.c && mv -f y.tab.c y_tab.c\\\\\
	-@test -f y.tab.h && mv -f y.tab.h y_tab.h\
  }\
  /^POTFILES:/,/^$/ s,\\\$@-t,t-\\$@,g\
  s,basename\\.o,,g\
  s,po-gram-gen2\\.h,po-gram_gen2.h,g\
  /^Makefile[ 	]*:/,/^$/ {\
    /CONFIG_FILES=/ s,\\\$(subdir)/\\\$@\\.in,&:\\$(subdir)/\\$@.in-in,\
  }
}

# Makefile.in.in is renamed to Makefile.in-in...
/^CONFIG_FILES=/,/^EOF/ {
  s|po/Makefile\.in|&:po/Makefile.in-in|
}

# ...and config.h.in into config.h-in
/^ *CONFIG_HEADERS=/,/^EOF/ {
  s|config\.h|&:config.h-in|
}

# The same as above but this time
# for configure scripts created with Autoconf 2.14a.
/^config_files="\\\\/,/^$/ {
  s|po/Makefile\.in|&:po/Makefile.in-in|
}
/^config_headers="\\\\/,/^$/ {
  s|config\.h|&:config.h-in|
}
/# Handling of arguments./,/^$/ {
  s|po/Makefile\.in|&:po/Makefile.in-in|2
  s|config\.h|&:config.h-in|2
}

# Replace (command) > /dev/null with `command > /dev/null`, since
# parenthesized commands always return zero status in the ported Bash,
# even if the named command doesn't exist
/if ([^|;]*null/{
  s,(,`,
  s,),,
  s,;  *then,`; then,
}

# DOS-style absolute file names should be supported as well
/\*) srcdir=/s,/\*,[\\\\/]* | [A-z]:[\\\\/]*,
/\$]\*) INSTALL=/s,\[/\$\]\*,[\\\\/$]* | [A-z]:[\\\\/]*,
/\$]\*) ac_rel_source=/s,\[/\$\]\*,[\\\\/$]* | [A-z]:[\\\\/]*,

# Switch the order of the two Sed commands, since DOS path names
# could include a colon
/ac_file_inputs=/s,\( -e "s%\^%\$ac_given_srcdir/%"\)\( -e "s%:% $ac_given_srcdir/%g"\),\2\1,

# Prevent the spliting of conftest.subs.
# The sed script: conftest.subs is split into 48 or 90 lines long files.
# This will produce sed scripts called conftest.s1, conftest.s2, etc.
# that will not work if conftest.subs contains a multi line sed command
# at line #90. In this case the first part of the sed command will be the
# last line of conftest.s1 and the rest of the command will be the first lines
# of conftest.s2. So both script will not work properly.
# This matches the configure script produced by Autoconf 2.12
/ac_max_sed_cmds=[0-9]/ s,=.*$,=`sed -n "$=" conftest.subs`,
# This matches the configure script produced by Autoconf 2.14a
/ac_max_sed_lines=[0-9]/ s,=.*$,=`sed -n "$=" $ac_cs_root.subs `,

# The following two items are changes needed for configuring
# and compiling across partitions.
# 1) The given srcdir value is always translated from the
#    "x:" syntax into "/dev/x" syntax while we run configure.
/^[ 	]*-srcdir=\*.*$/ a\
    ac_optarg=`echo "$ac_optarg" | sed "s,^\\([A-Za-z]\\):,/dev/\\1,"`
/set X `ls -Lt \$srcdir/ i\
   if `echo $srcdir | grep "^/dev/" - > /dev/null`; then\
     srcdir=`echo "$srcdir" | sed -e "s%^/dev/%%" -e "s%/%:/%"`\
   fi

#  2) We need links across partitions, so we will use "cp -pf" instead of "ln".
/# Make a symlink if possible; otherwise try a hard link./,/EOF/ {
  s,;.*then, 2>/dev/null || cp -pf \$srcdir/\$ac_source \$ac_dest&,
}
