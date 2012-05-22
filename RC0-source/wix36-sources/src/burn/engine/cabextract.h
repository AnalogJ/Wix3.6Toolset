//-------------------------------------------------------------------------------------------------
// <copyright file="cabextract.h" company="Microsoft">
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
//    Module: Cabinet
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


#if defined(__cplusplus)
extern "C" {
#endif


// function declarations

void CabExtractInitialize();
HRESULT CabExtractOpen(
    __in BURN_CONTAINER_CONTEXT* pContext,
    __in LPCWSTR wzFilePath
    );
HRESULT CabExtractNextStream(
    __in BURN_CONTAINER_CONTEXT* pContext,
    __inout_z LPWSTR* psczStreamName
    );
HRESULT CabExtractStreamToFile(
    __in BURN_CONTAINER_CONTEXT* pContext,
    __in_z LPCWSTR wzFileName
    );
HRESULT CabExtractStreamToBuffer(
    __in BURN_CONTAINER_CONTEXT* pContext,
    __out BYTE** ppbBuffer,
    __out SIZE_T* pcbBuffer
    );
HRESULT CabExtractSkipStream(
    __in BURN_CONTAINER_CONTEXT* pContext
    );
HRESULT CabExtractClose(
    __in BURN_CONTAINER_CONTEXT* pContext
    );


#if defined(__cplusplus)
}
#endif
