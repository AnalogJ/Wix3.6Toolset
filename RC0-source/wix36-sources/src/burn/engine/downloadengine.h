//-------------------------------------------------------------------------------------------------
// <copyright file="downloadengine.h" company="Microsoft">
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
//    Setup chainer/bootstrapper download engine for WiX toolset.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


#ifdef __cplusplus
extern "C" {
#endif

// structs


// functions

HRESULT WininetDownloadUrl(
    __in BURN_CACHE_CALLBACK* pCallback,
    __in BURN_DOWNLOAD_SOURCE* pDownloadSource,
    __in DWORD64 dw64AuthoredDownloadSize,
    __in LPCWSTR wzDestinationPath
    );


#ifdef __cplusplus
}
#endif
