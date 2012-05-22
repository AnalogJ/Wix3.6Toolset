//-------------------------------------------------------------------------------------------------
// <copyright file="mspengine.h" company="Microsoft">
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
//    Module: MSP Engine
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


#if defined(__cplusplus)
extern "C" {
#endif


// constants


// structures


// typedefs


// function declarations

HRESULT MspEngineParsePackageFromXml(
    __in IXMLDOMNode* pixnBundle,
    __in BURN_PACKAGE* pPackage
    );
void MspEnginePackageUninitialize(
    __in BURN_PACKAGE* pPackage
    );
HRESULT MspEngineDetectInitialize(
    __in BURN_PACKAGES* pPackages
    );
HRESULT MspEngineDetectPackage(
    __in BURN_PACKAGE* pPackage,
    __in BURN_USER_EXPERIENCE* pUserExperience
    );
HRESULT MspEnginePlanCalculatePackage(
    __in BURN_PACKAGE* pPackage,
    __in BURN_USER_EXPERIENCE* pUserExperience
    );
HRESULT MspEnginePlanAddPackage(
    __in_opt DWORD* /*pdwInsertSequence*/,
    __in BOOTSTRAPPER_DISPLAY display,
    __in BURN_PACKAGE* pPackage,
    __in BURN_PLAN* pPlan,
    __in BURN_LOGGING* pLog,
    __in BURN_VARIABLES* pVariables,
    __in_opt HANDLE hCacheEvent,
    __in BOOL fPlanPackageCacheRollback
    );
HRESULT MspEngineExecutePackage(
    __in_opt HWND hwndParent,
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in BURN_VARIABLES* pVariables,
    __in BOOL fRollback,
    __in PFN_MSIEXECUTEMESSAGEHANDLER pfnMessageHandler,
    __in LPVOID pvContext,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    );
void MspEngineSlipstreamUpdateState(
    __in BURN_PACKAGE* pMspPackage,
    __in BOOTSTRAPPER_ACTION_STATE execute,
    __in BOOTSTRAPPER_ACTION_STATE rollback
    );


#if defined(__cplusplus)
}
#endif
