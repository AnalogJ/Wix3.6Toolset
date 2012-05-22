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

HRESULT ScaScheduleIIS7Configuration();

HRESULT ScaIIS7ConfigTransaction(__in_z LPCWSTR wzBackup);

HRESULT ScaCreateApp7(__in_z LPCWSTR wzWebRoot, DWORD dwIsolation);

HRESULT ScaDeleteConfigElement(IIS_CONFIG_ACTION emElement, LPCWSTR wzSubKey);

HRESULT ScaWriteConfigString(__in_z const LPCWSTR wzValue);

HRESULT ScaWriteConfigID(IIS_CONFIG_ACTION emID);

HRESULT ScaWriteConfigInteger(DWORD dwValue);