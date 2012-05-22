#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="scaweblog7.h" company="Microsoft">
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
//    Custom Actions for handling log settings for a particular IIS Website
// </summary>
//-------------------------------------------------------------------------------------------------

// prototypes
HRESULT ScaGetWebLog7(
    __in_z LPCWSTR wzLog,
    __in WCA_WRAPQUERY_HANDLE hWebLogQuery,
    __out SCA_WEB_LOG* pswl
    );

HRESULT ScaWriteWebLog7(
    __in_z LPCWSTR wzRootOfWeb,
    const SCA_WEB_LOG *pswl
    );
