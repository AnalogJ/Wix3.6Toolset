#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="rmutil.h" company="Microsoft">
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
//    Header for Restart Manager utility functions.
// </summary>
//-------------------------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _RMU_SESSION *PRMU_SESSION;

HRESULT DAPI RmuJoinSession(
    __out PRMU_SESSION *ppSession,
    __in_z LPCWSTR wzSessionKey
    );

HRESULT DAPI RmuAddFile(
    __in PRMU_SESSION pSession,
    __in_z LPCWSTR wzPath
    );

HRESULT DAPI RmuAddProcessById(
    __in PRMU_SESSION pSession,
    __in DWORD dwProcessId
    );

HRESULT DAPI RmuAddProcessesByName(
    __in PRMU_SESSION pSession,
    __in_z LPCWSTR wzProcessName
    );

HRESULT DAPI RmuAddService(
    __in PRMU_SESSION pSession,
    __in_z LPCWSTR wzServiceName
    );

HRESULT DAPI RmuRegisterResources(
    __in PRMU_SESSION pSession
    );

HRESULT DAPI RmuEndSession(
    __in PRMU_SESSION pSession
    );

#ifdef __cplusplus
}
#endif
