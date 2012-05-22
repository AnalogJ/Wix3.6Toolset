//-------------------------------------------------------------------------------------------------
// <copyright file="uithread.h" company="Microsoft">
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
//    Message handler window
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


#if defined(__cplusplus)
extern "C" {
#endif


// functions

HRESULT UiCreateMessageWindow(
    __in HINSTANCE hInstance,
    __in BURN_ENGINE_STATE* pEngineState
    );

void UiCloseMessageWindow(
    __in BURN_ENGINE_STATE* pEngineState
    );

#if defined(__cplusplus)
}
#endif
