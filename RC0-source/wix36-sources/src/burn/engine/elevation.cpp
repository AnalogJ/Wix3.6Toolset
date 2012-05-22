//-------------------------------------------------------------------------------------------------
// <copyright file="elevation.cpp" company="Microsoft">
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
//    Burn elevated process handler.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"


const DWORD BURN_TIMEOUT = 5 * 60 * 1000; // TODO: is 5 minutes good?

typedef enum _BURN_ELEVATION_MESSAGE_TYPE
{
    BURN_ELEVATION_MESSAGE_TYPE_UNKNOWN,
    BURN_ELEVATION_MESSAGE_TYPE_APPLY_INITIALIZE,
    BURN_ELEVATION_MESSAGE_TYPE_APPLY_UNINITIALIZE,
    BURN_ELEVATION_MESSAGE_TYPE_SESSION_BEGIN,
    BURN_ELEVATION_MESSAGE_TYPE_SESSION_RESUME,
    BURN_ELEVATION_MESSAGE_TYPE_SESSION_END,
    BURN_ELEVATION_MESSAGE_TYPE_DETECT_RELATED_BUNDLES,
    BURN_ELEVATION_MESSAGE_TYPE_SAVE_STATE,
    BURN_ELEVATION_MESSAGE_TYPE_LAYOUT_BUNDLE,
    BURN_ELEVATION_MESSAGE_TYPE_CACHE_OR_LAYOUT_PAYLOAD,
    BURN_ELEVATION_MESSAGE_TYPE_CACHE_CLEANUP,
    BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_EXE_PACKAGE,
    BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSI_PACKAGE,
    BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSP_PACKAGE,
    BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSU_PACKAGE,
    BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_DEPENDENCY,
    BURN_ELEVATION_MESSAGE_TYPE_LAUNCH_EMBEDDED_CHILD,
    BURN_ELEVATION_MESSAGE_TYPE_CLEAN_PACKAGE,

    BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_PROGRESS,
    BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_ERROR,
    BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSI_MESSAGE,
    BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_FILES_IN_USE,
} BURN_ELEVATION_MESSAGE_TYPE;


// struct

typedef struct _BURN_ELEVATION_GENERIC_MESSAGE_CONTEXT
{
    PFN_GENERICMESSAGEHANDLER pfnGenericMessageHandler;
    LPVOID pvContext;
} BURN_ELEVATION_GENERIC_MESSAGE_CONTEXT;

typedef struct _BURN_ELEVATION_MSI_MESSAGE_CONTEXT
{
    PFN_MSIEXECUTEMESSAGEHANDLER pfnMessageHandler;
    LPVOID pvContext;
} BURN_ELEVATION_MSI_MESSAGE_CONTEXT;

typedef struct _BURN_ELEVATION_CHILD_MESSAGE_CONTEXT
{
    DWORD dwLoggingTlsId;
    HANDLE hPipe;
    HANDLE* phLock;
    BOOL* pfDisabledAutomaticUpdates;
    BURN_PACKAGES* pPackages;
    BURN_RELATED_BUNDLES* pRelatedBundles;
    BURN_PAYLOADS* pPayloads;
    BURN_VARIABLES* pVariables;
    BURN_REGISTRATION* pRegistration;
    BURN_USER_EXPERIENCE* pUserExperience;
} BURN_ELEVATION_CHILD_MESSAGE_CONTEXT;


// internal function declarations

static DWORD WINAPI ElevatedChildCacheThreadProc(
    __in LPVOID lpThreadParameter
    );
static HRESULT WaitForElevatedChildCacheThread(
    __in HANDLE hCacheThread,
    __in DWORD dwExpectedExitCode
    );
static HRESULT ProcessGenericExecuteMessages(
    __in BURN_PIPE_MESSAGE* pMsg,
    __in_opt LPVOID pvContext,
    __out DWORD* pdwResult
    );
static HRESULT ProcessMsiPackageMessages(
    __in BURN_PIPE_MESSAGE* pMsg,
    __in_opt LPVOID pvContext,
    __out DWORD* pdwResult
    );
static HRESULT ProcessElevatedChildMessage(
    __in BURN_PIPE_MESSAGE* pMsg,
    __in_opt LPVOID pvContext,
    __out DWORD* pdwResult
    );
static HRESULT ProcessElevatedChildCacheMessage(
    __in BURN_PIPE_MESSAGE* pMsg,
    __in_opt LPVOID pvContext,
    __out DWORD* pdwResult
    );
static HRESULT ProcessResult(
    __in DWORD dwResult,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    );
static HRESULT OnApplyInitialize(
    __in BURN_VARIABLES* pVariables,
    __in HANDLE* phLock,
    __in BOOL* pfDisabledWindowsUpdate,
    __in BYTE* pbData,
    __in DWORD cbData
    );
static HRESULT OnApplyUninitialize(
    __in HANDLE* phLock
    );
static HRESULT OnSessionBegin(
    __in BURN_REGISTRATION* pRegistration,
    __in BURN_VARIABLES* pVariables,
    __in BURN_USER_EXPERIENCE* pUserExperience,
    __in BYTE* pbData,
    __in DWORD cbData
    );
static HRESULT OnSessionResume(
    __in BURN_REGISTRATION* pRegistration,
    __in BYTE* pbData,
    __in DWORD cbData
    );
static HRESULT OnSessionEnd(
    __in BURN_REGISTRATION* pRegistration,
    __in BYTE* pbData,
    __in DWORD cbData
    );
static HRESULT OnSaveState(
    __in BURN_REGISTRATION* pRegistration,
    __in BYTE* pbData,
    __in DWORD cbData
    );
static HRESULT OnLayoutBundle(
    __in_z LPCWSTR wzExecutableName,
    __in BYTE* pbData,
    __in DWORD cbData
    );
static HRESULT OnCacheOrLayoutPayload(
    __in BURN_PACKAGES* pPackages,
    __in BURN_PAYLOADS* pPayloads,
    __in BYTE* pbData,
    __in DWORD cbData
    );
static void OnCacheCleanup(
    __in_z LPCWSTR wzBundleId
    );
static HRESULT OnExecuteExePackage(
    __in HANDLE hPipe,
    __in BURN_PACKAGES* pPackages,
    __in BURN_RELATED_BUNDLES* pRelatedBundles,
    __in BURN_VARIABLES* pVariables,
    __in BYTE* pbData,
    __in DWORD cbData
    );
static HRESULT OnExecuteMsiPackage(
    __in HANDLE hPipe,
    __in BURN_PACKAGES* pPackages,
    __in BURN_VARIABLES* pVariables,
    __in BYTE* pbData,
    __in DWORD cbData
    );
static HRESULT OnExecuteMspPackage(
    __in HANDLE hPipe,
    __in BURN_PACKAGES* pPackages,
    __in BURN_VARIABLES* pVariables,
    __in BYTE* pbData,
    __in DWORD cbData
    );
static HRESULT OnExecuteMsuPackage(
    __in HANDLE hPipe,
    __in BURN_PACKAGES* pPackages,
    __in BYTE* pbData,
    __in DWORD cbData
    );
static HRESULT OnExecuteDependencyAction(
    __in BURN_PACKAGES* pPackages,
    __in BURN_RELATED_BUNDLES* pRelatedBundles,
    __in BYTE* pbData,
    __in DWORD cbData
    );
static int GenericExecuteMessageHandler(
    __in GENERIC_EXECUTE_MESSAGE* pMessage,
    __in LPVOID pvContext
    );
static int MsiExecuteMessageHandler(
    __in WIU_MSI_EXECUTE_MESSAGE* pMessage,
    __in_opt LPVOID pvContext
    );
static HRESULT OnCleanPackage(
    __in BURN_PACKAGES* pPackages,
    __in BYTE* pbData,
    __in DWORD cbData
    );
static HRESULT OnDetectRelatedBundles(
    __in BURN_REGISTRATION *pRegistration,
    __in BYTE* pbData,
    __in DWORD cbData
    );


// function definitions

extern "C" HRESULT ElevationElevate(
    __in BURN_ENGINE_STATE* pEngineState,
    __in_opt HWND hwndParent
    )
{
    Assert(BURN_MODE_ELEVATED != pEngineState->mode);
    Assert(!pEngineState->companionConnection.sczName);
    Assert(!pEngineState->companionConnection.sczSecret);
    Assert(!pEngineState->companionConnection.hProcess);
    Assert(!pEngineState->companionConnection.dwProcessId);
    Assert(INVALID_HANDLE_VALUE == pEngineState->companionConnection.hPipe);
    Assert(INVALID_HANDLE_VALUE == pEngineState->companionConnection.hCachePipe);

    HRESULT hr = S_OK;
    int nResult = IDOK;
    HANDLE hPipesCreatedEvent = INVALID_HANDLE_VALUE;
    LPWSTR sczError = NULL;

    nResult = pEngineState->userExperience.pUserExperience->OnElevate();
    hr = UserExperienceInterpretResult(&pEngineState->userExperience, MB_OKCANCEL, nResult);
    ExitOnRootFailure(hr, "UX aborted elevation requirement.");

    hr = PipeCreateNameAndSecret(&pEngineState->companionConnection.sczName, &pEngineState->companionConnection.sczSecret);
    ExitOnFailure(hr, "Failed to create pipe name and client token.");

    hr = PipeCreatePipes(&pEngineState->companionConnection, TRUE, &hPipesCreatedEvent);
    ExitOnFailure(hr, "Failed to create pipe and cache pipe.");

    do
    {
        nResult = IDOK;

        // Create the elevated process and if successful, wait for it to connect.
        hr = PipeLaunchChildProcess(pEngineState->sczBundleEngineWorkingPath, &pEngineState->companionConnection, TRUE, hwndParent);
        if (SUCCEEDED(hr))
        {
            hr = PipeWaitForChildConnect(&pEngineState->companionConnection);
            ExitOnFailure(hr, "Failed to connect to elevated child process.");
        }
        else if (HRESULT_FROM_WIN32(ERROR_CANCELLED) == hr ||   // the user clicked "Cancel" on the elevation prompt or
                 HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED) == hr) // the elevation prompt timed out, provide the notification with the option to retry.
        {
            hr = HRESULT_FROM_WIN32(ERROR_INSTALL_USEREXIT); // treat the failure to elevate as a user choice.
            if (FAILED(StrAllocFromError(&sczError, hr, NULL)))
            {
                ReleaseNullStr(sczError);
            }

            nResult = pEngineState->userExperience.pUserExperience->OnError(BOOTSTRAPPER_ERROR_TYPE_ELEVATE, NULL, ERROR_INSTALL_USEREXIT, sczError, MB_ICONERROR | MB_RETRYCANCEL, 0, NULL, IDNOACTION);
        }
    } while (IDRETRY == nResult);
    ExitOnFailure(hr, "Failed to elevate.");

LExit:
    ReleaseStr(sczError);
    ReleaseHandle(hPipesCreatedEvent);

    if (FAILED(hr))
    {
        PipeConnectionUninitialize(&pEngineState->companionConnection);
    }

    return hr;
}

extern "C" HRESULT ElevationApplyInitialize(
    __in HANDLE hPipe,
    __in BURN_VARIABLES* pVariables,
    __in BOOTSTRAPPER_ACTION action,
    __in BURN_AU_PAUSE_ACTION auAction,
    __in BOOL fTakeSystemRestorePoint
    )
{
    HRESULT hr = S_OK;
    BYTE* pbData = NULL;
    SIZE_T cbData = 0;
    DWORD dwResult = 0;

    // serialize message data
    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)action);
    ExitOnFailure(hr, "Failed to write action to message buffer.");

    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)auAction);
    ExitOnFailure(hr, "Failed to write update action to message buffer.");

    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)fTakeSystemRestorePoint);
    ExitOnFailure(hr, "Failed to write system restore point action to message buffer.");
    
    hr = VariableSerialize(pVariables, FALSE, &pbData, &cbData);
    ExitOnFailure(hr, "Failed to write variables.");

    // send message
    hr = PipeSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_APPLY_INITIALIZE, pbData, cbData, NULL, NULL, &dwResult);
    ExitOnFailure(hr, "Failed to send message to per-machine process.");

    hr = (HRESULT)dwResult;

LExit:
    ReleaseBuffer(pbData);

    return hr;
}

extern "C" HRESULT ElevationApplyUninitialize(
    __in HANDLE hPipe
    )
{
    HRESULT hr = S_OK;
    BYTE* pbData = NULL;
    SIZE_T cbData = 0;
    DWORD dwResult = 0;

    // send message
    hr = PipeSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_APPLY_UNINITIALIZE, pbData, cbData, NULL, NULL, &dwResult);
    ExitOnFailure(hr, "Failed to send message to per-machine process.");

    hr = (HRESULT)dwResult;

LExit:
    ReleaseBuffer(pbData);

    return hr;
}

/*******************************************************************
 ElevationSessionBegin - 

*******************************************************************/
extern "C" HRESULT ElevationSessionBegin(
    __in HANDLE hPipe,
    __in_z LPCWSTR wzEngineWorkingPath,
    __in_z LPCWSTR wzResumeCommandLine,
    __in BURN_VARIABLES* pVariables,
    __in BOOTSTRAPPER_ACTION action,
    __in DWORD64 qwEstimatedSize
    )
{
    HRESULT hr = S_OK;
    BYTE* pbData = NULL;
    SIZE_T cbData = 0;
    DWORD dwResult = 0;

    // serialize message data
    hr = BuffWriteString(&pbData, &cbData, wzEngineWorkingPath);
    ExitOnFailure(hr, "Failed to write engine working path to message buffer.");

    hr = BuffWriteString(&pbData, &cbData, wzResumeCommandLine);
    ExitOnFailure(hr, "Failed to write resume command line to message buffer.");

    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)action);
    ExitOnFailure(hr, "Failed to write action to message buffer.");

    hr = BuffWriteNumber64(&pbData, &cbData, qwEstimatedSize);
    ExitOnFailure(hr, "Failed to write estimated size to message buffer.");

    hr = VariableSerialize(pVariables, FALSE, &pbData, &cbData);
    ExitOnFailure(hr, "Failed to write variables.");

    // send message
    hr = PipeSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_SESSION_BEGIN, pbData, cbData, NULL, NULL, &dwResult);
    ExitOnFailure(hr, "Failed to send message to per-machine process.");

    hr = (HRESULT)dwResult;

LExit:
    ReleaseBuffer(pbData);

    return hr;
}

/*******************************************************************
 ElevationSessionResume - 

*******************************************************************/
extern "C" HRESULT ElevationSessionResume(
    __in HANDLE hPipe,
    __in_z LPCWSTR wzResumeCommandLine
    )
{
    HRESULT hr = S_OK;
    BYTE* pbData = NULL;
    SIZE_T cbData = 0;
    DWORD dwResult = 0;

    // serialize message data
    hr = BuffWriteString(&pbData, &cbData, wzResumeCommandLine);
    ExitOnFailure(hr, "Failed to write resume command line to message buffer.");

    // send message
    hr = PipeSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_SESSION_RESUME, pbData, cbData, NULL, NULL, &dwResult);
    ExitOnFailure(hr, "Failed to send message to per-machine process.");

    hr = (HRESULT)dwResult;

LExit:
    ReleaseBuffer(pbData);

    return hr;
}

/*******************************************************************
 ElevationSessionEnd - 

*******************************************************************/
extern "C" HRESULT ElevationSessionEnd(
    __in HANDLE hPipe,
    __in BOOL fKeepRegistration,
    __in BOOL fSuspend,
    __in BOOTSTRAPPER_APPLY_RESTART restart
    )
{
    HRESULT hr = S_OK;
    BYTE* pbData = NULL;
    SIZE_T cbData = 0;
    DWORD dwResult = 0;

    // serialize message data
    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)fKeepRegistration);
    ExitOnFailure(hr, "Failed to write keep registration flag to message buffer.");

    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)fSuspend);
    ExitOnFailure(hr, "Failed to write suspend flag to message buffer.");

    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)restart);
    ExitOnFailure(hr, "Failed to write restart enum to message buffer.");

    // send message
    hr = PipeSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_SESSION_END, pbData, cbData, NULL, NULL, &dwResult);
    ExitOnFailure(hr, "Failed to send message to per-machine process.");

    hr = (HRESULT)dwResult;

LExit:
    ReleaseBuffer(pbData);

    return hr;
}

HRESULT ElevationDetectRelatedBundles(
    __in HANDLE hPipe
    )
{
    HRESULT hr = S_OK;
    BYTE* pbData = NULL;
    SIZE_T cbData = 0;
    DWORD dwResult = 0;

    // send message
    hr = PipeSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_DETECT_RELATED_BUNDLES, pbData, cbData, NULL, NULL, &dwResult);
    ExitOnFailure(hr, "Failed to send BURN_ELEVATION_MESSAGE_TYPE_DETECT_RELATED_BUNDLES message to per-machine process.");

    hr = (HRESULT)dwResult;

LExit:
    ReleaseBuffer(pbData);

    return hr;
}

/*******************************************************************
 ElevationSaveState - 

*******************************************************************/
HRESULT ElevationSaveState(
    __in HANDLE hPipe,
    __in_bcount(cbBuffer) BYTE* pbBuffer,
    __in SIZE_T cbBuffer
    )
{
    HRESULT hr = S_OK;
    DWORD dwResult = 0;

    // send message
    hr = PipeSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_SAVE_STATE, pbBuffer, (DWORD)cbBuffer, NULL, NULL, &dwResult);
    ExitOnFailure(hr, "Failed to send message to per-machine process.");

    hr = (HRESULT)dwResult;

LExit:
    return hr;
}

/*******************************************************************
 ElevationLayoutBundle - 

*******************************************************************/
extern "C" HRESULT ElevationLayoutBundle(
    __in HANDLE hPipe,
    __in_z LPCWSTR wzLayoutDirectory,
    __in_z LPCWSTR wzUnverifiedPath
    )
{
    HRESULT hr = S_OK;
    BYTE* pbData = NULL;
    SIZE_T cbData = 0;
    DWORD dwResult = 0;

    // serialize message data
    hr = BuffWriteString(&pbData, &cbData, wzLayoutDirectory);
    ExitOnFailure(hr, "Failed to write layout directory to message buffer.");

    hr = BuffWriteString(&pbData, &cbData, wzUnverifiedPath);
    ExitOnFailure(hr, "Failed to write payload unverified path to message buffer.");

    // send message
    hr = PipeSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_LAYOUT_BUNDLE, pbData, cbData, NULL, NULL, &dwResult);
    ExitOnFailure(hr, "Failed to send BURN_ELEVATION_MESSAGE_TYPE_LAYOUT_BUNDLE message to per-machine process.");

    hr = (HRESULT)dwResult;

LExit:
    ReleaseBuffer(pbData);

    return hr;
}

/*******************************************************************
 ElevationCacheOrLayoutPayload - 

*******************************************************************/
extern "C" HRESULT ElevationCacheOrLayoutPayload(
    __in HANDLE hPipe,
    __in_opt BURN_PACKAGE* pPackage,
    __in BURN_PAYLOAD* pPayload,
    __in_z_opt LPCWSTR wzLayoutDirectory,
    __in_z LPCWSTR wzUnverifiedPayloadPath,
    __in BOOL fMove
    )
{
    HRESULT hr = S_OK;
    BYTE* pbData = NULL;
    SIZE_T cbData = 0;
    DWORD dwResult = 0;

    // serialize message data
    hr = BuffWriteString(&pbData, &cbData, pPackage ? pPackage->sczId : NULL);
    ExitOnFailure(hr, "Failed to write package id to message buffer.");

    hr = BuffWriteString(&pbData, &cbData, pPayload->sczKey);
    ExitOnFailure(hr, "Failed to write payload id to message buffer.");

    hr = BuffWriteString(&pbData, &cbData, wzLayoutDirectory);
    ExitOnFailure(hr, "Failed to write layout directory to message buffer.");

    hr = BuffWriteString(&pbData, &cbData, wzUnverifiedPayloadPath);
    ExitOnFailure(hr, "Failed to write payload unverified path to message buffer.");

    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)fMove);
    ExitOnFailure(hr, "Failed to write move flag to message buffer.");

    // send message
    hr = PipeSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_CACHE_OR_LAYOUT_PAYLOAD, pbData, cbData, NULL, NULL, &dwResult);
    ExitOnFailure(hr, "Failed to send BURN_ELEVATION_MESSAGE_TYPE_CACHE_OR_LAYOUT_PAYLOAD message to per-machine process.");

    hr = (HRESULT)dwResult;

LExit:
    ReleaseBuffer(pbData);

    return hr;
}

/*******************************************************************
 ElevationCacheCleanup - 

*******************************************************************/
extern "C" HRESULT ElevationCacheCleanup(
    __in HANDLE hPipe
    )
{
    HRESULT hr = S_OK;
    DWORD dwResult = 0;

    // send message
    hr = PipeSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_CACHE_CLEANUP, NULL, 0, NULL, NULL, &dwResult);
    ExitOnFailure(hr, "Failed to send BURN_ELEVATION_MESSAGE_TYPE_CACHE_CLEANUP message to per-machine process.");

    hr = (HRESULT)dwResult;

LExit:
    return hr;}

/*******************************************************************
 ElevationExecuteExePackage - 

*******************************************************************/
extern "C" HRESULT ElevationExecuteExePackage(
    __in HANDLE hPipe,
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in BURN_VARIABLES* pVariables,
    __in BOOL fRollback,
    __in PFN_GENERICMESSAGEHANDLER pfnGenericMessageHandler,
    __in LPVOID pvContext,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    )
{
    HRESULT hr = S_OK;
    BYTE* pbData = NULL;
    SIZE_T cbData = 0;
    BURN_ELEVATION_GENERIC_MESSAGE_CONTEXT context = { };
    DWORD dwResult = 0;

    // serialize message data
    hr = BuffWriteString(&pbData, &cbData, pExecuteAction->exePackage.pPackage->sczId);
    ExitOnFailure(hr, "Failed to write package id to message buffer.");

    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)pExecuteAction->exePackage.action);
    ExitOnFailure(hr, "Failed to write action to message buffer.");

    hr = BuffWriteNumber(&pbData, &cbData, fRollback);
    ExitOnFailure(hr, "Failed to write rollback.");

    hr = BuffWriteString(&pbData, &cbData, pExecuteAction->exePackage.sczIgnoreDependencies);
    ExitOnFailure(hr, "Failed to write the list of dependencies to ignore to the message buffer.");

    hr = VariableSerialize(pVariables, FALSE, &pbData, &cbData);
    ExitOnFailure(hr, "Failed to write variables.");

    // send message
    context.pfnGenericMessageHandler = pfnGenericMessageHandler;
    context.pvContext = pvContext;

    hr = PipeSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_EXE_PACKAGE, pbData, cbData, ProcessGenericExecuteMessages, &context, &dwResult);
    ExitOnFailure(hr, "Failed to send BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_EXE_PACKAGE message to per-machine process.");

    hr = ProcessResult(dwResult, pRestart);

LExit:
    ReleaseBuffer(pbData);

    return hr;
}

/*******************************************************************
 ElevationExecuteMsiPackage - 

*******************************************************************/
extern "C" HRESULT ElevationExecuteMsiPackage(
    __in HANDLE hPipe,
    __in_opt HWND hwndParent,
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in BURN_VARIABLES* pVariables,
    __in BOOL fRollback,
    __in PFN_MSIEXECUTEMESSAGEHANDLER pfnMessageHandler,
    __in LPVOID pvContext,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    )
{
    HRESULT hr = S_OK;
    BYTE* pbData = NULL;
    SIZE_T cbData = 0;
    BURN_ELEVATION_MSI_MESSAGE_CONTEXT context = { };
    DWORD dwResult = 0;

    // serialize message data
    // TODO: for patching we might not have a package
    hr = BuffWriteString(&pbData, &cbData, pExecuteAction->msiPackage.pPackage->sczId);
    ExitOnFailure(hr, "Failed to write package id to message buffer.");

    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)hwndParent);
    ExitOnFailure(hr, "Failed to write parent hwnd to message buffer.");

    hr = BuffWriteString(&pbData, &cbData, pExecuteAction->msiPackage.sczLogPath);
    ExitOnFailure(hr, "Failed to write package log to message buffer.");

    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)pExecuteAction->msiPackage.uiLevel);
    ExitOnFailure(hr, "Failed to write UI level to message buffer.");

    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)pExecuteAction->msiPackage.action);
    ExitOnFailure(hr, "Failed to write action to message buffer.");

    // Feature actions.
    for (DWORD i = 0; i < pExecuteAction->msiPackage.pPackage->Msi.cFeatures; ++i)
    {
        hr = BuffWriteNumber(&pbData, &cbData, (DWORD)pExecuteAction->msiPackage.rgFeatures[i]);
        ExitOnFailure(hr, "Failed to write feature action to message buffer.");
    }

    // Slipstream patches actions.
    for (DWORD i = 0; i < pExecuteAction->msiPackage.pPackage->Msi.cSlipstreamMspPackages; ++i)
    {
        hr = BuffWriteNumber(&pbData, &cbData, (DWORD)pExecuteAction->msiPackage.rgSlipstreamPatches[i]);
        ExitOnFailure(hr, "Failed to write slipstream patch action to message buffer.");
    }

    hr = VariableSerialize(pVariables, FALSE, &pbData, &cbData);
    ExitOnFailure(hr, "Failed to write variables.");

    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)fRollback);
    ExitOnFailure(hr, "Failed to write rollback flag to message buffer.");


    // send message
    context.pfnMessageHandler = pfnMessageHandler;
    context.pvContext = pvContext;

    hr = PipeSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSI_PACKAGE, pbData, cbData, ProcessMsiPackageMessages, &context, &dwResult);
    ExitOnFailure(hr, "Failed to send BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSI_PACKAGE message to per-machine process.");

    hr = ProcessResult(dwResult, pRestart);

LExit:
    ReleaseBuffer(pbData);

    return hr;
}

/*******************************************************************
 ElevationExecuteMspPackage - 

*******************************************************************/
extern "C" HRESULT ElevationExecuteMspPackage(
    __in HANDLE hPipe,
    __in_opt HWND hwndParent,
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in BURN_VARIABLES* pVariables,
    __in BOOL fRollback,
    __in PFN_MSIEXECUTEMESSAGEHANDLER pfnMessageHandler,
    __in LPVOID pvContext,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    )
{
    HRESULT hr = S_OK;
    BYTE* pbData = NULL;
    SIZE_T cbData = 0;
    BURN_ELEVATION_MSI_MESSAGE_CONTEXT context = { };
    DWORD dwResult = 0;

    // serialize message data
    hr = BuffWriteString(&pbData, &cbData, pExecuteAction->mspTarget.pPackage->sczId);
    ExitOnFailure(hr, "Failed to write package id to message buffer.");

    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)hwndParent);
    ExitOnFailure(hr, "Failed to write parent hwnd to message buffer.");

    hr = BuffWriteString(&pbData, &cbData, pExecuteAction->mspTarget.sczTargetProductCode);
    ExitOnFailure(hr, "Failed to write target product code to message buffer.");

    hr = BuffWriteString(&pbData, &cbData, pExecuteAction->mspTarget.sczLogPath);
    ExitOnFailure(hr, "Failed to write package log to message buffer.");

    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)pExecuteAction->mspTarget.uiLevel);
    ExitOnFailure(hr, "Failed to write UI level to message buffer.");

    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)pExecuteAction->mspTarget.action);
    ExitOnFailure(hr, "Failed to write action to message buffer.");

    hr = BuffWriteNumber(&pbData, &cbData, pExecuteAction->mspTarget.cOrderedPatches);
    ExitOnFailure(hr, "Failed to write count of ordered patches to message buffer.");

    for (DWORD i = 0; i < pExecuteAction->mspTarget.cOrderedPatches; ++i)
    {
        hr = BuffWriteNumber(&pbData, &cbData, pExecuteAction->mspTarget.rgOrderedPatches[i].dwOrder);
        ExitOnFailure(hr, "Failed to write ordered patch order to message buffer.");

        hr = BuffWriteString(&pbData, &cbData, pExecuteAction->mspTarget.rgOrderedPatches[i].pPackage->sczId);
        ExitOnFailure(hr, "Failed to write ordered patch id to message buffer.");
    }

    hr = VariableSerialize(pVariables, FALSE, &pbData, &cbData);
    ExitOnFailure(hr, "Failed to write variables.");

    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)fRollback);
    ExitOnFailure(hr, "Failed to write rollback flag to message buffer.");

    // send message
    context.pfnMessageHandler = pfnMessageHandler;
    context.pvContext = pvContext;

    hr = PipeSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSP_PACKAGE, pbData, cbData, ProcessMsiPackageMessages, &context, &dwResult);
    ExitOnFailure(hr, "Failed to send BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSP_PACKAGE message to per-machine process.");

    hr = ProcessResult(dwResult, pRestart);

LExit:
    ReleaseBuffer(pbData);

    return hr;
}

/*******************************************************************
 ElevationExecuteMsuPackage - 

*******************************************************************/
extern "C" HRESULT ElevationExecuteMsuPackage(
    __in HANDLE hPipe,
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in BOOL fRollback,
    __in PFN_GENERICMESSAGEHANDLER pfnGenericMessageHandler,
    __in LPVOID pvContext,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    )
{
    HRESULT hr = S_OK;
    BYTE* pbData = NULL;
    SIZE_T cbData = 0;
    BURN_ELEVATION_GENERIC_MESSAGE_CONTEXT context = { };
    DWORD dwResult = 0;

    // serialize message data
    hr = BuffWriteString(&pbData, &cbData, pExecuteAction->msuPackage.pPackage->sczId);
    ExitOnFailure(hr, "Failed to write package id to message buffer.");

    hr = BuffWriteString(&pbData, &cbData, pExecuteAction->msuPackage.sczLogPath);
    ExitOnFailure(hr, "Failed to write package log to message buffer.");

    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)pExecuteAction->msuPackage.action);
    ExitOnFailure(hr, "Failed to write action to message buffer.");

    hr = BuffWriteNumber(&pbData, &cbData, fRollback);
    ExitOnFailure(hr, "Failed to write rollback.");

    // send message
    context.pfnGenericMessageHandler = pfnGenericMessageHandler;
    context.pvContext = pvContext;

    hr = PipeSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSU_PACKAGE, pbData, cbData, ProcessGenericExecuteMessages, &context, &dwResult);
    ExitOnFailure(hr, "Failed to send BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSU_PACKAGE message to per-machine process.");

    hr = ProcessResult(dwResult, pRestart);

LExit:
    ReleaseBuffer(pbData);

    return hr;
}

extern "C" HRESULT ElevationExecuteDependencyAction(
    __in HANDLE hPipe,
    __in BURN_EXECUTE_ACTION* pExecuteAction
    )
{
    HRESULT hr = S_OK;
    BYTE* pbData = NULL;
    SIZE_T cbData = 0;
    DWORD dwResult = 0;
    BOOTSTRAPPER_APPLY_RESTART restart = BOOTSTRAPPER_APPLY_RESTART_NONE;

    // Serialize the message data.
    hr = BuffWriteString(&pbData, &cbData, pExecuteAction->dependency.pPackage->sczId);
    ExitOnFailure(hr, "Failed to write package id to message buffer.");

    hr = BuffWriteString(&pbData, &cbData, pExecuteAction->dependency.sczBundleProviderKey);
    ExitOnFailure(hr, "Failed to write bundle dependency key to message buffer.");

    hr = BuffWriteNumber(&pbData, &cbData, pExecuteAction->dependency.action);
    ExitOnFailure(hr, "Failed to write action to message buffer.");

    // Send the message.
    hr = PipeSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_DEPENDENCY, pbData, cbData, NULL, NULL, &dwResult);
    ExitOnFailure(hr, "Failed to send BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_DEPENDENCY message to per-machine process.");

    // Ignore the restart since this action only results in registry writes.
    hr = ProcessResult(dwResult, &restart);

LExit:
    ReleaseBuffer(pbData);

    return hr;
}

/*******************************************************************
 ElevationCleanPackage - 

*******************************************************************/
extern "C" HRESULT ElevationCleanPackage(
    __in HANDLE hPipe,
    __in BURN_PACKAGE* pPackage
    )
{
    HRESULT hr = S_OK;
    BYTE* pbData = NULL;
    SIZE_T cbData = 0;
    DWORD dwResult = 0;

    // serialize message data
    hr = BuffWriteString(&pbData, &cbData, pPackage->sczId);
    ExitOnFailure(hr, "Failed to write clean package id to message buffer.");

    // send message
    hr = PipeSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_CLEAN_PACKAGE, pbData, cbData, NULL, NULL, &dwResult);
    ExitOnFailure(hr, "Failed to send BURN_ELEVATION_MESSAGE_TYPE_CLEAN_PACKAGE message to per-machine process.");

    hr = (HRESULT)dwResult;

LExit:
    ReleaseBuffer(pbData);

    return hr;
}

/*******************************************************************
 ElevationChildPumpMessages - 

*******************************************************************/
extern "C" HRESULT ElevationChildPumpMessages(
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
    )
{
    HRESULT hr = S_OK;
    BURN_ELEVATION_CHILD_MESSAGE_CONTEXT cacheContext = { };
    BURN_ELEVATION_CHILD_MESSAGE_CONTEXT context = { };
    HANDLE hCacheThread = NULL;
    BURN_PIPE_RESULT result = { };

    cacheContext.dwLoggingTlsId = dwLoggingTlsId;
    cacheContext.hPipe = hCachePipe;
    cacheContext.pPackages = pPackages;
    cacheContext.pRelatedBundles = pRelatedBundles;
    cacheContext.pPayloads = pPayloads;
    cacheContext.pVariables = pVariables;
    cacheContext.pRegistration = pRegistration;
    cacheContext.pUserExperience = pUserExperience;

    context.dwLoggingTlsId = dwLoggingTlsId;
    context.hPipe = hPipe;
    context.phLock = phLock;
    context.pfDisabledAutomaticUpdates = pfDisabledAutomaticUpdates;
    context.pPackages = pPackages;
    context.pRelatedBundles = pRelatedBundles;
    context.pPayloads = pPayloads;
    context.pVariables = pVariables;
    context.pRegistration = pRegistration;
    context.pUserExperience = pUserExperience;

    hCacheThread = ::CreateThread(NULL, 0, ElevatedChildCacheThreadProc, &cacheContext, 0, NULL);
    ExitOnNullWithLastError(hCacheThread, hr, "Failed to create elevated cache thread.");

    hr = PipePumpMessages(hPipe, ProcessElevatedChildMessage, &context, &result);
    ExitOnFailure(hr, "Failed to pump messages in child process.");

    // Wait for the cache thread and verify it gets the right result but don't fail if things
    // don't work out.
    WaitForElevatedChildCacheThread(hCacheThread, result.dwResult);

    *pdwChildExitCode = result.dwResult;
    *pfRestart = result.fRestart;

LExit:
    ReleaseHandle(hCacheThread);

    return hr;
}

extern "C" HRESULT ElevationChildResumeAutomaticUpdates()
{
    HRESULT hr = S_OK;

    hr = WuaResumeAutomaticUpdates();
    ExitOnFailure(hr, "Failed to resume automatic updates after pausing them, continuing...");

LExit:
    return hr;
}

// internal function definitions

static DWORD WINAPI ElevatedChildCacheThreadProc(
    __in LPVOID lpThreadParameter
    )
{
    HRESULT hr = S_OK;
    BURN_ELEVATION_CHILD_MESSAGE_CONTEXT* pContext = reinterpret_cast<BURN_ELEVATION_CHILD_MESSAGE_CONTEXT*>(lpThreadParameter);
    BOOL fComInitialized = FALSE;
    BURN_PIPE_RESULT result = { };

    if (!::TlsSetValue(pContext->dwLoggingTlsId, pContext->hPipe))
    {
        ExitWithLastError(hr, "Failed to set elevated cache pipe into thread local storage for logging.");
    }

    // initialize COM
    hr = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
    ExitOnFailure(hr, "Failed to initialize COM.");
    fComInitialized = TRUE;

    hr = PipePumpMessages(pContext->hPipe, ProcessElevatedChildCacheMessage, pContext, &result);
    ExitOnFailure(hr, "Failed to pump messages in child process.");

    hr = (HRESULT)result.dwResult;

LExit:
    if (fComInitialized)
    {
        ::CoUninitialize();
    }

    return (DWORD)hr;
}

static HRESULT WaitForElevatedChildCacheThread(
    __in HANDLE hCacheThread,
    __in DWORD dwExpectedExitCode
    )
{
    HRESULT hr = S_OK;
    DWORD dwExitCode = ERROR_SUCCESS;

    if (WAIT_OBJECT_0 != ::WaitForSingleObject(hCacheThread, BURN_TIMEOUT))
    {
        ExitWithLastError(hr, "Failed to wait for cache thread to terminate.");
    }

    if (!::GetExitCodeThread(hCacheThread, &dwExitCode))
    {
        ExitWithLastError(hr, "Failed to get cache thread exit code.");
    }

    AssertSz(dwExitCode == dwExpectedExitCode, "Cache thread should have exited with the expected exit code.");

LExit:
    return hr;
}

static HRESULT ProcessGenericExecuteMessages(
    __in BURN_PIPE_MESSAGE* pMsg,
    __in_opt LPVOID pvContext,
    __out DWORD* pdwResult
    )
{
    HRESULT hr = S_OK;
    SIZE_T iData = 0;
    BURN_ELEVATION_GENERIC_MESSAGE_CONTEXT* pContext = static_cast<BURN_ELEVATION_GENERIC_MESSAGE_CONTEXT*>(pvContext);
    LPWSTR sczMessage = NULL;
    DWORD cFiles = 0;
    LPWSTR* rgwzFiles = NULL;
    GENERIC_EXECUTE_MESSAGE message = { };

    hr = BuffReadNumber((BYTE*)pMsg->pvData, pMsg->cbData, &iData, &message.dwAllowedResults);
    ExitOnFailure(hr, "Failed to allowed results.");
    
    // Process the message.
    switch (pMsg->dwMessage)
    {
    case BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_PROGRESS:
        message.type = GENERIC_EXECUTE_MESSAGE_PROGRESS;

        // read message parameters
        hr = BuffReadNumber((BYTE*)pMsg->pvData, pMsg->cbData, &iData, &message.progress.dwPercentage);
        ExitOnFailure(hr, "Failed to progress.");
        break;

    case BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_ERROR:
        message.type = GENERIC_EXECUTE_MESSAGE_ERROR;

        // read message parameters
        hr = BuffReadNumber((BYTE*)pMsg->pvData, pMsg->cbData, &iData, &message.error.dwErrorCode);
        ExitOnFailure(hr, "Failed to read error code.");

        hr = BuffReadString((BYTE*)pMsg->pvData, pMsg->cbData, &iData, &sczMessage);
        ExitOnFailure(hr, "Failed to read message.");

        message.error.wzMessage = sczMessage;
        break;

    case BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_FILES_IN_USE:
        message.type = GENERIC_EXECUTE_MESSAGE_FILES_IN_USE;
        
        // read message parameters
        hr = BuffReadNumber((BYTE*)pMsg->pvData, pMsg->cbData, &iData, &cFiles);
        ExitOnFailure(hr, "Failed to read file count.");

        rgwzFiles = (LPWSTR*)MemAlloc(sizeof(LPWSTR*) * cFiles, TRUE);
        ExitOnNull(rgwzFiles, hr, E_OUTOFMEMORY, "Failed to allocate buffer for files in use.");

        for (DWORD i = 0; i < cFiles; ++i)
        {
            hr = BuffReadString((BYTE*)pMsg->pvData, pMsg->cbData, &iData, &rgwzFiles[i]);
            ExitOnFailure1(hr, "Failed to read file name: %u", i);
        }

        message.filesInUse.cFiles = cFiles;
        message.filesInUse.rgwzFiles = (LPCWSTR*)rgwzFiles;
        break;

    default:
        hr = E_INVALIDARG;
        ExitOnRootFailure(hr, "Invalid package message.");
        break;
    }

    // send message
    *pdwResult =  (DWORD)pContext->pfnGenericMessageHandler(&message, pContext->pvContext);;

LExit:
    ReleaseStr(sczMessage);

    if (rgwzFiles)
    {
        for (DWORD i = 0; i <= cFiles; ++i)
        {
            ReleaseStr(rgwzFiles[i]);
        }
        MemFree(rgwzFiles);
    }
    return hr;
}

static HRESULT ProcessMsiPackageMessages(
    __in BURN_PIPE_MESSAGE* pMsg,
    __in_opt LPVOID pvContext,
    __out DWORD* pdwResult
    )
{
    HRESULT hr = S_OK;
    SIZE_T iData = 0;
    WIU_MSI_EXECUTE_MESSAGE message = { };
    DWORD cMsiData = 0;
    LPWSTR* rgwzMsiData = NULL;
    BURN_ELEVATION_MSI_MESSAGE_CONTEXT* pContext = static_cast<BURN_ELEVATION_MSI_MESSAGE_CONTEXT*>(pvContext);
    LPWSTR sczMessage = NULL;

    // Read MSI extended message data.
    hr = BuffReadNumber((BYTE*)pMsg->pvData, pMsg->cbData, &iData, &cMsiData);
    ExitOnFailure(hr, "Failed to read MSI data count.");

    if (cMsiData)
    {
        rgwzMsiData = (LPWSTR*)MemAlloc(sizeof(LPWSTR*) * cMsiData, TRUE);
        ExitOnNull(rgwzMsiData, hr, E_OUTOFMEMORY, "Failed to allocate buffer to read MSI data.");

        for (DWORD i = 0; i < cMsiData; ++i)
        {
            hr = BuffReadString((BYTE*)pMsg->pvData, pMsg->cbData, &iData, &rgwzMsiData[i]);
            ExitOnFailure1(hr, "Failed to read MSI data: %u", i);
        }

        message.cData = cMsiData;
        message.rgwzData = (LPCWSTR*)rgwzMsiData;
    }

    hr = BuffReadNumber((BYTE*)pMsg->pvData, pMsg->cbData, &iData, (DWORD*)&message.dwAllowedResults);
    ExitOnFailure(hr, "Failed to read UI flags.");

    // Process the rest of the message.
    switch (pMsg->dwMessage)
    {
    case BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_PROGRESS:
        // read message parameters
        message.type = WIU_MSI_EXECUTE_MESSAGE_PROGRESS;

        hr = BuffReadNumber((BYTE*)pMsg->pvData, pMsg->cbData, &iData, &message.progress.dwPercentage);
        ExitOnFailure(hr, "Failed to read progress.");
        break;

    case BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_ERROR:
        // read message parameters
        message.type = WIU_MSI_EXECUTE_MESSAGE_ERROR;

        hr = BuffReadNumber((BYTE*)pMsg->pvData, pMsg->cbData, &iData, &message.error.dwErrorCode);
        ExitOnFailure(hr, "Failed to read error code.");

        hr = BuffReadString((BYTE*)pMsg->pvData, pMsg->cbData, &iData, &sczMessage);
        ExitOnFailure(hr, "Failed to read message.");
        message.error.wzMessage = sczMessage;
        break;

    case BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSI_MESSAGE:
        // read message parameters
        message.type = WIU_MSI_EXECUTE_MESSAGE_MSI_MESSAGE;

        hr = BuffReadNumber((BYTE*)pMsg->pvData, pMsg->cbData, &iData, (DWORD*)&message.msiMessage.mt);
        ExitOnFailure(hr, "Failed to read message type.");

        hr = BuffReadString((BYTE*)pMsg->pvData, pMsg->cbData, &iData, &sczMessage);
        ExitOnFailure(hr, "Failed to read message.");
        message.msiMessage.wzMessage = sczMessage;
        break;

    case BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_FILES_IN_USE:
        message.type = WIU_MSI_EXECUTE_MESSAGE_MSI_MESSAGE;
        message.msiFilesInUse.cFiles = cMsiData;
        message.msiFilesInUse.rgwzFiles = (LPCWSTR*)rgwzMsiData;
        break;

    default:
        hr = E_INVALIDARG;
        ExitOnRootFailure(hr, "Invalid package message.");
        break;
    }
    
    // send message
    *pdwResult = (DWORD)pContext->pfnMessageHandler(&message, pContext->pvContext);

LExit:
    ReleaseStr(sczMessage);

    if (rgwzMsiData)
    {
        for (DWORD i = 0; i < cMsiData; ++i)
        {
            ReleaseStr(rgwzMsiData[i]);
        }

        MemFree(rgwzMsiData);
    }

    return hr;
}

static HRESULT ProcessElevatedChildMessage(
    __in BURN_PIPE_MESSAGE* pMsg,
    __in_opt LPVOID pvContext,
    __out DWORD* pdwResult
    )
{
    HRESULT hr = S_OK;
    BURN_ELEVATION_CHILD_MESSAGE_CONTEXT* pContext = static_cast<BURN_ELEVATION_CHILD_MESSAGE_CONTEXT*>(pvContext);
    HRESULT hrResult = S_OK;
    DWORD dwPid = 0;

    switch (pMsg->dwMessage)
    {
    case BURN_ELEVATION_MESSAGE_TYPE_APPLY_INITIALIZE:
        hrResult = OnApplyInitialize(pContext->pVariables, pContext->phLock, pContext->pfDisabledAutomaticUpdates, (BYTE*)pMsg->pvData, pMsg->cbData);
        break;

    case BURN_ELEVATION_MESSAGE_TYPE_APPLY_UNINITIALIZE:
        hrResult = OnApplyUninitialize(pContext->phLock);
        break;

    case BURN_ELEVATION_MESSAGE_TYPE_SESSION_BEGIN:
        hrResult = OnSessionBegin(pContext->pRegistration, pContext->pVariables, pContext->pUserExperience, (BYTE*)pMsg->pvData, pMsg->cbData);
        break;

    case BURN_ELEVATION_MESSAGE_TYPE_SESSION_RESUME:
        hrResult = OnSessionResume(pContext->pRegistration, (BYTE*)pMsg->pvData, pMsg->cbData);
        break;

    case BURN_ELEVATION_MESSAGE_TYPE_SESSION_END:
        hrResult = OnSessionEnd(pContext->pRegistration, (BYTE*)pMsg->pvData, pMsg->cbData);
        break;

    case BURN_ELEVATION_MESSAGE_TYPE_DETECT_RELATED_BUNDLES:
        hrResult = OnDetectRelatedBundles(pContext->pRegistration, (BYTE*)pMsg->pvData, pMsg->cbData);
        break;

    case BURN_ELEVATION_MESSAGE_TYPE_SAVE_STATE:
        hrResult = OnSaveState(pContext->pRegistration, (BYTE*)pMsg->pvData, pMsg->cbData);
        break;

    case BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_EXE_PACKAGE:
        hrResult = OnExecuteExePackage(pContext->hPipe, pContext->pPackages, pContext->pRelatedBundles, pContext->pVariables, (BYTE*)pMsg->pvData, pMsg->cbData);
        break;

    case BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSI_PACKAGE:
        hrResult = OnExecuteMsiPackage(pContext->hPipe, pContext->pPackages, pContext->pVariables, (BYTE*)pMsg->pvData, pMsg->cbData);
        break;

    case BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSP_PACKAGE:
        hrResult = OnExecuteMspPackage(pContext->hPipe, pContext->pPackages, pContext->pVariables, (BYTE*)pMsg->pvData, pMsg->cbData);
        break;

    case BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSU_PACKAGE:
        hrResult = OnExecuteMsuPackage(pContext->hPipe, pContext->pPackages, (BYTE*)pMsg->pvData, pMsg->cbData);
        break;

    case BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_DEPENDENCY:
        hrResult = OnExecuteDependencyAction(pContext->pPackages, pContext->pRelatedBundles, (BYTE*)pMsg->pvData, pMsg->cbData);
        break;

    case BURN_ELEVATION_MESSAGE_TYPE_CLEAN_PACKAGE:
        hrResult = OnCleanPackage(pContext->pPackages, (BYTE*)pMsg->pvData, pMsg->cbData);
        break;

    default:
        hr = E_INVALIDARG;
        ExitOnRootFailure1(hr, "Unexpected elevated message sent to child process, msg: %u", pMsg->dwMessage);
    }

    *pdwResult = dwPid ? dwPid : (DWORD)hrResult;

LExit:
    return hr;
}

static HRESULT ProcessElevatedChildCacheMessage(
    __in BURN_PIPE_MESSAGE* pMsg,
    __in_opt LPVOID pvContext,
    __out DWORD* pdwResult
    )
{
    HRESULT hr = S_OK;
    BURN_ELEVATION_CHILD_MESSAGE_CONTEXT* pContext = static_cast<BURN_ELEVATION_CHILD_MESSAGE_CONTEXT*>(pvContext);
    HRESULT hrResult = S_OK;

    switch (pMsg->dwMessage)
    {
    case BURN_ELEVATION_MESSAGE_TYPE_LAYOUT_BUNDLE:
        hrResult = OnLayoutBundle(pContext->pRegistration->sczExecutableName, (BYTE*)pMsg->pvData, pMsg->cbData);
        break;

    case BURN_ELEVATION_MESSAGE_TYPE_CACHE_OR_LAYOUT_PAYLOAD:
        hrResult = OnCacheOrLayoutPayload(pContext->pPackages, pContext->pPayloads, (BYTE*)pMsg->pvData, pMsg->cbData);
        break;

    case BURN_ELEVATION_MESSAGE_TYPE_CACHE_CLEANUP:
        OnCacheCleanup(pContext->pRegistration->sczId);
        hrResult = S_OK;
        break;

    case BURN_ELEVATION_MESSAGE_TYPE_CLEAN_PACKAGE:
        hrResult = OnCleanPackage(pContext->pPackages, (BYTE*)pMsg->pvData, pMsg->cbData);
        break;

    default:
        hr = E_INVALIDARG;
        ExitOnRootFailure1(hr, "Unexpected elevated cache message sent to child process, msg: %u", pMsg->dwMessage);
    }

    *pdwResult = (DWORD)hrResult;

LExit:
    return hr;
}

static HRESULT ProcessResult(
    __in DWORD dwResult,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    )
{
    HRESULT hr = static_cast<HRESULT>(dwResult);
    if (HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_REQUIRED) == hr)
    {
        *pRestart = BOOTSTRAPPER_APPLY_RESTART_REQUIRED;
        hr = S_OK;
    }
    else if (HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_INITIATED) == hr)
    {
        *pRestart = BOOTSTRAPPER_APPLY_RESTART_INITIATED;
        hr = S_OK;
    }

    return hr;
}

static HRESULT OnApplyInitialize(
    __in BURN_VARIABLES* pVariables,
    __in HANDLE* phLock,
    __in BOOL* pfDisabledWindowsUpdate,
    __in BYTE* pbData,
    __in DWORD cbData
    )
{
    HRESULT hr = S_OK;
    SIZE_T iData = 0;
    DWORD dwAction = 0;
    DWORD dwAUAction = 0;
    DWORD dwTakeSystemRestorePoint = 0;
    LPWSTR sczBundleName = NULL;

    // deserialize message data
    hr = BuffReadNumber(pbData, cbData, &iData, &dwAction);
    ExitOnFailure(hr, "Failed to read action.");

    hr = BuffReadNumber(pbData, cbData, &iData, &dwAUAction);
    ExitOnFailure(hr, "Failed to read update action.");

    hr = BuffReadNumber(pbData, cbData, &iData, &dwTakeSystemRestorePoint);
    ExitOnFailure(hr, "Failed to read sysem restore point action.");

    hr = VariableDeserialize(pVariables, pbData, cbData, &iData);
    ExitOnFailure(hr, "Failed to read variables.");

    // initialize.
    hr = ApplyLock(TRUE, phLock);
    ExitOnFailure(hr, "Failed to acquire lock due to setup in other session.");

    // Attempt to pause AU with best effort.
    if (BURN_AU_PAUSE_ACTION_IFELEVATED == dwAUAction || BURN_AU_PAUSE_ACTION_IFELEVATED_NORESUME == dwAUAction)
    {
        hr = WuaPauseAutomaticUpdates();
        if (FAILED(hr))
        {
            LogId(REPORT_STANDARD, MSG_FAILED_PAUSE_AU, hr);
            hr = S_OK;
        }
        else if (BURN_AU_PAUSE_ACTION_IFELEVATED == dwAUAction)
        {
            *pfDisabledWindowsUpdate = TRUE;
        }
    }

    if (dwTakeSystemRestorePoint)
    {
        hr = VariableGetString(pVariables, BURN_BUNDLE_NAME, &sczBundleName);
        if (FAILED(hr))
        {
            hr = S_OK;
            ExitFunction();
        }

        LogId(REPORT_STANDARD, MSG_SYSTEM_RESTORE_POINT_STARTING);

        BOOTSTRAPPER_ACTION action = static_cast<BOOTSTRAPPER_ACTION>(dwAction);
        SRP_ACTION restoreAction = (BOOTSTRAPPER_ACTION_INSTALL == action) ? SRP_ACTION_INSTALL : (BOOTSTRAPPER_ACTION_UNINSTALL == action) ? SRP_ACTION_UNINSTALL : SRP_ACTION_MODIFY;
        hr = SrpCreateRestorePoint(sczBundleName, restoreAction);
        if (SUCCEEDED(hr))
        {
            LogId(REPORT_STANDARD, MSG_SYSTEM_RESTORE_POINT_SUCCEEDED);
        }
        else if (E_NOTIMPL == hr)
        {
            LogId(REPORT_STANDARD, MSG_SYSTEM_RESTORE_POINT_DISABLED);
            hr = S_OK;
        }
        else if (FAILED(hr))
        {
            LogId(REPORT_STANDARD, MSG_SYSTEM_RESTORE_POINT_FAILED, hr);
            hr = S_OK;
        }
    }

LExit:
    ReleaseStr(sczBundleName);
    return hr;
}

static HRESULT OnApplyUninitialize(
    __in HANDLE* phLock
    )
{
    Assert(phLock);

    // TODO: end system restore point.

    if (*phLock)
    {
        ::ReleaseMutex(*phLock);
        ::CloseHandle(*phLock);
        *phLock = NULL;
    }

    return S_OK;
}

static HRESULT OnSessionBegin(
    __in BURN_REGISTRATION* pRegistration,
    __in BURN_VARIABLES* pVariables,
    __in BURN_USER_EXPERIENCE* pUserExperience,
    __in BYTE* pbData,
    __in DWORD cbData
    )
{
    HRESULT hr = S_OK;
    SIZE_T iData = 0;
    LPWSTR sczEngineWorkingPath = NULL;
    DWORD action = 0;
    DWORD64 qwEstimatedSize = 0;

    // deserialize message data
    hr = BuffReadString(pbData, cbData, &iData, &sczEngineWorkingPath);
    ExitOnFailure(hr, "Failed to read engine working path.");

    hr = BuffReadString(pbData, cbData, &iData, &pRegistration->sczResumeCommandLine);
    ExitOnFailure(hr, "Failed to read resume command line.");

    hr = BuffReadNumber(pbData, cbData, &iData, &action);
    ExitOnFailure(hr, "Failed to read action.");

    hr = BuffReadNumber64(pbData, cbData, &iData, &qwEstimatedSize);
    ExitOnFailure(hr, "Failed to read estimated size.");

    hr = VariableDeserialize(pVariables, pbData, cbData, &iData);
    ExitOnFailure(hr, "Failed to read variables.");

    // begin session in per-machine process
    hr =  RegistrationSessionBegin(sczEngineWorkingPath, pRegistration, pVariables, pUserExperience, (BOOTSTRAPPER_ACTION)action, qwEstimatedSize, TRUE);
    ExitOnFailure(hr, "Failed to begin registration session.");

LExit:
    ReleaseStr(sczEngineWorkingPath);

    return hr;
}

static HRESULT OnSessionResume(
    __in BURN_REGISTRATION* pRegistration,
    __in BYTE* pbData,
    __in DWORD cbData
    )
{
    HRESULT hr = S_OK;
    SIZE_T iData = 0;

    // deserialize message data
    hr = BuffReadString(pbData, cbData, &iData, &pRegistration->sczResumeCommandLine);
    ExitOnFailure(hr, "Failed to read resume command line.");

    // suspend session in per-machine process
    hr = RegistrationSessionResume(pRegistration, TRUE);
    ExitOnFailure(hr, "Failed to suspend registration session.");

LExit:
    return hr;
}

static HRESULT OnSessionEnd(
    __in BURN_REGISTRATION* pRegistration,
    __in BYTE* pbData,
    __in DWORD cbData
    )
{
    HRESULT hr = S_OK;
    SIZE_T iData = 0;
    DWORD dwKeepRegistration = 0;
    DWORD dwSuspend = 0;
    DWORD dwRestart = 0;

    // deserialize message data
    hr = BuffReadNumber(pbData, cbData, &iData, &dwKeepRegistration);
    ExitOnFailure(hr, "Failed to read keep registration flag.");

    hr = BuffReadNumber(pbData, cbData, &iData, &dwSuspend);
    ExitOnFailure(hr, "Failed to read suspend flag.");

    hr = BuffReadNumber(pbData, cbData, &iData, &dwRestart);
    ExitOnFailure(hr, "Failed to read restart enum.");

    // suspend session in per-machine process
    hr = RegistrationSessionEnd(pRegistration, (BOOL)dwKeepRegistration, (BOOL)dwSuspend, (BOOTSTRAPPER_APPLY_RESTART)dwRestart, TRUE, NULL);
    ExitOnFailure(hr, "Failed to suspend registration session.");

LExit:
    return hr;
}

static HRESULT OnDetectRelatedBundles(
    __in BURN_REGISTRATION *pRegistration,
    __in BYTE* /*pbData*/,
    __in DWORD /*cbData*/
    )
{
    HRESULT hr = S_OK;

    hr = RegistrationDetectRelatedBundles(TRUE, NULL, pRegistration, NULL);
    ExitOnFailure(hr, "Failed to detect related bundles in elevated process");

LExit:
    return hr;
}

static HRESULT OnSaveState(
    __in BURN_REGISTRATION* pRegistration,
    __in BYTE* pbData,
    __in DWORD cbData
    )
{
    HRESULT hr = S_OK;

    // save state in per-machine process
    hr = RegistrationSaveState(pRegistration, pbData, cbData);
    ExitOnFailure(hr, "Failed to save state.");

LExit:
    return hr;
}

static HRESULT OnLayoutBundle(
    __in_z LPCWSTR wzExecutableName,
    __in BYTE* pbData,
    __in DWORD cbData
    )
{
    HRESULT hr = S_OK;
    SIZE_T iData = 0;
    LPWSTR sczLayoutDirectory = NULL;
    LPWSTR sczUnverifiedPath = NULL;

    // deserialize message data
    hr = BuffReadString(pbData, cbData, &iData, &sczLayoutDirectory);
    ExitOnFailure(hr, "Failed to read layout directory.");

    hr = BuffReadString(pbData, cbData, &iData, &sczUnverifiedPath);
    ExitOnFailure(hr, "Failed to read unverified bundle path.");

    // Layout the bundle.
    hr = CacheLayoutBundle(wzExecutableName, sczLayoutDirectory, sczUnverifiedPath);
    ExitOnFailure1(hr, "Failed to layout bundle from: %ls", sczUnverifiedPath);

LExit:
    ReleaseStr(sczUnverifiedPath);
    ReleaseStr(sczLayoutDirectory);

    return hr;
}

static HRESULT OnCacheOrLayoutPayload(
    __in BURN_PACKAGES* pPackages,
    __in BURN_PAYLOADS* pPayloads,
    __in BYTE* pbData,
    __in DWORD cbData
    )
{
    HRESULT hr = S_OK;
    SIZE_T iData = 0;
    LPWSTR scz = NULL;
    BURN_PACKAGE* pPackage = NULL;
    BURN_PAYLOAD* pPayload = NULL;
    LPWSTR sczLayoutDirectory = NULL;
    LPWSTR sczUnverifiedPayloadPath = NULL;
    BOOL fMove = FALSE;

    // deserialize message data
    hr = BuffReadString(pbData, cbData, &iData, &scz);
    ExitOnFailure(hr, "Failed to read package id.");

    if (scz && *scz)
    {
        hr = PackageFindById(pPackages, scz, &pPackage);
        ExitOnFailure1(hr, "Failed to find package: %ls", scz);
    }

    hr = BuffReadString(pbData, cbData, &iData, &scz);
    ExitOnFailure(hr, "Failed to read payload id.");

    hr = PayloadFindById(pPayloads, scz, &pPayload);
    ExitOnFailure1(hr, "Failed to find payload: %ls", scz);

    hr = BuffReadString(pbData, cbData, &iData, &sczLayoutDirectory);
    ExitOnFailure(hr, "Failed to read layout directory.");

    hr = BuffReadString(pbData, cbData, &iData, &sczUnverifiedPayloadPath);
    ExitOnFailure(hr, "Failed to read unverified payload path.");

    hr = BuffReadNumber(pbData, cbData, &iData, (DWORD*)&fMove);
    ExitOnFailure(hr, "Failed to read move flag.");

    // Layout payload.
    if (sczLayoutDirectory && *sczLayoutDirectory)
    {
        hr = CacheLayoutPayload(pPayload, sczLayoutDirectory, sczUnverifiedPayloadPath, fMove);
        ExitOnFailure2(hr, "Failed to layout payload from: %ls to %ls", sczUnverifiedPayloadPath, sczLayoutDirectory);
    }
    else if (pPackage) // complete payload.
    {
        hr = CacheCompletePayload(pPackage->fPerMachine, pPayload, pPackage->sczCacheId, sczUnverifiedPayloadPath, fMove);
        ExitOnFailure1(hr, "Failed to cache payload: %ls", pPayload->sczKey);
    }
    else
    {
        hr = E_INVALIDARG;
        ExitOnRootFailure(hr, "Invalid data passed to cache or layout payload.");
    }

LExit:
    ReleaseStr(sczUnverifiedPayloadPath);
    ReleaseStr(sczLayoutDirectory);
    ReleaseStr(scz);

    return hr;
}

static void OnCacheCleanup(
    __in_z LPCWSTR wzBundleId
    )
{
    CacheCleanup(TRUE, wzBundleId);
}

static HRESULT OnExecuteExePackage(
    __in HANDLE hPipe,
    __in BURN_PACKAGES* pPackages,
    __in BURN_RELATED_BUNDLES* pRelatedBundles,
    __in BURN_VARIABLES* pVariables,
    __in BYTE* pbData,
    __in DWORD cbData
    )
{
    HRESULT hr = S_OK;
    SIZE_T iData = 0;
    LPWSTR sczPackage = NULL;
    DWORD dwRollback = 0;
    BURN_EXECUTE_ACTION executeAction = { };
    LPWSTR sczIgnoreDependencies = NULL;
    BOOTSTRAPPER_APPLY_RESTART exeRestart = BOOTSTRAPPER_APPLY_RESTART_NONE;

    executeAction.type = BURN_EXECUTE_ACTION_TYPE_EXE_PACKAGE;

    // deserialize message data
    hr = BuffReadString(pbData, cbData, &iData, &sczPackage);
    ExitOnFailure(hr, "Failed to read exe package.");

    hr = BuffReadNumber(pbData, cbData, &iData, (DWORD*)&executeAction.exePackage.action);
    ExitOnFailure(hr, "Failed to read action.");

    hr = BuffReadNumber(pbData, cbData, &iData, &dwRollback);
    ExitOnFailure(hr, "Failed to read rollback.");

    hr = BuffReadString(pbData, cbData, &iData, &sczIgnoreDependencies);
    ExitOnFailure(hr, "Failed to read the list of dependencies to ignore.");

    hr = VariableDeserialize(pVariables, pbData, cbData, &iData);
    ExitOnFailure(hr, "Failed to read variables.");

    hr = PackageFindById(pPackages, sczPackage, &executeAction.exePackage.pPackage);
    if (E_NOTFOUND == hr)
    {
        hr = PackageFindRelatedById(pRelatedBundles, sczPackage, &executeAction.exePackage.pPackage);
    }
    ExitOnFailure1(hr, "Failed to find package: %ls", sczPackage);

    // Pass the list of dependencies to ignore, if any, to the related bundle.
    if (sczIgnoreDependencies && *sczIgnoreDependencies)
    {
        hr = StrAllocString(&executeAction.exePackage.sczIgnoreDependencies, sczIgnoreDependencies, 0);
        ExitOnFailure(hr, "Failed to allocate the list of dependencies to ignore.");
    }

    // execute EXE package
    hr = ExeEngineExecutePackage(&executeAction, pVariables, static_cast<BOOL>(dwRollback), GenericExecuteMessageHandler, hPipe, &exeRestart);
    ExitOnFailure(hr, "Failed to execute EXE package.");

LExit:
    ReleaseStr(sczIgnoreDependencies);
    ReleaseStr(sczPackage);
    PlanUninitializeExecuteAction(&executeAction);

    if (SUCCEEDED(hr))
    {
        if (BOOTSTRAPPER_APPLY_RESTART_REQUIRED == exeRestart)
        {
            hr = HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_REQUIRED);
        }
        else if (BOOTSTRAPPER_APPLY_RESTART_INITIATED == exeRestart)
        {
            hr = HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_INITIATED);
        }
    }

    return hr;
}

static HRESULT OnExecuteMsiPackage(
    __in HANDLE hPipe,
    __in BURN_PACKAGES* pPackages,
    __in BURN_VARIABLES* pVariables,
    __in BYTE* pbData,
    __in DWORD cbData
    )
{
    HRESULT hr = S_OK;
    SIZE_T iData = 0;
    LPWSTR sczPackage = NULL;
    HWND hwndParent = NULL;
    BOOL fRollback = 0;
    BURN_EXECUTE_ACTION executeAction = { };
    BOOTSTRAPPER_APPLY_RESTART msiRestart = BOOTSTRAPPER_APPLY_RESTART_NONE;

    executeAction.type = BURN_EXECUTE_ACTION_TYPE_MSI_PACKAGE;

    // deserialize message data
    hr = BuffReadString(pbData, cbData, &iData, &sczPackage);
    ExitOnFailure(hr, "Failed to read action.");

    hr = PackageFindById(pPackages, sczPackage, &executeAction.msiPackage.pPackage);
    ExitOnFailure1(hr, "Failed to find package: %ls", sczPackage);

    hr = BuffReadNumber(pbData, cbData, &iData, (DWORD*)&hwndParent);
    ExitOnFailure(hr, "Failed to read parent hwnd.");

    hr = BuffReadString(pbData, cbData, &iData, &executeAction.msiPackage.sczLogPath);
    ExitOnFailure(hr, "Failed to read package log.");

    hr = BuffReadNumber(pbData, cbData, &iData, (DWORD*)&executeAction.msiPackage.uiLevel);
    ExitOnFailure(hr, "Failed to read UI level.");

    hr = BuffReadNumber(pbData, cbData, &iData, (DWORD*)&executeAction.msiPackage.action);
    ExitOnFailure(hr, "Failed to read action.");

    // Read feature actions.
    if (executeAction.msiPackage.pPackage->Msi.cFeatures)
    {
        executeAction.msiPackage.rgFeatures = (BOOTSTRAPPER_FEATURE_ACTION*)MemAlloc(executeAction.msiPackage.pPackage->Msi.cFeatures * sizeof(BOOTSTRAPPER_FEATURE_ACTION), TRUE);
        ExitOnNull(executeAction.msiPackage.rgFeatures, hr, E_OUTOFMEMORY, "Failed to allocate memory for feature actions.");

        for (DWORD i = 0; i < executeAction.msiPackage.pPackage->Msi.cFeatures; ++i)
        {
            hr = BuffReadNumber(pbData, cbData, &iData, (DWORD*)&executeAction.msiPackage.rgFeatures[i]);
            ExitOnFailure(hr, "Failed to read feature action.");
        }
    }

    // Read slipstream patches actions.
    if (executeAction.msiPackage.pPackage->Msi.cSlipstreamMspPackages)
    {
        executeAction.msiPackage.rgSlipstreamPatches = (BOOTSTRAPPER_ACTION_STATE*)MemAlloc(executeAction.msiPackage.pPackage->Msi.cSlipstreamMspPackages * sizeof(BOOTSTRAPPER_ACTION_STATE), TRUE);
        ExitOnNull(executeAction.msiPackage.rgSlipstreamPatches, hr, E_OUTOFMEMORY, "Failed to allocate memory for slipstream patch actions.");
        
        for (DWORD i = 0; i < executeAction.msiPackage.pPackage->Msi.cSlipstreamMspPackages; ++i)
        {
            hr = BuffReadNumber(pbData, cbData, &iData, (DWORD*)&executeAction.msiPackage.rgSlipstreamPatches[i]);
            ExitOnFailure(hr, "Failed to read slipstream action.");
        }
    }

    hr = VariableDeserialize(pVariables, pbData, cbData, &iData);
    ExitOnFailure(hr, "Failed to read variables.");

    hr = BuffReadNumber(pbData, cbData, &iData, (DWORD*)&fRollback);
    ExitOnFailure(hr, "Failed to read rollback flag.");

    // execute MSI package
    hr = MsiEngineExecutePackage(hwndParent, &executeAction, pVariables, fRollback, MsiExecuteMessageHandler, hPipe, &msiRestart);
    ExitOnFailure(hr, "Failed to execute MSI package.");

LExit:
    ReleaseStr(sczPackage);
    PlanUninitializeExecuteAction(&executeAction);

    if (SUCCEEDED(hr))
    {
        if (BOOTSTRAPPER_APPLY_RESTART_REQUIRED == msiRestart)
        {
            hr = HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_REQUIRED);
        }
        else if (BOOTSTRAPPER_APPLY_RESTART_INITIATED == msiRestart)
        {
            hr = HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_INITIATED);
        }
    }

    return hr;
}

static HRESULT OnExecuteMspPackage(
    __in HANDLE hPipe,
    __in BURN_PACKAGES* pPackages,
    __in BURN_VARIABLES* pVariables,
    __in BYTE* pbData,
    __in DWORD cbData
    )
{
    HRESULT hr = S_OK;
    SIZE_T iData = 0;
    LPWSTR sczPackage = NULL;
    HWND hwndParent = NULL;
    BOOL fRollback = 0;
    BURN_EXECUTE_ACTION executeAction = { };
    BOOTSTRAPPER_APPLY_RESTART restart = BOOTSTRAPPER_APPLY_RESTART_NONE;

    executeAction.type = BURN_EXECUTE_ACTION_TYPE_MSP_TARGET;

    // deserialize message data
    hr = BuffReadString(pbData, cbData, &iData, &sczPackage);
    ExitOnFailure(hr, "Failed to read action.");

    hr = PackageFindById(pPackages, sczPackage, &executeAction.mspTarget.pPackage);
    ExitOnFailure1(hr, "Failed to find package: %ls", sczPackage);

    hr = BuffReadNumber(pbData, cbData, &iData, (DWORD*)&hwndParent);
    ExitOnFailure(hr, "Failed to read parent hwnd.");

    executeAction.mspTarget.fPerMachineTarget = TRUE; // we're in the elevated process, clearly we're targeting a per-machine product.

    hr = BuffReadString(pbData, cbData, &iData, &executeAction.mspTarget.sczTargetProductCode);
    ExitOnFailure(hr, "Failed to read package log.");

    hr = BuffReadString(pbData, cbData, &iData, &executeAction.mspTarget.sczLogPath);
    ExitOnFailure(hr, "Failed to read package log.");

    hr = BuffReadNumber(pbData, cbData, &iData, (DWORD*)&executeAction.mspTarget.uiLevel);
    ExitOnFailure(hr, "Failed to read UI level.");

    hr = BuffReadNumber(pbData, cbData, &iData, (DWORD*)&executeAction.mspTarget.action);
    ExitOnFailure(hr, "Failed to read action.");

    hr = BuffReadNumber(pbData, cbData, &iData, (DWORD*)&executeAction.mspTarget.cOrderedPatches);
    ExitOnFailure(hr, "Failed to read count of ordered patches.");

    if (executeAction.mspTarget.cOrderedPatches)
    {
        executeAction.mspTarget.rgOrderedPatches = (BURN_ORDERED_PATCHES*)MemAlloc(executeAction.mspTarget.cOrderedPatches * sizeof(BURN_ORDERED_PATCHES), TRUE);
        ExitOnNull(executeAction.mspTarget.rgOrderedPatches, hr, E_OUTOFMEMORY, "Failed to allocate memory for ordered patches.");

        for (DWORD i = 0; i < executeAction.mspTarget.cOrderedPatches; ++i)
        {
            hr = BuffReadNumber(pbData, cbData, &iData, &executeAction.mspTarget.rgOrderedPatches->dwOrder);
            ExitOnFailure(hr, "Failed to read ordered patch order number.");

            hr = BuffReadString(pbData, cbData, &iData, &sczPackage);
            ExitOnFailure(hr, "Failed to read ordered patch package id.");

            hr = PackageFindById(pPackages, sczPackage, &executeAction.mspTarget.rgOrderedPatches->pPackage);
            ExitOnFailure1(hr, "Failed to find ordered patch package: %ls", sczPackage);
        }
    }

    hr = VariableDeserialize(pVariables, pbData, cbData, &iData);
    ExitOnFailure(hr, "Failed to read variables.");

    hr = BuffReadNumber(pbData, cbData, &iData, (DWORD*)&fRollback);
    ExitOnFailure(hr, "Failed to read rollback flag.");

    // execute MSP package
    hr = MspEngineExecutePackage(hwndParent, &executeAction, pVariables, fRollback, MsiExecuteMessageHandler, hPipe, &restart);
    ExitOnFailure(hr, "Failed to execute MSP package.");

LExit:
    ReleaseStr(sczPackage);
    PlanUninitializeExecuteAction(&executeAction);

    if (SUCCEEDED(hr))
    {
        if (BOOTSTRAPPER_APPLY_RESTART_REQUIRED == restart)
        {
            hr = HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_REQUIRED);
        }
        else if (BOOTSTRAPPER_APPLY_RESTART_INITIATED == restart)
        {
            hr = HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_INITIATED);
        }
    }

    return hr;
}

static HRESULT OnExecuteMsuPackage(
    __in HANDLE hPipe,
    __in BURN_PACKAGES* pPackages,
    __in BYTE* pbData,
    __in DWORD cbData
    )
{
    HRESULT hr = S_OK;
    SIZE_T iData = 0;
    LPWSTR sczPackage = NULL;
    DWORD dwRollback = 0;
    BURN_EXECUTE_ACTION executeAction = { };
    BOOTSTRAPPER_APPLY_RESTART restart = BOOTSTRAPPER_APPLY_RESTART_NONE;

    executeAction.type = BURN_EXECUTE_ACTION_TYPE_MSU_PACKAGE;

    // deserialize message data
    hr = BuffReadString(pbData, cbData, &iData, &sczPackage);
    ExitOnFailure(hr, "Failed to read package id.");

    hr = BuffReadString(pbData, cbData, &iData, &executeAction.msuPackage.sczLogPath);
    ExitOnFailure(hr, "Failed to read package log.");

    hr = BuffReadNumber(pbData, cbData, &iData, reinterpret_cast<DWORD*>(&executeAction.msuPackage.action));
    ExitOnFailure(hr, "Failed to read action.");

    hr = BuffReadNumber(pbData, cbData, &iData, &dwRollback);
    ExitOnFailure(hr, "Failed to read rollback.");

    hr = PackageFindById(pPackages, sczPackage, &executeAction.msuPackage.pPackage);
    ExitOnFailure1(hr, "Failed to find package: %ls", sczPackage);

    // execute MSU package
    hr = MsuEngineExecutePackage(&executeAction, static_cast<BOOL>(dwRollback), GenericExecuteMessageHandler, hPipe, &restart);
    ExitOnFailure(hr, "Failed to execute MSU package.");

LExit:
    ReleaseStr(sczPackage);
    PlanUninitializeExecuteAction(&executeAction);

    if (SUCCEEDED(hr))
    {
        if (BOOTSTRAPPER_APPLY_RESTART_REQUIRED == restart)
        {
            hr = HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_REQUIRED);
        }
        else if (BOOTSTRAPPER_APPLY_RESTART_INITIATED == restart)
        {
            hr = HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_INITIATED);
        }
    }

    return hr;
}

static HRESULT OnExecuteDependencyAction(
    __in BURN_PACKAGES* pPackages,
    __in BURN_RELATED_BUNDLES* pRelatedBundles,
    __in BYTE* pbData,
    __in DWORD cbData
    )
{
    HRESULT hr = S_OK;
    SIZE_T iData = 0;
    LPWSTR sczPackage = NULL;
    BURN_EXECUTE_ACTION executeAction = { };

    executeAction.type = BURN_EXECUTE_ACTION_TYPE_DEPENDENCY;

    // Deserialize the message data.
    hr = BuffReadString(pbData, cbData, &iData, &sczPackage);
    ExitOnFailure(hr, "Failed to read package id from message buffer.");

    hr = BuffReadString(pbData, cbData, &iData, &executeAction.dependency.sczBundleProviderKey);
    ExitOnFailure(hr, "Failed to read bundle dependency key from message buffer.");

    hr = BuffReadNumber(pbData, cbData, &iData, reinterpret_cast<DWORD*>(&executeAction.dependency.action));
    ExitOnFailure(hr, "Failed to read action.");

    // Find the package again.
    hr = PackageFindById(pPackages, sczPackage, &executeAction.dependency.pPackage);
    if (E_NOTFOUND == hr)
    {
        hr = PackageFindRelatedById(pRelatedBundles, sczPackage, &executeAction.dependency.pPackage);
    }
    ExitOnFailure1(hr, "Failed to find package: %ls", sczPackage);

    // Execute the dependency action.
    hr = DependencyExecuteAction(&executeAction, TRUE);
    ExitOnFailure(hr, "Failed to execute dependency action.");

LExit:
    return hr;
}

static int GenericExecuteMessageHandler(
    __in GENERIC_EXECUTE_MESSAGE* pMessage,
    __in LPVOID pvContext
    )
{
    HRESULT hr = S_OK;
    int nResult = IDOK;
    HANDLE hPipe = (HANDLE)pvContext;
    BYTE* pbData = NULL;
    SIZE_T cbData = 0;
    DWORD dwMessage = 0;

    hr = BuffWriteNumber(&pbData, &cbData, pMessage->dwAllowedResults);
    ExitOnFailure(hr, "Failed to write UI flags.");

    switch(pMessage->type)
    {
    case GENERIC_EXECUTE_MESSAGE_PROGRESS:
        // serialize message data
        hr = BuffWriteNumber(&pbData, &cbData, pMessage->progress.dwPercentage);
        ExitOnFailure(hr, "Failed to write progress percentage to message buffer.");

        dwMessage = BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_PROGRESS;
        break;

    case GENERIC_EXECUTE_MESSAGE_ERROR:
        // serialize message data
        hr = BuffWriteNumber(&pbData, &cbData, pMessage->error.dwErrorCode);
        ExitOnFailure(hr, "Failed to write error code to message buffer.");

        hr = BuffWriteString(&pbData, &cbData, pMessage->error.wzMessage);
        ExitOnFailure(hr, "Failed to write message to message buffer.");

        dwMessage = BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_ERROR;
        break;

    case GENERIC_EXECUTE_MESSAGE_FILES_IN_USE:
        hr = BuffWriteNumber(&pbData, &cbData, pMessage->filesInUse.cFiles);
        ExitOnFailure(hr, "Failed to count of files in use to message buffer.");

        for (DWORD i = 0; i < pMessage->filesInUse.cFiles; ++i)
        {
            hr = BuffWriteString(&pbData, &cbData, pMessage->filesInUse.rgwzFiles[i]);
            ExitOnFailure(hr, "Failed to write file in use to message buffer.");
        }

        dwMessage = BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_FILES_IN_USE;
        break;
    }

    // send message
    hr = PipeSendMessage(hPipe, dwMessage, pbData, cbData, NULL, NULL, reinterpret_cast<DWORD*>(&nResult));
    ExitOnFailure(hr, "Failed to send message to per-user process.");

LExit:
    ReleaseBuffer(pbData);

    return nResult;
}

static int MsiExecuteMessageHandler(
    __in WIU_MSI_EXECUTE_MESSAGE* pMessage,
    __in_opt LPVOID pvContext
    )
{
    HRESULT hr = S_OK;
    int nResult = IDOK;
    HANDLE hPipe = (HANDLE)pvContext;
    BYTE* pbData = NULL;
    SIZE_T cbData = 0;
    DWORD dwMessage = 0;

    // Always send any extra data via the struct first.
    hr = BuffWriteNumber(&pbData, &cbData, pMessage->cData);
    ExitOnFailure(hr, "Failed to write MSI data count to message buffer.");

    for (DWORD i = 0; i < pMessage->cData; ++i)
    {
        hr = BuffWriteString(&pbData, &cbData, pMessage->rgwzData[i]);
        ExitOnFailure(hr, "Failed to write MSI data to message buffer.");
    }

    hr = BuffWriteNumber(&pbData, &cbData, pMessage->dwAllowedResults);
    ExitOnFailure(hr, "Failed to write UI flags.");

    switch (pMessage->type)
    {
    case WIU_MSI_EXECUTE_MESSAGE_PROGRESS:
        // serialize message data
        hr = BuffWriteNumber(&pbData, &cbData, pMessage->progress.dwPercentage);
        ExitOnFailure(hr, "Failed to write progress percentage to message buffer.");

        // set message id
        dwMessage = BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_PROGRESS;
        break;

    case WIU_MSI_EXECUTE_MESSAGE_ERROR:
        // serialize message data
        hr = BuffWriteNumber(&pbData, &cbData, pMessage->error.dwErrorCode);
        ExitOnFailure(hr, "Failed to write error code to message buffer.");

        hr = BuffWriteString(&pbData, &cbData, pMessage->error.wzMessage);
        ExitOnFailure(hr, "Failed to write message to message buffer.");

        // set message id
        dwMessage = BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_ERROR;
        break;

    case WIU_MSI_EXECUTE_MESSAGE_MSI_MESSAGE:
        // serialize message data
        hr = BuffWriteNumber(&pbData, &cbData, (DWORD)pMessage->msiMessage.mt);
        ExitOnFailure(hr, "Failed to write MSI message type to message buffer.");

        hr = BuffWriteString(&pbData, &cbData, pMessage->msiMessage.wzMessage);
        ExitOnFailure(hr, "Failed to write message to message buffer.");

        // set message id
        dwMessage = BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSI_MESSAGE;
        break;

    case WIU_MSI_EXECUTE_MESSAGE_MSI_FILES_IN_USE:
        // NOTE: we do not serialize other message data here because all the "files in use" are in the data above.

        // set message id
        dwMessage = BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_FILES_IN_USE;
        break;

    default:
        hr = E_UNEXPECTED;
        ExitOnFailure1(hr, "Invalid message type: %d", pMessage->type);
    }

    // send message
    hr = PipeSendMessage(hPipe, dwMessage, pbData, cbData, NULL, NULL, (DWORD*)&nResult);
    ExitOnFailure(hr, "Failed to send message to per-machine process.");

LExit:
    ReleaseBuffer(pbData);

    return nResult;
}

static HRESULT OnCleanPackage(
    __in BURN_PACKAGES* pPackages,
    __in BYTE* pbData,
    __in DWORD cbData
    )
{
    HRESULT hr = S_OK;
    SIZE_T iData = 0;
    LPWSTR sczPackage = NULL;
    BURN_PACKAGE* pPackage = NULL;

    // deserialize message data
    hr = BuffReadString(pbData, cbData, &iData, &sczPackage);
    ExitOnFailure(hr, "Failed to read package id.");

    hr = PackageFindById(pPackages, sczPackage, &pPackage);
    ExitOnFailure1(hr, "Failed to find package: %ls", sczPackage);

    // Remove the package from the cache.
    hr = CacheRemovePackage(TRUE, pPackage->sczId, pPackage->sczCacheId);
    ExitOnFailure1(hr, "Failed to remove from cache package: %ls", pPackage->sczId);

LExit:
    ReleaseStr(sczPackage);
    return hr;
}
