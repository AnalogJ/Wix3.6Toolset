//-------------------------------------------------------------------------------------------------
// <copyright file="catalog.h" company="Microsoft">
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
//    Module: Core
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


#if defined(__cplusplus)
extern "C" {
#endif

// structs

typedef struct _BURN_CATALOG
{
    LPWSTR sczKey;
    LPWSTR sczPayload;

    // mutable members
    LPWSTR sczLocalFilePath; // location of extracted or downloaded copy
    HANDLE hFile;
} BURN_CATALOG;

typedef struct _BURN_CATALOGS
{
    BURN_CATALOG* rgCatalogs;
    DWORD cCatalogs;
} BURN_CATALOGS;

typedef struct _BURN_PAYLOADS BURN_PAYLOADS;


// functions

HRESULT CatalogsParseFromXml(
    __in BURN_CATALOGS* pCatalogs,
    __in IXMLDOMNode* pixnBundle
    );
HRESULT CatalogFindById(
    __in BURN_CATALOGS* pCatalogs,
    __in_z LPCWSTR wzId,
    __out BURN_CATALOG** ppCatalog
    );
HRESULT CatalogLoadFromPayload(
    __in BURN_CATALOGS* pCatalogs,
    __in BURN_PAYLOADS* pPayloads
    );
HRESULT CatalogElevatedUpdateCatalogFile(
    __in BURN_CATALOGS* pCatalogs,
    __in_z LPCWSTR wzId,
    __in_z LPCWSTR wzPath
    );
void CatalogUninitialize(
    __in BURN_CATALOGS* pCatalogs
    );

#if defined(__cplusplus)
}
#endif
