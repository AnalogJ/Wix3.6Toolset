//-------------------------------------------------------------------------------------------------
// <copyright file="msuengine.h" company="Microsoft">
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
//    Module: MSI Engine
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


#if defined(__cplusplus)
extern "C" {
#endif


// function declarations

HRESULT MsuEngineParsePackageFromXml(
    __in IXMLDOMNode* pixnMsiPackage,
    __in BURN_PACKAGE* pPackage
    );
void MsuEnginePackageUninitialize(
    __in BURN_PACKAGE* pPackage
    );
HRESULT MsuEngineDetectPackage(
    __in BURN_PACKAGE* pPackage,
    __in BURN_VARIABLES* pVariables
    );
HRESULT MsuEnginePlanCalculatePackage(
    __in BURN_PACKAGE* pPackage
    );
HRESULT MsuEnginePlanAddPackage(
    __in_opt DWORD *pdwInsertSequence,
    __in BURN_PACKAGE* pPackage,
    __in BURN_PLAN* pPlan,
    __in BURN_LOGGING* pLog,
    __in BURN_VARIABLES* pVariables,
    __in HANDLE hCacheEvent,
    __in BOOL fPlanPackageCacheRollback
    );
HRESULT MsuEngineExecutePackage(
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in BOOL fRollback,
    __in PFN_GENERICMESSAGEHANDLER pfnGenericMessageHandler,
    __in LPVOID pvContext,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    );


#if defined(__cplusplus)
}
#endif
