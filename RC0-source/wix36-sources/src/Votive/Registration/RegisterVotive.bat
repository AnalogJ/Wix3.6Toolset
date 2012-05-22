::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: RegisterVotive.bat
:: -------------------
:: Preprocesses all of the various support files for working with Votive in a development
:: environment and then registers Votive with Visual Studio 2005.
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

@echo off
setlocal

:: When running under 64-bit Windows, the registry key is stored under the Wow6432Node.
set VS_REGKEYROOT=HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\VisualStudio\8.0Exp
if /i [%PROCESSOR_ARCHITECTURE%]==[AMD64] set VS_REGKEYROOT=HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Microsoft\VisualStudio\8.0Exp

:: Make sure the SDK is installed (the experimental hive is registered).
reg query %VS_REGKEYROOT%\Setup\VS /v EnvironmentPath > NUL 2> NUL
if not %errorlevel% == 0 (
	echo VS SDK for Visual Studio 2005 is not installed. Skipping registration.
	goto End
)

:: Try to get the paths to Visual Studio
for /f "tokens=3*" %%a in ('reg query %VS_REGKEYROOT%\Setup\VS /v EnvironmentPath ^| find "EnvironmentPath"') do set DEVENVPATH_2005=%%a %%b
for /f "tokens=3*" %%a in ('reg query %VS_REGKEYROOT%\Setup\VS /v EnvironmentDirectory ^| find "EnvironmentDirectory"') do set DEVENVDIR_2005=%%a %%b

:: GUIDs
set PACKAGE_GUID={E0EE8E7D-F498-459E-9E90-2B3D73124AD5}
set PROJECT_GUID={930C7802-8A8C-48F9-8165-68863BCCD9DD}
set XML_EDITOR_GUID_2005={FA3CD31E-987B-443A-9B81-186104E8DAC1}

set TARGETROOT=%WIX_ROOT%\build\debug\x86

:: Toools declarations
set VOTIVE_PREPROCESSOR=%TARGETROOT%\VotivePP.exe

:: Directories
set SCONCE_TARGETDIR=%TARGETROOT%\
set SCONCE_TARGETPATH=%SCONCE_TARGETDIR%sconce2005.dll
set VOTIVE_TARGETDIR=%TARGETROOT%\
set VOTIVE_TARGETPATH=%VOTIVE_TARGETDIR%votive2005.dll
set WIXTOOLSDIR=%TARGETROOT%\
set ITEMTEMPLATESDIR=%DEVENVDIR_2005%ItemTemplatesExp\WiX\
set PROJECTTEMPLATESDIR=%DEVENVDIR_2005%ProjectTemplatesExp\WiX\

:: Write the .reg files and use reg.exe to import the files
set REG_FILE=%~dp0Register.reg
echo Writing package registry settings for Visual Studio 2005...
"%VOTIVE_PREPROCESSOR%" -bs "%~dp0Register.reg.pp" "%REG_FILE%" VS_REGKEYROOT="%VS_REGKEYROOT%" DLLPATH="%VOTIVE_TARGETPATH%" DLLDIR="%VOTIVE_TARGETDIR%\" SCONCEPATH="%SCONCE_TARGETPATH%" ITEMTEMPLATESDIR="%ITEMTEMPLATESDIR%\" PROJECTTEMPLATESDIR="%PROJECTTEMPLATESDIR%\" WIXTOOLSDIR="%WIXTOOLSDIR%\" DEVENVPATH="%DEVENVPATH_2005%" PACKAGE_GUID=%PACKAGE_GUID% PROJECT_GUID=%PROJECT_GUID% XML_EDITOR_GUID=%XML_EDITOR_GUID_2005% VS_VERSION=8.0 VS_VERSION_YEAR=2005

reg import "%REG_FILE%"
if exist "%REG_FILE%" del /q /f "%REG_FILE%"
echo Registering Votive with Visual Studio 2005 Exp...
echo "%DEVENVPATH_2005%" /setup /rootsuffix Exp
"%DEVENVPATH_2005%" /setup /rootsuffix Exp
echo.

:End
endlocal
exit /b 0
