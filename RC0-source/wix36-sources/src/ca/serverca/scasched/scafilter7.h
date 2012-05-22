#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="scafilter7.h" company="Microsoft">
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
//    IIS Filter functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

#include "scaweb.h"

// prototypes
UINT __stdcall ScaFiltersRead7(
    __in SCA_WEB7* pswList,
    __in WCA_WRAPQUERY_HANDLE hWebBaseQuery, 
    __inout SCA_FILTER** ppsfList,
    __inout LPWSTR *ppwzCustomActionData
    );

HRESULT ScaFiltersInstall7(
    SCA_FILTER* psfList
    );

HRESULT ScaFiltersUninstall7(
    SCA_FILTER* psfList
    );
