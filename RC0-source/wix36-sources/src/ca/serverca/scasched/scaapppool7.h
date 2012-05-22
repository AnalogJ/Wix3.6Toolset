#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="scaapppool7.h" company="Microsoft">
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
//    IIS Application Pool functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

#include "scauser.h"

// Identity
#define APATTR_NETSERVICE 0x0001 // Network Service
#define APATTR_LOCSERVICE 0x0002 // Local Service
#define APATTR_LOCSYSTEM 0x0004 // Local System
#define APATTR_OTHERUSER 0x0008 // Other User

// prototypes
HRESULT ScaFindAppPool7(
    __in LPCWSTR wzAppPool,
    __out_ecount(cchName) LPWSTR wzName,
    __in DWORD cchName,
    __in SCA_APPPOOL *psapList
    );

HRESULT ScaAppPoolInstall7(
    __in SCA_APPPOOL* psapList
    );

HRESULT ScaAppPoolUninstall7(
    __in SCA_APPPOOL* psapList
    );

HRESULT ScaWriteAppPool7(
    __in const SCA_APPPOOL* psap
    );

HRESULT ScaRemoveAppPool7(
    __in const SCA_APPPOOL* psap
    );
