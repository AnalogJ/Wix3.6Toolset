// <copyright file="shim.cpp" company="Microsoft">
//  Copyright (c) Microsoft Corporation.  All rights reserved.
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
//  Creative Commons shim implementation.
// </summary>
//
#include "precomp.h"

// global globals
HMODULE vhModule = NULL;


BOOL APIENTRY DllMain(
    __in HMODULE hModule,
    __in DWORD dwReason,
    __in LPVOID lpReserved
    )
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        ::DisableThreadLibraryCalls(hModule);

        vhModule = hModule;
        ClrLoaderInitialize();
        UpdateThreadInitialize();

        break;
    case DLL_PROCESS_DETACH:
        UpdateThreadUninitialize();
        ClrLoaderUninitialize();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }

    return TRUE;
}


STDAPI DllCanUnloadNow()
{
    return S_OK;
}


STDAPI DllGetClassObject(
    __in const CLSID & rclsid,
    __in const IID & riid,
    __out void ** ppv
    )
{
    HRESULT hr = S_OK;
    IClassFactory *pClassFactory = NULL;

    *ppv = NULL; 

    hr = CreateClassFactory(&pClassFactory);
    ExitOnFailure(hr, "Failed to create class factory.");

    hr = pClassFactory->QueryInterface(riid, ppv);
    ExitOnFailure(hr, "Failed to query interface on class factory.");

LExit:
    ReleaseObject(pClassFactory);
    return hr;
}
