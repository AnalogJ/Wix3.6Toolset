//-------------------------------------------------------------------------------------------------
// <copyright file="svcutil.cpp" company="Microsoft">
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
//    Windows service helper functions.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

/********************************************************************
SvcQueryConfig - queries the configuration of a service

********************************************************************/
extern "C" HRESULT DAPI SvcQueryConfig(
    __in SC_HANDLE sch,
    __out QUERY_SERVICE_CONFIGW** ppConfig
    )
{
    HRESULT hr = S_OK;
    QUERY_SERVICE_CONFIGW* pConfig = NULL;
    DWORD cbConfig = 0;

    if (!::QueryServiceConfigW(sch, NULL, 0, &cbConfig))
    {
        DWORD er = ::GetLastError();
        if (ERROR_INSUFFICIENT_BUFFER == er)
        {
            pConfig = static_cast<QUERY_SERVICE_CONFIGW*>(MemAlloc(cbConfig, TRUE));
            ExitOnNull(pConfig, hr, E_OUTOFMEMORY, "Failed to allocate memory to get configuration.");

            if (!::QueryServiceConfigW(sch, pConfig, cbConfig, &cbConfig))
            {
                ExitWithLastError(hr, "Failed to read service configuration.");
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(er);
            ExitOnFailure(hr, "Failed to query service configuration.");
        }
    }

    *ppConfig = pConfig;
    pConfig = NULL;

LExit:
    ReleaseMem(pConfig);

    return hr;
}
