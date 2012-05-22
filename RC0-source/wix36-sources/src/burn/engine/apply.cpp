//-------------------------------------------------------------------------------------------------
// <copyright file="apply.cpp" company="Microsoft">
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
//    Module: Core
//
//    Apply phase functions.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"


const DWORD BURN_CACHE_MAX_RECOMMENDED_VERIFY_TRYAGAIN_ATTEMPTS = 2;

// structs

struct BURN_CACHE_ACQUIRE_PROGRESS_CONTEXT
{
    BURN_USER_EXPERIENCE* pUX;
    BURN_CONTAINER* pContainer;
    BURN_PACKAGE* pPackage;
    BURN_PAYLOAD* pPayload;
    DWORD64 qwCacheProgress;
    DWORD64 qwTotalCacheSize;

    BOOL fCancel;
    BOOL fError;
};

typedef struct _BURN_EXECUTE_CONTEXT
{
    BURN_USER_EXPERIENCE* pUX;
    BOOL fRollback;
    BURN_PACKAGE* pExecutingPackage;
    DWORD cExecutedPackages;
    DWORD cExecutePackagesTotal;
    DWORD* pcOverallProgressTicks;
} BURN_EXECUTE_CONTEXT;


// internal function declarations

static HRESULT ExtractContainer(
    __in BURN_USER_EXPERIENCE* pUX,
    __in BURN_CONTAINER* pContainer,
    __in_z LPCWSTR wzContainerPath,
    __in_ecount(cExtractPayloads) BURN_EXTRACT_PAYLOAD* rgExtractPayloads,
    __in DWORD cExtractPayloads
    );
static DWORD64 GetCacheActionSuccessProgress(
    __in BURN_CACHE_ACTION* pCacheAction
    );
static HRESULT LayoutBundle(
    __in BURN_USER_EXPERIENCE* pUX,
    __in HANDLE hPipe,
    __in_z LPCWSTR wzExecutableName,
    __in_z LPCWSTR wzLayoutDirectory,
    __in_z LPCWSTR wzUnverifiedPath,
    __in DWORD64 qwSuccessfulCacheProgress,
    __in DWORD64 qwTotalCacheSize
    );
static HRESULT AcquireContainerOrPayload(
    __in BURN_USER_EXPERIENCE* pUX,
    __in BURN_VARIABLES* pVariables,
    __in_opt BURN_CONTAINER* pContainer,
    __in_opt BURN_PACKAGE* pPackage,
    __in_opt BURN_PAYLOAD* pPayload,
    __in LPCWSTR wzDestinationPath,
    __in DWORD64 qwSuccessfulCacheProgress,
    __in DWORD64 qwTotalCacheSize
    );
static HRESULT LayoutOrCachePayload(
    __in BURN_USER_EXPERIENCE* pUX,
    __in HANDLE hPipe,
    __in_opt BURN_PACKAGE* pPackage,
    __in BURN_PAYLOAD* pPayload,
    __in DWORD64 qwSuccessfullyCacheProgress,
    __in DWORD64 qwTotalCacheSize,
    __in_z_opt LPCWSTR wzLayoutDirectory,
    __in_z LPCWSTR wzUnverifiedPath,
    __in BOOL fMove,
    __in DWORD cTryAgainAttempts,
    __out BOOL* pfRetry
    );
static HRESULT PromptForSource(
    __in BURN_USER_EXPERIENCE* pUX,
    __in_z LPCWSTR wzPackageOrContainerId,
    __in_z_opt LPCWSTR wzPayloadId,
    __in_z LPCWSTR wzLocalSource,
    __in_z_opt LPCWSTR wzDownloadSource,
    __out BOOL* pfRetry,
    __out BOOL* pfDownload
    );
static HRESULT CopyPayload(
    __in BURN_CACHE_ACQUIRE_PROGRESS_CONTEXT* pProgress,
    __in_z LPCWSTR wzSourcePath,
    __in_z LPCWSTR wzDestinationPath
    );
static HRESULT DownloadPayload(
    __in BURN_CACHE_ACQUIRE_PROGRESS_CONTEXT* pProgress,
    __in_z LPCWSTR wzDestinationPath
    );
static DWORD CALLBACK CacheProgressRoutine(
    __in LARGE_INTEGER TotalFileSize,
    __in LARGE_INTEGER TotalBytesTransferred,
    __in LARGE_INTEGER StreamSize,
    __in LARGE_INTEGER StreamBytesTransferred,
    __in DWORD dwStreamNumber,
    __in DWORD dwCallbackReason,
    __in HANDLE hSourceFile,
    __in HANDLE hDestinationFile,
    __in_opt LPVOID lpData
    );
static void DoRollbackCache(
    __in BURN_USER_EXPERIENCE* pUX,
    __in BURN_PLAN* pPlan,
    __in HANDLE hPipe,
    __in DWORD dwCheckpoint
    );
static HRESULT DoExecuteAction(
    __in BURN_ENGINE_STATE* pEngineState,
    __in_opt HWND hwndParent,
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in_opt HANDLE hCacheThread,
    __in BURN_EXECUTE_CONTEXT* pContext,
    __inout BURN_ROLLBACK_BOUNDARY** ppRollbackBoundary,
    __out DWORD* pdwCheckpoint,
    __out BOOL* pfKeepRegistration,
    __out BOOL* pfSuspend,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    );
static HRESULT DoRollbackActions(
    __in BURN_ENGINE_STATE* pEngineState,
    __in_opt HWND hwndParent,
    __in BURN_EXECUTE_CONTEXT* pContext,
    __in DWORD dwCheckpoint,
    __out BOOL* pfKeepRegistration,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    );
static HRESULT ExecuteExePackage(
    __in BURN_ENGINE_STATE* pEngineState,
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in BURN_EXECUTE_CONTEXT* pContext,
    __in BOOL fRollback,
    __out BOOL* pfRetry,
    __out BOOL* pfSuspend,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    );
static HRESULT ExecuteMsiPackage(
    __in BURN_ENGINE_STATE* pEngineState,
    __in_opt HWND hwndParent,
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in BURN_EXECUTE_CONTEXT* pContext,
    __in BOOL fRollback,
    __out BOOL* pfRetry,
    __out BOOL* pfSuspend,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    );
static HRESULT ExecuteMspPackage(
    __in BURN_ENGINE_STATE* pEngineState,
    __in_opt HWND hwndParent,
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in BURN_EXECUTE_CONTEXT* pContext,
    __in BOOL fRollback,
    __out BOOL* pfRetry,
    __out BOOL* pfSuspend,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    );
static HRESULT ExecuteMsuPackage(
    __in BURN_ENGINE_STATE* pEngineState,
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in BURN_EXECUTE_CONTEXT* pContext,
    __in BOOL fRollback,
    __out BOOL* pfRetry,
    __out BOOL* pfSuspend,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    );
static HRESULT ExecuteDependencyAction(
    __in BURN_ENGINE_STATE* pEngineState,
    __in BURN_EXECUTE_ACTION* pAction,
    __in BURN_EXECUTE_CONTEXT* pContext
    );
static HRESULT CleanPackage(
    __in HANDLE hElevatedPipe,
    __in BURN_PACKAGE* pPackage
    );
static int GenericExecuteMessageHandler(
    __in GENERIC_EXECUTE_MESSAGE* pMessage,
    __in LPVOID pvContext
    );
static int MsiExecuteMessageHandler(
    __in WIU_MSI_EXECUTE_MESSAGE* pMessage,
    __in_opt LPVOID pvContext
    );
static HRESULT ReportOverallProgressTicks(
    __in BURN_USER_EXPERIENCE* pUX,
    __in BOOL fRollback,
    __in DWORD cOverallProgressTicksTotal,
    __in DWORD cOverallProgressTicks
    );
static HRESULT ExecutePackageComplete(
    __in BURN_USER_EXPERIENCE* pUX,
    __in BURN_PACKAGE* pPackage,
    __in HRESULT hrOverall,
    __in HRESULT hrExecute,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart,
    __out BOOL* pfRetry,
    __out BOOL* pfSuspend
    );


// function definitions

extern "C" void ApplyReset(
    __in BURN_USER_EXPERIENCE* pUX,
    __in BURN_PACKAGES* pPackages
    )
{
    UserExperienceExecuteReset(pUX);

    for (DWORD i = 0; i < pPackages->cPackages; ++i)
    {
        BURN_PACKAGE* pPackage = pPackages->rgPackages + i;
        pPackage->hrCacheResult = S_OK;
    }
}

extern "C" HRESULT ApplyLock(
    __in BOOL /*fPerMachine*/,
    __out HANDLE* /*phLock*/
    )
{
    HRESULT hr = S_OK;
#if 0 // eventually figure out the correct way to support this. In its current form, embedded bundles (including related bundles) are hosed.
    DWORD er = ERROR_SUCCESS;
    HANDLE hLock = NULL;

    hLock = ::CreateMutexW(NULL, TRUE, fPerMachine ? L"Global\\WixBurnExecutionLock" : L"Local\\WixBurnExecutionLock");
    ExitOnNullWithLastError(hLock, hr, "Failed to create lock.");

    er = ::GetLastError();
    if (ERROR_ALREADY_EXISTS == er)
    {
        ExitFunction1(hr = HRESULT_FROM_WIN32(ERROR_INSTALL_ALREADY_RUNNING));
    }

    *phLock = hLock;
    hLock = NULL;

LExit:
    ReleaseHandle(hLock);
#endif
    return hr;
}

extern "C" HRESULT ApplyRegister(
    __in BURN_ENGINE_STATE* pEngineState
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczEngineWorkingPath = NULL;

    int nResult = pEngineState->userExperience.pUserExperience->OnRegisterBegin();
    hr = UserExperienceInterpretExecuteResult(&pEngineState->userExperience, FALSE, MB_OKCANCEL, nResult);
    ExitOnRootFailure(hr, "UX aborted register begin.");

    // Detect related bundles so they can be registered correctly.
    if (pEngineState->registration.fPerMachine)
    {
        hr = ElevationDetectRelatedBundles(pEngineState->companionConnection.hPipe);
        ExitOnFailure(hr, "Failed to detect related bundles in elevated process");
    }

    // If we have a resume mode that suggests the bundle is on the machine.
    if (BOOTSTRAPPER_RESUME_TYPE_REBOOT_PENDING < pEngineState->command.resumeType)
    {
        // resume previous session
        if (pEngineState->registration.fPerMachine)
        {
            hr =  ElevationSessionResume(pEngineState->companionConnection.hPipe, pEngineState->registration.sczResumeCommandLine);
            ExitOnFailure(hr, "Failed to resume registration session in per-machine process.");
        }
        else
        {
            hr =  RegistrationSessionResume(&pEngineState->registration, FALSE);
            ExitOnFailure(hr, "Failed to resume registration session.");
        }
    }
    else // need to complete registration on the machine.
    {
        hr = CacheCalculateBundleWorkingPath(pEngineState->registration.sczId, pEngineState->registration.sczExecutableName, &sczEngineWorkingPath);
        ExitOnFailure(hr, "Failed to calculate working path for engine.");

        // begin new session
        if (pEngineState->registration.fPerMachine)
        {
            hr = ElevationSessionBegin(pEngineState->companionConnection.hPipe, sczEngineWorkingPath, pEngineState->registration.sczResumeCommandLine, &pEngineState->variables, pEngineState->plan.action, pEngineState->plan.qwEstimatedSize);
            ExitOnFailure(hr, "Failed to begin registration session in per-machine process.");
        }
        else
        {
            hr = RegistrationSessionBegin(sczEngineWorkingPath, &pEngineState->registration, &pEngineState->variables, &pEngineState->userExperience, pEngineState->plan.action, pEngineState->plan.qwEstimatedSize, FALSE);
            ExitOnFailure(hr, "Failed to begin registration session.");
        }
    }

    // Try to save engine state.
    hr = CoreSaveEngineState(pEngineState);
    if (FAILED(hr))
    {
        LogErrorId(hr, MSG_STATE_NOT_SAVED, NULL, NULL, NULL);
        hr = S_OK;
    }

LExit:
    pEngineState->userExperience.pUserExperience->OnRegisterComplete(hr);
    ReleaseStr(sczEngineWorkingPath);

    return hr;
}

extern "C" HRESULT ApplyUnregister(
    __in BURN_ENGINE_STATE* pEngineState,
    __in BOOL fKeepRegistration,
    __in BOOL fSuspend,
    __in BOOTSTRAPPER_APPLY_RESTART restart
    )
{
    HRESULT hr = S_OK;

    pEngineState->userExperience.pUserExperience->OnUnregisterBegin();

    if (pEngineState->registration.fPerMachine)
    {
        hr = ElevationSessionEnd(pEngineState->companionConnection.hPipe, fKeepRegistration, fSuspend, restart);
        ExitOnFailure(hr, "Failed to end session in per-machine process.");
    }

    hr = RegistrationSessionEnd(&pEngineState->registration, fKeepRegistration, fSuspend, restart, FALSE, &pEngineState->resumeMode);
    ExitOnFailure(hr, "Failed to end session in per-user process.");

LExit:
    pEngineState->userExperience.pUserExperience->OnUnregisterComplete(hr);

    return hr;
}

extern "C" HRESULT ApplyCache(
    __in BURN_USER_EXPERIENCE* pUX,
    __in BURN_VARIABLES* pVariables,
    __in BURN_PLAN* pPlan,
    __in HANDLE hPipe,
    __inout DWORD* pcOverallProgressTicks,
    __out BOOL* pfRollback
    )
{
    HRESULT hr = S_OK;
    DWORD dwCheckpoint = 0;
    BOOL fRetry = FALSE;
    DWORD iRetryAction = BURN_PLAN_INVALID_ACTION_INDEX;
    BURN_PACKAGE* pStartedPackage = NULL;
    DWORD64 qwSuccessfulCachedProgress = 0;

    // Allow us to retry and skip packages.
    DWORD iPackageStartAction = BURN_PLAN_INVALID_ACTION_INDEX;
    DWORD iPackageCompleteAction = BURN_PLAN_INVALID_ACTION_INDEX;

    int nResult = pUX->pUserExperience->OnCacheBegin();
    hr = UserExperienceInterpretExecuteResult(pUX, FALSE, MB_OKCANCEL, nResult);
    ExitOnRootFailure(hr, "UX aborted cache.");

    do
    {
        hr = S_OK;
        fRetry = FALSE;

        // Allow us to retry just a payload.
        BURN_PAYLOAD* pRetryPayload = NULL;
        DWORD iRetryPayloadAction = BURN_PLAN_INVALID_ACTION_INDEX;

        // cache actions
        for (DWORD i = (BURN_PLAN_INVALID_ACTION_INDEX == iRetryAction) ? 0 : iRetryAction; SUCCEEDED(hr) && i < pPlan->cCacheActions; ++i)
        {
            BURN_CACHE_ACTION* pCacheAction = pPlan->rgCacheActions + i;
            BOOL fRetryPayload = FALSE;

            if (pCacheAction->fSkipUntilRetried)
            {
                // If this action was retried, let's make sure it will not be skipped any longer.
                if (iRetryAction == i)
                {
                    pCacheAction->fSkipUntilRetried = FALSE;
                }
                else // skip the action.
                {
                    // If we skipped it, we can assume it was successful so add the action's progress now.
                    qwSuccessfulCachedProgress += GetCacheActionSuccessProgress(pCacheAction);
                    continue;
                }
            }

            switch (pCacheAction->type)
            {
            case BURN_CACHE_ACTION_TYPE_CHECKPOINT:
                dwCheckpoint = pCacheAction->checkpoint.dwId;
                break;

            case BURN_CACHE_ACTION_TYPE_LAYOUT_BUNDLE:
                hr = LayoutBundle(pUX, hPipe, pCacheAction->bundleLayout.sczExecutableName, pCacheAction->bundleLayout.sczLayoutDirectory, pCacheAction->bundleLayout.sczUnverifiedPath, qwSuccessfulCachedProgress, pPlan->qwCacheSizeTotal);
                if (SUCCEEDED(hr))
                {
                    qwSuccessfulCachedProgress += pCacheAction->bundleLayout.qwBundleSize;
                    ++(*pcOverallProgressTicks);

                    hr = ReportOverallProgressTicks(pUX, FALSE, pPlan->cOverallProgressTicksTotal, *pcOverallProgressTicks);
                    if (FAILED(hr))
                    {
                        LogErrorId(hr, MSG_USER_CANCELED, L"layout bundle", NULL, NULL);
                    }
                }
                break;

            case BURN_CACHE_ACTION_TYPE_PACKAGE_START:
                iPackageStartAction = i; // if we retry this package, we'll start here in the plan.
                iPackageCompleteAction = pCacheAction->packageStart.iPackageCompleteAction; // if we ignore this package, we'll start after the complete action in the plan.
                pStartedPackage = pCacheAction->packageStart.pPackage;

                nResult = pUX->pUserExperience->OnCachePackageBegin(pStartedPackage->sczId, pCacheAction->packageStart.cCachePayloads, pCacheAction->packageStart.qwCachePayloadSizeTotal);
                hr = UserExperienceInterpretExecuteResult(pUX, FALSE, MB_OKCANCEL, nResult);
                if (FAILED(hr))
                {
                    LogErrorId(hr, MSG_USER_CANCELED, L"begin cache package", pStartedPackage->sczId, NULL);
                }
                break;

            case BURN_CACHE_ACTION_TYPE_ACQUIRE_CONTAINER:
                hr = AcquireContainerOrPayload(pUX, pVariables, pCacheAction->resolveContainer.pContainer, NULL, NULL, pCacheAction->resolveContainer.sczUnverifiedPath, qwSuccessfulCachedProgress, pPlan->qwCacheSizeTotal);
                if (SUCCEEDED(hr))
                {
                    qwSuccessfulCachedProgress += pCacheAction->resolveContainer.pContainer->qwFileSize;
                }
                else
                {
                    LogErrorId(hr, MSG_FAILED_ACQUIRE_CONTAINER, pCacheAction->resolveContainer.pContainer->sczId, pCacheAction->resolveContainer.sczUnverifiedPath, NULL);
                }
                break;

            case BURN_CACHE_ACTION_TYPE_EXTRACT_CONTAINER:
                // If this action is to be skipped until the acquire action is not skipped and the other
                // action is still being skipped then skip this action.
                if (BURN_PLAN_INVALID_ACTION_INDEX != pCacheAction->extractContainer.iSkipUntilAcquiredByAction && pPlan->rgCacheActions[pCacheAction->extractContainer.iSkipUntilAcquiredByAction].fSkipUntilRetried)
                {
                    // TODO: Note there is a potential bug here where retry can cause this cost to be added multiple times.
                    qwSuccessfulCachedProgress += pCacheAction->extractContainer.qwTotalExtractSize;
                    break;
                }

                hr = ExtractContainer(pUX, pCacheAction->extractContainer.pContainer, pCacheAction->extractContainer.sczContainerUnverifiedPath, pCacheAction->extractContainer.rgPayloads, pCacheAction->extractContainer.cPayloads);
                if (SUCCEEDED(hr))
                {
                    qwSuccessfulCachedProgress += pCacheAction->extractContainer.qwTotalExtractSize;
                }
                else
                {
                    LogErrorId(hr, MSG_FAILED_EXTRACT_CONTAINER, pCacheAction->extractContainer.pContainer->sczId, pCacheAction->extractContainer.sczContainerUnverifiedPath, NULL);
                }
                break;

            case BURN_CACHE_ACTION_TYPE_ACQUIRE_PAYLOAD:
                hr = AcquireContainerOrPayload(pUX, pVariables, NULL, pCacheAction->resolvePayload.pPackage, pCacheAction->resolvePayload.pPayload, pCacheAction->resolvePayload.sczUnverifiedPath, qwSuccessfulCachedProgress, pPlan->qwCacheSizeTotal);
                if (SUCCEEDED(hr))
                {
                    qwSuccessfulCachedProgress += pCacheAction->resolvePayload.pPayload->qwFileSize;
                }
                else
                {
                    LogErrorId(hr, MSG_FAILED_ACQUIRE_PAYLOAD, pCacheAction->resolvePayload.pPayload->sczKey, pCacheAction->resolvePayload.sczUnverifiedPath, NULL);
                }
                break;

            case BURN_CACHE_ACTION_TYPE_CACHE_PAYLOAD:
                hr = LayoutOrCachePayload(pUX, pCacheAction->cachePayload.pPackage->fPerMachine ? hPipe : INVALID_HANDLE_VALUE, pCacheAction->cachePayload.pPackage, pCacheAction->cachePayload.pPayload, qwSuccessfulCachedProgress, pPlan->qwCacheSizeTotal, NULL, pCacheAction->cachePayload.sczUnverifiedPath, pCacheAction->cachePayload.fMove, pCacheAction->cachePayload.cTryAgainAttempts, &fRetryPayload);
                if (FAILED(hr))
                {
                    LogErrorId(hr, MSG_FAILED_CACHE_PAYLOAD, pCacheAction->cachePayload.pPayload->sczKey, pCacheAction->cachePayload.sczUnverifiedPath, NULL);

                    if (fRetryPayload)
                    {
                        pRetryPayload = pCacheAction->cachePayload.pPayload;
                        iRetryPayloadAction = pCacheAction->cachePayload.iTryAgainAction;

                        ++pCacheAction->cachePayload.cTryAgainAttempts;
                    }
                }
                break;

            case BURN_CACHE_ACTION_TYPE_LAYOUT_PAYLOAD:
                hr = LayoutOrCachePayload(pUX, hPipe, pCacheAction->layoutPayload.pPackage, pCacheAction->layoutPayload.pPayload, qwSuccessfulCachedProgress, pPlan->qwCacheSizeTotal, pCacheAction->layoutPayload.sczLayoutDirectory, pCacheAction->layoutPayload.sczUnverifiedPath, pCacheAction->layoutPayload.fMove, pCacheAction->layoutPayload.cTryAgainAttempts, &fRetryPayload);
                if (FAILED(hr))
                {
                    LogErrorId(hr, MSG_FAILED_LAYOUT_PAYLOAD, pCacheAction->layoutPayload.pPayload->sczKey, pCacheAction->layoutPayload.sczLayoutDirectory, pCacheAction->layoutPayload.sczUnverifiedPath);

                    if (fRetryPayload)
                    {
                        pRetryPayload = pCacheAction->layoutPayload.pPayload;
                        iRetryPayloadAction = pCacheAction->layoutPayload.iTryAgainAction;

                        ++pCacheAction->layoutPayload.cTryAgainAttempts;
                    }
                }
                break;

            case BURN_CACHE_ACTION_TYPE_PACKAGE_STOP:
                AssertSz(pStartedPackage == pCacheAction->packageStop.pPackage, "Expected package started cached to be the same as the package checkpointed.");

                hr = ReportOverallProgressTicks(pUX, FALSE, pPlan->cOverallProgressTicksTotal, *pcOverallProgressTicks + 1);
                if (FAILED(hr))
                {
                    LogErrorId(hr, MSG_USER_CANCELED, L"end cache package", NULL, NULL);
                }
                else
                {
                    ++(*pcOverallProgressTicks);

                    pUX->pUserExperience->OnCachePackageComplete(pStartedPackage->sczId, hr, IDNOACTION);

                    pStartedPackage->hrCacheResult = hr;

                    iPackageStartAction = BURN_PLAN_INVALID_ACTION_INDEX;
                    iPackageCompleteAction = BURN_PLAN_INVALID_ACTION_INDEX;
                    pStartedPackage = NULL;
                }
                break;

            case BURN_CACHE_ACTION_TYPE_SIGNAL_SYNCPOINT:
                if (!::SetEvent(pCacheAction->syncpoint.hEvent))
                {
                    ExitWithLastError(hr, "Failed to set syncpoint event.");
                }
                break;

            default:
                AssertSz(FALSE, "Unknown cache action.");
                break;
            }
        }

        if (BURN_PLAN_INVALID_ACTION_INDEX != iRetryPayloadAction)
        {
            Assert(pRetryPayload);

            LogErrorId(hr, MSG_APPLY_RETRYING_PAYLOAD, pRetryPayload->sczKey, NULL, NULL);

            // Reduce the successful progress since we're retrying the payload.
            BURN_CACHE_ACTION* pRetryCacheAction = pPlan->rgCacheActions + iRetryPayloadAction;
            DWORD64 qwRetryCacheActionSuccessProgress = GetCacheActionSuccessProgress(pRetryCacheAction);
            Assert(qwSuccessfulCachedProgress >= qwRetryCacheActionSuccessProgress);
            qwSuccessfulCachedProgress -= qwRetryCacheActionSuccessProgress;

            iRetryAction = iRetryPayloadAction;
            fRetry = TRUE;
        }
        else if (pStartedPackage)
        {
            Assert(BURN_PLAN_INVALID_ACTION_INDEX != iPackageStartAction);
            Assert(BURN_PLAN_INVALID_ACTION_INDEX != iPackageCompleteAction);

            nResult = pUX->pUserExperience->OnCachePackageComplete(pStartedPackage->sczId, hr, SUCCEEDED(hr) || pStartedPackage->fVital ? IDNOACTION : IDIGNORE);
            nResult = UserExperienceCheckExecuteResult(pUX, FALSE, MB_ABORTRETRYIGNORE, nResult);
            if (FAILED(hr))
            {
                if (IDRETRY == nResult)
                {
                    LogErrorId(hr, MSG_APPLY_RETRYING_PACKAGE, pStartedPackage->sczId, NULL, NULL);

                    iRetryAction = iPackageStartAction;
                    fRetry = TRUE;
                }
                else if (IDIGNORE == nResult && !pStartedPackage->fVital) // ignore non-vital download failures.
                {
                    LogId(REPORT_STANDARD, MSG_APPLY_CONTINUING_NONVITAL_PACKAGE, pStartedPackage->sczId, hr);

                    ++(*pcOverallProgressTicks); // add progress even though we didn't fully cache the package.

                    iRetryAction = iPackageCompleteAction + 1;
                    fRetry = TRUE;
                }
            }

            pStartedPackage->hrCacheResult = hr;

            iPackageStartAction = BURN_PLAN_INVALID_ACTION_INDEX;
            iPackageCompleteAction = BURN_PLAN_INVALID_ACTION_INDEX;
            pStartedPackage = NULL;
        }
        else
        {
            Assert(BURN_PLAN_INVALID_ACTION_INDEX == iPackageStartAction);
            Assert(BURN_PLAN_INVALID_ACTION_INDEX == iPackageCompleteAction);
        }
    } while (fRetry);

LExit:
    Assert(NULL == pStartedPackage);
    Assert(BURN_PLAN_INVALID_ACTION_INDEX == iPackageStartAction);
    Assert(BURN_PLAN_INVALID_ACTION_INDEX == iPackageCompleteAction);

    if (FAILED(hr))
    {
        DoRollbackCache(pUX, pPlan, hPipe, dwCheckpoint);
        *pfRollback = TRUE;
    }

    // Clean up any remanents in the cache.
    if (INVALID_HANDLE_VALUE != hPipe)
    {
        ElevationCacheCleanup(hPipe);
    }

    CacheCleanup(FALSE, pPlan->wzBundleId);

    pUX->pUserExperience->OnCacheComplete(hr);
    return hr;
}

extern "C" HRESULT ApplyExecute(
    __in BURN_ENGINE_STATE* pEngineState,
    __in_opt HWND hwndParent,
    __in_opt HANDLE hCacheThread,
    __inout DWORD* pcOverallProgressTicks,
    __out BOOL* pfKeepRegistration,
    __out BOOL* pfRollback,
    __out BOOL* pfSuspend,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    )
{
    HRESULT hr = S_OK;
    DWORD dwCheckpoint = 0;
    BURN_EXECUTE_CONTEXT context = { };
    int nResult = 0;
    BURN_ROLLBACK_BOUNDARY* pRollbackBoundary = NULL;
    BOOL fSeekNextRollbackBoundary = FALSE;

    context.pUX = &pEngineState->userExperience;
    context.cExecutePackagesTotal = pEngineState->plan.cExecutePackagesTotal;
    context.pcOverallProgressTicks = pcOverallProgressTicks;

    // Send execute begin to BA.
    nResult = pEngineState->userExperience.pUserExperience->OnExecuteBegin(pEngineState->plan.cExecutePackagesTotal);
    hr = UserExperienceInterpretExecuteResult(&pEngineState->userExperience, FALSE, MB_OKCANCEL, nResult);
    ExitOnRootFailure(hr, "BA aborted execute begin.");

    // Do execute actions.
    for (DWORD i = 0; i < pEngineState->plan.cExecuteActions; ++i)
    {
        BURN_EXECUTE_ACTION* pExecuteAction = &pEngineState->plan.rgExecuteActions[i];
        if (pExecuteAction->fDeleted)
        {
            continue;
        }

        // If we are seeking the next rollback boundary, skip if this action wasn't it.
        if (fSeekNextRollbackBoundary)
        {
            if (BURN_EXECUTE_ACTION_TYPE_ROLLBACK_BOUNDARY == pExecuteAction->type)
            {
                continue;
            }
            else
            {
                fSeekNextRollbackBoundary = FALSE;
            }
        }

        // Execute the action.
        hr = DoExecuteAction(pEngineState, hwndParent, pExecuteAction, hCacheThread, &context, &pRollbackBoundary, &dwCheckpoint, pfKeepRegistration, pfSuspend, pRestart);

        if (*pfSuspend || BOOTSTRAPPER_APPLY_RESTART_INITIATED == *pRestart)
        {
            ExitFunction();
        }

        if (FAILED(hr))
        {
            // If we failed, but rollback is disabled just bail with our error code.
            if (pEngineState->fDisableRollback)
            {
                *pfRollback = TRUE;
                break;
            }
            else // the action failed, roll back to previous rollback boundary.
            {
                HRESULT hrRollback = DoRollbackActions(pEngineState, hwndParent, &context, dwCheckpoint, pfKeepRegistration, pRestart);
                UNREFERENCED_PARAMETER(hrRollback);

                // If the rollback boundary is vital, end execution here.
                if (pRollbackBoundary && pRollbackBoundary->fVital)
                {
                    *pfRollback = TRUE;
                    break;
                }

                // Move forward to next rollback boundary.
                fSeekNextRollbackBoundary = TRUE;
            }
        }
    }

LExit:
    // Send execute complete to BA.
    pEngineState->userExperience.pUserExperience->OnExecuteComplete(hr);

    return hr;
}

extern "C" void ApplyClean(
    __in BURN_USER_EXPERIENCE* /*pUX*/,
    __in BURN_PLAN* pPlan,
    __in HANDLE hPipe
    )
{
    HRESULT hr = S_OK;

    for (DWORD i = 0; i < pPlan->cCleanActions; ++i)
    {
        BURN_CLEAN_ACTION* pCleanAction = pPlan->rgCleanActions + i;

        hr = CleanPackage(hPipe, pCleanAction->pPackage);
    }
}

// internal helper functions

static HRESULT ExtractContainer(
    __in BURN_USER_EXPERIENCE* /*pUX*/,
    __in BURN_CONTAINER* pContainer,
    __in_z LPCWSTR wzContainerPath,
    __in_ecount(cExtractPayloads) BURN_EXTRACT_PAYLOAD* rgExtractPayloads,
    __in DWORD cExtractPayloads
    )
{
    HRESULT hr = S_OK;
    BURN_CONTAINER_CONTEXT context = { };
    LPWSTR sczExtractPayloadId = NULL;

    hr = ContainerOpen(&context, pContainer, wzContainerPath);
    ExitOnFailure1(hr, "Failed to open container: %ls.", pContainer->sczId);

    while (S_OK == (hr = ContainerNextStream(&context, &sczExtractPayloadId)))
    {
        BOOL fExtracted = FALSE;

        for (DWORD iExtract = 0; iExtract < cExtractPayloads; ++iExtract)
        {
            BURN_EXTRACT_PAYLOAD* pExtract = rgExtractPayloads + iExtract;
            if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, sczExtractPayloadId, -1, pExtract->pPayload->sczSourcePath, -1))
            {
                // TODO: Send progress when extracting stream to file.
                hr = ContainerStreamToFile(&context, pExtract->sczUnverifiedPath);
                ExitOnFailure2(hr, "Failed to extract payload: %ls from container: %ls", sczExtractPayloadId, pContainer->sczId);

                fExtracted = TRUE;
                break;
            }
        }

        if (!fExtracted)
        {
            hr = ContainerSkipStream(&context);
            ExitOnFailure2(hr, "Failed to skip the extraction of payload: %ls from container: %ls", sczExtractPayloadId, pContainer->sczId);
        }
    }

    if (E_NOMOREITEMS == hr)
    {
        hr = S_OK;
    }
    ExitOnFailure1(hr, "Failed to extract all payloads from container: %ls", pContainer->sczId);

LExit:
    ReleaseStr(sczExtractPayloadId);
    ContainerClose(&context);

    return hr;
}

static DWORD64 GetCacheActionSuccessProgress(
    __in BURN_CACHE_ACTION* pCacheAction
    )
{
    switch (pCacheAction->type)
    {
    case BURN_CACHE_ACTION_TYPE_LAYOUT_BUNDLE:
        return pCacheAction->bundleLayout.qwBundleSize;

    case BURN_CACHE_ACTION_TYPE_EXTRACT_CONTAINER:
        return pCacheAction->extractContainer.qwTotalExtractSize;

    case BURN_CACHE_ACTION_TYPE_ACQUIRE_CONTAINER:
        return pCacheAction->resolveContainer.pContainer->qwFileSize;

    case BURN_CACHE_ACTION_TYPE_ACQUIRE_PAYLOAD:
        return pCacheAction->resolvePayload.pPayload->qwFileSize;
    }

    return 0;
}

static HRESULT LayoutBundle(
    __in BURN_USER_EXPERIENCE* pUX,
    __in HANDLE hPipe,
    __in_z LPCWSTR wzExecutableName,
    __in_z LPCWSTR wzLayoutDirectory,
    __in_z LPCWSTR wzUnverifiedPath,
    __in DWORD64 qwSuccessfulCacheProgress,
    __in DWORD64 qwTotalCacheSize
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczBundlePath = NULL;
    LPWSTR sczDestinationPath = NULL;
    int nEquivalentPaths = 0;
    BURN_CACHE_ACQUIRE_PROGRESS_CONTEXT progress = { };
    BOOL fRetry = FALSE;

    hr = PathForCurrentProcess(&sczBundlePath, NULL);
    ExitOnFailure(hr, "Failed to get path to bundle to layout.");

    hr = PathConcat(wzLayoutDirectory, wzExecutableName, &sczDestinationPath);
    ExitOnFailure(hr, "Failed to concat layout path for bundle.");

    // If the destination path is the currently running bundle, bail.
    hr = PathCompare(sczBundlePath, sczDestinationPath, &nEquivalentPaths);
    ExitOnFailure(hr, "Failed to determine if layout bundle path was equivalent with current process path.");

    if (CSTR_EQUAL == nEquivalentPaths)
    {
        ExitFunction1(hr = S_OK);
    }

    progress.pUX = pUX;
    progress.qwCacheProgress = qwSuccessfulCacheProgress;
    progress.qwTotalCacheSize = qwTotalCacheSize;

    do
    {
        hr = S_OK;
        fRetry = FALSE;

        do
        {
            progress.fCancel = FALSE;

            int nResult = pUX->pUserExperience->OnCacheAcquireBegin(NULL, NULL, BOOTSTRAPPER_CACHE_OPERATION_COPY, sczBundlePath);
            hr = UserExperienceInterpretExecuteResult(pUX, FALSE, MB_OKCANCEL, nResult);
            ExitOnRootFailure(hr, "BA aborted cache acquire begin.");

            hr = CopyPayload(&progress, sczBundlePath, wzUnverifiedPath);
            // Error handling happens after sending complete message to BA.

            nResult = pUX->pUserExperience->OnCacheAcquireComplete(NULL, NULL, hr, IDNOACTION);
            nResult = UserExperienceCheckExecuteResult(pUX, FALSE, MB_RETRYCANCEL, nResult);
            if (FAILED(hr) && IDRETRY == nResult)
            {
                hr = S_FALSE;
            }
            ExitOnFailure2(hr, "Failed to copy bundle from: '%ls' to: '%ls'", sczBundlePath, wzUnverifiedPath);
        } while (S_FALSE == hr);

        if (FAILED(hr))
        {
            break;
        }

        do
        {
            int nResult = pUX->pUserExperience->OnCacheVerifyBegin(NULL, NULL);
            hr = UserExperienceInterpretExecuteResult(pUX, FALSE, MB_OKCANCEL, nResult);
            ExitOnRootFailure(hr, "UX aborted cache verify begin.");

            if (INVALID_HANDLE_VALUE != hPipe)
            {
                hr = ElevationLayoutBundle(hPipe, wzLayoutDirectory, wzUnverifiedPath);
            }
            else
            {
                hr = CacheLayoutBundle(wzExecutableName, wzLayoutDirectory, wzUnverifiedPath);
            }

            nResult = pUX->pUserExperience->OnCacheVerifyComplete(NULL, NULL, hr, IDNOACTION);
            if (FAILED(hr))
            {
                nResult = UserExperienceCheckExecuteResult(pUX, FALSE, MB_RETRYTRYAGAIN, nResult);
                if (IDRETRY == nResult)
                {
                    hr = S_FALSE; // retry verify.
                }
                else if (IDTRYAGAIN == nResult)
                {
                    fRetry = TRUE; // go back and retry acquire.
                }
            }
        } while (S_FALSE == hr);
    } while (fRetry);
    LogExitOnFailure2(hr, MSG_FAILED_LAYOUT_BUNDLE, "Failed to layout bundle: %ls to layout directory: %ls", sczBundlePath, wzLayoutDirectory);

LExit:
    ReleaseStr(sczDestinationPath);
    ReleaseStr(sczBundlePath);

    return hr;
}

static HRESULT AcquireContainerOrPayload(
    __in BURN_USER_EXPERIENCE* pUX,
    __in BURN_VARIABLES* pVariables,
    __in_opt BURN_CONTAINER* pContainer,
    __in_opt BURN_PACKAGE* pPackage,
    __in_opt BURN_PAYLOAD* pPayload,
    __in LPCWSTR wzDestinationPath,
    __in DWORD64 qwSuccessfulCacheProgress,
    __in DWORD64 qwTotalCacheSize
    )
{
    AssertSz(pContainer || pPayload, "Must provide a container or a payload.");

    HRESULT hr = S_OK;
    int nEquivalentPaths = 0;
    LPCWSTR wzPackageOrContainerId = pContainer ? pContainer->sczId : pPackage ? pPackage->sczId : NULL;
    LPCWSTR wzPayloadId = pPayload ? pPayload->sczKey : NULL;
    LPCWSTR wzRelativePath = pContainer ? pContainer->sczFilePath : pPayload->sczFilePath;
    LPWSTR sczSourceFullPath = NULL;
    BURN_CACHE_ACQUIRE_PROGRESS_CONTEXT progress = { };
    BOOL fRetry = FALSE;

    progress.pContainer = pContainer;
    progress.pPackage = pPackage;
    progress.pPayload = pPayload;
    progress.pUX = pUX;
    progress.qwCacheProgress = qwSuccessfulCacheProgress;
    progress.qwTotalCacheSize = qwTotalCacheSize;

    do
    {
        LPCWSTR wzDownloadUrl = pContainer ? pContainer->downloadSource.sczUrl : pPayload->downloadSource.sczUrl;
        LPCWSTR wzSourcePath = pContainer ? pContainer->sczSourcePath : pPayload->sczSourcePath;

        BOOL fFoundLocal = FALSE;
        BOOL fCopy = FALSE;
        BOOL fDownload = FALSE;

        fRetry = FALSE;
        progress.fCancel = FALSE;

        hr = CacheFindLocalSource(wzSourcePath, pVariables, &fFoundLocal, &sczSourceFullPath);
        ExitOnFailure(hr, "Failed to search local source.");

        // If the file exists locally, copy it.
        if (fFoundLocal)
        {
            // If the source path and destination path are different, do the copy (otherwise there's no point).
            hr = PathCompare(sczSourceFullPath, wzDestinationPath, &nEquivalentPaths);
            ExitOnFailure(hr, "Failed to determine if payload source path was equivalent to the destination path.");

            fCopy = (CSTR_EQUAL != nEquivalentPaths);
        }
        else // can't find the file locally so prompt for source.
        {
            DWORD dwLogId = pContainer ? (wzPayloadId ? MSG_PROMPT_CONTAINER_PAYLOAD_SOURCE : MSG_PROMPT_CONTAINER_SOURCE) : pPackage ? MSG_PROMPT_PACKAGE_PAYLOAD_SOURCE : MSG_PROMPT_BUNDLE_PAYLOAD_SOURCE;
            LogId(REPORT_STANDARD, dwLogId, wzPackageOrContainerId ? wzPackageOrContainerId : L"", wzPayloadId ? wzPayloadId : L"", sczSourceFullPath);

            hr = PromptForSource(pUX, wzPackageOrContainerId, wzPayloadId, sczSourceFullPath, wzDownloadUrl, &fRetry, &fDownload);

            // If the BA requested download then ensure a download url is available (it may have been set
            // during PromptForSource so we need to check again).
            if (fDownload)
            {
                wzDownloadUrl = pContainer ? pContainer->downloadSource.sczUrl : pPayload->downloadSource.sczUrl;
                if (!wzDownloadUrl || !*wzDownloadUrl)
                {
                    hr = E_INVALIDARG;
                }
            }

            // Log the error
            LogExitOnFailure1(hr, MSG_PAYLOAD_FILE_NOT_PRESENT, "Failed while prompting for source (original path '%ls').", sczSourceFullPath);
        }

        if (fCopy)
        {
            int nResult = pUX->pUserExperience->OnCacheAcquireBegin(wzPackageOrContainerId, wzPayloadId, BOOTSTRAPPER_CACHE_OPERATION_COPY, sczSourceFullPath);
            hr = UserExperienceInterpretExecuteResult(pUX, FALSE, MB_OKCANCEL, nResult);
            ExitOnRootFailure(hr, "BA aborted cache acquire begin.");

            hr = CopyPayload(&progress, sczSourceFullPath, wzDestinationPath);
            // Error handling happens after sending complete message to BA.

            // We successfully copied from a source location, set that as the last used source.
            if (SUCCEEDED(hr))
            {
                CacheSetLastUsedSource(pVariables, sczSourceFullPath, wzRelativePath);
            }
        }
        else if (fDownload)
        {
            int nResult = pUX->pUserExperience->OnCacheAcquireBegin(wzPackageOrContainerId, wzPayloadId, BOOTSTRAPPER_CACHE_OPERATION_DOWNLOAD, wzDownloadUrl);
            hr = UserExperienceInterpretExecuteResult(pUX, FALSE, MB_OKCANCEL, nResult);
            ExitOnRootFailure(hr, "BA aborted cache download payload begin.");

            hr = DownloadPayload(&progress, wzDestinationPath);
            // Error handling happens after sending complete message to BA.
        }

        if (fCopy || fDownload)
        {
            int nResult = pUX->pUserExperience->OnCacheAcquireComplete(wzPackageOrContainerId, wzPayloadId, hr, IDNOACTION);
            nResult = UserExperienceCheckExecuteResult(pUX, FALSE, MB_RETRYCANCEL, nResult);
            if (FAILED(hr) && IDRETRY == nResult)
            {
                fRetry = TRUE;
                hr = S_OK;
            }
        }
        ExitOnFailure2(hr, "Failed to acquire payload from: '%ls' to working path: '%ls'", fCopy ? sczSourceFullPath : wzDownloadUrl, wzDestinationPath);
    } while (fRetry);
    ExitOnFailure(hr, "Failed to find external payload to cache.");

LExit:
    ReleaseStr(sczSourceFullPath);

    return hr;
}

static HRESULT LayoutOrCachePayload(
    __in BURN_USER_EXPERIENCE* pUX,
    __in HANDLE hPipe,
    __in_opt BURN_PACKAGE* pPackage,
    __in BURN_PAYLOAD* pPayload,
    __in DWORD64 qwSuccessfulCachedProgress,
    __in DWORD64 qwTotalCacheSize,
    __in_z_opt LPCWSTR wzLayoutDirectory,
    __in_z LPCWSTR wzUnverifiedPath,
    __in BOOL fMove,
    __in DWORD cTryAgainAttempts,
    __out BOOL* pfRetry
    )
{
    HRESULT hr = S_OK;
    LARGE_INTEGER liPayloadSize = { };
    LARGE_INTEGER liZero = { };
    BURN_CACHE_ACQUIRE_PROGRESS_CONTEXT progress = { };

    Assert(qwSuccessfulCachedProgress >= pPayload->qwFileSize);

    liPayloadSize.QuadPart = pPayload->qwFileSize;
    progress.pPackage = pPackage;
    progress.pPayload = pPayload;
    progress.pUX = pUX;
    progress.qwCacheProgress = qwSuccessfulCachedProgress - pPayload->qwFileSize; // remove the payload size, since it was marked successful thus included in the successful size already.
    progress.qwTotalCacheSize = qwTotalCacheSize;

    *pfRetry = FALSE;

    do
    {
        int nResult = pUX->pUserExperience->OnCacheVerifyBegin(pPackage ? pPackage->sczId : NULL, pPayload->sczKey);
        hr = UserExperienceInterpretExecuteResult(pUX, FALSE, MB_OKCANCEL, nResult);
        ExitOnRootFailure(hr, "UX aborted cache verify begin.");

        if (INVALID_HANDLE_VALUE != hPipe) // pass the decision off to the elevated process.
        {
            hr = ElevationCacheOrLayoutPayload(hPipe, pPackage, pPayload, wzLayoutDirectory, wzUnverifiedPath, fMove);
        }
        else if (wzLayoutDirectory) // layout the payload.
        {
            hr = CacheLayoutPayload(pPayload, wzLayoutDirectory, wzUnverifiedPath, fMove);
        }
        else // complete the payload.
        {
            Assert(pPackage);

            hr = CacheCompletePayload(pPackage->fPerMachine, pPayload, pPackage->sczCacheId, wzUnverifiedPath, fMove);
        }

        // If succeeded, send 100% complete here. If the payload was already cached this is the first progress the BA
        // will get.
        if (SUCCEEDED(hr))
        {
            CacheProgressRoutine(liPayloadSize, liPayloadSize, liZero, liZero, 0, 0, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, &progress);
            if (progress.fCancel)
            {
                hr = HRESULT_FROM_WIN32(ERROR_INSTALL_USEREXIT);
            }
            else if (progress.fError)
            {
                hr = HRESULT_FROM_WIN32(ERROR_INSTALL_FAILURE);
            }
            ExitOnRootFailure1(hr, "BA aborted verify of payload: %ls", pPayload->sczKey);
        }

        nResult = pUX->pUserExperience->OnCacheVerifyComplete(pPackage ? pPackage->sczId : NULL, pPayload->sczKey, hr, FAILED(hr) && cTryAgainAttempts < BURN_CACHE_MAX_RECOMMENDED_VERIFY_TRYAGAIN_ATTEMPTS ? IDTRYAGAIN : IDNOACTION);
        nResult = UserExperienceCheckExecuteResult(pUX, FALSE, MB_RETRYTRYAGAIN, nResult);
        if (FAILED(hr))
        {
            if (IDRETRY == nResult)
            {
                hr = S_FALSE; // retry verify.
            }
            else if (IDTRYAGAIN == nResult)
            {
                *pfRetry = TRUE; // go back and retry acquire.
            }
        }
    } while (S_FALSE == hr);

LExit:
    return hr;
}

static HRESULT PromptForSource(
    __in BURN_USER_EXPERIENCE* pUX,
    __in_z LPCWSTR wzPackageOrContainerId,
    __in_z_opt LPCWSTR wzPayloadId,
    __in_z LPCWSTR wzLocalSource,
    __in_z_opt LPCWSTR wzDownloadSource,
    __out BOOL* pfRetry,
    __out BOOL* pfDownload
    )
{
    HRESULT hr = S_OK;

    UserExperienceDeactivateEngine(pUX);

    int nResult = pUX->pUserExperience->OnResolveSource(wzPackageOrContainerId, wzPayloadId, wzLocalSource, wzDownloadSource);
    switch (nResult)
    {
    case IDRETRY:
        *pfRetry = TRUE;
        break;

    case IDDOWNLOAD:
        *pfDownload = TRUE;
        break;

    case IDNOACTION: __fallthrough;
    case IDOK: __fallthrough;
    case IDYES:
        hr = E_FILENOTFOUND;
        break;

    case IDABORT: __fallthrough;
    case IDCANCEL: __fallthrough;
    case IDNO:
        hr = HRESULT_FROM_WIN32(ERROR_INSTALL_USEREXIT);
        break;

    case IDERROR:
        hr = HRESULT_FROM_WIN32(ERROR_INSTALL_FAILURE);
        break;

    default:
        hr = E_INVALIDARG;
        break;
    }

    UserExperienceActivateEngine(pUX, NULL);
    return hr;
}

static HRESULT CopyPayload(
    __in BURN_CACHE_ACQUIRE_PROGRESS_CONTEXT* pProgress,
    __in_z LPCWSTR wzSourcePath,
    __in_z LPCWSTR wzDestinationPath
    )
{
    HRESULT hr = S_OK;
    DWORD dwFileAttributes = 0;
    LPCWSTR wzPackageOrContainerId = pProgress->pContainer ? pProgress->pContainer->sczId : pProgress->pPackage ? pProgress->pPackage->sczId : L"";
    LPCWSTR wzPayloadId = pProgress->pPayload ? pProgress->pPayload->sczKey : L"";

    DWORD dwLogId = pProgress->pContainer ? (pProgress->pPayload ? MSG_ACQUIRE_CONTAINER_PAYLOAD : MSG_ACQUIRE_CONTAINER) : pProgress->pPackage ? MSG_ACQUIRE_PACKAGE_PAYLOAD : MSG_ACQUIRE_BUNDLE_PAYLOAD;
    LogId(REPORT_STANDARD, dwLogId, wzPackageOrContainerId, wzPayloadId, "copy", wzSourcePath);

    // If the destination file already exists, clear the readonly bit to avoid E_ACCESSDENIED.
    if (FileExistsEx(wzDestinationPath, &dwFileAttributes))
    {
        if (FILE_ATTRIBUTE_READONLY & dwFileAttributes)
        {
            dwFileAttributes &= ~FILE_ATTRIBUTE_READONLY;
            if (!::SetFileAttributes(wzDestinationPath, dwFileAttributes))
            {
                ExitWithLastError1(hr, "Failed to clear readonly bit on payload destination path: %ls", wzDestinationPath);
            }
        }
    }

    if (!::CopyFileExW(wzSourcePath, wzDestinationPath, CacheProgressRoutine, pProgress, &pProgress->fCancel, 0))
    {
        if (pProgress->fCancel)
        {
            hr = HRESULT_FROM_WIN32(ERROR_INSTALL_USEREXIT);
            ExitOnRootFailure2(hr, "BA aborted copy of payload from: '%ls' to: %ls.", wzSourcePath, wzDestinationPath);
        }
        else
        {
            ExitWithLastError2(hr, "Failed attempt to copy payload from: '%ls' to: %ls.", wzSourcePath, wzDestinationPath);
        }
    }

LExit:
    return hr;
}

static HRESULT DownloadPayload(
    __in BURN_CACHE_ACQUIRE_PROGRESS_CONTEXT* pProgress,
    __in_z LPCWSTR wzDestinationPath
    )
{
    HRESULT hr = S_OK;
    DWORD dwFileAttributes = 0;
    LPCWSTR wzPackageOrContainerId = pProgress->pContainer ? pProgress->pContainer->sczId : pProgress->pPackage ? pProgress->pPackage->sczId : L"";
    LPCWSTR wzPayloadId = pProgress->pPayload ? pProgress->pPayload->sczKey : L"";
    BURN_DOWNLOAD_SOURCE* pDownloadSource = pProgress->pContainer ? &pProgress->pContainer->downloadSource : &pProgress->pPayload->downloadSource;
    DWORD64 qwDownloadSize = pProgress->pContainer ? pProgress->pContainer->qwFileSize : pProgress->pPayload->qwFileSize;
    BURN_CACHE_CALLBACK callback = { };

    DWORD dwLogId = pProgress->pContainer ? (pProgress->pPayload ? MSG_ACQUIRE_CONTAINER_PAYLOAD : MSG_ACQUIRE_CONTAINER) : pProgress->pPackage ? MSG_ACQUIRE_PACKAGE_PAYLOAD : MSG_ACQUIRE_BUNDLE_PAYLOAD;
    LogId(REPORT_STANDARD, dwLogId, wzPackageOrContainerId, wzPayloadId, "download", pDownloadSource->sczUrl);

    // If the destination file already exists, clear the readonly bit to avoid E_ACCESSDENIED.
    if (FileExistsEx(wzDestinationPath, &dwFileAttributes))
    {
        if (FILE_ATTRIBUTE_READONLY & dwFileAttributes)
        {
            dwFileAttributes &= ~FILE_ATTRIBUTE_READONLY;
            if (!::SetFileAttributes(wzDestinationPath, dwFileAttributes))
            {
                ExitWithLastError1(hr, "Failed to clear readonly bit on payload destination path: %ls", wzDestinationPath);
            }
        }
    }

    callback.pfnProgress = CacheProgressRoutine;
    callback.pfnCancel = NULL; // TODO: set this
    callback.pv = pProgress;

    // If the protocol is specially marked, "bits" let's use that.
    if (L'b' == pDownloadSource->sczUrl[0] &&
        L'i' == pDownloadSource->sczUrl[1] &&
        L't' == pDownloadSource->sczUrl[2] &&
        L's' == pDownloadSource->sczUrl[3] &&
        (L':' == pDownloadSource->sczUrl[4] || (L's' == pDownloadSource->sczUrl[4] && L':' == pDownloadSource->sczUrl[5]))
        )
    {
        hr = BitsDownloadUrl(&callback, pDownloadSource, wzDestinationPath);
    }
    else // wininet handles everything else.
    {
        hr = WininetDownloadUrl(&callback, pDownloadSource, qwDownloadSize, wzDestinationPath);
    }
    ExitOnFailure2(hr, "Failed attempt to download URL: '%ls' to: '%ls'", pDownloadSource->sczUrl, wzDestinationPath);

LExit:
    return hr;
}

static DWORD CALLBACK CacheProgressRoutine(
    __in LARGE_INTEGER TotalFileSize,
    __in LARGE_INTEGER TotalBytesTransferred,
    __in LARGE_INTEGER /*StreamSize*/,
    __in LARGE_INTEGER /*StreamBytesTransferred*/,
    __in DWORD /*dwStreamNumber*/,
    __in DWORD /*dwCallbackReason*/,
    __in HANDLE /*hSourceFile*/,
    __in HANDLE /*hDestinationFile*/,
    __in_opt LPVOID lpData
    )
{
    DWORD dwResult = PROGRESS_CONTINUE;
    BURN_CACHE_ACQUIRE_PROGRESS_CONTEXT* pProgress = static_cast<BURN_CACHE_ACQUIRE_PROGRESS_CONTEXT*>(lpData);
    LPCWSTR wzPackageOrContainerId = pProgress->pContainer ? pProgress->pContainer->sczId : pProgress->pPackage ? pProgress->pPackage->sczId : NULL;
    LPCWSTR wzPayloadId = pProgress->pPayload ? pProgress->pPayload->sczKey : NULL;
    DWORD64 qwCacheProgress = pProgress->qwCacheProgress + TotalBytesTransferred.QuadPart;
    DWORD dwOverallPercentage = static_cast<DWORD>(qwCacheProgress * 100 / pProgress->qwTotalCacheSize);

    int nResult = pProgress->pUX->pUserExperience->OnCacheAcquireProgress(wzPackageOrContainerId, wzPayloadId, TotalBytesTransferred.QuadPart, TotalFileSize.QuadPart, dwOverallPercentage);
    nResult = UserExperienceCheckExecuteResult(pProgress->pUX, FALSE, MB_OKCANCEL, nResult);
    switch (nResult)
    {
    case IDOK: __fallthrough;
    case IDNOACTION: __fallthrough;
    case IDYES: __fallthrough;
    case IDRETRY: __fallthrough;
    case IDIGNORE: __fallthrough;
    case IDTRYAGAIN: __fallthrough;
    case IDCONTINUE:
        dwResult = PROGRESS_CONTINUE;
        break;

    case IDCANCEL: __fallthrough;
    case IDABORT: __fallthrough;
    case IDNO:
        dwResult = PROGRESS_CANCEL;
        pProgress->fCancel = TRUE;
        break;

    case IDERROR: __fallthrough;
    default:
        dwResult = PROGRESS_CANCEL;
        pProgress->fError = TRUE;
        break;
    }

    return dwResult;
}

static void DoRollbackCache(
    __in BURN_USER_EXPERIENCE* /*pUX*/,
    __in BURN_PLAN* pPlan,
    __in HANDLE hPipe,
    __in DWORD dwCheckpoint
    )
{
    HRESULT hr = S_OK;
    DWORD iCheckpoint = 0;

    // Scan to last checkpoint.
    for (DWORD i = 0; i < pPlan->cRollbackCacheActions; ++i)
    {
        BURN_CACHE_ACTION* pRollbackCacheAction = &pPlan->rgRollbackCacheActions[i];

        if (BURN_CACHE_ACTION_TYPE_CHECKPOINT == pRollbackCacheAction->type && pRollbackCacheAction->checkpoint.dwId == dwCheckpoint)
        {
            iCheckpoint = i;
            break;
        }
    }

    // Rollback cache actions.
    if (iCheckpoint)
    {
        // i has to be a signed integer so it doesn't get decremented to 0xFFFFFFFF.
        for (int i = iCheckpoint - 1; i >= 0; --i)
        {
            BURN_CACHE_ACTION* pRollbackCacheAction = &pPlan->rgRollbackCacheActions[i];

            switch (pRollbackCacheAction->type)
            {
            case BURN_CACHE_ACTION_TYPE_CHECKPOINT:
                break;

            case BURN_CACHE_ACTION_TYPE_ROLLBACK_PACKAGE:
                hr = CleanPackage(hPipe, pRollbackCacheAction->rollbackPackage.pPackage);
                break;

            default:
                AssertSz(FALSE, "Invalid rollback cache action.");
                break;
            }
        }
    }
}

static HRESULT DoExecuteAction(
    __in BURN_ENGINE_STATE* pEngineState,
    __in_opt HWND hwndParent,
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in_opt HANDLE hCacheThread,
    __in BURN_EXECUTE_CONTEXT* pContext,
    __inout BURN_ROLLBACK_BOUNDARY** ppRollbackBoundary,
    __out DWORD* pdwCheckpoint,
    __out BOOL* pfKeepRegistration,
    __out BOOL* pfSuspend,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    )
{
    Assert(!pExecuteAction->fDeleted);

    HRESULT hr = S_OK;
    HANDLE rghWait[2] = { };
    BOOTSTRAPPER_APPLY_RESTART restart = BOOTSTRAPPER_APPLY_RESTART_NONE;
    BOOL fRetry = FALSE;

    pContext->fRollback = FALSE;

    do
    {
        switch (pExecuteAction->type)
        {
        case BURN_EXECUTE_ACTION_TYPE_CHECKPOINT:
            *pdwCheckpoint = pExecuteAction->checkpoint.dwId;
            break;

        case BURN_EXECUTE_ACTION_TYPE_WAIT_SYNCPOINT:
            // wait for cache sync-point
            rghWait[0] = pExecuteAction->syncpoint.hEvent;
            rghWait[1] = hCacheThread;
            switch (::WaitForMultipleObjects(rghWait[1] ? 2 : 1, rghWait, FALSE, INFINITE))
            {
            case WAIT_OBJECT_0:
                break;

            case WAIT_OBJECT_0 + 1:
                if (!::GetExitCodeThread(hCacheThread, (DWORD*)&hr))
                {
                    ExitWithLastError(hr, "Failed to get cache thread exit code.");
                }

                if (SUCCEEDED(hr))
                {
                    hr = E_UNEXPECTED;
                }
                ExitOnFailure(hr, "Cache thread exited unexpectedly.");

            case WAIT_FAILED: __fallthrough;
            default:
                ExitWithLastError(hr, "Failed to wait for cache check-point.");
            }
            break;

        case BURN_EXECUTE_ACTION_TYPE_EXE_PACKAGE:
            hr = ExecuteExePackage(pEngineState, pExecuteAction, pContext, FALSE, &fRetry, pfSuspend, &restart);
            ExitOnFailure(hr, "Failed to execute EXE package.");
            break;

        case BURN_EXECUTE_ACTION_TYPE_MSI_PACKAGE:
            hr = ExecuteMsiPackage(pEngineState, hwndParent, pExecuteAction, pContext, FALSE, &fRetry, pfSuspend, &restart);
            ExitOnFailure(hr, "Failed to execute MSI package.");
            break;

        case BURN_EXECUTE_ACTION_TYPE_MSP_TARGET:
            hr = ExecuteMspPackage(pEngineState, hwndParent, pExecuteAction, pContext, FALSE, &fRetry, pfSuspend, &restart);
            ExitOnFailure(hr, "Failed to execute MSP package.");
            break;

        case BURN_EXECUTE_ACTION_TYPE_MSU_PACKAGE:
            hr = ExecuteMsuPackage(pEngineState, pExecuteAction, pContext, FALSE, &fRetry, pfSuspend, &restart);
            ExitOnFailure(hr, "Failed to execute MSU package.");
            break;

        case BURN_EXECUTE_ACTION_TYPE_DEPENDENCY:
            hr = ExecuteDependencyAction(pEngineState, pExecuteAction, pContext);
            ExitOnFailure(hr, "Failed to execute dependency action.");
            break;

        case BURN_EXECUTE_ACTION_TYPE_REGISTRATION:
            *pfKeepRegistration = pExecuteAction->registration.fKeep;
            break;

        case BURN_EXECUTE_ACTION_TYPE_ROLLBACK_BOUNDARY:
            *ppRollbackBoundary = pExecuteAction->rollbackBoundary.pRollbackBoundary;
            break;

        case BURN_EXECUTE_ACTION_TYPE_SERVICE_STOP: __fallthrough;
        case BURN_EXECUTE_ACTION_TYPE_SERVICE_START: __fallthrough;
        default:
            hr = E_UNEXPECTED;
            ExitOnFailure(hr, "Invalid execute action.");
        }

        if (*pRestart < restart)
        {
            *pRestart = restart;
        }
    } while (fRetry && *pRestart < BOOTSTRAPPER_APPLY_RESTART_INITIATED);

LExit:
    return hr;
}

static HRESULT DoRollbackActions(
    __in BURN_ENGINE_STATE* pEngineState,
    __in_opt HWND hwndParent,
    __in BURN_EXECUTE_CONTEXT* pContext,
    __in DWORD dwCheckpoint,
    __out BOOL* pfKeepRegistration,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    )
{
    HRESULT hr = S_OK;
    DWORD iCheckpoint = 0;
    BOOL fRetryIgnored = FALSE;
    BOOL fSuspendIgnored = FALSE;

    pContext->fRollback = TRUE;

    // scan to last checkpoint
    for (DWORD i = 0; i < pEngineState->plan.cRollbackActions; ++i)
    {
        BURN_EXECUTE_ACTION* pRollbackAction = &pEngineState->plan.rgRollbackActions[i];
        if (pRollbackAction->fDeleted)
        {
            continue;
        }

        if (BURN_EXECUTE_ACTION_TYPE_CHECKPOINT == pRollbackAction->type)
        {
            if (pRollbackAction->checkpoint.dwId == dwCheckpoint)
            {
                iCheckpoint = i;
                break;
            }
        }
    }

    // execute rollback actions
    if (iCheckpoint)
    {
        // i has to be a signed integer so it doesn't get decremented to 0xFFFFFFFF.
        for (int i = iCheckpoint - 1; i >= 0; --i)
        {
            BURN_EXECUTE_ACTION* pRollbackAction = &pEngineState->plan.rgRollbackActions[i];
            if (pRollbackAction->fDeleted)
            {
                continue;
            }

            BOOTSTRAPPER_APPLY_RESTART restart = BOOTSTRAPPER_APPLY_RESTART_NONE;
            switch (pRollbackAction->type)
            {
            case BURN_EXECUTE_ACTION_TYPE_CHECKPOINT:
                break;

            case BURN_EXECUTE_ACTION_TYPE_EXE_PACKAGE:
                hr = ExecuteExePackage(pEngineState, pRollbackAction, pContext, TRUE, &fRetryIgnored, &fSuspendIgnored, &restart);
                TraceError(hr, "Failed to rollback EXE package.");
                hr = S_OK;
                break;

            case BURN_EXECUTE_ACTION_TYPE_MSI_PACKAGE:
                hr = ExecuteMsiPackage(pEngineState, hwndParent, pRollbackAction, pContext, TRUE, &fRetryIgnored, &fSuspendIgnored, &restart);
                TraceError(hr, "Failed to rollback MSI package.");
                hr = S_OK;
                break;

            case BURN_EXECUTE_ACTION_TYPE_MSP_TARGET:
                hr = ExecuteMspPackage(pEngineState, hwndParent, pRollbackAction, pContext, TRUE, &fRetryIgnored, &fSuspendIgnored, &restart);
                TraceError(hr, "Failed to rollback MSP package.");
                hr = S_OK;
                break;

            case BURN_EXECUTE_ACTION_TYPE_MSU_PACKAGE:
                hr = ExecuteMsuPackage(pEngineState, pRollbackAction, pContext, TRUE, &fRetryIgnored, &fSuspendIgnored, &restart);
                TraceError(hr, "Failed to rollback MSU package.");
                hr = S_OK;
                break;

            case BURN_EXECUTE_ACTION_TYPE_DEPENDENCY:
                hr = ExecuteDependencyAction(pEngineState, pRollbackAction, pContext);
                TraceError(hr, "Failed to rollback dependency action.");
                hr = S_OK;
                break;

            case BURN_EXECUTE_ACTION_TYPE_REGISTRATION:
                *pfKeepRegistration = pRollbackAction->registration.fKeep;
                break;

            case BURN_EXECUTE_ACTION_TYPE_ROLLBACK_BOUNDARY:
                ExitFunction1(hr = S_OK);

            case BURN_EXECUTE_ACTION_TYPE_UNCACHE_PACKAGE:
                hr = CleanPackage(pEngineState->companionConnection.hPipe, pRollbackAction->uncachePackage.pPackage);
                break;

            case BURN_EXECUTE_ACTION_TYPE_SERVICE_STOP: __fallthrough;
            case BURN_EXECUTE_ACTION_TYPE_SERVICE_START: __fallthrough;
            default:
                hr = E_UNEXPECTED;
                ExitOnFailure1(hr, "Invalid rollback action: %d.", pRollbackAction->type);
            }

            if (*pRestart < restart)
            {
                *pRestart = restart;
            }
        }
    }

LExit:
    return hr;
}

static HRESULT ExecuteExePackage(
    __in BURN_ENGINE_STATE* pEngineState,
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in BURN_EXECUTE_CONTEXT* pContext,
    __in BOOL fRollback,
    __out BOOL* pfRetry,
    __out BOOL* pfSuspend,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    )
{
    HRESULT hr = S_OK;
    HRESULT hrExecute = S_OK;
    GENERIC_EXECUTE_MESSAGE message = { };
    int nResult = 0;

    if (FAILED(pExecuteAction->exePackage.pPackage->hrCacheResult))
    {
        LogId(REPORT_STANDARD, MSG_APPLY_SKIPPED_FAILED_CACHED_PACKAGE, pExecuteAction->exePackage.pPackage->sczId, pExecuteAction->exePackage.pPackage->hrCacheResult);
        ExitFunction1(hr = S_OK);
    }

    Assert(pContext->fRollback == fRollback);
    pContext->pExecutingPackage = pExecuteAction->exePackage.pPackage;

    // send package execute begin to UX
    nResult = pEngineState->userExperience.pUserExperience->OnExecutePackageBegin(pExecuteAction->exePackage.pPackage->sczId, !fRollback);
    hr = UserExperienceInterpretExecuteResult(&pEngineState->userExperience, fRollback, MB_OKCANCEL, nResult);
    ExitOnRootFailure(hr, "UX aborted execute EXE package begin.");

    message.type = GENERIC_EXECUTE_MESSAGE_PROGRESS;
    message.dwAllowedResults = MB_OKCANCEL;
    message.progress.dwPercentage = fRollback ? 100 : 0;
    nResult = GenericExecuteMessageHandler(&message, pContext);
    hr = UserExperienceInterpretExecuteResult(&pEngineState->userExperience, fRollback, message.dwAllowedResults, nResult);
    ExitOnRootFailure(hr, "UX aborted EXE progress.");

    // Execute package.
    if (pExecuteAction->exePackage.pPackage->fPerMachine)
    {
        hrExecute = ElevationExecuteExePackage(pEngineState->companionConnection.hPipe, pExecuteAction, &pEngineState->variables, fRollback, GenericExecuteMessageHandler, pContext, pRestart);
        ExitOnFailure(hrExecute, "Failed to configure per-machine EXE package.");
    }
    else
    {
        hrExecute = ExeEngineExecutePackage(pExecuteAction, &pEngineState->variables, fRollback, GenericExecuteMessageHandler, pContext, pRestart);
        ExitOnFailure(hrExecute, "Failed to configure per-user EXE package.");
    }

    message.type = GENERIC_EXECUTE_MESSAGE_PROGRESS;
    message.dwAllowedResults = MB_OKCANCEL;
    message.progress.dwPercentage = fRollback ? 0 : 100;
    nResult = GenericExecuteMessageHandler(&message, pContext);
    hr = UserExperienceInterpretExecuteResult(&pEngineState->userExperience, fRollback, message.dwAllowedResults, nResult);
    ExitOnRootFailure(hr, "UX aborted EXE progress.");

    pContext->cExecutedPackages += fRollback ? -1 : 1;
    (*pContext->pcOverallProgressTicks) += fRollback ? -1 : 1;

    hr = ReportOverallProgressTicks(&pEngineState->userExperience, fRollback, pEngineState->plan.cOverallProgressTicksTotal, *pContext->pcOverallProgressTicks);
    ExitOnRootFailure(hr, "UX aborted EXE package execute progress.");

LExit:
    hr = ExecutePackageComplete(&pEngineState->userExperience, pExecuteAction->exePackage.pPackage, hr, hrExecute, pRestart, pfRetry, pfSuspend);
    return hr;
}

static HRESULT ExecuteMsiPackage(
    __in BURN_ENGINE_STATE* pEngineState,
    __in_opt HWND hwndParent,
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in BURN_EXECUTE_CONTEXT* pContext,
    __in BOOL fRollback,
    __out BOOL* pfRetry,
    __out BOOL* pfSuspend,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    )
{
    HRESULT hr = S_OK;
    HRESULT hrExecute = S_OK;
    int nResult = 0;

    if (FAILED(pExecuteAction->msiPackage.pPackage->hrCacheResult))
    {
        LogId(REPORT_STANDARD, MSG_APPLY_SKIPPED_FAILED_CACHED_PACKAGE, pExecuteAction->msiPackage.pPackage->sczId, pExecuteAction->msiPackage.pPackage->hrCacheResult);
        ExitFunction1(hr = S_OK);
    }

    Assert(pContext->fRollback == fRollback);
    pContext->pExecutingPackage = pExecuteAction->msiPackage.pPackage;

    // send package execute begin to UX
    nResult = pEngineState->userExperience.pUserExperience->OnExecutePackageBegin(pExecuteAction->msiPackage.pPackage->sczId, !fRollback);
    hr = UserExperienceInterpretExecuteResult(&pEngineState->userExperience, fRollback, MB_OKCANCEL, nResult);
    ExitOnRootFailure(hr, "UX aborted execute MSI package begin.");

    // execute package
    if (pExecuteAction->msiPackage.pPackage->fPerMachine)
    {
        hrExecute = ElevationExecuteMsiPackage(pEngineState->companionConnection.hPipe, hwndParent, pExecuteAction, &pEngineState->variables, fRollback, MsiExecuteMessageHandler, pContext, pRestart);
        ExitOnFailure(hrExecute, "Failed to configure per-machine MSI package.");
    }
    else
    {
        hrExecute = MsiEngineExecutePackage(hwndParent, pExecuteAction, &pEngineState->variables, fRollback, MsiExecuteMessageHandler, pContext, pRestart);
        ExitOnFailure(hrExecute, "Failed to configure per-user MSI package.");
    }

    pContext->cExecutedPackages += fRollback ? -1 : 1;
    (*pContext->pcOverallProgressTicks) += fRollback ? -1 : 1;

    hr = ReportOverallProgressTicks(&pEngineState->userExperience, fRollback, pEngineState->plan.cOverallProgressTicksTotal, *pContext->pcOverallProgressTicks);
    ExitOnRootFailure(hr, "UX aborted MSI package execute progress.");

LExit:
    hr = ExecutePackageComplete(&pEngineState->userExperience, pExecuteAction->msiPackage.pPackage, hr, hrExecute, pRestart, pfRetry, pfSuspend);
    return hr;
}

static HRESULT ExecuteMspPackage(
    __in BURN_ENGINE_STATE* pEngineState,
    __in_opt HWND hwndParent,
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in BURN_EXECUTE_CONTEXT* pContext,
    __in BOOL fRollback,
    __out BOOL* pfRetry,
    __out BOOL* pfSuspend,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    )
{
    HRESULT hr = S_OK;
    HRESULT hrExecute = S_OK;
    int nResult = 0;

    if (FAILED(pExecuteAction->mspTarget.pPackage->hrCacheResult))
    {
        LogId(REPORT_STANDARD, MSG_APPLY_SKIPPED_FAILED_CACHED_PACKAGE, pExecuteAction->mspTarget.pPackage->sczId, pExecuteAction->mspTarget.pPackage->hrCacheResult);
        ExitFunction1(hr = S_OK);
    }

    Assert(pContext->fRollback == fRollback);
    pContext->pExecutingPackage = pExecuteAction->mspTarget.pPackage;

    // send package execute begin to UX
    nResult = pEngineState->userExperience.pUserExperience->OnExecutePackageBegin(pExecuteAction->mspTarget.pPackage->sczId, !fRollback);
    hr = UserExperienceInterpretExecuteResult(&pEngineState->userExperience, fRollback, MB_OKCANCEL, nResult);
    ExitOnRootFailure(hr, "UX aborted execute MSP package begin.");

    // Now send all the patches that target this product code.
    for (DWORD i = 0; i < pExecuteAction->mspTarget.cOrderedPatches; ++i)
    {
        BURN_PACKAGE* pMspPackage = pExecuteAction->mspTarget.rgOrderedPatches[i].pPackage;

        nResult = pEngineState->userExperience.pUserExperience->OnExecutePatchTarget(pMspPackage->sczId, pExecuteAction->mspTarget.sczTargetProductCode);
        hr = UserExperienceInterpretExecuteResult(&pEngineState->userExperience, fRollback, MB_OKCANCEL, nResult);
        ExitOnRootFailure(hr, "BA aborted execute MSP target.");
    }

    // execute package
    if (pExecuteAction->mspTarget.fPerMachineTarget)
    {
        hrExecute = ElevationExecuteMspPackage(pEngineState->companionConnection.hPipe, hwndParent, pExecuteAction, &pEngineState->variables, fRollback, MsiExecuteMessageHandler, pContext, pRestart);
        ExitOnFailure(hrExecute, "Failed to configure per-machine MSP package.");
    }
    else
    {
        hrExecute = MspEngineExecutePackage(hwndParent, pExecuteAction, &pEngineState->variables, fRollback, MsiExecuteMessageHandler, pContext, pRestart);
        ExitOnFailure(hrExecute, "Failed to configure per-user MSP package.");
    }

    pContext->cExecutedPackages += fRollback ? -1 : 1;
    (*pContext->pcOverallProgressTicks) += fRollback ? -1 : 1;

    hr = ReportOverallProgressTicks(&pEngineState->userExperience, fRollback, pEngineState->plan.cOverallProgressTicksTotal, *pContext->pcOverallProgressTicks);
    ExitOnRootFailure(hr, "UX aborted MSP package execute progress.");

LExit:
    hr = ExecutePackageComplete(&pEngineState->userExperience, pExecuteAction->mspTarget.pPackage, hr, hrExecute, pRestart, pfRetry, pfSuspend);
    return hr;
}

static HRESULT ExecuteMsuPackage(
    __in BURN_ENGINE_STATE* pEngineState,
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in BURN_EXECUTE_CONTEXT* pContext,
    __in BOOL fRollback,
    __out BOOL* pfRetry,
    __out BOOL* pfSuspend,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    )
{
    HRESULT hr = S_OK;
    HRESULT hrExecute = S_OK;
    GENERIC_EXECUTE_MESSAGE message = { };
    int nResult = 0;

    if (FAILED(pExecuteAction->msuPackage.pPackage->hrCacheResult))
    {
        LogId(REPORT_STANDARD, MSG_APPLY_SKIPPED_FAILED_CACHED_PACKAGE, pExecuteAction->msuPackage.pPackage->sczId, pExecuteAction->msuPackage.pPackage->hrCacheResult);
        ExitFunction1(hr = S_OK);
    }

    Assert(pContext->fRollback == fRollback);
    pContext->pExecutingPackage = pExecuteAction->msuPackage.pPackage;

    // send package execute begin to UX
    nResult = pEngineState->userExperience.pUserExperience->OnExecutePackageBegin(pExecuteAction->msuPackage.pPackage->sczId, !fRollback);
    hr = UserExperienceInterpretExecuteResult(&pEngineState->userExperience, fRollback, MB_OKCANCEL, nResult);
    ExitOnRootFailure(hr, "UX aborted execute MSU package begin.");

    message.type = GENERIC_EXECUTE_MESSAGE_PROGRESS;
    message.dwAllowedResults = MB_OKCANCEL;
    message.progress.dwPercentage = fRollback ? 100 : 0;
    nResult = GenericExecuteMessageHandler(&message, pContext);
    hr = UserExperienceInterpretExecuteResult(&pEngineState->userExperience, fRollback, message.dwAllowedResults, nResult);
    ExitOnRootFailure(hr, "UX aborted MSU progress.");

    // execute package
    if (pExecuteAction->msuPackage.pPackage->fPerMachine)
    {
        hrExecute = ElevationExecuteMsuPackage(pEngineState->companionConnection.hPipe, pExecuteAction, fRollback, GenericExecuteMessageHandler, pContext, pRestart);
        ExitOnFailure(hrExecute, "Failed to configure per-machine MSU package.");
    }
    else
    {
        hrExecute = E_UNEXPECTED;
        ExitOnFailure(hr, "MSU packages cannot be per-user.");
    }

    message.type = GENERIC_EXECUTE_MESSAGE_PROGRESS;
    message.dwAllowedResults = MB_OKCANCEL;
    message.progress.dwPercentage = fRollback ? 0 : 100;
    nResult = GenericExecuteMessageHandler(&message, pContext);
    hr = UserExperienceInterpretExecuteResult(&pEngineState->userExperience, fRollback, message.dwAllowedResults, nResult);
    ExitOnRootFailure(hr, "UX aborted MSU progress.");

    pContext->cExecutedPackages += fRollback ? -1 : 1;
    (*pContext->pcOverallProgressTicks) += fRollback ? -1 : 1;

    hr = ReportOverallProgressTicks(&pEngineState->userExperience, fRollback, pEngineState->plan.cOverallProgressTicksTotal, *pContext->pcOverallProgressTicks);
    ExitOnRootFailure(hr, "UX aborted MSU package execute progress.");

LExit:
    hr = ExecutePackageComplete(&pEngineState->userExperience, pExecuteAction->msuPackage.pPackage, hr, hrExecute, pRestart, pfRetry, pfSuspend);
    return hr;
}

static HRESULT ExecuteDependencyAction(
    __in BURN_ENGINE_STATE* pEngineState,
    __in BURN_EXECUTE_ACTION* pAction,
    __in BURN_EXECUTE_CONTEXT* /*pContext*/
    )
{
    HRESULT hr = S_OK;

    if (pAction->dependency.pPackage->fPerMachine)
    {
        hr = ElevationExecuteDependencyAction(pEngineState->companionConnection.hPipe, pAction);
        ExitOnFailure(hr, "Failed to register the dependency on per-machine package.");
    }
    else
    {
        hr = DependencyExecuteAction(pAction, pEngineState->registration.fPerMachine);
        ExitOnFailure(hr, "Failed to register the dependency on per-user package.");
    }

LExit:
    return hr;
}

static HRESULT CleanPackage(
    __in HANDLE hElevatedPipe,
    __in BURN_PACKAGE* pPackage
    )
{
    HRESULT hr = S_OK;

    if (pPackage->fPerMachine)
    {
        hr = ElevationCleanPackage(hElevatedPipe, pPackage);
    }
    else
    {
        hr = CacheRemovePackage(FALSE, pPackage->sczId, pPackage->sczCacheId);
    }

    return hr;
}

static int GenericExecuteMessageHandler(
    __in GENERIC_EXECUTE_MESSAGE* pMessage,
    __in LPVOID pvContext
    )
{
    BURN_EXECUTE_CONTEXT* pContext = (BURN_EXECUTE_CONTEXT*)pvContext;
    int nResult = IDNOACTION;

    switch (pMessage->type)
    {
    case GENERIC_EXECUTE_MESSAGE_PROGRESS:
        {
            DWORD dwOverallProgress = ((pContext->cExecutedPackages * 100 + pMessage->progress.dwPercentage) * 100) / (pContext->cExecutePackagesTotal * 100);
            nResult = pContext->pUX->pUserExperience->OnExecuteProgress(pContext->pExecutingPackage->sczId, pMessage->progress.dwPercentage, dwOverallProgress);
        }
        break;

    case GENERIC_EXECUTE_MESSAGE_ERROR:
        nResult = pContext->pUX->pUserExperience->OnError(BOOTSTRAPPER_ERROR_TYPE_EXE_PACKAGE, pContext->pExecutingPackage->sczId, pMessage->error.dwErrorCode, pMessage->error.wzMessage, pMessage->dwAllowedResults, 0, NULL, IDNOACTION);
        break;

    case GENERIC_EXECUTE_MESSAGE_FILES_IN_USE:
        nResult = pContext->pUX->pUserExperience->OnExecuteFilesInUse(pContext->pExecutingPackage->sczId, pMessage->filesInUse.cFiles, pMessage->filesInUse.rgwzFiles);
        break;
    }

    nResult = UserExperienceCheckExecuteResult(pContext->pUX, pContext->fRollback, pMessage->dwAllowedResults, nResult);
    return nResult;
}

static int MsiExecuteMessageHandler(
    __in WIU_MSI_EXECUTE_MESSAGE* pMessage,
    __in_opt LPVOID pvContext
    )
{
    BURN_EXECUTE_CONTEXT* pContext = (BURN_EXECUTE_CONTEXT*)pvContext;
    int nResult = IDNOACTION;

    switch (pMessage->type)
    {
    case WIU_MSI_EXECUTE_MESSAGE_PROGRESS:
        {
        DWORD dwOverallProgress = ((pContext->cExecutedPackages * 100 + pMessage->progress.dwPercentage) * 100) / (pContext->cExecutePackagesTotal * 100);
        nResult = pContext->pUX->pUserExperience->OnExecuteProgress(pContext->pExecutingPackage->sczId, pMessage->progress.dwPercentage, dwOverallProgress);
        }
        break;

    case WIU_MSI_EXECUTE_MESSAGE_ERROR:
        nResult = pContext->pUX->pUserExperience->OnError(BOOTSTRAPPER_ERROR_TYPE_WINDOWS_INSTALLER, pContext->pExecutingPackage->sczId, pMessage->error.dwErrorCode, pMessage->error.wzMessage, pMessage->dwAllowedResults, pMessage->cData, pMessage->rgwzData, pMessage->nResultRecommendation);
        break;

    case WIU_MSI_EXECUTE_MESSAGE_MSI_MESSAGE:
        nResult = pContext->pUX->pUserExperience->OnExecuteMsiMessage(pContext->pExecutingPackage->sczId, pMessage->msiMessage.mt, pMessage->dwAllowedResults, pMessage->msiMessage.wzMessage, pMessage->cData, pMessage->rgwzData, pMessage->nResultRecommendation);
        break;

    case WIU_MSI_EXECUTE_MESSAGE_MSI_FILES_IN_USE:
        nResult = pContext->pUX->pUserExperience->OnExecuteFilesInUse(pContext->pExecutingPackage->sczId, pMessage->msiFilesInUse.cFiles, pMessage->msiFilesInUse.rgwzFiles);
        break;
    }

    nResult = UserExperienceCheckExecuteResult(pContext->pUX, pContext->fRollback, pMessage->dwAllowedResults, nResult);
    return nResult;
}

static HRESULT ReportOverallProgressTicks(
    __in BURN_USER_EXPERIENCE* pUX,
    __in BOOL fRollback,
    __in DWORD cOverallProgressTicksTotal,
    __in DWORD cOverallProgressTicks
    )
{
    HRESULT hr = S_OK;

    int nResult = pUX->pUserExperience->OnProgress(0, cOverallProgressTicks * 100 / cOverallProgressTicksTotal);
    hr = UserExperienceInterpretExecuteResult(pUX, fRollback, MB_OKCANCEL, nResult);

    return hr;
}

static HRESULT ExecutePackageComplete(
    __in BURN_USER_EXPERIENCE* pUX,
    __in BURN_PACKAGE* pPackage,
    __in HRESULT hrOverall,
    __in HRESULT hrExecute,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart,
    __out BOOL* pfRetry,
    __out BOOL* pfSuspend
    )
{
    HRESULT hr = FAILED(hrOverall) ? hrOverall : hrExecute; // if the overall function failed use that otherwise use the execution result.

    // send package execute complete to UX
    int nResult = pUX->pUserExperience->OnExecutePackageComplete(pPackage->sczId, hr, *pRestart, FAILED(hrOverall) || SUCCEEDED(hrExecute) || pPackage->fVital ? IDNOACTION : IDIGNORE);
    if (IDRESTART == nResult)
    {
        *pRestart = BOOTSTRAPPER_APPLY_RESTART_INITIATED;
    }
    *pfRetry = (FAILED(hrExecute) && IDRETRY == nResult); // allow retry only on failures.
    *pfSuspend = (IDSUSPEND == nResult);

    // If we're retrying, leave a message in the log file and say everything is okay.
    if (*pfRetry)
    {
        LogId(REPORT_STANDARD, MSG_APPLY_RETRYING_PACKAGE, pPackage->sczId, hrExecute);
        hr = S_OK;
    }
    else if (SUCCEEDED(hrOverall) && FAILED(hrExecute) && IDIGNORE == nResult && !pPackage->fVital) // If we *only* failed to execute and the BA ignored this *not-vital* package, say everything is okay.
    {
        LogId(REPORT_STANDARD, MSG_APPLY_CONTINUING_NONVITAL_PACKAGE, pPackage->sczId, hrExecute);
        hr = S_OK;
    }

    return hr;
}
