//-------------------------------------------------------------------------------------------------
// <copyright file="scaweberr7.cpp" company="Microsoft">
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
//    IIS Web Error functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

HRESULT ScaWriteWebError7(
    __in_z LPCWSTR wzWebName,
    __in_z LPCWSTR wzRoot,
    SCA_WEB_ERROR* psweList
    )
{
    HRESULT hr = S_OK;

    hr = ScaWriteConfigID(IIS_WEBERROR_BEGIN);
    ExitOnFailure(hr, "Fail to write webError begin ID");

    hr = ScaWriteConfigString(wzWebName);
    ExitOnFailure(hr, "Fail to write webError Web Key");

    hr = ScaWriteConfigString(wzRoot);
    ExitOnFailure(hr, "Fail to write webError Vdir key");

    // Loop through the HTTP headers
    for (SCA_WEB_ERROR* pswe = psweList; pswe; pswe = pswe->psweNext)
    {
        hr = ScaWriteConfigID(IIS_WEBERROR);
        ExitOnFailure(hr, "Fail to write webError ID");

        hr = ScaWriteConfigInteger(pswe->iErrorCode);
        ExitOnFailure(hr, "Fail to write webError code");

        hr = ScaWriteConfigInteger(pswe->iSubCode);
        ExitOnFailure(hr, "Fail to write webError subcode");

        //just write one
        if (*(pswe->wzFile))
        {
            hr = ScaWriteConfigString(pswe->wzFile);
            ExitOnFailure(hr, "Fail to write webError file");
            hr = ScaWriteConfigInteger(0);
            ExitOnFailure(hr, "Fail to write webError file code");
        }
        else if (*(pswe->wzURL))
        {
            hr = ScaWriteConfigString(pswe->wzURL);
            ExitOnFailure(hr, "Fail to write webError URL");
            hr = ScaWriteConfigInteger(1);
            ExitOnFailure(hr, "Fail to write webError URL code");
        }
    }

    hr = ScaWriteConfigID(IIS_WEBERROR_END);
    ExitOnFailure(hr, "Fail to write httpHeader end ID");

LExit:
    return hr;

}

static HRESULT AddWebErrorToList(SCA_WEB_ERROR** ppsweList)
{
    HRESULT hr = S_OK;

    SCA_WEB_ERROR* pswe = static_cast<SCA_WEB_ERROR*>(MemAlloc(sizeof(SCA_WEB_ERROR), TRUE));
    ExitOnNull(pswe, hr, E_OUTOFMEMORY, "failed to allocate memory for new web error list element");

    pswe->psweNext = *ppsweList;
    *ppsweList = pswe;

LExit:
    return hr;
}

HRESULT ScaWebErrorCheckList7(SCA_WEB_ERROR* psweList)
{
    if (!psweList)
    {
        return S_OK;
    }

    while (psweList)
    {
        WcaLog(LOGMSG_STANDARD, "WebError code: %d subcode: %d for parent: %ls not used!", psweList->iErrorCode, psweList->iSubCode, psweList->wzParentValue);
        psweList = psweList->psweNext;
    }

    return E_FAIL;
}

