#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="svcutil.h" company="Microsoft">
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
//    Header for Windows service helper functions.
// </summary>
//-------------------------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif


#define ReleaseServiceHandle(h) if (h) { ::CloseServiceHandle(h); h = NULL; }


HRESULT DAPI SvcQueryConfig(
    __in SC_HANDLE sch,
    __out QUERY_SERVICE_CONFIGW** ppConfig
    );


#ifdef __cplusplus
}
#endif
