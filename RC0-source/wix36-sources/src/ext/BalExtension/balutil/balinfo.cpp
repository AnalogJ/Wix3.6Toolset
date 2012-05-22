//-------------------------------------------------------------------------------------------------
// <copyright file="balinfo.cpp" company="Microsoft">
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
// Bootstrapper Application Layer information utility.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

// prototypes
static HRESULT ParsePackagesFromXml(
    __in BAL_INFO_PACKAGES* pPackages,
    __in IXMLDOMDocument* pixdManifest
    );


DAPI_(HRESULT) BalInfoParseFromXml(
    __in BAL_INFO_BUNDLE* pBundle,
    __in IXMLDOMDocument* pixdManifest
    )
{
    HRESULT hr = S_OK;
    IXMLDOMNode* pNode = NULL;

    hr = XmlSelectSingleNode(pixdManifest, L"/BootstrapperApplicationData/WixBundleProperties", &pNode);
    if (S_OK == hr)
    {
        hr = XmlGetAttributeEx(pNode, L"DisplayName", &pBundle->sczName);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to read bundle information display name.");
        }

        hr = XmlGetAttributeEx(pNode, L"LogPathVariable", &pBundle->sczLogVariable);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to read bundle information log path variable.");
        }
    }
    ExitOnFailure(hr, "Failed to select bundle information.");

    hr = ParsePackagesFromXml(&pBundle->packages, pixdManifest);
    BalExitOnFailure(hr, "Failed to parse package information from bootstrapper application data.");

LExit:
    ReleaseObject(pNode);

    return hr;
}


DAPI_(HRESULT) BalInfoFindPackageById(
    __in BAL_INFO_PACKAGES* pPackages,
    __in LPCWSTR wzId,
    __out BAL_INFO_PACKAGE** ppPackage
    )
{
    *ppPackage = NULL;

    for (DWORD i = 0; i < pPackages->cPackages; ++i)
    {
        if (CSTR_EQUAL == ::CompareStringW(LOCALE_NEUTRAL, 0, wzId, -1, pPackages->rgPackages[i].sczId, -1))
        {
            *ppPackage = pPackages->rgPackages + i;
            break;
        }
    }

    return *ppPackage ? S_OK : E_NOTFOUND;
}


DAPI_(void) BalInfoUninitialize(
    __in BAL_INFO_BUNDLE* pBundle
    )
{
    for (DWORD i = 0; i < pBundle->packages.cPackages; ++i)
    {
        ReleaseStr(pBundle->packages.rgPackages[i].sczDisplayName);
        ReleaseStr(pBundle->packages.rgPackages[i].sczDescription);
        ReleaseStr(pBundle->packages.rgPackages[i].sczId);
    }

    ReleaseMem(pBundle->packages.rgPackages);

    ReleaseStr(pBundle->sczName);
    ReleaseStr(pBundle->sczLogVariable);
    memset(pBundle, 0, sizeof(BAL_INFO_BUNDLE));
}


static HRESULT ParsePackagesFromXml(
    __in BAL_INFO_PACKAGES* pPackages,
    __in IXMLDOMDocument* pixdManifest
    )
{
    HRESULT hr = S_OK;
    IXMLDOMNodeList* pNodeList = NULL;
    IXMLDOMNode* pNode = NULL;
    BAL_INFO_PACKAGE* prgPackages = NULL;
    DWORD cPackages = 0;
    LPWSTR sczType = NULL;

    hr = XmlSelectNodes(pixdManifest, L"/BootstrapperApplicationData/WixPackageProperties", &pNodeList);
    ExitOnFailure(hr, "Failed to select all packages.");

    hr = pNodeList->get_length(reinterpret_cast<long*>(&cPackages));
    ExitOnFailure(hr, "Failed to get the package count.");

    prgPackages = static_cast<BAL_INFO_PACKAGE*>(MemAlloc(sizeof(BAL_INFO_PACKAGE) * cPackages, TRUE));
    ExitOnNull(prgPackages, hr, E_OUTOFMEMORY, "Failed to allocate memory for packages.");

    DWORD iPackage = 0;
    while (S_OK == (hr = XmlNextElement(pNodeList, &pNode, NULL)))
    {
        hr = XmlGetAttributeEx(pNode, L"Package", &prgPackages[iPackage].sczId);
        ExitOnFailure(hr, "Failed to get package identifier for package.");

        hr = XmlGetAttributeEx(pNode, L"DisplayName", &prgPackages[iPackage].sczDisplayName);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to get description for package.");
        }

        hr = XmlGetAttributeEx(pNode, L"Description", &prgPackages[iPackage].sczDescription);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to get description for package.");
        }

        hr = XmlGetAttributeEx(pNode, L"PackageType", &sczType);
        ExitOnFailure(hr, "Failed to get description for package.");

        if (CSTR_EQUAL == ::CompareStringW(LOCALE_NEUTRAL, 0, L"Exe", -1, sczType, -1))
        {
            prgPackages[iPackage].type = BAL_INFO_PACKAGE_TYPE_EXE;
        }
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_NEUTRAL, 0, L"Msi", -1, sczType, -1))
        {
            prgPackages[iPackage].type = BAL_INFO_PACKAGE_TYPE_MSI;
        }
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_NEUTRAL, 0, L"Msp", -1, sczType, -1))
        {
            prgPackages[iPackage].type = BAL_INFO_PACKAGE_TYPE_MSP;
        }
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_NEUTRAL, 0, L"Msu", -1, sczType, -1))
        {
            prgPackages[iPackage].type = BAL_INFO_PACKAGE_TYPE_MSU;
        }

        hr = XmlGetYesNoAttribute(pNode, L"Permanent", &prgPackages[iPackage].fPermanent);
        ExitOnFailure(hr, "Failed to get permanent setting for package.");

        hr = XmlGetYesNoAttribute(pNode, L"Vital", &prgPackages[iPackage].fVital);
        ExitOnFailure(hr, "Failed to get vital setting for package.");

        ++iPackage;
        ReleaseNullObject(pNode);
    }
    ExitOnFailure(hr, "Failed to parse all package property elements.");

    if (S_FALSE == hr)
    {
        hr = S_OK;
    }

    pPackages->cPackages = cPackages;
    pPackages->rgPackages = prgPackages;
    prgPackages = NULL;

LExit:
    ReleaseStr(sczType);
    ReleaseMem(prgPackages);
    ReleaseObject(pNode);
    ReleaseObject(pNodeList);

    return hr;
}