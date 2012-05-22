//-------------------------------------------------------------------------------------------------
// <copyright file="CheckReboot.cpp" company="Microsoft">
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
//    Code to schedule reboot for deferred custom actions whcih cannot.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"


/********************************************************************
WixCheckRebootRequired - entry point for WixCheckRebootRequired Custom Action

 called as Type 1 CustomAction (binary DLL) from Windows Installer 
 in InstallExecuteSequence after InstallFinalize
********************************************************************/
extern "C" UINT __stdcall WixCheckRebootRequired(
    __in MSIHANDLE hInstall
    )
{
    HRESULT hr = S_OK;
    DWORD er = ERROR_SUCCESS;

    hr = WcaInitialize(hInstall, "WixCheckRebootRequired");
    ExitOnFailure(hr, "failed to initialize");

    if (WcaDidDeferredActionRequireReboot())
    {
        WcaLog(LOGMSG_STANDARD, "Reboot required by deferred CustomAction.");

        er = ::MsiSetMode(hInstall, MSIRUNMODE_REBOOTATEND, TRUE);
        hr = HRESULT_FROM_WIN32(er);
        ExitOnFailure(hr, "Failed to schedule reboot.");
    }

LExit:

    if (FAILED(hr))
        er = ERROR_INSTALL_FAILURE;
    return WcaFinalize(er);
}
