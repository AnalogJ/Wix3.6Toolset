::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: make.bat
:: --------
:: Builds the WiX toolset using NAnt.
::
:: Copyright (c) Microsoft Corporation.  All rights reserved.
::
:: The use and distribution terms for this software are covered by the
:: Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
:: which can be found in the file CPL.TXT at the root of this distribution.
:: By using this software in any fashion, you are agreeing to be bound by
:: the terms of this license.
::
:: You must not remove this notice, or any other, from this software.
::
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::
:: In order to fully build WiX, you must have the following Frameworks and
:: SDKs installed:
::    * NAnt version 0.9 Beta 1
::    * .NET Framework 2.0, 3.0, and 3.5
::    * Windows SDK for Windows Server 2008 and .NET Framework 3.5
::    * Visual C++ 2008 Express or any edition of Visual Studio 2008 with 
::        Visual C++ default libraries installed
::    * HTML Help SDK 1.4 or higher
::
:: To build and install Sconce and Votive, you must have Visual Studio 2008,
:: Standard edition or higher)
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

@echo off
setlocal

:: Cache some environment variables.
set WIX_ROOT=%~dp0

:: Set the default arguments
set FLAVOR=debug
set ACTION=
set VERBOSE=
set TEST=
set MSBUILDVERBOSITY=minimal

:ParseArgs
:: Parse the incoming arguments
if /i "%1"==""        goto Build
if /i "%1"=="-?"      goto Syntax
if /i "%1"=="-h"      goto Syntax
if /i "%1"=="-help"   goto Syntax
if /i "%1"=="-l"      (set LOG=-logfile:%2)      & shift & shift & goto ParseArgs
if /i "%1"=="-v"      (set VERBOSE=-v & set MSBUILDVERBOSITY=detailed) & shift & goto ParseArgs
if /i "%1"=="debug"   (set FLAVOR=debug)         & shift & goto ParseArgs
if /i "%1"=="ship"    (set FLAVOR=ship)          & shift & goto ParseArgs
if /i "%1"=="prefast" (set CODEANALYSIS=prefast) & shift & goto ParseArgs
if /i "%1"=="build"   (set ACTION=build)         & shift & goto ParseArgs
if /i "%1"=="full"    (set ACTION=build)         & shift & goto ParseArgs
if /i "%1"=="clean"   (set ACTION=clean)         & shift & goto ParseArgs
if /i "%1"=="inc"     (set ACTION=)              & shift & goto ParseArgs
if /i "%1"=="nozip"   (set NOZIP=-D:zip=false)   & shift & goto ParseArgs
if /i "%1"=="test"    (set TEST=%TEST% -D:runbvts=true) & shift & goto ParseArgs
if /i "%1"=="testruntime"    (set TEST=%TEST% -D:runruntimebvts=true) & shift & goto ParseArgs
goto Error

:Build
pushd %WIX_ROOT%

nant.exe -buildfile:"%WIX_ROOT%wix.build" %ACTION% -D:flavor=%FLAVOR% -D:msbuildverbosity=%MSBUILDVERBOSITY% %LOG% %VERBOSE% %NOZIP% %TEST%

popd
goto End

:Error
echo Invalid command line parameter: %1
echo.

:Syntax
echo %~nx0 [-?] [-v] [debug or ship] [prefast] [clean or build or full or inc]
echo.
echo   -?         : this help
echo   -l log     : log build output to "log" file.
echo   -v         : verbose messages
echo   debug      : builds a debug version of the WiX toolset (default)
echo   ship       : builds a release (ship) version of the WiX toolset
echo   prefast    : include PREfast code analysis (C++, x86 only)
echo   clean      : cleans the build
echo   build      : does a full rebuild
echo   full       : same as build
echo   inc        : does an incremental rebuild (default)
echo   nozip      : does not build zip files
echo   test       : run bvt suite after building
echo   testruntime: run runtime bvt suite (test that may change the machine state)
echo.
echo.

:End
endlocal
