//-------------------------------------------------------------------------------------------------
// <copyright file="elevation.h" company="Microsoft">
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
//    Module: Elevation
//
//    Burn elevation process handler.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


#ifdef __cplusplus
extern "C" {
#endif


// Parent (per-user process) side functions.
HRESULT ElevationElevate(
    __in BURN_ENGINE_STATE* pEngineState,
    __in_opt HWND hwndParent
    );
HRESULT ElevationApplyInitialize(
    __in HANDLE hPipe,
    __in BURN_VARIABLES* pVariables,
    __in BOOTSTRAPPER_ACTION action,
    __in BURN_AU_PAUSE_ACTION auAction,
    __in BOOL fTakeSystemRestorePoint
    );
HRESULT ElevationApplyUninitialize(
    __in HANDLE hPipe
    );
HRESULT ElevationSessionBegin(
    __in HANDLE hPipe,
    __in_z LPCWSTR wzEngineWorkingPath,
    __in_z LPCWSTR wzResumeCommandLine,
    __in BURN_VARIABLES* pVariables,
    __in BOOTSTRAPPER_ACTION action,
    __in DWORD64 qwEstimatedSize
    );
HRESULT ElevationSessionResume(
    __in HANDLE hPipe,
    __in_z LPCWSTR wzResumeCommandLine
    );
HRESULT ElevationSessionEnd(
    __in HANDLE hPipe,
    __in BOOL fKeepRegistration,
    __in BOOL fSuspend,
    __in BOOTSTRAPPER_APPLY_RESTART restart
    );
HRESULT ElevationDetectRelatedBundles(
    __in HANDLE hPipe
    );
HRESULT ElevationSaveState(
    __in HANDLE hPipe,
    __in_bcount(cbBuffer) BYTE* pbBuffer,
    __in SIZE_T cbBuffer
    );
HRESULT ElevationLayoutBundle(
    __in HANDLE hPipe,
    __in_z LPCWSTR wzLayoutDirectory,
    __in_z LPCWSTR wzUnverifiedPath
    );
HRESULT ElevationCacheOrLayoutPayload(
    __in HANDLE hPipe,
    __in_opt BURN_PACKAGE* pPackage,
    __in BURN_PAYLOAD* pPayload,
    __in_z_opt LPCWSTR wzLayoutDirectory,
    __in_z LPCWSTR wzUnverifiedPayloadPath,
    __in BOOL fMove
    );
HRESULT ElevationCacheCleanup(
    __in HANDLE hPipe
    );
HRESULT ElevationExecuteExePackage(
    __in HANDLE hPipe,
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in BURN_VARIABLES* pVariables,
    __in BOOL fRollback,
    __in PFN_GENERICMESSAGEHANDLER pfnGenericExecuteProgress,
    __in LPVOID pvContext,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    );
HRESULT ElevationExecuteMsiPackage(
    __in HANDLE hPipe,
    __in_opt HWND hwndParent,
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in BURN_VARIABLES* pVariables,
    __in BOOL fRollback,
    __in PFN_MSIEXECUTEMESSAGEHANDLER pfnMessageHandler,
    __in LPVOID pvContext,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    );
HRESULT ElevationExecuteMspPackage(
    __in HANDLE hPipe,
    __in_opt HWND hwndParent,
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in BURN_VARIABLES* pVariables,
    __in BOOL fRollback,
    __in PFN_MSIEXECUTEMESSAGEHANDLER pfnMessageHandler,
    __in LPVOID pvContext,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    );
HRESULT ElevationExecuteMsuPackage(
    __in HANDLE hPipe,
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in BOOL fRollback,
    __in PFN_GENERICMESSAGEHANDLER pfnGenericExecuteProgress,
    __in LPVOID pvContext,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    );
HRESULT ElevationExecuteDependencyAction(
    __in HANDLE hPipe,
    __in BURN_EXECUTE_ACTION* pExecuteAction
    );
HRESULT ElevationLaunchElevatedChild(
    __in HANDLE hPipe,
    __in BURN_PACKAGE* pPackage,
    __in LPCWSTR wzPipeName,
    __in LPCWSTR wzPipeToken,
    __out DWORD* pdwChildPid
    );
HRESULT ElevationCleanPackage(
    __in HANDLE hPipe,
    __in BURN_PACKAGE* pPackage
    );

// Child (per-machine process) side functions.
HRESULT ElevationChildPumpMessages(
    __in DWORD dwLoggingTlsId,
    __in HANDLE hPipe,
    __in HANDLE hCachePipe,
    __in BURN_PACKAGES* pPackages,
    __in BURN_RELATED_BUNDLES* pRelatedBundles,
    __in BURN_PAYLOADS* pPayloads,
    __in BURN_VARIABLES* pVariables,
    __in BURN_REGISTRATION* pRegistration,
    __in BURN_USER_EXPERIENCE* pUserExperience,
    __out HANDLE* phLock,
    __out BOOL* pfDisabledAutomaticUpdates,
    __out DWORD* pdwChildExitCode,
    __out BOOL* pfRestart
    );
HRESULT ElevationChildResumeAutomaticUpdates();

#ifdef __cplusplus
}
#endif
