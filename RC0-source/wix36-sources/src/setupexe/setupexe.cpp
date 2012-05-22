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
// Setup executable for ClickThrough.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

#define INVALID_LCID -1


enum CONFIGURE_ACTION
{
    CONFIGURE_ACTION_NONE,
    CONFIGURE_ACTION_INSTALL,
    CONFIGURE_ACTION_REINSTALL,
    CONFIGURE_ACTION_RECACHE,
    CONFIGURE_ACTION_MINOR_UPGRADE,
    CONFIGURE_ACTION_MAJOR_UPGRADE,
    CONFIGURE_ACTION_PATCH,
    CONFIGURE_ACTION_UNINSTALL,
};


typedef struct
{
    WCHAR wzProductCode[39];

    WCHAR wzTempPath[MAX_PATH];
    WCHAR wzCachedPath[MAX_PATH];

    BOOL bCached;
    BOOL bInstalled;
} SETUP_ROLLBACK_PACKAGE;


HINSTANCE vhInstance = NULL;
LPCSTR vwzLicenseText = NULL;

#if 0
HRESULT LaunchAndWaitForExecutable(
    __in LPCWSTR wzExecutablePath,
    __in LPCWSTR wzExecutableArgs,
    __in BOOL bInstall
    )
{
    HRESULT hr = S_OK;

    WCHAR wzCommandLine[MAX_PATH];
    STARTUPINFOW startupInfo = { 0 };
    PROCESS_INFORMATION procInfo = { 0 };

    hr = ::StringCchPrintfW(wzCommandLine, countof(wzCommandLine), L"%s %s", wzExecutablePath, wzExecutableArgs ? wzExecutableArgs : L"");
    ExitOnFailure(hr, "Failed to generate commandline for msiexec.exe.");

    startupInfo.cb = sizeof(startupInfo);
    if (!::CreateProcessW(NULL, wzCommandLine, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &procInfo))
    {
        ExitOnLastError1(hr, "Failed to CreateProcess on path: %S", wzExecutablePath);
    }

    if (WAIT_OBJECT_0 != ::WaitForSingleObject(procInfo.hProcess, INFINITE))
    {
        ExitOnFailure1(hr = E_UNEXPECTED, "Unexpected terminated process: %S", wzExecutablePath);
    }

LExit:
    if (procInfo.hProcess)
    {
        ::CloseHandle(procInfo.hProcess);
    }

    if (procInfo.hThread)
    {
        ::CloseHandle(procInfo.hThread);
    }

    return hr;
}
#endif


HRESULT ConfigureMsi(
    __in_opt LPCWSTR wzProductCode,
    __in_opt LPCWSTR wzOriginalMsiName,
    __in_opt LPCWSTR wzMsiPath,
    __in_opt LPCWSTR wzMsiInstallArgs,
    __in_opt LPCWSTR wzMsiLog,
    __in CONFIGURE_ACTION action,
    __in DWORD dwUiLevel,
    __in BOOL fIgnoreFailures,
    __in BOOL fPatchForceTarget
    )
{
    Assert(wzProductCode || wzMsiPath);

    HRESULT hr = S_OK;
    DWORD er = ERROR_SUCCESS;

    INSTALLUI_HANDLERW pfnExternalUI = NULL;
    LPWSTR pwzLogFile = NULL;
    LPWSTR pwzProperties = NULL;
    LPWSTR pwzFileNameNoExtension = NULL;
    LPVOID pvErrorMessage = NULL;

    ::MsiSetInternalUI(static_cast<INSTALLUILEVEL>(dwUiLevel), NULL);

    //pfnExternalUI = ::MsiSetExternalUIW(pfnInstallEngineCallback, INSTALLLOGMODE_PROGRESS|INSTALLLOGMODE_FATALEXIT|INSTALLLOGMODE_ERROR
    //    |INSTALLLOGMODE_WARNING|INSTALLLOGMODE_USER|INSTALLLOGMODE_INFO
    //    |INSTALLLOGMODE_RESOLVESOURCE|INSTALLLOGMODE_OUTOFDISKSPACE
    //    |INSTALLLOGMODE_ACTIONSTART|INSTALLLOGMODE_ACTIONDATA
    //    |INSTALLLOGMODE_COMMONDATA|INSTALLLOGMODE_PROGRESS|INSTALLLOGMODE_INITIALIZE
    //    |INSTALLLOGMODE_TERMINATE|INSTALLLOGMODE_SHOWDIALOG, pvCallbackContext);

    if (NULL != wzMsiLog)
    {
        er = ::MsiEnableLogW(INSTALLLOGMODE_FATALEXIT|INSTALLLOGMODE_ERROR|INSTALLLOGMODE_WARNING
            |INSTALLLOGMODE_USER|INSTALLLOGMODE_INFO|INSTALLLOGMODE_RESOLVESOURCE
            |INSTALLLOGMODE_OUTOFDISKSPACE|INSTALLLOGMODE_ACTIONSTART|INSTALLLOGMODE_ACTIONDATA
            |INSTALLLOGMODE_COMMONDATA|INSTALLLOGMODE_PROPERTYDUMP|INSTALLLOGMODE_VERBOSE,
            wzMsiLog, INSTALLLOGATTRIBUTES_APPEND);
        hr = HRESULT_FROM_WIN32(er);
        TraceError1(hr, "failed to set logging: %S", wzMsiLog);
    }
    else if (NULL != wzOriginalMsiName)
    {
        hr = FileStripExtension(wzOriginalMsiName, &pwzFileNameNoExtension);
        TraceError(hr, "failed to remove filename extension");
        if (SUCCEEDED(hr))
        {
            hr = FileCreateTempW(pwzFileNameNoExtension, L"log", &pwzLogFile, NULL);
            TraceError(hr, "failed to create temp file");
            if (SUCCEEDED(hr))
            {
                er = ::MsiEnableLogW(INSTALLLOGMODE_FATALEXIT|INSTALLLOGMODE_ERROR|INSTALLLOGMODE_WARNING
                    |INSTALLLOGMODE_USER|INSTALLLOGMODE_INFO|INSTALLLOGMODE_RESOLVESOURCE
                    |INSTALLLOGMODE_OUTOFDISKSPACE|INSTALLLOGMODE_ACTIONSTART|INSTALLLOGMODE_ACTIONDATA
                    |INSTALLLOGMODE_COMMONDATA|INSTALLLOGMODE_PROPERTYDUMP|INSTALLLOGMODE_VERBOSE,
                    pwzLogFile, 0);
                hr = HRESULT_FROM_WIN32(er);
                TraceError1(hr, "failed to set logging: %S", pwzLogFile);
            }
        }

    }

    // Ignore all failures setting up the UI handler and
    // logging, they aren't important enough to abort the
    // install attempt.
    hr = S_OK;

    // set up our properties
    hr = StrAllocString(&pwzProperties, wzMsiInstallArgs ? wzMsiInstallArgs : L"", 0);
    ExitOnFailure1(hr, "Failed to allocate string for MSI install arguments: %S", wzMsiInstallArgs);

    //
    // Do the actual action.
    //
    switch (action)
    {
    case CONFIGURE_ACTION_MAJOR_UPGRADE: __fallthrough;
    case CONFIGURE_ACTION_INSTALL:
        er = ::MsiInstallProductW(wzMsiPath, pwzProperties);
        hr = HRESULT_FROM_WIN32(er);
        break;
    case CONFIGURE_ACTION_PATCH:
        er = ::MsiApplyPatchW(wzMsiPath, NULL, INSTALLTYPE_DEFAULT, pwzProperties);
        hr = HRESULT_FROM_WIN32(er);
        break;
    case CONFIGURE_ACTION_RECACHE:
        hr = StrAllocConcat(&pwzProperties, L" REINSTALLMODE=\"vdmus\"", 0);
        ExitOnFailure(hr, "failed to format property: REINSTALLMODE");

        //hr = ::MsiSourceListClearAllW(wzProductCode, NULL, 0);
        //hr = HRESULT_FROM_WIN32(hr);
        //ExitOnFailure(hr, "failed to clear MSI source list");

        //hr = StrAllocString(&wzSourceDirectory, pwi->wzMsiPath, 0);
        //ExitOnFailure(hr, "failed to allocate source buffer");
        //if (wzLastSlash = wcsrchr(wzSourceDirectory, L'\\'))
        //{
        //    *wzLastSlash = 0;
        //}
        //hr = ::MsiSourceListAddSourceW(wzProductCode, NULL, 0, wzSourceDirectory);
        //hr = HRESULT_FROM_WIN32(hr);
        //ExitOnFailure(hr, "failed to add to MSI source list");

        //hr = ::MsiSourceListForceResolutionW(wzProductCode, NULL, 0);
        //hr = HRESULT_FROM_WIN32(hr);
        //ExitOnFailure(hr, "failed to force MSI source resolution");

        er = ::MsiConfigureProductExW(wzProductCode, 0, INSTALLSTATE_DEFAULT, pwzProperties);
        hr = HRESULT_FROM_WIN32(er);
        break;
    case CONFIGURE_ACTION_REINSTALL:
        hr = StrAllocConcat(&pwzProperties, L" REINSTALL=ALL REINSTALLMODE=\"omus\"", 0);
        ExitOnFailure(hr, "failed to format properties: REINSTALL and REINSTALLMODE");

        er = ::MsiInstallProductW(wzMsiPath, pwzProperties);
        hr = HRESULT_FROM_WIN32(er);
        break;
    case CONFIGURE_ACTION_MINOR_UPGRADE:
        hr = StrAllocConcat(&pwzProperties, L" REINSTALL=ALL REINSTALLMODE=\"vomus\"", 0);
        ExitOnFailure(hr, "failed to format properties: REINSTALL and REINSTALLMODE");

        er = ::MsiInstallProductW(wzMsiPath, pwzProperties);
        hr = HRESULT_FROM_WIN32(er);
        break;
    case CONFIGURE_ACTION_UNINSTALL:
        hr = StrAllocConcat(&pwzProperties, L" REMOVE=ALL", 0);
        ExitOnFailure(hr, "failed to format property: REMOVE");

        er = ::MsiInstallProductW(wzMsiPath, pwzProperties);
        hr = HRESULT_FROM_WIN32(er);
        break;
    }

    if (FAILED(hr))
    {
        if (fIgnoreFailures)
        {
            hr = S_FALSE; // success, but did not install
        }
        else
        {
            DWORD cchErrorMessage = 0;

            switch(er)
            {
            case ERROR_INSTALL_FAILURE:
            case ERROR_INSTALL_USEREXIT:
                // Don't show an error for those cases, as Windows Installer has shown 
                // some UI at that point and the user should be able to handle it.
                break;
            case ERROR_PATCH_TARGET_NOT_FOUND:
                // If we weren't asked to show an error on a non-applicable patch, skip it
                if (!fPatchForceTarget)
                {
                    hr = S_FALSE; // success, but did not install
                    break;
                }
                __fallthrough;
            case ERROR_UNKNOWN_PRODUCT:
            case ERROR_INSTALL_PACKAGE_REJECTED:
            case ERROR_CREATE_FAILED:
            case ERROR_INSTALL_PLATFORM_UNSUPPORTED:
                cchErrorMessage = ::FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&pvErrorMessage, 0, NULL);
                if (0 < cchErrorMessage)
                {
                    ::MessageBoxExW(NULL, static_cast<PCWSTR>(pvErrorMessage), L"Install Error", MB_ICONERROR | MB_OK, 0);
                }
                break;
            default:
                cchErrorMessage = ::FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, HRESULT_FROM_WIN32(ERROR_PROCESS_ABORTED), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&pvErrorMessage, 0, NULL);
                if (0 < cchErrorMessage)
                {
                    ::MessageBoxExW(NULL, static_cast<PCWSTR>(pvErrorMessage), L"Install Error", MB_ICONERROR | MB_OK, 0);
                }
                break; 
            }
        }
    }

LExit:
    if (pfnExternalUI)  // unset the UI handler
    {
        ::MsiSetExternalUIW(pfnExternalUI, 0, NULL);
    }

    if (pvErrorMessage)
    {
        ::LocalFree(pvErrorMessage);
    }

    ReleaseStr(pwzProperties);
    ReleaseStr(pwzLogFile);
    ReleaseStr(pwzFileNameNoExtension);

    return hr;
}


HRESULT CacheMsi(
    __in SETUP_INSTALL_PACKAGE* pPackage,
    __in LPCWSTR wzTempMsiPath,
    __in BOOL bAllUsers,
    __out LPWSTR* ppwzCachedMsiPath
    )
{
    Assert(ppwzCachedMsiPath);

    HRESULT hr = S_OK;
    DWORD dwAppDataFlags = bAllUsers ? CSIDL_COMMON_APPDATA : CSIDL_LOCAL_APPDATA;
    WCHAR wzLocalAppPath[MAX_PATH];
    WCHAR wzProductCode[39];
    WCHAR wzVersion[36];

    // actually cache the MSI now
    hr = ::SHGetFolderPathW(NULL, dwAppDataFlags | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, wzLocalAppPath);
    ExitOnFailure(hr, "Failed to get local app data folder.");

    hr = ::StringFromGUID2(pPackage->guidProductCode, wzProductCode, countof(wzProductCode));
    ExitOnFailure(hr, "Failed to convert ProductCode GUID to string.");

    hr = ::StringCchPrintfW(wzVersion, countof(wzVersion), L"%d.%d.%d.%d", pPackage->dwVersionMajor >> 16, pPackage->dwVersionMajor & 0xFFFF,
                                                                           pPackage->dwVersionMinor >> 16, pPackage->dwVersionMinor & 0xFFFF);
    ExitOnFailure(hr, "Failed to format version string.");

    hr = StrAllocFormatted(ppwzCachedMsiPath, L"%ls\\Applications\\Cache\\%lsv%ls.msi", wzLocalAppPath, wzProductCode, wzVersion);
    ExitOnFailure(hr, "Failed to allocate formatted.");

    hr = FileEnsureMove(wzTempMsiPath, *ppwzCachedMsiPath, TRUE, TRUE);
    if (FAILED(hr) && TRUE == bAllUsers)
    {
       // Attempted to place file in All Users Cache, but failed
       hr = CacheMsi(pPackage, wzTempMsiPath, !bAllUsers, ppwzCachedMsiPath);
       ExitOnFailure(hr, "Failed to move MSI to cache, second chance.");
    }
    else
    {
        ExitOnFailure(hr, "Failed to move MSI to cache.");
    }

LExit:
    return hr;
}

HRESULT CacheMst(
    __in SETUP_INSTALL_PACKAGE* pPackage,
    __in LPCWSTR wzTempMstPath,
    __in BOOL bAllUsers,
    __out LPWSTR* ppwzCachedMstPath
    )
{
    Assert(ppwzCachedMstPath);

    HRESULT hr = S_OK;
    DWORD dwAppDataFlags = bAllUsers ? CSIDL_COMMON_APPDATA : CSIDL_LOCAL_APPDATA;
    WCHAR wzLocalAppPath[MAX_PATH];
    WCHAR wzProductCode[39];
    WCHAR wzVersion[36];

    // actually cache the MSI now
    hr = ::SHGetFolderPathW(NULL, dwAppDataFlags | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, wzLocalAppPath);
    ExitOnFailure(hr, "Failed to get local app data folder.");

    hr = ::StringFromGUID2(pPackage->guidProductCode, wzProductCode, countof(wzProductCode));
    ExitOnFailure(hr, "Failed to convert ProductCode GUID to string.");

    hr = ::StringCchPrintfW(wzVersion, countof(wzVersion), L"%d.%d.%d.%d", pPackage->dwVersionMajor >> 16, pPackage->dwVersionMajor & 0xFFFF,
                                                                           pPackage->dwVersionMinor >> 16, pPackage->dwVersionMinor & 0xFFFF);
    ExitOnFailure(hr, "Failed to format version string.");

    hr = StrAllocFormatted(ppwzCachedMstPath, L"%ls\\Applications\\Cache\\%lsv%ls.mst", wzLocalAppPath, wzProductCode, wzVersion);
    ExitOnFailure(hr, "Failed to allocate formatted.");

    hr = FileEnsureMove(wzTempMstPath, *ppwzCachedMstPath, TRUE, TRUE);
    if (FAILED(hr) && TRUE == bAllUsers)
    {
       // Attempted to place file in All Users Cache, but failed
       hr = CacheMst(pPackage, wzTempMstPath, !bAllUsers, ppwzCachedMstPath);
       ExitOnFailure(hr, "Failed to move MST to cache, second chance.");
    }
    else
    {
        ExitOnFailure(hr, "Failed to move MST to cache.");
    }

LExit:
    return hr;
}

HRESULT DetectMsi(
    __in LPCWSTR wzProductCode,
    __in DWORD dwMajorVersion,
    __in DWORD dwMinorVersion,
    __out CONFIGURE_ACTION* pAction
    )
{
    Assert(wzProductCode);
    Assert(pAction);

    HRESULT hr = S_OK;
    UINT er = ERROR_SUCCESS;
    WCHAR wzInstalledVersion[36];
    DWORD cchInstalledVersion = countof(wzInstalledVersion);
    DWORD dwInstalledMajorVersion = 0;
    DWORD dwInstalledMinorVersion = 0;
    CONFIGURE_ACTION action = CONFIGURE_ACTION_NONE;

    er = ::MsiGetProductInfoW(wzProductCode, L"VersionString", wzInstalledVersion, &cchInstalledVersion);
    if (ERROR_SUCCESS == er)
    {
        hr = FileVersionFromString(wzInstalledVersion, &dwInstalledMajorVersion, &dwInstalledMinorVersion);
        ExitOnFailure2(hr, "Failed to convert version: %S to DWORDs for ProductCode: %S", wzInstalledVersion, wzProductCode);

        //
        // Take a look at the version and determine if this is a potential
        // reinstall, minor upgrade, a major upgrade (just install) otherwise, there
        // is a newer version so no work necessary.
        //
        if ((dwMajorVersion == dwInstalledMajorVersion) && (dwMinorVersion == dwInstalledMinorVersion))
        {
            action = CONFIGURE_ACTION_REINSTALL;
        }
        else if (((dwMajorVersion >> 16) == (dwInstalledMajorVersion >> 16) && dwMajorVersion > dwInstalledMajorVersion) ||
            (dwMajorVersion == dwInstalledMajorVersion && dwMinorVersion > dwInstalledMinorVersion))
        {
            // minor upgrade "10.1.X.X" > "10.0.X.X" or "10.3.2.0" > "10.3.1.0"
            action = CONFIGURE_ACTION_MINOR_UPGRADE;
        }
        else if (dwMajorVersion > dwInstalledMajorVersion) // major upgrade "11.X.X.X" > "10.X.X.X"
        {
            Assert((dwMajorVersion >> 16) > (dwInstalledMajorVersion >> 16));
            action = CONFIGURE_ACTION_MAJOR_UPGRADE;
        }
        else // newer version installed "14.X.X.X" < "15.X.X.X", skip
        {
            action = CONFIGURE_ACTION_NONE;
        }

        hr = S_OK;
    }
    else if (ERROR_UNKNOWN_PRODUCT == er || ERROR_UNKNOWN_PROPERTY == er)
    {
        action = CONFIGURE_ACTION_INSTALL;
        hr = S_OK;
    }
    else
    {
        hr = HRESULT_FROM_WIN32(er);
        ExitOnFailure1(hr, "Failed to get product information for ProductCode: %S", wzProductCode);
    }

    *pAction = action;

LExit:
    return hr;
}


HRESULT ExtractMsi(
    __in const SETUP_INSTALL_PACKAGE* pPackage,
    __out LPWSTR* ppwzTempMsiPath
    )
{
    HRESULT hr = S_OK;
    DWORD cchTempMsiPath = MAX_PATH;
    DWORD cch = 0;
    WCHAR wzProductCode[39];

    // Drop the package into the temp directory.
    hr = StrAlloc(ppwzTempMsiPath, cchTempMsiPath);
    ExitOnFailure(hr, "Failed to allocate string for temp path.");

    cch = ::GetTempPathW(cchTempMsiPath, *ppwzTempMsiPath);
    if (!cch || cch >= cchTempMsiPath)
    {
        ExitWithLastError(hr, "Failed to GetTempPath.");
    }

    hr = StrAllocConcat(ppwzTempMsiPath, L"\\", 0);
    ExitOnFailure(hr, "Failed to append to temporary directory.");

    hr = ::StringFromGUID2(pPackage->guidProductCode, wzProductCode, countof(wzProductCode));
    ExitOnFailure(hr, "Failed to convert ProductCode GUID to string.");

    hr = StrAllocConcat(ppwzTempMsiPath, wzProductCode, 0);
    ExitOnFailure(hr, "Failed to append to temporary directory.");

    hr = StrAllocConcat(ppwzTempMsiPath, L"\\", 0);
    ExitOnFailure(hr, "Failed to append to temporary directory.");

    hr = DirEnsureExists(*ppwzTempMsiPath, NULL);
    ExitOnFailure(hr, "Failed to create temporary directory.");

    // Use the actual MSI name
    hr = StrAllocConcat(ppwzTempMsiPath, pPackage->wzFilename, 0);
    ExitOnFailure(hr, "Failed to append MSI to temporary directory.");

    hr = ResExportDataToFile(pPackage->szSource, *ppwzTempMsiPath, CREATE_ALWAYS);
    ExitOnFailure1(hr, "Failed to write MSI package to %S.", *ppwzTempMsiPath);

LExit:
    return hr;
}


HRESULT ExtractMsiToDir(
    __in LPCWSTR pwzMsiOutputPath
    )
{
    HRESULT hr = S_OK;
    SETUP_INSTALL_CHAIN* pSetupChain = NULL;
    DWORD cbSetupChain = 0;
    LPWSTR pwzFullFilePath = NULL;

    hr = DirEnsureExists(pwzMsiOutputPath, NULL);
    ExitOnFailure(hr, "Failed creating/verifying directory.");

    hr = ResReadData(NULL, "MANIFEST", (LPVOID*)&pSetupChain, &cbSetupChain);
    ExitOnFailure(hr, "Failed to get manifest for setup chain.");

    for (BYTE i = 0; i < pSetupChain->cPackages; ++i)
    {
        const SETUP_INSTALL_PACKAGE* pPackage = pSetupChain->rgPackages + i;

        hr = StrAllocString(&pwzFullFilePath, pwzMsiOutputPath, 0);
        ExitOnFailure(hr, "Failed to allocate string and copy path for full file path.");

        hr = StrAllocConcat(&pwzFullFilePath, L"\\", 0);
        ExitOnFailure(hr, "Failed to concatenate filename into full file path.");

        hr = StrAllocConcat(&pwzFullFilePath, pPackage->wzFilename, 0);
        ExitOnFailure(hr, "Failed to concatenate filename into full file path.");

        hr = ResExportDataToFile(pPackage->szSource, pwzFullFilePath, CREATE_ALWAYS);
        ExitOnFailure1(hr, "Failed to write msi package to %S.", pwzFullFilePath);
    }

LExit:
    ReleaseStr(pwzFullFilePath);
    return hr;
}


HRESULT ExtractMst(
    __in const SETUP_INSTALL_TRANSFORM* pTransform,
    __out LPWSTR* ppwzTempMstPath
    )
{
    HRESULT hr = S_OK;
    DWORD cchTempMsiPath = MAX_PATH;

    // Drop the package into the temp directory.
    hr = StrAlloc(ppwzTempMstPath, cchTempMsiPath);
    ExitOnFailure(hr, "Failed to allocate string for temp path.");

    hr = DirCreateTempPath(L"MST", *ppwzTempMstPath, cchTempMsiPath);
    ExitOnFailure(hr, "Failed to generate a temporary filename.");

    hr = ResExportDataToFile(pTransform->szTransform, *ppwzTempMstPath, CREATE_ALWAYS);
    ExitOnFailure1(hr, "Failed to write MSI transform to %S.", *ppwzTempMstPath);

LExit:
    return hr;
}


void UISetupComplete(
    __in HRESULT hrStatus
    )
{
    HRESULT hr = S_OK;
    LPWSTR pwzProductName = NULL;
    LPWSTR pwzFormat = NULL;
    LPWSTR pwzDisplay = NULL;
    UINT uiType = MB_OK | MB_SETFOREGROUND;

    if (HRESULT_FROM_WIN32(ERROR_INSTALL_USEREXIT) == hrStatus)
    {
        ExitFunction();
    }

    hr = ResReadString(vhInstance, SETUP_RESOURCE_IDS_PRODUCTNAME, &pwzProductName);
    ExitOnFailure(hr, "Failed to load product name.");

    if (SUCCEEDED(hrStatus))
    {
        hr = ResReadString(vhInstance, IDS_SUCCESSFORMAT, &pwzFormat);
        ExitOnFailure(hr, "Failed to load success format.");

        uiType |= MB_ICONINFORMATION;
    }
    else if (HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_REQUIRED) == hrStatus ||
              HRESULT_FROM_WIN32(ERROR_SUCCESS_RESTART_REQUIRED) == hrStatus ||
              HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_INITIATED) == hrStatus)
    {
        hr = ResReadString(vhInstance, IDS_REBOOTFORMAT, &pwzFormat);
        ExitOnFailure(hr, "Failed to load reboot format.");

        uiType |= MB_ICONWARNING;
    }
    else
    {
        hr = ResReadString(vhInstance, IDS_FAILEDFORMAT, &pwzFormat);
        ExitOnFailure(hr, "Failed to load failed format.");

        uiType |= MB_ICONERROR;
    }

    hr = StrAllocFormatted(&pwzDisplay, pwzFormat, pwzProductName);
    ExitOnFailure1(hr, "Failed to set display with product name: %S", pwzProductName);

    ::MessageBoxW(NULL, pwzDisplay, pwzProductName, uiType);

LExit:
    ReleaseStr(pwzDisplay);
    ReleaseStr(pwzFormat);
    ReleaseStr(pwzProductName);

    return;
}

HRESULT UIUpdateInstallButtonState(
    __in HWND hwndDlg
    )
{
    HRESULT hr = S_OK;
    HWND hwndInstall = NULL;
    BOOL bAccepted = ::IsDlgButtonChecked(hwndDlg, IDC_CHECKACCEPT);

    hwndInstall = ::GetDlgItem(hwndDlg, IDC_INSTALL);
    ExitOnNullWithLastError(hwndInstall, hr, "Failed to get install button.");

    ::EnableWindow(hwndInstall, bAccepted);

LExit:
    return hr;
}


HRESULT UIInitDialog(
    __in HWND hwndDlg
    )
{
    HRESULT hr = S_OK;
    LPWSTR pwzProductName = NULL;
    LPWSTR pwzIntroductionFormat = NULL;
    LPWSTR pwzIntroduction = NULL;

    // First, enable/disable the Install button based on the state of
    // the license accept check box.
    UIUpdateInstallButtonState(hwndDlg);

    // Second, populate the license text box with the license.
    if (!::SetDlgItemText(hwndDlg, IDC_RTFLICENSE, vwzLicenseText))
    {
        hr = E_UNEXPECTED;
    }

    // Third, get the product name and update the dialog title with it.
    hr = ResReadString(vhInstance, SETUP_RESOURCE_IDS_PRODUCTNAME, &pwzProductName);
    ExitOnFailure(hr, "Failed to load product name.");

    if (!::SetWindowTextW(hwndDlg, pwzProductName))
    {
        ExitWithLastError1(hr, "Failed to set dialog title to product name: %S", pwzProductName);
    }

    // Finally, write the license agreement introduction.
    hr = ResReadString(vhInstance, IDS_INTRODUCTIONFORMAT, &pwzIntroductionFormat);
    ExitOnFailure(hr, "Failed to load introduction format.");

    hr = StrAllocFormatted(&pwzIntroduction, pwzIntroductionFormat, pwzProductName);
    ExitOnFailure1(hr, "Failed to set introduction with product name: %S", pwzProductName);

    if (!::SetDlgItemTextW(hwndDlg, IDC_INTRODUCTION, pwzIntroduction))
    {
        ExitWithLastError1(hr, "Failed to set introduction to product name: %S", pwzProductName);
    }

LExit:
    ReleaseStr(pwzIntroduction);
    ReleaseStr(pwzIntroductionFormat);
    ReleaseStr(pwzProductName);

    return hr;
}


INT_PTR CALLBACK UIDialogProc(
    __in HWND hwndDlg,
    __in UINT uMsg,
    __in WPARAM wParam,
    __in LPARAM lParam
    )
{
    UNREFERENCED_PARAMETER(lParam);

    HRESULT hr = S_OK;
    BOOL bProcessed = FALSE;

    switch (uMsg)
    {
        case WM_INITDIALOG:
            hr = UIInitDialog(hwndDlg);
            ExitOnFailure(hr, "Failed to initialize UI.");

            bProcessed = TRUE;
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
            case IDC_CHECKACCEPT:
                hr = UIUpdateInstallButtonState(hwndDlg);
                ExitOnFailure(hr, "Failed to update install button state.");

                bProcessed = TRUE;
                break;

            case IDC_INSTALL:
                ::EndDialog(hwndDlg, 0);

                bProcessed = TRUE;
                break;

            case IDCANCEL:
                ::EndDialog(hwndDlg, ERROR_INSTALL_USEREXIT);

                bProcessed = TRUE;
                break;
            }
    }

LExit:
    return bProcessed;
}


HRESULT DisplayLicenseText(
    __in LPCSTR wzLicenseText
    )
{
    HRESULT hr = S_OK;
    HMODULE richEd = NULL;
    INT_PTR iResult = 0;

    // Initialize common controls.
    ::InitCommonControls();

    // Load rich edit before trying to create the controls.
    hr = LoadSystemLibrary(L"Riched20.dll", &richEd);
    ExitOnFailure(hr, "Failed to load Rich Edit Control.");

    vwzLicenseText = wzLicenseText;

    // Create the dialog and show it.
    iResult = ::DialogBoxW(vhInstance, MAKEINTRESOURCEW(IDD_LICENSEDIALOG), NULL, UIDialogProc);
    if (-1 == iResult)
    {
        ExitWithLastError(hr, "Failed to create dialog box.");
    }

    hr = HRESULT_FROM_WIN32(iResult);

LExit:
    vwzLicenseText = NULL;
    return hr;
}


BOOL MatchLanguage(
    __in DWORD dwLang,
    __in DWORD dwTransformLang,
    __in BOOL fExact)
{
    BOOL fLanguageMatch = FALSE;
    if (INVALID_LCID == dwLang)
    {
        LANGID userLangID = ::GetUserDefaultUILanguage();
        dwLang = MAKELCID(userLangID, SORT_DEFAULT);
    }

    if (fExact)
    {
        fLanguageMatch = (dwLang == dwTransformLang);
    }
    else
    {
        // Convert Languages as appropriate
        if (0x04 == PRIMARYLANGID(dwLang))
        {
            switch(dwLang)
            {
            // Match Simplified Chinese
            case 0x0804: __fallthrough;
            case 0x1004:
                dwLang = 0x0804;
                break;

            // Match Traditional Chinese
            case 0x0404: __fallthrough;
            case 0x0c04: __fallthrough;
            case 0x1404:
                dwLang = 0x0404;
                break;
            }

            fLanguageMatch = (dwLang == dwTransformLang);
        }
        else
        {
            // Try best effort match to match primary language
            fLanguageMatch = (PRIMARYLANGID(dwLang) == PRIMARYLANGID(dwTransformLang));
        }
    }
    
    return fLanguageMatch;
}

HRESULT ProcessSetupChain(
    __in_opt LPCSTR wzLicenseText,
    __in_opt LPCWSTR wzMsiArgs,
    __in_opt DWORD dwLocaleId,
    __in_opt LPCWSTR wzMsiLog,
    __in DWORD dwUiLevel,
    __in CONFIGURE_ACTION action
    )
{
    HRESULT hr = S_OK;

    SETUP_INSTALL_CHAIN* pSetupChain = NULL;
    DWORD cbSetupChain = 0;

    SETUP_ROLLBACK_PACKAGE* prgRollbackPackages = NULL;
    BYTE cRollbackPackages = 0;

    LPWSTR pwzTempMsiPath = NULL;
    LPWSTR pwzTempMstPath = NULL;
    LPWSTR pwzCachedMsiPath = NULL;
    LPWSTR pwzCachedMstPath = NULL;
    LPWSTR pwzMsiArgsPackage = NULL;
    LPCWSTR wzMsiPath;
    LPVOID pvErrorMessage = NULL;

    // If we have a license to display, let's go do that.
    if (wzLicenseText)
    {
        hr = DisplayLicenseText(wzLicenseText);
        ExitOnFailure(hr, "Failed while displaying license text.");
    }

    hr = ResReadData(NULL, "MANIFEST", (LPVOID*)&pSetupChain, &cbSetupChain);
    ExitOnFailure(hr, "Failed to get manifest for setup chain.");

    // Create enough space to track rollback information for all possible packages in the chain.
    prgRollbackPackages = static_cast<SETUP_ROLLBACK_PACKAGE*>(MemAlloc(sizeof(SETUP_ROLLBACK_PACKAGE) * pSetupChain->cPackages, TRUE));
    ExitOnNull(prgRollbackPackages, hr, E_OUTOFMEMORY, "Failed to allocate rollback chain.");

    const SETUP_INSTALL_TRANSFORM* rgTransforms =
        static_cast<const SETUP_INSTALL_TRANSFORM*>((const void*)(pSetupChain->rgPackages + pSetupChain->cPackages));

    // First try perfect match
    for(BYTE i = 0; i < pSetupChain->cTransforms; ++i)
    {
        const SETUP_INSTALL_TRANSFORM* pTransform = rgTransforms + i;

        // Compare Locales
        if (MatchLanguage(dwLocaleId, pTransform->dwLocaleId, TRUE))
        {
            // Get the MST out of the setup.exe.
            hr = ExtractMst(pTransform, &pwzTempMstPath);
            ExitOnFailure(hr, "Failed to extract transform to temp path.");
        }
    }

    // Try best effort match to match some transform via Primary Language IDs
    if(!pwzTempMstPath)
    {
        // If we have transforms, extract them to temp path
        for(BYTE j = 0; j < pSetupChain->cTransforms; ++j)
        {
            const SETUP_INSTALL_TRANSFORM* pTransform = rgTransforms + j;

            // Compare Locales
            if (MatchLanguage(dwLocaleId, pTransform->dwLocaleId, FALSE))
            {
                // Get the MST out of the setup.exe.
                hr = ExtractMst(pTransform, &pwzTempMstPath);
                ExitOnFailure(hr, "Failed to extract transform to temp path.");
            }
        }
    }

    if (wzMsiLog)
    {
        //Try our best to delete the user specified log before we write to it
        ::DeleteFileW(wzMsiLog);
    }

    for (BYTE k = 0; k < pSetupChain->cPackages; ++k)
    {
        BOOL bDeleteTempPath = FALSE;
        DWORD dwUiLevelPackage = dwUiLevel;
        CONFIGURE_ACTION actionPackage = action;

        SETUP_INSTALL_PACKAGE* pPackage = pSetupChain->rgPackages + k;
        SETUP_ROLLBACK_PACKAGE* pRollbackPackage = prgRollbackPackages + cRollbackPackages;

        hr = StrAllocString(&pwzMsiArgsPackage, wzMsiArgs ? wzMsiArgs : L"", 0);
        ExitOnFailure(hr, "Failed to allocate string for MSI arguments.");

        hr = ::StringFromGUID2(pPackage->guidProductCode, pRollbackPackage->wzProductCode, countof(pRollbackPackage->wzProductCode));
        ExitOnFailure(hr, "Failed to convert ProductCode GUID to string.");

        if (pPackage->dwAttributes & SETUP_INSTALL_CHAIN_PATCH)
        {
            actionPackage = CONFIGURE_ACTION_PATCH;
        }
        else if (actionPackage != CONFIGURE_ACTION_UNINSTALL)
        {
            //
            // Try to detect the MSI to determine what action is necessary.
            //
            hr = DetectMsi(pRollbackPackage->wzProductCode, pPackage->dwVersionMajor, pPackage->dwVersionMinor, &actionPackage);
            ExitOnFailure(hr, "Failed during MSI detection.");
        }

        //
        // If there is no action, move on to the next package.  If we need to do a minor upgrade
        // but the package isn't configured to allow minor upgrades, bail
        //
        if (CONFIGURE_ACTION_NONE == actionPackage)
        {
            continue;
        }
        else if (CONFIGURE_ACTION_MINOR_UPGRADE == actionPackage && !(pPackage->dwAttributes & SETUP_INSTALL_CHAIN_MINOR_UPGRADE_ALLOWED))
        {
            hr = HRESULT_FROM_WIN32(ERROR_PRODUCT_VERSION);
            DWORD cchErrorMessage = ::FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&pvErrorMessage, 0, NULL);
            if (0 < cchErrorMessage)
            {
                ::MessageBoxExW(NULL, static_cast<PCWSTR>(pvErrorMessage), L"Install Error", MB_ICONERROR | MB_OK, 0);
            }
            ExitOnFailure(hr, "Cannot upgrade previously installed package.");
        }

        ++cRollbackPackages;

        // Get the MSI out of the setup.exe.
        hr = ExtractMsi(pPackage, &pwzTempMsiPath);
        ExitOnFailure(hr, "Failed to extract MSI to temp path.");

        bDeleteTempPath = TRUE; // extracted temp MSI path, so we'll need to clean it up at the end of this loop

        hr = ::StringCchCopyW(pRollbackPackage->wzTempPath, countof(pRollbackPackage->wzTempPath), pwzTempMsiPath);
        ExitOnFailure(hr, "Failed to copy temp path into rollback chain.");

        // If use transform is set for this package, and transform exists use transform
        if(pwzTempMstPath && (pPackage->dwAttributes & SETUP_INSTALL_CHAIN_USE_TRANSFORM))
        {
            // Use a secure-at-source transform to prevent breaking the MSI signature and enable source resiliency
            hr = StrAllocConcat(&pwzMsiArgsPackage, L" TRANSFORMS=@", 0);
            ExitOnFailure(hr, "Failed to append TRANSFORMS property to MSI args.");

            if (pPackage->dwAttributes & SETUP_INSTALL_CHAIN_CACHE)
            {
                // Cache the transform to our application cache.
                hr = CacheMst(pPackage, pwzTempMstPath, pPackage->dwAttributes & SETUP_INSTALL_CHAIN_ALLUSERS, &pwzCachedMstPath);
                ExitOnFailure(hr, "Failed to cache MST file.");

                // temp MST has been cached (moved), so we shouldn't delete the temp file any longer.
                ReleaseNullStr(pwzTempMstPath);
                
                // The transform will be next to the app cache directory
                hr = StrAllocConcat(&pwzMsiArgsPackage, FileFromPath(pwzCachedMstPath), 0);
                ExitOnFailure(hr, "Failed to append transform filename to MSI args.");
            }
            else
            {
                // The transform will be next to the MSI in the temp directory
                hr = StrAllocConcat(&pwzMsiArgsPackage, FileFromPath(pwzTempMstPath), 0);
                ExitOnFailure(hr, "Failed to append transform filename to MSI args.");
            }

        }

        if (pPackage->dwAttributes & SETUP_INSTALL_CHAIN_CACHE)
        {
            // Cache the package to our application cache.
            hr = CacheMsi(pPackage, pwzTempMsiPath, pPackage->dwAttributes & SETUP_INSTALL_CHAIN_ALLUSERS, &pwzCachedMsiPath);
            ExitOnFailure(hr, "Failed to cache MSI file.");

            bDeleteTempPath = FALSE; // temp MSI has been cached (moved), so we shouldn't delete the temp file any longer.

            wzMsiPath = pwzCachedMsiPath;

            hr = ::StringCchCopyW(pRollbackPackage->wzCachedPath, countof(pRollbackPackage->wzCachedPath), pwzCachedMsiPath);
            ExitOnFailure(hr, "Failed to copy cached path into rollback chain.");

            pRollbackPackage->bCached = TRUE;
        }
        else
        {
            wzMsiPath = pwzTempMsiPath;
        }

        if ((pPackage->dwAttributes & SETUP_INSTALL_CHAIN_SHOW_UI) == 0)
        {
            dwUiLevelPackage = INSTALLUILEVEL_BASIC;

            if (pPackage->dwAttributes & SETUP_INSTALL_CHAIN_IGNORE_FAILURES)
            {
                dwUiLevelPackage |= INSTALLUILEVEL_PROGRESSONLY; // don't throw any scary error dialogs
            }
        }

        // Install the cached package.
        hr = ConfigureMsi(pRollbackPackage->wzProductCode, pPackage->wzFilename, wzMsiPath, pwzMsiArgsPackage, wzMsiLog, actionPackage,
            dwUiLevelPackage, pPackage->dwAttributes & SETUP_INSTALL_CHAIN_IGNORE_FAILURES, 
            pPackage->dwAttributes & SETUP_INSTALL_CHAIN_PATCH_FORCE_TARGET);
        ExitOnFailure1(hr, "Failed to launch and wait for cached MSI: %ls", wzMsiPath);

        if (S_FALSE == hr)
        {
            if (pRollbackPackage->bCached)
            {
                ::DeleteFileW(pRollbackPackage->wzCachedPath);

                *pRollbackPackage->wzCachedPath = L'\0';
                pRollbackPackage->bCached = FALSE;
            }
        }
        else
        {
            pRollbackPackage->bInstalled = TRUE;
        }

        if (bDeleteTempPath)
        {
            ::DeleteFileW(pwzTempMsiPath);
        }
    }

LExit:
    if (FAILED(hr) &&
        HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_REQUIRED) != hr &&
        HRESULT_FROM_WIN32(ERROR_SUCCESS_RESTART_REQUIRED) != hr &&
        HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_INITIATED) != hr)
    {
        for (BYTE i = 0; i < cRollbackPackages; ++i)
        {
            const SETUP_ROLLBACK_PACKAGE* pRollbackPackage = prgRollbackPackages + i;

            if (pRollbackPackage->bInstalled)
            {
                ConfigureMsi(pRollbackPackage->wzProductCode, NULL, NULL, NULL, wzMsiLog, CONFIGURE_ACTION_UNINSTALL, INSTALLUILEVEL_BASIC, TRUE, FALSE);
            }

            if (*pRollbackPackage->wzCachedPath)
            {
                ::DeleteFileW(pRollbackPackage->wzCachedPath);
            }

            if (*pRollbackPackage->wzTempPath)
            {
                ::DeleteFileW(pRollbackPackage->wzTempPath);
            }
        }
    }

    if (pwzTempMstPath)
    {
        ::DeleteFileW(pwzTempMstPath);
    }

    if (prgRollbackPackages)
    {
        MemFree(prgRollbackPackages);
    }

    if (pvErrorMessage)
    {
        ::LocalFree(pvErrorMessage);
    }
    ReleaseStr(pwzTempMsiPath);
    ReleaseStr(pwzTempMstPath);
    ReleaseStr(pwzCachedMsiPath);
    ReleaseStr(pwzCachedMstPath);
    ReleaseStr(pwzMsiArgsPackage);

    //UISetupComplete(hr);

    return hr;
}


int WINAPI WinMain(
    __in HINSTANCE hInstance,
    __in_opt HINSTANCE hPrevInstance,
    __in LPSTR lpCmdLine,
    __in int nCmdShow
    )
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    HRESULT hr = S_OK;
    int argc = 0;
    LPWSTR *argv = NULL;

    BOOL bDropMsi = FALSE;
    BOOL bDropOnlyMsi = FALSE;
    BOOL bShowHelp = FALSE;
    LPCWSTR wzDropDir = NULL;
    LPWSTR wzMsiArgs = NULL;
    LPWSTR wzMsiLog = NULL;
    LPSTR wzLicenseText = NULL;
    LPWSTR wzProductName = NULL;
    LPWSTR wzHelp = NULL;
    DWORD cbLicenseText = 0;
    DWORD dwLocaleId = (DWORD)INVALID_LCID;
    DWORD dwUiLevel = INSTALLUILEVEL_FULL;

    CONFIGURE_ACTION action = CONFIGURE_ACTION_INSTALL; // assume this will be a simple install

    vhInstance = hInstance;

    argv = ::CommandLineToArgvW(::GetCommandLineW(), &argc);
    ExitOnNullWithLastError(argv, hr, "Failed to get command line.");

    //
    // Process command-line arguments.
    //
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == L'-' || argv[i][0] == L'/')
        {
            if ((CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"out", -1)) ||
                (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"extract", -1)))
            {
                if (i + 1 >= argc)
                {
                    ExitOnFailure(hr = E_INVALIDARG, "Must specify a filename with the /out switch.");
                }

                bDropMsi = TRUE;
                wzDropDir = argv[++i];
            }
            else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"o1", -1))
            {
                if (i + 1 >= argc)
                {
                    ExitOnFailure(hr = E_INVALIDARG, "Must specify a filename with the /o1 switch.");
                }

                bDropOnlyMsi = TRUE;
                wzDropDir = argv[++i];
            }
            else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"msicl", -1))
            {
                StrAllocConcat(&wzMsiArgs, argv[++i], 0);
                StrAllocConcat(&wzMsiArgs, L" ", 0);
            }
            else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"quiet", -1))
            {
                dwUiLevel = INSTALLUILEVEL_NONE;
            }
            else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"passive", -1))
            {
                dwUiLevel = INSTALLUILEVEL_BASIC;
            }
            else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"norestart", -1))
            {
                StrAllocConcat(&wzMsiArgs, L"REBOOT=ReallySuppress ", 0);
            }
            else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"forcerestart", -1))
            {
                StrAllocConcat(&wzMsiArgs, L"REBOOT=Force ", 0);
            }
            else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"promptrestart", -1))
            {
                StrAllocConcat(&wzMsiArgs, L"REBOOTPROMPT= ", 0);
            }
            else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"uninstall", -1))
            {
                if (INSTALLUILEVEL_FULL == dwUiLevel)
                {
                    dwUiLevel = INSTALLUILEVEL_BASIC;
                }
                action = CONFIGURE_ACTION_UNINSTALL;
            }
            else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"log", -1))
            {
                StrAllocString(&wzMsiLog, argv[++i], 0);
            }
            else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"lang", -1))
            {
                if (i + 1 >= argc)
                {
                    ExitOnFailure(hr = E_INVALIDARG, "Must specify a Locale ID with the /lang switch.");
                }
                dwLocaleId = _wtoi(argv[++i]);
            }
            else if ((CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"?", -1)) ||
                (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"h", -1)) ||
                (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"help", -1)))
            {               
                bShowHelp = TRUE;
            }
            else
            {
                ExitOnFailure1(hr = E_INVALIDARG, "Bad commandline argument: %S", argv[i]);
            }
        }
        else
        {
            ExitOnFailure1(hr = E_INVALIDARG, "Bad commandline argument: %S", argv[i]);
        }
    }

    //
    // Do what we came here to do.
    //
    if (bShowHelp)
    {
        hr = ResReadString(vhInstance, SETUP_RESOURCE_IDS_PRODUCTNAME, &wzProductName);
        ExitOnFailure(hr, "Failed to load product name.");
        hr = ResReadString(vhInstance, IDS_HELP, &wzHelp);
        ExitOnFailure(hr, "Failed to load help.");

        ::MessageBoxW(NULL, wzHelp, wzProductName, MB_OK);
    }
    else if (bDropMsi)
    {
        hr = ExtractMsiToDir(wzDropDir);
        ExitOnFailure(hr, "Failure in extracting msi.");
    }
    else if (bDropOnlyMsi)
    {
        SETUP_INSTALL_CHAIN* pSetupChain = NULL;
        DWORD cbSetupChain = 0;

        hr = ResReadData(NULL, "MANIFEST", (LPVOID*)&pSetupChain, &cbSetupChain);
        ExitOnFailure(hr, "Failed to get manifest for setup chain.");

        const SETUP_INSTALL_PACKAGE* pPackage = pSetupChain->rgPackages + 0;

        hr = ResExportDataToFile(pPackage->szSource, wzDropDir, CREATE_ALWAYS);
        ExitOnFailure1(hr, "Failed to write msi package to %S.", wzDropDir);
    }
    else // Default is to install the package.
    {
        hr = ResReadData(NULL, MAKEINTRESOURCE(SETUP_RESOURCE_IDS_LICENSETEXT), (LPVOID*)&wzLicenseText, &cbLicenseText);
        if (HRESULT_FROM_WIN32(ERROR_RESOURCE_NAME_NOT_FOUND) == hr)
        {
            // No license is okay.
            AssertSz(!wzLicenseText, "License text should be null if the resource was not found.");
            hr = S_OK;
        }
        ExitOnFailure(hr, "Failed to load license text.");

        hr = ProcessSetupChain(wzLicenseText, wzMsiArgs, dwLocaleId, wzMsiLog, dwUiLevel, action);
        ExitOnFailure(hr, "Failed to process setup chain.");
    }

LExit:
    // UISetupComplete(hr);
    ReleaseStr(wzMsiArgs);
    ReleaseStr(wzProductName);
    ReleaseStr(wzHelp);
    ReleaseStr(wzMsiLog);

    return SCODE_CODE(hr);
}
