@echo off
echo Configuring GNU Gettext for DJGPP v2.x...
Rem The SmallEnv tests protect against fixed and too small size
Rem of the environment in stock DOS shell.

Rem Find out if NLS is wanted or not
Rem and where the sources are.
Rem We always default to NLS support
Rem and to in place configuration.
set NLS=enabled
if not "%NLS%" == "enabled" goto SmallEnv
set XSRC=.
if not "%XSRC%" == "." goto SmallEnv

Rem This checks the case:
Rem   %1 contains the NLS option.
Rem   %2 contains the src path option.
if "%1" == "" goto InPlace
if "%1" == "NLS" goto SrcDir2
if not "%1" == "no-NLS" goto SrcDir1
set NLS=disabled
if not "%NLS%" == "disabled" goto SmallEnv
:SrcDir2
Rem Find out where the sources are
if "%2" == "" goto InPlace
set XSRC=%2
if not "%XSRC%" == "%2" goto SmallEnv
goto NotInPlace

Rem This checks the case:
Rem   %1 contains the src path option.
Rem   %2 contains the NLS option.
:SrcDir1
Rem Find out where the sources are
if "%1" == "" goto InPlace
set XSRC=%1
if not "%XSRC%" == "%1" goto SmallEnv
if "%2" == "" goto NotInPlace
if "%2" == "NLS" goto NotInPlace
if not "%2" == "no-NLS" goto NotInPlace
set NLS=disabled
if not "%NLS%" == "disabled" goto SmallEnv

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

Rem Let libtool use _libs all the time.
test -f %XSRC%/ltconfig.orig
if errorlevel 1 update %XSRC%/ltconfig %XSRC%/ltconfig.orig
sed "/objdir=/s|\.libs|_libs|" %XSRC%/ltconfig > ltconfig.tmp
if errorlevel 1 goto SedError
update ltconfig.tmp %XSRC%/ltconfig
rm ltconfig.tmp

Rem While building the binaries in src/ subdir an intermediary
Rem file called po-gram-gen2.h is generated from po-gram-gen.h.
Rem Both resolve to the same 8.3 filename. po-gram-gen2.h will
Rem be renamed to po-gram_gen2.h and src/po-lex.c must be fixed
Rem accordingly.
test -f %XSRC%/src/po-lex.orig
if errorlevel 1 update %XSRC%/src/po-lex.c %XSRC%/src/po-lex.orig
sed "s/po-gram-gen2.h/po-gram_gen2.h/g" %XSRC%/src/po-lex.c > po-lex.tmp
if errorlevel 1 goto SedError
update ./po-lex.tmp %XSRC%/src/po-lex.c
rm ./po-lex.tmp

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
sh ./configure --srcdir=%XSRC% --enable-nls --with-included-gettext
if errorlevel 1 goto CfgError
echo Done.
goto ScriptEditing

:WithoutNLS
echo Running the ./configure script...
sh ./configure --srcdir=%XSRC% --disable-nls
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
set CONFIG_SITE=
set HOSTNAME=
set NLS=
set XSRC=
