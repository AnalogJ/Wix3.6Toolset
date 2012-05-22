#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="scaproperty7.h" company="Microsoft">
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
//    IIS Property functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

// Settings
#define wzIISPROPERTY_IIS5_ISOLATION_MODE L"IIs5IsolationMode"
#define wzIISPROPERTY_MAX_GLOBAL_BANDWIDTH L"MaxGlobalBandwidth"
#define wzIISPROPERTY_LOG_IN_UTF8 L"LogInUTF8"
#define wzIISPROPERTY_ETAG_CHANGENUMBER L"ETagChangeNumber"

// prototypes
HRESULT ScaPropertyInstall7(
    SCA_PROPERTY* pspList
    );

HRESULT ScaPropertyUninstall7(
    SCA_PROPERTY* pspList
    );

HRESULT ScaWriteProperty7(
    const SCA_PROPERTY* psp
    );

HRESULT ScaRemoveProperty7(
    SCA_PROPERTY* psp
    );

