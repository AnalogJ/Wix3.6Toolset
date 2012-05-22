//-------------------------------------------------------------------------------------------------
// <copyright file="scaiis7.cpp" company="Microsoft">
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
//    IIS7 functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

#define COST_IIS_WRITEKEY 10

HRESULT ScaIIS7ConfigTransaction(LPCWSTR wzBackup)
{
    HRESULT hr = S_OK;

    hr = WcaDoDeferredAction(L"StartIIS7ConfigTransaction", wzBackup, COST_IIS_TRANSACTIONS);
    ExitOnFailure(hr, "Failed to schedule StartIIS7ConfigTransaction");

    hr = WcaDoDeferredAction(L"RollbackIIS7ConfigTransaction", wzBackup, 0);   // rollback cost is irrelevant
    ExitOnFailure(hr, "Failed to schedule RollbackIIS7ConfigTransaction");

    hr = WcaDoDeferredAction(L"CommitIIS7ConfigTransaction", wzBackup, 0);  // commit is free
    ExitOnFailure(hr, "Failed to schedule StartIIS7ConfigTransaction");

LExit:
    return hr;
}

HRESULT ScaWriteConfigString(const LPCWSTR wzValue)
{
    HRESULT hr = S_OK;
    WCHAR* pwzCustomActionData = NULL;

    hr = WcaWriteStringToCaData(wzValue, &pwzCustomActionData);
    ExitOnFailure(hr, "Failed to add metabase delete key directive to CustomActionData");

    hr = ScaAddToIisConfiguration(pwzCustomActionData, COST_IIS_WRITEKEY);
    ExitOnFailure2(hr, "Failed to add ScaWriteMetabaseValue action data: %ls, cost: %d", pwzCustomActionData, COST_IIS_WRITEKEY);

LExit:
    ReleaseStr(pwzCustomActionData);

    return hr;
}

HRESULT ScaWriteConfigInteger(DWORD dwValue)
{
    HRESULT hr = S_OK;
    WCHAR* pwzCustomActionData = NULL;

    hr = WcaWriteIntegerToCaData(dwValue, &pwzCustomActionData);
    ExitOnFailure(hr, "Failed to add metabase delete key directive to CustomActionData");

    hr = ScaAddToIisConfiguration(pwzCustomActionData, COST_IIS_WRITEKEY);
    ExitOnFailure2(hr, "Failed to add ScaWriteMetabaseValue action data: %ls, cost: %d", pwzCustomActionData, COST_IIS_WRITEKEY);

LExit:
    ReleaseStr(pwzCustomActionData);

    return hr;
}

HRESULT ScaWriteConfigID(IIS_CONFIG_ACTION emID)
{
    HRESULT hr = S_OK;
    WCHAR* pwzCustomActionData = NULL;

    hr = WcaWriteIntegerToCaData(emID, &pwzCustomActionData);
    ExitOnFailure(hr, "Failed to add metabase delete key directive to CustomActionData");

    hr = ScaAddToIisConfiguration(pwzCustomActionData, COST_IIS_WRITEKEY);
    ExitOnFailure2(hr, "Failed to add ScaWriteMetabaseValue action data: %ls, cost: %d", pwzCustomActionData, COST_IIS_WRITEKEY);

LExit:
    ReleaseStr(pwzCustomActionData);

    return hr;
}

