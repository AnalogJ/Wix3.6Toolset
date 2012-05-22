//-------------------------------------------------------------------------------------------------
// <copyright file="stub.cpp" company="Microsoft">
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
// Setup chainer/bootstrapper executable for WiX toolset.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"


int WINAPI wWinMain(
    __in HINSTANCE hInstance,
    __in_opt HINSTANCE /* hPrevInstance */,
    __in_z_opt LPWSTR lpCmdLine,
    __in int nCmdShow
    )
{
    ::HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    HRESULT hr = S_OK;
    DWORD dwExitCode = 0;

    // call run
    hr = EngineRun(hInstance, lpCmdLine, nCmdShow, &dwExitCode);
    ExitOnFailure(hr, "Failed to run application.");

LExit:
    return FAILED(hr) ? (int)hr : (int)dwExitCode;
}
