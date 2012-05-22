@echo off
goto LBegin

rem
rem If %1 exists in the file system, remove first (if any) existing occurrence
rem of %1 from PATH and prepend it.  PATH happens to get cleaned up to the
rem furthest point examined.  Only to be called from 'Setup PATH' section
rem below, with _exactly_ one space before argument.  Caller must ensure that
rem %PATH% does not consist entirely of semicolons to avoid infinite loop.
rem
rem FUTURE: remove duplicates (when CUR != COMP, set HEAD=HEAD;CUR)?  As side
rem effects, this would clean all of PATH (eliminating any CLEANPATH call in
rem 'Setup PATH' section), and be much slower on PATHs with many components.
rem FUTURE: remove non-existent entries (split set PATH=COMP/HEAD/TAIL in two,
rem and move if not exist COMP goto EOF between them)?  Also would be slower.
rem
:ENSUREPATH
set _COMP=%*
if "%_COMP:~0,1%"==" " set _COMP=%_COMP:~1%
if not exist "%_COMP%" goto :EOF
set _HEAD=
set _CUR=
set _TAIL=%PATH%
:LEnsureLoop
if "%_TAIL%"=="" goto LEndSearch
for /F "delims=; tokens=1* " %%i in ("%_TAIL%") do ((set _CUR=%%i) & (set _TAIL=%%j))
if /I "%_CUR%"=="%_COMP%" goto LEndSearch
set _HEAD=%_HEAD%;%_CUR%
goto LEnsureLoop
:LEndSearch
if not "%_TAIL%" == "" set _TAIL=;%_TAIL%
set PATH=%_COMP%%_HEAD%%_TAIL%
%_E% PATH to include %_COMP%
goto :EOF

rem
rem Compress multiple and remove trailing semicolons in PATH.  Caller must
rem ensure that PATH does not consist entirely of semicolons on entry to avoid
rem infinite loop.
rem
rem for /F is preferable to PATH=PATH:;;=; construct which can require multiple
rem instances of PATH on one line, drastically reducing the maximum length PATH
rem supported.  _HEAD is initialized non-empty to avoid _HEAD:~n expansion bug
rem if _HEAD is empty.  Loop is unrolled for performance.
rem
:CLEANPATH
set _HEAD=x
set _TAIL=%PATH%
:LCleanLoop
for /F "delims=; tokens=1-8*" %%i in ("%_TAIL%") do set _HEAD=%_HEAD%;%%i;%%j;%%k;%%l;%%m;%%n;%%o;%%p& set _TAIL=%%q
if not "%_TAIL%"=="" goto LCleanLoop
:LCleanTrimSemi
if "%_HEAD:~-1%" == ";" set _HEAD=%_HEAD:~0,-1%& goto LCleanTrimSemi
if "%_HEAD:~-1%" == " " set _HEAD=%_HEAD:~0,-1%& goto LCleanTrimSemi
set PATH=%_HEAD:~2%
goto :EOF

rem --- start of delivery environment setup ---

:LBegin

set _P=%~dp0
set _E=echo ^>nul
set _S=
set _N=
set _D=


rem --- parse arguments ---

:LParseArgs
if "%1" == "" goto LAfterParseArgs
if /I "%1" == "verbose" (set _E=echo wixenv.bat - setting) & goto LShiftArgs
if /I "%1" == "v"       (set _E=echo wixenv.bat - setting) & goto LShiftArgs
if /I "%1" == "nopath"  (set _S=) & (set _N=Yes) & goto LShiftArgs
if /I "%1" == "n"       (set _S=) & (set _N=Yes) & goto LShiftArgs
if /I "%1" == "setpath" (set _N=) & (set _S=Yes) & goto LShiftArgs
if /I "%1" == "s"       (set _N=) & (set _S=Yes) & goto LShiftArgs
if /I "%1" == "dtools"  (set _D=Yes) & goto LShiftArgs
if /I "%1" == "d"       (set _D=Yes) & goto LShiftArgs
echo wixenv.bat - ignoring unknown argument '%1'
:LShiftArgs
shift
goto LParseArgs
:LAfterParseArgs


rem --- find DTOOLS which finds everything else ---

if "%_D%"=="" goto LPath

if "%DTOOLS%" == "" if exist "%_P%..\..\dtools\dtools.snt" for /d %%j in ("%_P%..\..\dtools") do ((set DTOOLS=%%~fj) & (%_E% DTOOLS to %%~fj))
if "%DTOOLS%" == "" (echo wixenv.bat - error: unable to find DTOOLS) & goto LEnd
set OTOOLS=%DTOOLS%

rem --- find core directories ---

rem for %%k in (.. .) do if "%SRCROOT%" == "" if exist "%DTOOLS%\%%k\d.snt" for /d %%j in (%DTOOLS%\%%k) do ((set SRCROOT=%%~fj) & (%_E% SRCROOT to %%~fj))
rem if "%SRCROOT%" == "" for /d %%j in (%DTOOLS%\..) do ((set SRCROOT=%%~fj) & (%_E% SRCROOT to %%~fj))

rem if "%BUILDROOT%" == "" for /d %%j in (%SRCROOT%\..\Build) do ((set BUILDROOT=%%~fj) & (%_E% BUILDROOT to %%~fj))
rem if not exist %BUILDROOT% md %BUILDROOT%

rem if "%LOGROOT%" == "" for /d %%j in (%SRCROOT%\..\Logs) do ((set LOGROOT=%%~fj) & (%_E% LOGROOT to %%~fj))
rem if not exist %LOGROOT% md %LOGROOT%

rem if "%SETUPROOT%" == "" for /d %%j in (%SRCROOT%\..\Setup) do ((set SETUPROOT=%%~fj) & (%_E% SETUPROOT to %%~fj))
rem if not exist %SETUPROOT% md %SETUPROOT%
rem set SETUP=%SETUPROOT%

rem if "%TARGETROOT%" == "" for /d %%j in (%SRCROOT%\..\Target) do ((set TARGETROOT=%%~fj) & (%_E% TARGETROOT to %%~fj))
rem if not exist %TARGETROOT% md %TARGETROOT%


rem --- Setup other variables ---

rem Nmake propagates macros to spawned processes only if corresponding variable
rem already exists in calling environment.  Define INCLUDE and LIB to ensure
rem they are propagated from makefiles to cl.exe invocations.
rem FUTURE: set them unconditionally when no builds are broken by doing so
if "%INCLUDE%" == "" set INCLUDE=%SRCROOT%
if "%LIB%" == "" set LIB=%SRCROOT%


rem -- set search path ---

:LPath
if not "%_N%" == "" goto LAfterPath
if not "%_S%" == "" (set PATH=) & (%_E% PATH to ^<empty^>)

setlocal

rem remove any quotation marks from PATH (to avoid syntax errors below)
rem ensure it's non-empty (because substitution doesn't work on empty strings)
set PATH="%PATH%
set PATH=%PATH:"=%

rem ensure PATH is not entirely semicolons (to avoid ENSUREPATH infinite loop)
:LTrimSemi
if "%PATH%" == "" goto LEnsurePath
if "%PATH:~0,1%" == ";" set PATH=%PATH:~1% & goto LTrimSemi

:LEnsurePath
if "%Denv_Framework_20%" == "" (set Wixenv_Framework_20=v2.0.50727)
if "%Denv_Framework_35%" == "" (set Wixenv_Framework_35=v3.5)

call :ENSUREPATH %SystemRoot%
call :ENSUREPATH %SystemRoot%\system32
call :ENSUREPATH %SystemRoot%\Microsoft.Net\Framework\%Wixenv_Framework_20%
call :ENSUREPATH %SystemRoot%\Microsoft.Net\Framework\%Wixenv_Framework_35%
if not "%_D%"=="" call :ENSUREPATH %DTOOLS%\bin\SDPack
if not "%_D%"=="" call :ENSUREPATH %DTOOLS%\bin\x86
if not "%_D%"=="" call :ENSUREPATH %DTOOLS%\bin

for %%I in ("%_P%..\external\nant\bin\") do set NANTDEFAULT=%%~dpI

if exist "%programfiles(x86)%\nant\bin\nant.exe" set NANTPATH=%programfiles(x86)%\nant\bin\
if exist "%programfiles%\nant\bin\nant.exe" set NANTPATH=%programfiles%\nant\bin\
if exist "%SYSTEMDRIVE%\nant\bin\nant.exe" set NANTPATH=%SYSTEMDRIVE%\nant\bin\
if exist "%NANTDEFAULT%nant.exe" set NANTPATH=%NANTDEFAULT%
if not "%NANTPATH%"=="" call :ENSUREPATH %NANTPATH%
if "%NANTPATH%"=="" echo. && echo ERROR: Nant not found at expected location (%NANTDEFAULT%), or in path. Please place nant there, or add it to your path.

rem Ensure we have the VCpackages directory in the path (necessary for vcbuild.exe)
if NOT "%PROCESSOR_ARCHITEW6432%"=="" goto WOW64KEY
if "%PROCESSOR_ARCHITECTURE%"=="x86" goto REGULARKEY
if NOT "%PROCESSOR_ARCHITECTURE%"=="x86" goto WOW64KEY
rem default to the non-wow64 key
goto REGULARKEY

:WOW64KEY
set REGKEY=HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node
goto INITVCVARS

:REGULARKEY
set REGKEY=HKEY_LOCAL_MACHINE\SOFTWARE
goto INITVCVARS

:INITVCVARS
set _VCVARS=

call :INITVC9VARS 2> nul
call :INITVC10VARS 2> nul
call :CLEANPATH

rem VC was not initialized. Currently we require VS to build Burn, which has a dependency on ATL that does not ship in the standalone SDK.
if "%_VCVARS%" == "" (
echo ERROR: VC variables not initialized. Please ensure that at least one of the following supported products is installed or call the appropriate script manually, such as vcvars32.bat.
echo ERROR:    Visual Studio 2008
echo ERROR:    Visual Studio 2010
)
goto LEndLocal

:INITVC9VARS
for /f "tokens=2*" %%A IN ('REG QUERY "%REGKEY%\Microsoft\VisualStudio\9.0\Setup\VC" /v ProductDir') do set _VCDIR=%%~sB
if exist "%_VCDIR%bin\vcvars32.bat" ((call "%_VCDIR%bin\vcvars32.bat") & (set _VCVARS=9.0))
goto :EOF

:INITVC10VARS
for /f "tokens=2*" %%A IN ('REG QUERY "%REGKEY%\Microsoft\VisualStudio\10.0\Setup\VC" /v ProductDir') do set _VCDIR=%%~sB
if exist "%_VCDIR%bin\vcvars32.bat" ((call "%_VCDIR%bin\vcvars32.bat") & (set _VCVARS=10.0))
goto :EOF

:LEndLocal
endlocal & set PATH=%PATH%
:LAfterPath

rem -- end of script ---

:LEnd

cd /d "%_P%.."
set WIX_ROOT=%CD%
title wixenv

set _P=
set _E=
set _S=
set _N=
set _D=

rem -- user environment script ---
if exist "%USERPROFILE%\wix\userenv.bat" (
	echo wixenv.bat - sourcing user environment script
	call "%USERPROFILE%\wix\userenv.bat"
)
