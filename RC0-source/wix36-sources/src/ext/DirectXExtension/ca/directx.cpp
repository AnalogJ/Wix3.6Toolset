//-------------------------------------------------------------------------------------------------
// <copyright file="directx.cpp" company="Microsoft">
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
//    DirectX capabilities custom action.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

/********************************************************************
WixQueryDirectXCaps - entry point for WixQueryDirectXCaps CA

 Called as Type 1 custom action (DLL from the Binary table) from
 Windows Installer to set properties that identify the DirectX
 capabilities ("caps") of the system.
********************************************************************/
extern "C" UINT __stdcall WixQueryDirectXCaps(
    __in MSIHANDLE hInstall
    )
{
#if 0
    ::MessageBoxA(0, "break into debugger now please", "---->> ATTACH TO ME!", MB_ICONEXCLAMATION);
#endif

    HRESULT hr = S_OK;
    DWORD er = ERROR_SUCCESS;
    LPDIRECT3D9 pD3D = NULL;

    hr = WcaInitialize(hInstall, "WixQueryDirectXCaps");
    ExitOnFailure(hr, "failed to initialize");

    pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    ExitOnNull(pD3D, hr, E_FAIL, "Direct3DCreate9 failed");

    D3DCAPS9 d3dCaps;
    hr = pD3D->GetDeviceCaps(
        0,                 // first adapter
        D3DDEVTYPE_HAL,    // fail on non-HAL devices
        &d3dCaps
    );
    ExitOnFailure(hr, "GetDeviceCaps call failed");

    int iVertexShaderVersion = D3DSHADER_VERSION_MAJOR(d3dCaps.VertexShaderVersion) * 100 + D3DSHADER_VERSION_MINOR(d3dCaps.VertexShaderVersion);
    WcaSetIntProperty(L"WIX_DIRECTX_VERTEXSHADERVERSION", iVertexShaderVersion);

    int iPixelShaderVersion = D3DSHADER_VERSION_MAJOR(d3dCaps.PixelShaderVersion) * 100 + D3DSHADER_VERSION_MINOR(d3dCaps.PixelShaderVersion);
    WcaSetIntProperty(L"WIX_DIRECTX_PIXELSHADERVERSION", iPixelShaderVersion);

LExit:
    ReleaseObject(pD3D);
    return WcaFinalize(er = FAILED(hr) ? ERROR_INSTALL_FAILURE : er);
}
    
    
