@echo off
echo Configuring GNU Gettext for DJGPP v2.x...
Rem The SmallEnv tests protect against fixed and too small size
Rem of the environment in stock DOS shell.

Rem Find out if NLS is wanted or not,
Rem if static or shared libraries are wanted
Rem and where the sources are.
Rem We always default to NLS support,
Rem static libraries, and to in place configuration.
set ARGS=
set NLS=enabled
if not "%NLS%" == "enabled" goto SmallEnv
set STATIC_LIBS=enabled
if not "%STATIC_LIBS%" == "enabled" goto SmallEnv
set XSRC=.
if not "%XSRC%" == "." goto SmallEnv

Rem Loop over all arguments.
Rem Special arguments are: NLS, XSRC and STATIC_LIBS.
Rem All other arguments are stored unchanged into ARGS.
:ArgLoop
set ARGSFLAG=1
if not "%ARGSFLAG%" == "1" goto SmallEnv
if not "%1" == "NLS" if not "%1" == "nls" if not "%1" == "NO-NLS" if not "%1" == "no-NLS" if not "%1" == "no-nls" goto StaticLibsOpt
if "%1" == "no-nls" set NLS=disabled
if "%1" == "no-NLS" set NLS=disabled
if "%1" == "NO-NLS" set NLS=disabled
if not "%NLS%" == "disabled" goto SmallEnv
set ARGSFLAG=0
if not "%ARGSFLAG%" == "0" goto SmallEnv
shift
:StaticLibsOpt
set ARGSFLAG=1
if not "%ARGSFLAG%" == "1" goto SmallEnv
if not "%1" == "static" if not "%1" == "STATIC" if not "%1" == "shared" if not "%1" == "SHARED" goto SrcDirOpt
if "%1" == "shared" set STATIC_LIBS=disabled
if "%1" == "SHARED" set STATIC_LIBS=disabled
if not "%STATIC_LIBS%" == "disabled" goto SmallEnv
set ARGSFLAG=0
if not "%ARGSFLAG%" == "0" goto SmallEnv
shift
:SrcDirOpt
set ARGSFLAG=1
if not "%ARGSFLAG%" == "1" goto SmallEnv
echo %1 | grep -q "/"
if errorlevel 1 goto NextArg
set XSRC=%1
if not "%XSRC%" == "%1" goto SmallEnv
set ARGSFLAG=0
if not "%ARGSFLAG%" == "0" goto SmallEnv
:NextArg
if "%ARGSFLAG%" == "1" set _ARGS=%ARGS% %1
if "%ARGSFLAG%" == "1" if not "%_ARGS%" == "%ARGS% %1" goto SmallEnv
set ARGS=%_ARGS%
set _ARGS=
shift
if not "%1" == "" goto ArgLoop
set ARGSFLAG=

if "%STATIC_LIBS%" == "enabled" set _ARGS=--enable-static --disable-shared %ARGS%
if not "%_ARGS%" == "--enable-static --disable-shared %ARGS%" goto SmallEnv
if "%STATIC_LIBS%" == "disabled" set _ARGS=--disable-static --enable-shared %ARGS%
if "%STATIC_LIBS%" == "disabled" if not "%_ARGS%" == "--disable-static --enable-shared %ARGS%" goto SmallEnv
set ARGS=%_ARGS%
set _ARGS=

if "%XSRC%" == "." goto InPlace

:NotInPlace
redir -e /dev/null update %XSRC%/configure.orig ./configure
test -f ./configure
if errorlevel 1 update %XSRC%/configure ./configure

:InPlace
Rem Update configuration files
echo Updating configuration scripts...
test -f ./configure.orig
if errorlevel 1 update configure configure.orig
sed -f %XSRC%/djgpp/config.sed configure.orig > configure
if errorlevel 1 goto SedError

Rem Make sure they have a config.site file
set CONFIG_SITE=%XSRC%/djgpp/config.site
if not "%CONFIG_SITE%" == "%XSRC%/djgpp/config.site" goto SmallEnv

Rem Make sure crucial file names are not munged by unpacking
test -f %XSRC%/config.h.in
if not errorlevel 1 mv -f %XSRC%/config.h.in %XSRC%/config.h-in
test -f %XSRC%/po/Makefile.in.in
if not errorlevel 1 mv -f %XSRC%/po/Makefile.in.in %XSRC%/po/Makefile.in-in

Rem While building the binaries in src/ subdir an intermediary
Rem file called po-gram-gen2.h is generated from po-gram-gen.h.
Rem Both resolve to the same 8.3 filename. po-gram-gen2.h will
Rem be renamed to po-gram_gen2.h and src/po-lex.c must be fixed
Rem accordingly.
test -f %XSRC%/src/po-lex.orig
if errorlevel 1 update %XSRC%/src/po-lex.c %XSRC%/src/po-lex.orig
sed "s/po-gram-gen2.h/po-gram_gen2.h/g" %XSRC%/src/po-lex.orig > po-lex.tmp
if errorlevel 1 goto SedError
mv ./po-lex.tmp %XSRC%/src/po-lex.c

Rem This is required because DOS/Windows are case-insensitive
Rem to file names, and "make install" will do nothing if Make
Rem finds a file called `install'.
if exist INSTALL mv -f INSTALL INSTALL.txt

Rem install-sh is required by the configure script but clashes with the
Rem various Makefile install-foo targets, so we MUST have it before the
Rem script runs and rename it afterwards
test -f %XSRC%/install-sh
if not errorlevel 1 goto NoRen0
test -f %XSRC%/install-sh.sh
if not errorlevel 1 mv -f %XSRC%/install-sh.sh %XSRC%/install-sh
:NoRen0

Rem Set HOSTNAME so it shows in config.status
if not "%HOSTNAME%" == "" goto hostdone
if "%windir%" == "" goto msdos
set OS=MS-Windows
if not "%OS%" == "MS-Windows" goto SmallEnv
goto haveos
:msdos
set OS=MS-DOS
if not "%OS%" == "MS-DOS" goto SmallEnv
:haveos
if not "%USERNAME%" == "" goto haveuname
if not "%USER%" == "" goto haveuser
echo No USERNAME and no USER found in the environment, using default values
set HOSTNAME=Unknown PC
if not "%HOSTNAME%" == "Unknown PC" goto SmallEnv
goto userdone
:haveuser
set HOSTNAME=%USER%'s PC
if not "%HOSTNAME%" == "%USER%'s PC" goto SmallEnv
goto userdone
:haveuname
set HOSTNAME=%USERNAME%'s PC
if not "%HOSTNAME%" == "%USERNAME%'s PC" goto SmallEnv
:userdone
set _HOSTNAME=%HOSTNAME%, %OS%
if not "%_HOSTNAME%" == "%HOSTNAME%, %OS%" goto SmallEnv
set HOSTNAME=%_HOSTNAME%
:hostdone
set _HOSTNAME=
set OS=

if "%NLS%" == "disabled" goto WithoutNLS
echo Running the ./configure script...
sh ./configure --srcdir=%XSRC% --enable-nls --with-included-gettext %ARGS%
if errorlevel 1 goto CfgError
echo Done.
goto ScriptEditing

:WithoutNLS
echo Running the ./configure script...
sh ./configure --srcdir=%XSRC% --disable-nls %ARGS%
if errorlevel 1 goto CfgError
echo Done.

:ScriptEditing
Rem DJGPP specific editing of test scripts.
test -f %XSRC%/tests/stamp-test
if not errorlevel 1 goto End
if "%XSRC%" == "." goto NoDirChange
cd | sed "s|:.*$|:|" > cd_BuildDir.bat
cd | sed "s|^.:|cd |" >> cd_BuildDir.bat
mv -f cd_BuildDir.bat %XSRC%/cd_BuildDir.bat
echo %XSRC% | sed -e "s|^/dev/||" -e "s|/|:|" -e "s|:.*$|:|g" > cd_SrcDir.bat
echo %XSRC% | sed -e "s|^/dev/||" -e "s|/|:/|" -e "s|^.*:|cd |" -e "s|^\.\.|cd &|" -e "s|/|\\|g" >> cd_SrcDir.bat
call cd_SrcDir.bat
call djgpp\edtests.bat
call cd_BuildDir.bat
rm -f cd_SrcDir.bat cd_BuildDir.bat %XSRC%/cd_BuildDir.bat
goto End
:NoDirChange
call djgpp\edtests.bat
goto End

:SedError
echo ./configure script editing failed!
goto End

:CfgError
echo ./configure script exited abnormally!
goto End

:SmallEnv
echo Your environment size is too small.  Enlarge it and run me again.
echo Configuration NOT done!

:End
test -f %XSRC%/install-sh.sh
if not errorlevel 1 goto NoRen1
test -f %XSRC%/install-sh
if not errorlevel 1 mv -f %XSRC%/install-sh %XSRC%/install-sh.sh
:NoRen1
set ARGS=
set CONFIG_SITE=
set HOSTNAME=
set NLS=
set XSRC=
