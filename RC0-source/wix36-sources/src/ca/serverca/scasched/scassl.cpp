//-------------------------------------------------------------------------------------------------
// <copyright file="scassl.cpp" company="Microsoft">
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
//    IIS SSL functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

enum eSslCertificateQuery { scqStoreName = 1, scqHash, scqWeb };

static HRESULT AddSslCertificateToList(
    __in SCA_WEB_SSL_CERTIFICATE** ppswscList
    );


HRESULT ScaSslCertificateRead(
    __in LPCWSTR wzWebId,
    __in WCA_WRAPQUERY_HANDLE hSslCertQuery,
    __inout SCA_WEB_SSL_CERTIFICATE** ppswscList
    )
{
    HRESULT hr = S_OK;

    MSIHANDLE hRec;
    SCA_WEB_SSL_CERTIFICATE* pswsc = NULL;
    LPWSTR pwzData = NULL;

    WcaFetchWrappedReset(hSslCertQuery);

    // Get the certificate information.
    while (S_OK == (hr = WcaFetchWrappedRecordWhereString(hSslCertQuery, scqWeb, wzWebId, &hRec)))
    {
        hr = AddSslCertificateToList(ppswscList);
        ExitOnFailure(hr, "failed to add ssl certificate to list");

        pswsc = *ppswscList;

        hr = WcaGetRecordString(hRec, scqStoreName, &pwzData);
        ExitOnFailure(hr, "Failed to get web ssl certificate store name.");

        hr = ::StringCchCopyW(pswsc->wzStoreName, countof(pswsc->wzStoreName), pwzData);
        ExitOnFailure(hr, "Failed to copy web ssl certificate store name.");

        hr = WcaGetRecordString(hRec, scqHash, &pwzData);
        ExitOnFailure(hr, "Failed to get hash for web ssl certificate.");

        hr = StrHexDecode(pwzData, pswsc->rgbSHA1Hash, countof(pswsc->rgbSHA1Hash));
        ExitOnFailure2(hr, "Failed to decode certificate hash for web: %ls, data: %ls", wzWebId, pwzData);
    }

    if (E_NOMOREITEMS == hr)
    {
        hr = S_OK;
    }
    ExitOnFailure(hr, "Failed to read IIsWebSiteCertificates table.");

LExit:
    ReleaseStr(pwzData);
    return hr;
}


HRESULT ScaSslCertificateWriteMetabase(
    __in IMSAdminBase* piMetabase,
    __in LPCWSTR wzWebBase,
    __in SCA_WEB_SSL_CERTIFICATE* pswscList
    )
{
    HRESULT hr = S_OK;
    BLOB blob;

    for (SCA_WEB_SSL_CERTIFICATE* pswsc = pswscList; pswsc; pswsc = pswsc->pNext)
    {
        // Write: /W3SVC/1:SslCertStoreName = "MY", "CA", "Root", etc.
        hr = ScaWriteMetabaseValue(piMetabase, wzWebBase, L"", MD_SSL_CERT_STORE_NAME, METADATA_INHERIT, IIS_MD_UT_SERVER, STRING_METADATA, static_cast<LPVOID>(pswsc->wzStoreName));
        ExitOnFailure(hr, "Failed to write SslCertStoreName");

        // Write: /W3SVC/1:SslCertHash = <blob>
        blob.pBlobData = pswsc->rgbSHA1Hash;
        blob.cbSize = countof(pswsc->rgbSHA1Hash);
        hr = ScaWriteMetabaseValue(piMetabase, wzWebBase, L"", MD_SSL_CERT_HASH, METADATA_INHERIT, IIS_MD_UT_SERVER, BINARY_METADATA, static_cast<LPVOID>(&blob));
        ExitOnFailure(hr, "Failed to write SslCertHash");
    }

LExit:
    return hr;
}


void ScaSslCertificateFreeList(
    __in SCA_WEB_SSL_CERTIFICATE* pswscList
    )
{
    SCA_WEB_SSL_CERTIFICATE* pswscDelete = pswscList;
    while (pswscList)
    {
        pswscDelete = pswscList;
        pswscList = pswscList->pNext;

        MemFree(pswscDelete);
    }
}


static HRESULT AddSslCertificateToList(
    __in SCA_WEB_SSL_CERTIFICATE** ppswscList
    )
{
    HRESULT hr = S_OK;

    SCA_WEB_SSL_CERTIFICATE* pswsc = static_cast<SCA_WEB_SSL_CERTIFICATE*>(MemAlloc(sizeof(SCA_WEB_SSL_CERTIFICATE), TRUE));
    ExitOnNull(pswsc, hr, E_OUTOFMEMORY, "failed to allocate memory for new SSL certificate list element");

    pswsc->pNext = *ppswscList;
    *ppswscList = pswsc;

LExit:
    return hr;
}
