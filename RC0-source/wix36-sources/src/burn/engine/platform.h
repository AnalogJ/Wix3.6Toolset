//-------------------------------------------------------------------------------------------------
// <copyright file="platform.h" company="Microsoft">
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
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


#if defined(__cplusplus)
extern "C" {
#endif


// typedefs

typedef BOOL (WINAPI *PFN_INITIATESYSTEMSHUTDOWNEXW)(
    __in_opt LPWSTR lpMachineName,
    __in_opt LPWSTR lpMessage,
    __in DWORD dwTimeout,
    __in BOOL bForceAppsClosed,
    __in BOOL bRebootAfterShutdown,
    __in DWORD dwReason
    );


// variable declarations

extern PFN_INITIATESYSTEMSHUTDOWNEXW vpfnInitiateSystemShutdownExW;


// function declarations

void PlatformInitialize();


#if defined(__cplusplus)
}
#endif
