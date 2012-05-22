#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="scaiis.h" company="Microsoft">
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
//    IIS functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

HRESULT ScaMetabaseTransaction(__in_z LPCWSTR wzBackup);

HRESULT ScaCreateWeb(IMSAdminBase* piMetabase, LPCWSTR wzWeb, LPCWSTR wzWebBase);

HRESULT ScaDeleteApp(IMSAdminBase* piMetabase, LPCWSTR wzWebRoot);

HRESULT ScaCreateApp(IMSAdminBase* piMetabase, LPCWSTR wzWebRoot,
                     DWORD dwIsolation);

HRESULT ScaCreateMetabaseKey(IMSAdminBase* piMetabase, LPCWSTR wzRootKey,
                             LPCWSTR wzSubKey);

HRESULT ScaDeleteMetabaseKey(IMSAdminBase* piMetabase, LPCWSTR wzRootKey,
                             LPCWSTR wzSubKey);

HRESULT ScaWriteMetabaseValue(IMSAdminBase* piMetabase, LPCWSTR wzRootKey,
                              LPCWSTR wzSubKey, DWORD dwIdentifier,
                              DWORD dwAttributes, DWORD dwUserType,
                              DWORD dwDataType, LPVOID pvData);

HRESULT ScaDeleteMetabaseValue(IMSAdminBase* piMetabase, LPCWSTR wzRootKey,
                              LPCWSTR wzSubKey, DWORD dwIdentifier,
                              DWORD dwDataType);

HRESULT ScaWriteConfigurationScript(LPCWSTR pwzCaScriptKey);

HRESULT ScaAddToIisConfiguration(LPCWSTR pwzData, DWORD dwCost);

HRESULT ScaLoadMetabase(IMSAdminBase** piMetabase);