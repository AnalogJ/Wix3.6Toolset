//-------------------------------------------------------------------------------------------------
// <copyright file="engine.h" company="Microsoft">
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
//    Setup chainer/bootstrapper stub entry-points for WiX toolset.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


#if defined(__cplusplus)
extern "C" {
#endif


// function declarations

HRESULT EngineRun(
    __in HINSTANCE hInstance,
    __in_z_opt LPCWSTR wzCommandLine,
    __in int nCmdShow,
    __out DWORD* pdwExitCode
    );


#if defined(__cplusplus)
}
#endif
