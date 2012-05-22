#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="scawebdir7.h" company="Microsoft">
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
//    IIS Web Directory functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------


struct SCA_WEBDIR7
{
    // darwin information
    WCHAR wzKey[MAX_DARWIN_KEY + 1];
    WCHAR wzComponent[MAX_DARWIN_KEY + 1];
    INSTALLSTATE isInstalled;
    INSTALLSTATE isAction;


    // iis configuation information
    WCHAR wzPath[MAX_PATH];
    WCHAR wzWebSite[MAX_PATH];

    BOOL fHasProperties;
    SCA_WEB_PROPERTIES swp;

    BOOL fHasApplication;
    SCA_WEB_APPLICATION swapp;

    SCA_WEBDIR7* pswdNext;
};


// prototypes
UINT __stdcall ScaWebDirsRead7(
    __in SCA_WEB7* pswList,
    __in WCA_WRAPQUERY_HANDLE hUserQuery,
    __in WCA_WRAPQUERY_HANDLE hWebBaseQuery,
    __in WCA_WRAPQUERY_HANDLE hWebDirPropQuery,
    __in WCA_WRAPQUERY_HANDLE hWebAppQuery,
    __in WCA_WRAPQUERY_HANDLE hWebAppExtQuery,
    __inout LPWSTR *ppwzCustomActionData,
    __out SCA_WEBDIR7** ppswdList
    );

HRESULT ScaWebDirsInstall7(
    __in SCA_WEBDIR7* pswdList,
    __in SCA_APPPOOL* psapList
    );

HRESULT ScaWebDirsUninstall7(
    __in SCA_WEBDIR7* pswdList
    );

void ScaWebDirsFreeList7(
    __in SCA_WEBDIR7* pswdList
    );
