//-------------------------------------------------------------------------------------------------
// <copyright file="BurnCoreForUX.h" company="Microsoft">
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
//    Module: Core
//
//    Setup chainer/bootstrapper UX core for WiX toolset.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


#if defined(__cplusplus)
extern "C" {
#endif

// constants

enum WM_BURN
{
    WM_BURN_FIRST = WM_APP + 0xFFF, // this enum value must always be first.

    WM_BURN_DETECT,
    WM_BURN_PLAN,
    WM_BURN_ELEVATE,
    WM_BURN_APPLY,
    WM_BURN_QUIT,

    WM_BURN_LAST, // this enum value must always be last.
};

// function declarations

HRESULT EngineForApplicationCreate(
    __in BURN_ENGINE_STATE* pEngineState,
    __in DWORD dwThreadId,
    __out IBootstrapperEngine** ppEngineForApplication
    );

#if defined(__cplusplus)
}
#endif
