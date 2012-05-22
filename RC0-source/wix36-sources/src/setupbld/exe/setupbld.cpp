//-------------------------------------------------------------------------------------------------
// <copyright file="main.cpp" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
//
// <summary>
// Setup executable builder for ClickThrough.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

// prototypes
extern "C" HRESULT CreateSetup(
    __in LPCWSTR wzSetupStub,
    __in_opt LPCWSTR wzProductName,
    __in_opt LPCWSTR wzLicensePath,
    __in_opt LPCWSTR wzBitmapPath,
    __in CREATE_SETUP_PACKAGE rgPackages[],
    __in CREATE_SETUP_TRANSFORMS rgTransforms[],
    __in BYTE cPackages,
    __in BYTE cTransforms,
    __in LPCWSTR wzOutput
    );


static void ShowHelp();


int __cdecl wmain(
    __in int argc,
    __in_ecount(argc) PWSTR argv[]
    )
{
    HRESULT hr = S_OK;
    LPCWSTR wzSetup = NULL;
    LPCWSTR wzBitmap = NULL;
    LPCWSTR wzLicense = NULL;
    LPCWSTR wzTitle = NULL;
    LPCWSTR wzOut = NULL;

    CREATE_SETUP_PACKAGE rgCreateSetupPackages[128] = { 0 };
    CREATE_SETUP_TRANSFORMS rgCreateSetupTransforms[128] = { 0 };
    BYTE cCreateSetupPackages = 0;
    BYTE cCreateSetupTransforms = 0;

    MemInitialize();

    hr = ConsoleInitialize();
    ExitOnFailure(hr, "failed to initialize console");

    //
    // Parse the command-line.
    //
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == L'-' || argv[i][0] == L'/')
        {
            if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"?", -1) ||
                CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"help", -1))
            {
                ShowHelp();
                ExitFunction1(hr = S_OK);
            }
            else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"o", -1) ||
                     CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"out", -1))
            {
                if (i + 1 >= argc)
                {
                    ConsoleExitOnFailure(hr = E_INVALIDARG, CONSOLE_COLOR_RED, "Must specify a filename with the -out switch.");
                }

                wzOut = argv[++i];
            }
            else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"bitmap", -1))
            {
                if (i + 1 >= argc)
                {
                    ConsoleExitOnFailure(hr = E_INVALIDARG, CONSOLE_COLOR_RED, "Must specify a filename with the -bitmap switch.");
                }

                wzBitmap = argv[++i];
            }
            else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"license", -1))
            {
                if (i + 1 >= argc)
                {
                    ConsoleExitOnFailure(hr = E_INVALIDARG, CONSOLE_COLOR_RED, "Must specify a filename with the -license switch.");
                }

                wzLicense = argv[++i];
            }
            else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"title", -1))
            {
                if (i + 1 >= argc)
                {
                    ConsoleExitOnFailure(hr = E_INVALIDARG, CONSOLE_COLOR_RED, "Must specify a string with the -title switch.");
                }

                wzTitle = argv[++i];
            }
            else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"setup", -1))
            {
                if (i + 1 >= argc)
                {
                    ConsoleExitOnFailure(hr = E_INVALIDARG, CONSOLE_COLOR_RED, "Must specify a filename with the -setup switch.");
                }

                wzSetup = argv[++i];
            }
            else if (argv[i][1] == L'm')
            {
                if (i + 1 >= argc)
                {
                    ConsoleExitOnFailure(hr = E_INVALIDARG, CONSOLE_COLOR_RED, "Must specify a filename with the -m switch.");
                }
                else if (countof(rgCreateSetupPackages) <= cCreateSetupPackages)
                {
                    ConsoleExitOnFailure(hr = E_INVALIDARG, CONSOLE_COLOR_RED, "Too many MSI and MSP files to package.");
                }

                LPCWSTR pwz = &argv[i][2];
                while (pwz && *pwz)
                {
                    switch (*pwz)
                    {
                    case L'c':
                        rgCreateSetupPackages[cCreateSetupPackages].fCache = TRUE;
                        break;
                    case L'i':
                        rgCreateSetupPackages[cCreateSetupPackages].fIgnoreFailures = TRUE;
                        break;
                    case L'l':
                        rgCreateSetupPackages[cCreateSetupPackages].fLink = TRUE;
                        break;
                    case L'p':
                        rgCreateSetupPackages[cCreateSetupPackages].fPrivileged = TRUE;
                        break;
                    case L's':
                        rgCreateSetupPackages[cCreateSetupPackages].fShowUI = TRUE;
                        break;
                    case L'u':
                        rgCreateSetupPackages[cCreateSetupPackages].fMinorUpgrade = TRUE;
                        break;
                    case L't':
                        rgCreateSetupPackages[cCreateSetupPackages].fUseTransform = TRUE;
                        break;
                    default:
                        ConsoleExitOnFailure1(hr = E_INVALIDARG, CONSOLE_COLOR_RED, "Invalid MSI option: %s", pwz);
                    }

                    pwz++;
                }

                rgCreateSetupPackages[cCreateSetupPackages].wzSourcePath = argv[++i];
                cCreateSetupPackages++;
            }
            else if (argv[i][1] == L'p')
            {
                if (i + 1 >= argc)
                {
                    ConsoleExitOnFailure(hr = E_INVALIDARG, CONSOLE_COLOR_RED, "Must specify a filename with the -p switch.");
                }
                else if (countof(rgCreateSetupPackages) <= cCreateSetupPackages)
                {
                    ConsoleExitOnFailure(hr = E_INVALIDARG, CONSOLE_COLOR_RED, "Too many MSI and MSP files to package.");
                }
                
                LPCWSTR pwz = &argv[i][2];
                while (pwz && *pwz)
                {
                    switch (*pwz)
                    {
                    case L'i':
                        rgCreateSetupPackages[cCreateSetupPackages].fIgnoreFailures = TRUE;
                        break;
                    case L'm':
                        rgCreateSetupPackages[cCreateSetupPackages].fPatchForceTarget = TRUE;
                        break;
                    default:
                        ConsoleExitOnFailure1(hr = E_INVALIDARG, CONSOLE_COLOR_RED, "Invalid Patch option: %s", pwz);
                    }

                    pwz++;
                }
                rgCreateSetupPackages[cCreateSetupPackages].fPatch = TRUE;
                rgCreateSetupPackages[cCreateSetupPackages].wzSourcePath = argv[++i];
                cCreateSetupPackages++;
            }
            else if (argv[i][1] == L'u')
            {
                if (i + 2 >= argc)
                {
                    ConsoleExitOnFailure(hr = E_INVALIDARG, CONSOLE_COLOR_RED, "Must specify a transform locale identifier and transform filename with the -t switch.");
                }
                else if (countof(rgCreateSetupPackages) <= cCreateSetupPackages)
                {
                    ConsoleExitOnFailure(hr = E_INVALIDARG, CONSOLE_COLOR_RED, "Too many MST files to package.");
                }

                rgCreateSetupTransforms[cCreateSetupTransforms].dwLocaleId = _wtoi(argv[++i]);

                rgCreateSetupTransforms[cCreateSetupTransforms].wzSourcePath = argv[++i];
                cCreateSetupTransforms++;
            }
            else
            {
                ShowHelp();
                ConsoleExitOnFailure1(hr = E_INVALIDARG,  CONSOLE_COLOR_RED, "Bad commandline argument: %S", argv[i]);
            }
        }
    }

    if(NULL == wzSetup || NULL == wzOut)
    {
        ShowHelp();
        goto LExit;
    }

    if (0 == cCreateSetupPackages)
    {
        hr = E_INVALIDARG;
        ConsoleExitOnFailure(hr, CONSOLE_COLOR_RED, "Must specify at least one MSI file to package.");
    }

    hr = CreateSetup(wzSetup, wzTitle, wzLicense, wzBitmap, rgCreateSetupPackages, rgCreateSetupTransforms, cCreateSetupPackages, cCreateSetupTransforms,wzOut);
    ConsoleExitOnFailure(hr, CONSOLE_COLOR_RED, "Failed to CreateSetup.");

LExit:
    ConsoleUninitialize();
    MemUninitialize();

    return SCODE_CODE(hr);
}


static void ShowHelp()
{
    DWORD dwArg = 0;
    DWORD dwVerMinor = 0;
    DWORD dwVerMajor = 0;
    DWORD cchFileName = MAX_PATH;
    WCHAR wzFileName[MAX_PATH] = {0};
    VS_FIXEDFILEINFO *vInfo = NULL;
    UINT uInfoSize = 0;
    BYTE* lpBuff = NULL;
    HRESULT hr = S_OK;

    DWORD cch = ::GetModuleFileNameW(NULL, wzFileName, cchFileName);
    if(0 == cch)
    {
        ConsoleExitOnLastError(hr, CONSOLE_COLOR_RED, "No version info available" );
    }

    DWORD dwInfoSize = ::GetFileVersionInfoSizeW(wzFileName, &dwArg);
    if(0 == dwInfoSize)
    {
        ConsoleExitOnLastError(hr, CONSOLE_COLOR_RED, "No version info available" );
    }

    lpBuff = (BYTE*)MemAlloc(dwInfoSize, true);
    if(!lpBuff)
    {
        ConsoleExitOnLastError(hr, CONSOLE_COLOR_RED, "Out of Memory");
    }

    if(0 == ::GetFileVersionInfoW(wzFileName, 0, dwInfoSize, lpBuff))
    {
        ConsoleExitOnLastError(hr, CONSOLE_COLOR_RED, "Error retrieving version info");
    }

    if(0 == ::VerQueryValueW(lpBuff, L"\\", (LPVOID*)&vInfo,  &uInfoSize))
    {        
        ConsoleExitOnFailure(hr = E_ACCESSDENIED, CONSOLE_COLOR_RED, "Error retrieving version info");
    }

    if(0 == uInfoSize)
    {
        ConsoleExitOnFailure(hr = E_ACCESSDENIED, CONSOLE_COLOR_RED, "Version information not available");
    }

    dwVerMajor = vInfo->dwFileVersionMS;
    dwVerMinor = vInfo->dwFileVersionLS;

    ConsoleWriteLine(CONSOLE_COLOR_NORMAL, "Microsoft (R) Windows Installer Xml Self-Extracting Compiler version %d.%d.%d.%d", dwVerMajor>>16, dwVerMajor&0xFFFF, dwVerMinor>>16, dwVerMinor&0xFFFF);
    ConsoleWriteLine(CONSOLE_COLOR_NORMAL, "Copyright (C) Microsoft Corporation 2006. All rights reserved.");
    ConsoleWriteLine(CONSOLE_COLOR_NORMAL, " ");
    ConsoleWriteLine(CONSOLE_COLOR_NORMAL, " usage:  setupbld.exe [-?] -out outputFile -m[clipsu] msiFile [msiFile ...] -setup setupStubFile");
    ConsoleWriteLine(CONSOLE_COLOR_NORMAL, " ");
    ConsoleWriteLine(CONSOLE_COLOR_NORMAL, "   -out         specify output file");
    ConsoleWriteLine(CONSOLE_COLOR_NORMAL, "   -m[clipsut]  specify msi files to include in exe");
    ConsoleWriteLine(CONSOLE_COLOR_NORMAL, "        c       enable msi caching");
    ConsoleWriteLine(CONSOLE_COLOR_NORMAL, "        l       link (NOT IMPLEMENTED)");
    ConsoleWriteLine(CONSOLE_COLOR_NORMAL, "        i       ignore any errors during install");
    ConsoleWriteLine(CONSOLE_COLOR_NORMAL, "        p       privileged (NOT IMPLEMENTED)");
    ConsoleWriteLine(CONSOLE_COLOR_NORMAL, "        s       show embedded msi ui");
    ConsoleWriteLine(CONSOLE_COLOR_NORMAL, "        u       enable minor upgrades");
    ConsoleWriteLine(CONSOLE_COLOR_NORMAL, "        t       apply UI transforms to this MSI");
    ConsoleWriteLine(CONSOLE_COLOR_NORMAL, "      filename  msi filename");
    ConsoleWriteLine(CONSOLE_COLOR_NORMAL, "   -p[im]       specify msp files to include in exe");
    ConsoleWriteLine(CONSOLE_COLOR_NORMAL, "        i       ignore any errors during install");
    ConsoleWriteLine(CONSOLE_COLOR_NORMAL, "        m       show an error if the patch target product is missing");
    ConsoleWriteLine(CONSOLE_COLOR_NORMAL, "      filename  msp filename");
    ConsoleWriteLine(CONSOLE_COLOR_NORMAL, "   -u           specify UI transform files to include in exe");
    ConsoleWriteLine(CONSOLE_COLOR_NORMAL, "      locale    locale identifier for transform");
    ConsoleWriteLine(CONSOLE_COLOR_NORMAL, "      filename  mst filename");
    ConsoleWriteLine(CONSOLE_COLOR_NORMAL, "   -bitmap      specify bitmap file");
    ConsoleWriteLine(CONSOLE_COLOR_NORMAL, "   -license     specify license file");
    ConsoleWriteLine(CONSOLE_COLOR_NORMAL, "   -title       specify title name");
    ConsoleWriteLine(CONSOLE_COLOR_NORMAL, "   -setup       specify path to the input setup stub");
    ConsoleWriteLine(CONSOLE_COLOR_NORMAL, "   -?           this help information");

LExit:
    ReleaseMem(lpBuff);
    return;
}
