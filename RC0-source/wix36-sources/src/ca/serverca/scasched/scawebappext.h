#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="scawebappext.h" company="Microsoft">
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
//    Functions for dealing with Web Application Extensions in Server CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

// structs
struct SCA_WEB_APPLICATION_EXTENSION
{
    WCHAR wzExtension[MAX_DARWIN_COLUMN + 1];

    WCHAR wzVerbs[MAX_DARWIN_COLUMN + 1];
    WCHAR wzExecutable[MAX_DARWIN_COLUMN + 1];
    int iAttributes;

    SCA_WEB_APPLICATION_EXTENSION* pswappextNext;
};


// prototypes
HRESULT ScaWebAppExtensionsRead(
    __in LPCWSTR wzApplication,
    __in WCA_WRAPQUERY_HANDLE hWebAppExtQuery,
    __inout SCA_WEB_APPLICATION_EXTENSION** ppswappextList
    );

HRESULT ScaWebAppExtensionsWrite(
    __in IMSAdminBase* piMetabase,
    __in LPCWSTR wzRootOfWeb,
    __in SCA_WEB_APPLICATION_EXTENSION* pswappextList
    );

void ScaWebAppExtensionsFreeList(
    __in SCA_WEB_APPLICATION_EXTENSION* pswappextList
    );
