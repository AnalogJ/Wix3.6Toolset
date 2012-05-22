#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="scawebprop.h" company="Microsoft">
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
//    IIS Web Directory Property functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

#include "scauser.h"
 
// global sql queries provided for optimization
extern LPCWSTR vcsWebDirPropertiesQuery;


// structs
struct SCA_WEB_PROPERTIES
{
	WCHAR wzKey[MAX_DARWIN_KEY + 1];

	int iAccess;

	int iAuthorization;
	BOOL fHasUser;
	SCA_USER scau;
	BOOL fIIsControlledPassword;

	BOOL fLogVisits;
	BOOL fIndex;

	BOOL fHasDefaultDoc;
	WCHAR wzDefaultDoc[MAX_DARWIN_COLUMN + 1];

	BOOL fHasHttpExp;
	WCHAR wzHttpExp[MAX_DARWIN_COLUMN + 1];

	BOOL fAspDetailedError;

	int iCacheControlMaxAge;

	BOOL fHasCacheControlCustom;
	WCHAR wzCacheControlCustom[MAX_DARWIN_COLUMN + 1];

	BOOL fNoCustomError;

	int iAccessSSLFlags;

	WCHAR wzAuthenticationProviders[MAX_DARWIN_COLUMN + 1];
};
 

// prototypes
HRESULT ScaGetWebDirProperties(
    __in LPCWSTR pwzProperties,
    __in WCA_WRAPQUERY_HANDLE hUserQuery,
    __in WCA_WRAPQUERY_HANDLE hWebDirPropQuery,
    __inout SCA_WEB_PROPERTIES* pswp
    );

HRESULT ScaWriteWebDirProperties(
    __in IMSAdminBase* piMetabase,
    __in LPCWSTR wzRootOfWeb, 
    __inout SCA_WEB_PROPERTIES* pswp
    );

