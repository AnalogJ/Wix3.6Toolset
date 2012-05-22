//-------------------------------------------------------------------------------------------------
// <copyright file="srputil.h" company="Microsoft">
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
//  System restore point helper functions.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


#ifdef __cplusplus
extern "C" {
#endif


enum SRP_ACTION
{
    SRP_ACTION_UNKNOWN,
    SRP_ACTION_UNINSTALL,
    SRP_ACTION_INSTALL,
    SRP_ACTION_MODIFY,
};


/********************************************************************
 SrpInitialize - initializes system restore point functionality.

*******************************************************************/
DAPI_(HRESULT) SrpInitialize(
    __in BOOL fInitializeComSecurity
    );

/********************************************************************
 SrpUninitialize - uninitializes system restore point functionality.

*******************************************************************/
DAPI_(void) SrpUninitialize();

/********************************************************************
 SrpCreateRestorePoint - creates a system restore point.

*******************************************************************/
DAPI_(HRESULT) SrpCreateRestorePoint(
    __in_z LPCWSTR wzApplicationName,
    __in SRP_ACTION action
    );

#ifdef __cplusplus
}
#endif

