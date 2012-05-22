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

// prototype
static HRESULT UpdateResourceFromFile(
    __in HANDLE hResourceUpdate,
    __in LPCSTR szResourceType,
    __in LPCSTR szResourceId,
    __in WORD wLanguage,
    __in LPCWSTR wzFilePath
    );

static HRESULT GetProductCodeAndVersionAndAllUsers(
    __in LPCWSTR wzMsiPath,
    __out_ecount(cchProductCode) LPWSTR wzProductCode,
    __in DWORD cchProductCode,
    __out_ecount(cchProductVersion) LPWSTR wzProductVersion,
    __in DWORD cchProductVersion,
    __out int* piAllUsers
    );


extern "C" BOOL WINAPI DllMain(
    __in HINSTANCE hInst,
    __in ULONG ulReason,
    __in LPVOID
    ) throw()
{
    UNREFERENCED_PARAMETER(hInst);

    switch(ulReason)
    {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }

    return TRUE;
}


extern "C" HRESULT CreateSetup(
    __in LPCWSTR wzSetupStub,
    __in_opt LPCWSTR wzProductName,
    __in_opt LPCWSTR wzLicensePath,
    __in_opt LPCWSTR wzBitmapPath,
    __in_ecount(cPackages) const CREATE_SETUP_PACKAGE rgPackages[],
    __in_ecount_opt(cTransforms) const CREATE_SETUP_TRANSFORMS rgTransforms[],
    __in BYTE cPackages,
    __in BYTE cTransforms,
    __in LPCWSTR wzOutput
    )
{
    HRESULT hr = S_OK;
    SETUP_INSTALL_CHAIN* pSetupChain = NULL;
    DWORD cbSetupChain = 0;
    HANDLE hUpdate = NULL;
    LPSTR pszProductName = NULL;

    if (0 == cPackages)
    {
        ExitFunction();
    }

    //
    // Calculate the total size of the setup chain.  That is take the size of the header then add the size of the packages
    // for the total number of packages (minus one because the SETUP_INSTALL_CHA__in comes with a package).
    //
    cbSetupChain = sizeof(SETUP_INSTALL_CHAIN) + (sizeof(SETUP_INSTALL_PACKAGE) * (cPackages - 1)) + (sizeof(SETUP_INSTALL_TRANSFORM) * cTransforms);

    pSetupChain = (SETUP_INSTALL_CHAIN*)MemAlloc(cbSetupChain, TRUE);
    ExitOnNull(pSetupChain, hr, E_OUTOFMEMORY, "Failed to allocate complete setup chain.");

    if (!::CopyFileW(wzSetupStub, wzOutput, FALSE))
    {
      ExitWithLastError1(hr, "Failed to copy setup.exe stub to output: %S.", wzOutput);
    }

    hUpdate = ::BeginUpdateResourceW(wzOutput, FALSE);
    ExitOnNullWithLastError(hUpdate, hr, "Failed to ::BeginUpdateResourcesW.");

    pSetupChain->dwRevision = 0;
    pSetupChain->cPackages = cPackages;
    SETUP_INSTALL_PACKAGE* pPackage = pSetupChain->rgPackages;
    for (BYTE i = 0; i < cPackages; ++i)
    {
        hr = ::StringCchPrintfA(pPackage->szSource, countof(pPackage->szSource), "MSI%02d", i);
        ExitOnFailure(hr, "Failed to generate package resource identifier.");

        pPackage->dwAttributes = 0;

        if (rgPackages[i].fPatch)
        {
            ZeroMemory(&pPackage->guidProductCode, sizeof(pPackage->guidProductCode));
            pPackage->dwVersionMajor = 0;
            pPackage->dwVersionMinor = 0;
            pPackage->dwAttributes |= SETUP_INSTALL_CHAIN_PATCH;
            pPackage->dwAttributes |= rgPackages[i].fIgnoreFailures ? SETUP_INSTALL_CHAIN_IGNORE_FAILURES : 0;
            pPackage->dwAttributes |= rgPackages[i].fPatchForceTarget ? SETUP_INSTALL_CHAIN_PATCH_FORCE_TARGET : 0;
        }
        else
        {
            WCHAR wzProductCode[39];
            WCHAR wzProductVersion[36];
            int iAllUsers = 0;

            hr = GetProductCodeAndVersionAndAllUsers(rgPackages[i].wzSourcePath, wzProductCode, countof(wzProductCode), wzProductVersion, countof(wzProductVersion), &iAllUsers);
            ExitOnFailure1(hr, "Failed to get product code and version from MSI: %S", rgPackages[i].wzSourcePath);

            hr = ::IIDFromString(wzProductCode, &pPackage->guidProductCode);
            ExitOnFailure(hr, "Failed to convert product code to GUID.");

            hr = FileVersionFromString(wzProductVersion, &pPackage->dwVersionMajor, &pPackage->dwVersionMinor);
            ExitOnFailure(hr, "Failed to get version from string.");

            pPackage->dwAttributes |= iAllUsers == 1 ? SETUP_INSTALL_CHAIN_ALLUSERS : 0;
            pPackage->dwAttributes |= rgPackages[i].fPrivileged ? SETUP_INSTALL_CHAIN_PRIVILEGED : 0;
            pPackage->dwAttributes |= rgPackages[i].fCache ? SETUP_INSTALL_CHAIN_CACHE : 0;
            pPackage->dwAttributes |= rgPackages[i].fShowUI ? SETUP_INSTALL_CHAIN_SHOW_UI : 0;
            pPackage->dwAttributes |= rgPackages[i].fIgnoreFailures ? SETUP_INSTALL_CHAIN_IGNORE_FAILURES : 0;
            pPackage->dwAttributes |= rgPackages[i].fMinorUpgrade ? SETUP_INSTALL_CHAIN_MINOR_UPGRADE_ALLOWED : 0;
            pPackage->dwAttributes |= rgPackages[i].fLink ? SETUP_INSTALL_CHAIN_LINK : 0;
            pPackage->dwAttributes |= rgPackages[i].fUseTransform ? SETUP_INSTALL_CHAIN_USE_TRANSFORM : 0;
        }

        hr = ::StringCchCopyW(pPackage->wzFilename, countof(pPackage->wzFilename), FileFromPath(rgPackages[i].wzSourcePath));
        ExitOnFailure(hr, "Failed to copy filename into setup chain.");

        hr = UpdateResourceFromFile(hUpdate, RT_RCDATA, pPackage->szSource, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), rgPackages[i].wzSourcePath);
        ExitOnFailure1(hr, "Failed to update resource from path: %S", rgPackages[i].wzSourcePath);

        ++pPackage;
    }

    pSetupChain->cTransforms = cTransforms;
    SETUP_INSTALL_TRANSFORM* pTransform = static_cast<SETUP_INSTALL_TRANSFORM*>((void*)pPackage);
    for (BYTE i = 0; i < cTransforms; ++i)
    {
        hr= ::StringCchPrintfA(pTransform->szTransform, countof(pTransform->szTransform), "MST%02d", i);
        ExitOnFailure(hr, "Failed to generate transform resource identifier.");

        pTransform->dwLocaleId = rgTransforms[i].dwLocaleId;

        hr = UpdateResourceFromFile(hUpdate, RT_RCDATA, pTransform->szTransform, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), rgTransforms[i].wzSourcePath);
        ExitOnFailure1(hr, "Failed to update resource from path: %S", rgPackages[i].wzSourcePath);

        ++pTransform;
    }

    if (!::UpdateResourceA(hUpdate, RT_RCDATA, "MANIFEST", MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), pSetupChain, cbSetupChain))
    {
        ExitWithLastError(hr, "Failed to ::UpdateResource.");
    }

    if (wzLicensePath && *wzLicensePath)
    {
        hr = UpdateResourceFromFile(hUpdate, RT_RCDATA, MAKEINTRESOURCE(SETUP_RESOURCE_IDS_LICENSETEXT), MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), wzLicensePath);
        ExitOnFailure1(hr, "Failed to update resource with license file: %S", wzLicensePath);
    }

    if (wzBitmapPath && *wzBitmapPath)
    {
        hr = UpdateResourceFromFile(hUpdate, RT_BITMAP, MAKEINTRESOURCE(SETUP_RESOURCE_IDB_BACKGROUND), MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), wzBitmapPath);
        ExitOnFailure1(hr, "Failed to update resource with background bitmap: %S", wzBitmapPath);
    }

    if (!::EndUpdateResource(hUpdate, FALSE))
    {
        ExitWithLastError(hr, "Failed to ::EndUpdateResource.");
    }

    hUpdate = NULL;
    
    if (wzProductName && *wzProductName)
    {
        hr = ResWriteString(wzOutput, SETUP_RESOURCE_IDS_PRODUCTNAME, wzProductName, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
        ExitOnFailure(hr, "Failed to set product string");
    }

LExit:
    ReleaseStr(pszProductName);

    if (hUpdate)
    {
        ::EndUpdateResource(hUpdate, TRUE);
    }

    if (FAILED(hr))
    {
        ::DeleteFileW(wzOutput);
    }

    if (pSetupChain)
    {
        MemFree(pSetupChain);
    }

    return hr;
}


extern "C" HRESULT CreateSimpleSetup(
    __in LPCWSTR wzSetupStub,
    __in LPCWSTR wzMsi,
    __in LPCWSTR wzOutput
    )
{
    HRESULT hr = S_OK;

    CREATE_SETUP_PACKAGE createSetupPackage[1];
    createSetupPackage[0].wzSourcePath = wzMsi;
    createSetupPackage[0].fPrivileged = FALSE;
    createSetupPackage[0].fCache = TRUE;

    hr = CreateSetup(wzSetupStub, NULL, NULL, NULL, createSetupPackage, NULL, countof(createSetupPackage), 0, wzOutput);
    ExitOnFailure(hr, "Failed to CreateSetup.");

LExit:
    return hr;
}


static HRESULT UpdateResourceFromFile(
    __in HANDLE hResourceUpdate,
    __in LPCSTR szResourceType,
    __in LPCSTR szResourceId,
    __in WORD wLanguage,
    __in LPCWSTR wzFilePath
    )
{
    HRESULT hr = S_OK;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD cbFile = 0;
    HANDLE hMap = NULL;
    PVOID pvFile = NULL;

    hFile = ::CreateFileW(wzFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        ExitWithLastError1(hr, "Failed to open file: %S.", wzFilePath);
    }

    cbFile = ::GetFileSize(hFile, NULL);
    if (!cbFile)
    {
        ExitWithLastError1(hr, "Failed to get file size for file: %S.", wzFilePath);
    }

    hMap = ::CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    ExitOnNullWithLastError1(hMap, hr, "Failed to CreateFileMapping for %S.", wzFilePath);

    pvFile = ::MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, cbFile);
    ExitOnNullWithLastError1(pvFile, hr, "Failed to MapViewOfFile for %S.", wzFilePath);

    //pvFile = (LPVOID)((BYTE*)pvFile + sizeof(WORD));
    //cbFile -= sizeof(WORD);

    if (!::UpdateResourceA(hResourceUpdate, szResourceType, szResourceId, wLanguage, pvFile, cbFile))
    {
        ExitWithLastError1(hr, "Failed to insert file: %S.", wzFilePath);
    }

LExit:
    if (pvFile)
    {
        ::UnmapViewOfFile(pvFile);
        pvFile = NULL;
    }

    if (hMap)
    {
        ::CloseHandle(hMap);
        hMap = NULL;
    }

    if (INVALID_HANDLE_VALUE != hFile)
    {
        ::CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
    }

    return hr;
}


static HRESULT GetProductCodeAndVersionAndAllUsers(
    __in LPCWSTR wzMsiPath,
    __out_ecount(cchProductCode) LPWSTR wzProductCode,
    __in DWORD cchProductCode,
    __out_ecount(cchProductVersion) LPWSTR wzProductVersion,
    __in DWORD cchProductVersion,
    __out int* piAllUsers
    )
{
    Assert(wzMsiPath);
    Assert(wzProductCode);
    Assert(wzProductVersion);
    Assert(piAllUsers);

    DWORD er = ERROR_SUCCESS;
    HRESULT hr = S_OK;

    PMSIHANDLE hDatabase;
    PMSIHANDLE hView;
    PMSIHANDLE hRec;

    // open a view on the Property table.
    er = ::MsiOpenDatabaseW(wzMsiPath, (LPCWSTR)MSIDBOPEN_READONLY, &hDatabase);
    hr = HRESULT_FROM_WIN32(er);
    ExitOnFailure(hr, "Failed to open MSI database");

    er = ::MsiDatabaseOpenViewW(hDatabase, L"SELECT `Value` FROM `Property` WHERE `Property`=?", &hView);
    hr = HRESULT_FROM_WIN32(er);
    ExitOnFailure(hr, "failed to open view on Property table.");

    hRec = ::MsiCreateRecord(1);
    ExitOnNull(hRec, hr, E_OUTOFMEMORY, "Failed to create record.");

    // Get the ProductCode.
    er = ::MsiRecordSetStringW(hRec, 1, L"ProductCode");
    hr = HRESULT_FROM_WIN32(er);
    ExitOnFailure(hr, "Failed to set string 'ProductCode' into record.");

    er = ::MsiViewExecute(hView, hRec);
    hr = HRESULT_FROM_WIN32(er);
    ExitOnFailure(hr, "failed to execute view to get ProductCode.");

    er = ::MsiViewFetch(hView, &hRec);
    hr = HRESULT_FROM_WIN32(er);
    ExitOnFailure(hr, "failed to fetch 'ProductCode' from view.");

    er = ::MsiRecordGetStringW(hRec, 1, wzProductCode, &cchProductCode);
    hr = HRESULT_FROM_WIN32(er);
    ExitOnFailure(hr, "failed to fetch 'ProductCode' from record.");

    // Get the ProductVersion.
    er = ::MsiRecordSetStringW(hRec, 1, L"ProductVersion");
    hr = HRESULT_FROM_WIN32(er);
    ExitOnFailure(hr, "Failed to set string 'ProductVersion' into record.");

    er = ::MsiViewExecute(hView, hRec);
    hr = HRESULT_FROM_WIN32(er);
    ExitOnFailure(hr, "failed to execute view to get ProductVersion");

    er = ::MsiViewFetch(hView, &hRec);
    hr = HRESULT_FROM_WIN32(er);
    ExitOnFailure(hr, "failed to fetch 'ProductVersion' from view.");

    er = ::MsiRecordGetStringW(hRec, 1, wzProductVersion, &cchProductVersion);
    hr = HRESULT_FROM_WIN32(er);
    ExitOnFailure(hr, "failed to fetch 'ProductVersion' from record.");

    // Get the ALLUSERS.
    er = ::MsiRecordSetStringW(hRec, 1, L"ALLUSERS");
    hr = HRESULT_FROM_WIN32(er);
    ExitOnFailure(hr, "Failed to set string 'ALLUSERS' into record.");

    er = ::MsiViewExecute(hView, hRec);
    hr = HRESULT_FROM_WIN32(er);
    ExitOnFailure(hr, "failed to execute view to get 'ALLUSERS' property");

    er = ::MsiViewFetch(hView, &hRec);
    if (ERROR_NO_MORE_ITEMS == er)
    {
        *piAllUsers = 0;
        ExitFunction1(hr = S_OK);
    }
    hr = HRESULT_FROM_WIN32(er);
    ExitOnFailure(hr, "failed to fetch 'ALLUSERS' from view.");

    hr = S_OK;
    *piAllUsers = ::MsiRecordGetInteger(hRec, 1);
    if (MSI_NULL_INTEGER == *piAllUsers)
    {
        *piAllUsers = 0;
    }

LExit:
    return hr;
}
