//-------------------------------------------------------------------------------------------------
// <copyright file="mqutilexec.cpp" company="Microsoft">
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
//    MSMQ Custom Action utility functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"


// function definitions

HRESULT PcaGuidToRegFormat(
    LPWSTR pwzGuid,
    LPWSTR pwzDest,
    SIZE_T cchDest
    )
{
    HRESULT hr = S_OK;

    GUID guid = GUID_NULL;
    int cch = 0;

    WCHAR wz[39];
    ::ZeroMemory(wz, sizeof(wz));

    cch = lstrlenW(pwzGuid);

    if (38 == cch && L'{' == pwzGuid[0] && L'}' == pwzGuid[37])
        StringCchCopyW(wz, countof(wz), pwzGuid);
    else if (36 == cch)
        StringCchPrintfW(wz, countof(wz), L"{%s}", pwzGuid);
    else
        ExitFunction1(hr = E_INVALIDARG);

    // convert string to guid
    hr = ::CLSIDFromString(wz, &guid);
    ExitOnFailure(hr, "Failed to parse guid string");

    // convert guid to string
    if (0 == ::StringFromGUID2(guid, pwzDest, cchDest))
        ExitOnFailure(hr = E_FAIL, "Failed to convert guid to string");

    hr = S_OK;

LExit:
    return hr;
}
