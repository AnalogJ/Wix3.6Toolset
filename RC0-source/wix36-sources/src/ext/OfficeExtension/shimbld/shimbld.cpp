//-------------------------------------------------------------------------------------------------
// <copyright file="shimbld.cpp" company="Microsoft">
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
// Setup executable builder for CTOA SHIM.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"


extern "C" HRESULT __stdcall UpdateShim(
    __in LPCWSTR wzShimPath,
    __in LPCWSTR wzApplicationId,
    __in LPCWSTR wzClrVersion,
    __in LPCWSTR wzAssemblyName,
    __in LPCWSTR wzClassName,
    __in LPCWSTR wzOutputPath
    )
{
    HRESULT hr = S_OK;
    BOOL bCreatedOutputFile = FALSE;

    ExitOnNull(wzShimPath, hr, E_INVALIDARG, "Must specify a shim path.");
    ExitOnNull(wzApplicationId, hr, E_INVALIDARG, "Must specify an application id.");
    ExitOnNull(wzAssemblyName, hr, E_INVALIDARG, "Must specify an assembly name.");
    ExitOnNull(wzClassName, hr, E_INVALIDARG, "Must specify a class name.");
    ExitOnNull(wzOutputPath, hr, E_INVALIDARG, "Must specify an output path.");

    if (!::CopyFileW(wzShimPath, wzOutputPath, FALSE))
    {
        ExitWithLastError2(hr, "Failed to copy shim path: %S to output path: %S", wzShimPath, wzOutputPath);
    }
    
    bCreatedOutputFile = TRUE;

    hr = ResWriteString(wzOutputPath, IDS_APPLICATIONID, wzApplicationId, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
    ExitOnFailure(hr, "Failed to update application id.");

    hr = ResWriteString(wzOutputPath, IDS_ASSEMBLYNAME, wzAssemblyName, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
    ExitOnFailure(hr, "Failed to update assembly name.");

    hr = ResWriteString(wzOutputPath, IDS_CLASSNAME, wzClassName, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
    ExitOnFailure(hr, "Failed to update assembly name.");

    if (wzClrVersion)
    {
        hr = ResWriteString(wzOutputPath, IDS_CLRVERSION, wzClrVersion, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
        ExitOnFailure(hr, "Failed to update clr version.");
    }

LExit:
    if (FAILED(hr) && bCreatedOutputFile)
    {
        ::DeleteFileW(wzOutputPath);
    }

    return SCODE_CODE(hr);
}
