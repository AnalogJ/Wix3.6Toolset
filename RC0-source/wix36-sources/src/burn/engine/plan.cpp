//-------------------------------------------------------------------------------------------------
// <copyright file="plan.cpp" company="Microsoft">
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
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

// internal function definitions

static void UninitializeCacheAction(
    __in BURN_CACHE_ACTION* pCacheAction
    );
static void ResetPlannedPackageState(
    __in BURN_PACKAGE* pPackage
    );
static HRESULT GetActionDefaultRequestState(
    __in BOOTSTRAPPER_ACTION action,
    __in BOOL fPermanent,
    __out BOOTSTRAPPER_REQUEST_STATE* pRequestState
    );
static HRESULT AddCachePackage(
    __in BURN_PLAN* pPlan,
    __in BURN_PACKAGE* pPackage,
    __out HANDLE* phSyncpointEvent
    );
static HRESULT AddCacheSlipstreamMsps(
    __in BURN_PLAN* pPlan,
    __in BURN_PACKAGE* pPackage
    );
static BOOL AlreadyPlannedCachePackage(
    __in BURN_PLAN* pPlan,
    __in_z LPCWSTR wzPackageId,
    __out HANDLE* phSyncpointEvent
    );
static DWORD GetNextCheckpointId();
static HRESULT AppendCacheAction(
    __in BURN_PLAN* pPlan,
    __out BURN_CACHE_ACTION** ppCacheAction
    );
static HRESULT AppendRollbackCacheAction(
    __in BURN_PLAN* pPlan,
    __out BURN_CACHE_ACTION** ppCacheAction
    );
static HRESULT AppendCacheOrLayoutPayloadAction(
    __in BURN_PLAN* pPlan,
    __in_opt BURN_PACKAGE* pPackage,
    __in DWORD iPackageStartAction,
    __in BURN_PAYLOAD* pPayload,
    __in BOOL fPayloadCached,
    __in_z_opt LPCWSTR wzLayoutDirectory
    );
static BOOL FindContainerCacheAction(
    __in BURN_CACHE_ACTION_TYPE type,
    __in BURN_PLAN* pPlan,
    __in BURN_CONTAINER* pContainer,
    __in DWORD iSearchStart,
    __in DWORD iSearchEnd,
    __out_opt BURN_CACHE_ACTION** ppCacheAction,
    __out_opt DWORD* piCacheAction
    );
static HRESULT CreateContainerAcquireAndExtractAction(
    __in BURN_PLAN* pPlan,
    __in BURN_CONTAINER* pContainer,
    __in DWORD iPackageStartAction,
    __in BOOL fPayloadCached,
    __out BURN_CACHE_ACTION** ppContainerExtractAction,
    __out DWORD* piContainerTryAgainAction
    );
static HRESULT AddExtractPayload(
    __in BURN_CACHE_ACTION* pCacheAction,
    __in_opt BURN_PACKAGE* pPackage,
    __in BURN_PAYLOAD* pPayload,
    __in_z LPCWSTR wzPayloadWorkingPath
    );
static BURN_CACHE_ACTION* ProcessSharedPayload(
    __in BURN_PLAN* pPlan,
    __in BURN_PAYLOAD* pPayload
    );
static HRESULT RemoveUnnecessaryActions(
    __in BOOL fExecute,
    __in BURN_EXECUTE_ACTION* rgActions,
    __in DWORD cActions
    );
static HRESULT FinalizeSlipstreamPatchActions(
    __in BOOL fExecute,
    __in BURN_EXECUTE_ACTION* rgActions,
    __in DWORD cActions
    );

// function definitions

extern "C" void PlanReset(
    __in BURN_PLAN* pPlan,
    __in BURN_PACKAGES* pPackages
    )
{
    if (pPlan->rgCacheActions)
    {
        for (DWORD i = 0; i < pPlan->cCacheActions; ++i)
        {
            UninitializeCacheAction(&pPlan->rgCacheActions[i]);
        }
        MemFree(pPlan->rgCacheActions);
    }

    if (pPlan->rgExecuteActions)
    {
        for (DWORD i = 0; i < pPlan->cExecuteActions; ++i)
        {
            PlanUninitializeExecuteAction(&pPlan->rgExecuteActions[i]);
        }
        MemFree(pPlan->rgExecuteActions);
    }

    if (pPlan->rgRollbackActions)
    {
        for (DWORD i = 0; i < pPlan->cRollbackActions; ++i)
        {
            PlanUninitializeExecuteAction(&pPlan->rgRollbackActions[i]);
        }
        MemFree(pPlan->rgRollbackActions);
    }

    if (pPlan->rgCleanActions)
    {
        // Nothing needs to be freed inside clean actions today.
        MemFree(pPlan->rgCleanActions);
    }

    if (pPlan->rgPlannedProviders)
    {
        ReleaseDependencyArray(pPlan->rgPlannedProviders, pPlan->cPlannedProviders);
    }

    memset(pPlan, 0, sizeof(BURN_PLAN));

    // Reset the planned actions for each package.
    if (pPackages->rgPackages)
    {
        for (DWORD i = 0; i < pPackages->cPackages; ++i)
        {
            ResetPlannedPackageState(&pPackages->rgPackages[i]);
        }
    }
}

extern "C" void PlanUninitializeExecuteAction(
    __in BURN_EXECUTE_ACTION* pExecuteAction
    )
{
    switch (pExecuteAction->type)
    {
    case BURN_EXECUTE_ACTION_TYPE_EXE_PACKAGE:
        ReleaseStr(pExecuteAction->exePackage.sczIgnoreDependencies);
        break;

    case BURN_EXECUTE_ACTION_TYPE_MSI_PACKAGE:
        ReleaseStr(pExecuteAction->msiPackage.sczLogPath);
        ReleaseMem(pExecuteAction->msiPackage.rgFeatures);
        ReleaseMem(pExecuteAction->msiPackage.rgSlipstreamPatches);
        ReleaseMem(pExecuteAction->msiPackage.rgOrderedPatches);
        break;

    case BURN_EXECUTE_ACTION_TYPE_MSP_TARGET:
        ReleaseStr(pExecuteAction->mspTarget.sczTargetProductCode);
        ReleaseStr(pExecuteAction->mspTarget.sczLogPath);
        ReleaseMem(pExecuteAction->mspTarget.rgOrderedPatches);
        break;

    case BURN_EXECUTE_ACTION_TYPE_MSU_PACKAGE:
        ReleaseStr(pExecuteAction->msuPackage.sczLogPath);
        break;

    case BURN_EXECUTE_ACTION_TYPE_SERVICE_STOP: __fallthrough;
    case BURN_EXECUTE_ACTION_TYPE_SERVICE_START:
        ReleaseStr(pExecuteAction->service.sczServiceName);
        break;

    case BURN_EXECUTE_ACTION_TYPE_DEPENDENCY:
        ReleaseStr(pExecuteAction->dependency.sczBundleProviderKey);
        break;
    }
}

extern "C" HRESULT PlanSetVariables(
    __in BOOTSTRAPPER_ACTION action,
    __in BURN_VARIABLES* pVariables
    )
{
    HRESULT hr = S_OK;

    hr = VariableSetNumeric(pVariables, BURN_BUNDLE_ACTION, action, TRUE);
    ExitOnFailure(hr, "Failed to set the bundle action built-in variable.");

LExit:
    return hr;
}

extern "C" HRESULT PlanDefaultPackageRequestState(
    __in BURN_PACKAGE_TYPE packageType,
    __in BOOTSTRAPPER_PACKAGE_STATE currentState,
    __in BOOL fPermanent,
    __in BOOTSTRAPPER_ACTION action,
    __in BURN_VARIABLES* pVariables,
    __in_z_opt LPCWSTR wzInstallCondition,
    __in BOOTSTRAPPER_RELATION_TYPE relationType,
    __out BOOTSTRAPPER_REQUEST_STATE* pRequestState
    )
{
    HRESULT hr = S_OK;
    BOOTSTRAPPER_REQUEST_STATE defaultRequestState = BOOTSTRAPPER_REQUEST_STATE_NONE;
    BOOL fCondition = FALSE;

    // If doing layout, then always default to requesting the file be cached.
    if (BOOTSTRAPPER_ACTION_LAYOUT == action)
    {
        *pRequestState = BOOTSTRAPPER_REQUEST_STATE_CACHE;
    }
    else if (BOOTSTRAPPER_RELATION_PATCH == relationType && BURN_PACKAGE_TYPE_MSP == packageType)
    {
        // For patch related bundles, only install a patch if currently absent during install, modify, or repair.
        if (BOOTSTRAPPER_PACKAGE_STATE_ABSENT == currentState && BOOTSTRAPPER_ACTION_INSTALL <= action)
        {
            *pRequestState = BOOTSTRAPPER_REQUEST_STATE_PRESENT;
        }
        else
        {
            *pRequestState = BOOTSTRAPPER_REQUEST_STATE_NONE;
        }
    }
    else if (BOOTSTRAPPER_PACKAGE_STATE_SUPERSEDED == currentState && !BOOTSTRAPPER_ACTION_UNINSTALL == action)
    {
        // Superseded means the package is on the machine but not active, so only uninstall operations are allowed.
        // All other operations do nothing.
        *pRequestState = BOOTSTRAPPER_REQUEST_STATE_NONE;
    }
    else if (BOOTSTRAPPER_PACKAGE_STATE_OBSOLETE == currentState && !(BOOTSTRAPPER_ACTION_UNINSTALL == action && BURN_PACKAGE_TYPE_MSP == packageType))
    {
        // Obsolete means the package is not on the machine and should not be installed, *except* patches can be obsolete
        // and present so allow them to be removed during uninstall. Everyone else, gets nothing.
        *pRequestState = BOOTSTRAPPER_REQUEST_STATE_NONE;
    }
    else // pick the best option for the action state and install condition.
    {
        hr = GetActionDefaultRequestState(action, fPermanent, &defaultRequestState);
        ExitOnFailure(hr, "Failed to get default request state for action.");

        // If there is an install condition (and we're doing an install) evaluate the condition
        // to determine whether to use the default request state or make the package absent.
        if (BOOTSTRAPPER_ACTION_UNINSTALL != action && wzInstallCondition && *wzInstallCondition)
        {
            hr = ConditionEvaluate(pVariables, wzInstallCondition, &fCondition);
            ExitOnFailure(hr, "Failed to evaluate install condition.");

            *pRequestState = fCondition ? defaultRequestState : BOOTSTRAPPER_REQUEST_STATE_ABSENT;
        }
        else // just set the package to the default request state.
        {
            *pRequestState = defaultRequestState;
        }
    }

LExit:
    return hr;
}

extern "C" HRESULT PlanLayoutBundle(
    __in BURN_PLAN* pPlan,
    __in_z LPCWSTR wzExecutableName,
    __in DWORD64 qwBundleSize,
    __in BURN_VARIABLES* pVariables,
    __in BURN_PAYLOADS* pPayloads,
    __out_z LPWSTR* psczLayoutDirectory
    )
{
    HRESULT hr = S_OK;
    BURN_CACHE_ACTION* pCacheAction = NULL;
    LPWSTR sczExecutablePath = NULL;
    LPWSTR sczLayoutDirectory = NULL;

    // Get the layout directory.
    hr = VariableGetString(pVariables, BURN_BUNDLE_LAYOUT_DIRECTORY, &sczLayoutDirectory);
    if (E_NOTFOUND == hr) // if not set, use the current directory as the layout directory.
    {
        hr = PathForCurrentProcess(&sczExecutablePath, NULL);
        ExitOnFailure(hr, "Failed to get path for current executing process as layout directory.");

        hr = PathGetDirectory(sczExecutablePath, &sczLayoutDirectory);
        ExitOnFailure(hr, "Failed to get executing process as layout directory.");
    }
    ExitOnFailure(hr, "Failed to get bundle layout directory property.");

    hr = PathBackslashTerminate(&sczLayoutDirectory);
    ExitOnFailure(hr, "Failed to ensure layout directory is backslash terminated.");

    // Plan the layout of the bundle engine itself.
    hr = AppendCacheAction(pPlan, &pCacheAction);
    ExitOnFailure(hr, "Failed to append bundle start action.");

    pCacheAction->type = BURN_CACHE_ACTION_TYPE_LAYOUT_BUNDLE;

    hr = StrAllocString(&pCacheAction->bundleLayout.sczExecutableName, wzExecutableName, 0);
    ExitOnFailure(hr, "Failed to to copy executable name for bundle.");

    hr = StrAllocString(&pCacheAction->bundleLayout.sczLayoutDirectory, sczLayoutDirectory, 0);
    ExitOnFailure(hr, "Failed to to copy layout directory for bundle.");

    hr = CacheCalculateBundleLayoutWorkingPath(pPlan->wzBundleId, &pCacheAction->bundleLayout.sczUnverifiedPath);
    ExitOnFailure(hr, "Failed to calculate bundle layout working path.");

    pCacheAction->bundleLayout.qwBundleSize = qwBundleSize;

    pPlan->qwCacheSizeTotal += qwBundleSize;

    ++pPlan->cOverallProgressTicksTotal;

    // Plan the layout of layout-only payloads.
    for (DWORD i = 0; i < pPayloads->cPayloads; ++i)
    {
        BURN_PAYLOAD* pPayload = pPayloads->rgPayloads + i;
        if (pPayload->fLayoutOnly)
        {
            // TODO: determine if a payload already exists in the layout and pass appropriate value fPayloadCached 
            // (instead of always FALSE).
            hr = AppendCacheOrLayoutPayloadAction(pPlan, NULL, BURN_PLAN_INVALID_ACTION_INDEX, pPayload, FALSE, sczLayoutDirectory);
            ExitOnFailure(hr, "Failed to plan layout payload.");
        }
    }

    *psczLayoutDirectory = sczLayoutDirectory;
    sczLayoutDirectory = NULL;

LExit:
    ReleaseStr(sczLayoutDirectory);
    ReleaseStr(sczExecutablePath);

    return hr;
}

extern "C" HRESULT PlanLayoutPackage(
    __in BURN_PLAN* pPlan,
    __in BURN_PACKAGE* pPackage,
    __in_z LPCWSTR wzLayoutDirectory
    )
{
    HRESULT hr = S_OK;
    BURN_CACHE_ACTION* pCacheAction = NULL;
    DWORD iPackageStartAction = 0;

    hr = AppendCacheAction(pPlan, &pCacheAction);
    ExitOnFailure(hr, "Failed to append package start action.");

    pCacheAction->type = BURN_CACHE_ACTION_TYPE_PACKAGE_START;
    pCacheAction->packageStart.pPackage = pPackage;

    // Remember the index for the package start action (which is now the last in the cache
    // actions array) because the array may be resized later and move around in memory.
    iPackageStartAction = pPlan->cCacheActions - 1;

    // If any of the package payloads are not cached, add them to the plan.
    for (DWORD i = 0; i < pPackage->cPayloads; ++i)
    {
        BURN_PACKAGE_PAYLOAD* pPackagePayload = &pPackage->rgPayloads[i];

        // TODO: determine if a payload already exists in the layout and pass appropriate value fPayloadCached 
        // (instead of always FALSE).
        hr = AppendCacheOrLayoutPayloadAction(pPlan, pPackage, iPackageStartAction, pPackagePayload->pPayload, FALSE, wzLayoutDirectory);
        ExitOnFailure(hr, "Failed to append cache action.");

        Assert(BURN_CACHE_ACTION_TYPE_PACKAGE_START == pPlan->rgCacheActions[iPackageStartAction].type);
        ++pPlan->rgCacheActions[iPackageStartAction].packageStart.cCachePayloads;
        pPlan->rgCacheActions[iPackageStartAction].packageStart.qwCachePayloadSizeTotal += pPackagePayload->pPayload->qwFileSize;
    }

    // Create package stop action.
    hr = AppendCacheAction(pPlan, &pCacheAction);
    ExitOnFailure(hr, "Failed to append cache action.");

    pCacheAction->type = BURN_CACHE_ACTION_TYPE_PACKAGE_STOP;
    pCacheAction->packageStop.pPackage = pPackage;

    // Update the start action with the location of the complete action.
    pPlan->rgCacheActions[iPackageStartAction].packageStart.iPackageCompleteAction = pPlan->cCacheActions - 1;

    ++pPlan->cOverallProgressTicksTotal;

LExit:
    return hr;
}

extern "C" HRESULT PlanExecutePackage(
    __in BOOTSTRAPPER_DISPLAY display,
    __in BURN_USER_EXPERIENCE* pUserExperience,
    __in BURN_PLAN* pPlan,
    __in BURN_PACKAGE* pPackage,
    __in BURN_LOGGING* pLog,
    __in BURN_VARIABLES* pVariables,
    __in_z LPCWSTR wzBundleProviderKey,
    __inout HANDLE* phSyncpointEvent
    )
{
    HRESULT hr = S_OK;
    BOOL fNeedsCache = FALSE;

    // Calculate execute actions.
    switch (pPackage->type)
    {
    case BURN_PACKAGE_TYPE_EXE:
        hr = ExeEnginePlanCalculatePackage(pPackage, FALSE);
        break;

    case BURN_PACKAGE_TYPE_MSI:
        hr = MsiEnginePlanCalculatePackage(pPackage, pVariables, pUserExperience);
        break;

    case BURN_PACKAGE_TYPE_MSP:
        hr = MspEnginePlanCalculatePackage(pPackage, pUserExperience);
        break;

    case BURN_PACKAGE_TYPE_MSU:
        hr = MsuEnginePlanCalculatePackage(pPackage);
        break;

    default:
        hr = E_UNEXPECTED;
        ExitOnFailure(hr, "Invalid package type.");
    }
    ExitOnFailure1(hr, "Failed to calculate plan actions for package: %ls", pPackage->sczId);

    // Calculate package states based on reference count and plan certain dependency actions prior to planning the package execute action.
    hr = DependencyPlanPackageBegin(NULL, pPackage, pPlan, wzBundleProviderKey);
    ExitOnFailure1(hr, "Failed to plan dependency actions to unregister package: %ls", pPackage->sczId);

    // Exe packages require the package for all operations (even uninstall).
    if (BURN_PACKAGE_TYPE_EXE == pPackage->type)
    {
        fNeedsCache = (BOOTSTRAPPER_ACTION_STATE_NONE != pPackage->execute || BOOTSTRAPPER_ACTION_STATE_NONE != pPackage->rollback);
    }
    else // the other engine types can uninstall without the original package.
    {
        fNeedsCache = (BOOTSTRAPPER_ACTION_STATE_UNINSTALL < pPackage->execute || BOOTSTRAPPER_ACTION_STATE_UNINSTALL < pPackage->rollback);
    }

    if (fNeedsCache)
    {
        // If this is an MSI package with slipstream MSPs, ensure the MSPs are cached first.
        if (BURN_PACKAGE_TYPE_MSI == pPackage->type && 0 < pPackage->Msi.cSlipstreamMspPackages)
        {
            hr = AddCacheSlipstreamMsps(pPlan, pPackage);
            ExitOnFailure(hr, "Failed to plan slipstream patches for package.");
        }

        hr = AddCachePackage(pPlan, pPackage, phSyncpointEvent);
        ExitOnFailure(hr, "Failed to plan cache package.");
    }

    // Add the cache and install size to estimated size if it will be on the machine at the end of the install
    if (BOOTSTRAPPER_REQUEST_STATE_PRESENT == pPackage->requested || (BOOTSTRAPPER_PACKAGE_STATE_PRESENT == pPackage->currentState && BOOTSTRAPPER_REQUEST_STATE_ABSENT < pPackage->requested))
    {
        // If the package will remain in the cache, add the package size to the estimated size
        if (pPackage->fCache)
        {
            pPlan->qwEstimatedSize += pPackage->qwSize;
        }

        // If the package will end up installed on the machine, add the install size to the estimated size.
        if (BOOTSTRAPPER_REQUEST_STATE_CACHE < pPackage->requested)
        {
            // MSP packages get cached automatically by windows installer with any embedded cabs, so include that in the size as well
            if (BURN_PACKAGE_TYPE_MSP == pPackage->type)
            {
                pPlan->qwEstimatedSize += pPackage->qwSize;
            }

            pPlan->qwEstimatedSize += pPackage->qwInstallSize;
        }
    }

    // Add execute actions.
    switch (pPackage->type)
    {
    case BURN_PACKAGE_TYPE_EXE:
        hr = ExeEnginePlanAddPackage(NULL, pPackage, pPlan, pLog, pVariables, *phSyncpointEvent, pPackage->fAcquire);
        break;

    case BURN_PACKAGE_TYPE_MSI:
        hr = MsiEnginePlanAddPackage(NULL, display, pPackage, pPlan, pLog, pVariables, *phSyncpointEvent, pPackage->fAcquire);
        break;

    case BURN_PACKAGE_TYPE_MSP:
        hr = MspEnginePlanAddPackage(NULL, display, pPackage, pPlan, pLog, pVariables, *phSyncpointEvent, pPackage->fAcquire);
        break;

    case BURN_PACKAGE_TYPE_MSU:
        hr = MsuEnginePlanAddPackage(NULL, pPackage, pPlan, pLog, pVariables, *phSyncpointEvent, pPackage->fAcquire);
        break;

    default:
        hr = E_UNEXPECTED;
        ExitOnFailure(hr, "Invalid package type.");
    }
    ExitOnFailure1(hr, "Failed to add plan actions for package: %ls", pPackage->sczId);

    // Plan certain dependency actions after planning the package execute action.
    hr = DependencyPlanPackageComplete(pPackage, pPlan, wzBundleProviderKey);
    ExitOnFailure1(hr, "Failed to plan dependency actions to register package: %ls", pPackage->sczId);

    // If we are going to take any action on this package, add progress for it.
    if (BOOTSTRAPPER_ACTION_STATE_NONE != pPackage->execute || BOOTSTRAPPER_ACTION_STATE_NONE != pPackage->rollback)
    {
        LoggingIncrementPackageSequence();

        ++pPlan->cExecutePackagesTotal;
        ++pPlan->cOverallProgressTicksTotal;

        // If package is per-machine and is being executed, flag the plan to be per-machine as well.
        if (pPackage->fPerMachine)
        {
            pPlan->fPerMachine = TRUE;
        }
    }

LExit:
    return hr;
}

extern "C" HRESULT PlanRelatedBundles(
    __in BOOTSTRAPPER_ACTION action,
    __in BURN_USER_EXPERIENCE* pUserExperience,
    __in BURN_RELATED_BUNDLES* pRelatedBundles,
    __in DWORD64 qwBundleVersion,
    __in BURN_PLAN* pPlan,
    __in BURN_LOGGING* pLog,
    __in BURN_VARIABLES* pVariables,
    __in_z LPCWSTR wzBundleProviderKey,
    __inout HANDLE* phSyncpointEvent,
    __in DWORD dwExecuteActionEarlyIndex
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczIgnoreDependencies = NULL;

    // Get the list of dependencies to ignore to pass to related bundles.
    hr = DependencyAllocIgnoreDependencies(pPlan, &sczIgnoreDependencies);
    ExitOnFailure(hr, "Failed to get the list of dependencies to ignore.");

    for (DWORD i = 0; i < pRelatedBundles->cRelatedBundles; ++i)
    {
        DWORD *pdwInsertIndex = NULL;
        BURN_RELATED_BUNDLE* pRelatedBundle = pRelatedBundles->rgRelatedBundles + i;
        pRelatedBundle->package.defaultRequested = BOOTSTRAPPER_REQUEST_STATE_NONE;
        pRelatedBundle->package.requested = BOOTSTRAPPER_REQUEST_STATE_NONE;

        switch (pRelatedBundle->relationType)
        {
        case BOOTSTRAPPER_RELATION_UPGRADE:
            pRelatedBundle->package.requested = qwBundleVersion > pRelatedBundle->qwVersion ? BOOTSTRAPPER_REQUEST_STATE_ABSENT : BOOTSTRAPPER_REQUEST_STATE_NONE;
            break;
        case BOOTSTRAPPER_RELATION_PATCH: __fallthrough;
        case BOOTSTRAPPER_RELATION_ADDON:
            // Addon and patch bundles will be passed a list of dependencies to ignore for planning.
            hr = StrAllocString(&pRelatedBundle->package.Exe.sczIgnoreDependencies, sczIgnoreDependencies, 0);
            ExitOnFailure(hr, "Failed to copy the list of dependencies to ignore.");

            if (BOOTSTRAPPER_ACTION_UNINSTALL == action)
            {
                pRelatedBundle->package.requested = BOOTSTRAPPER_REQUEST_STATE_ABSENT;

                // Uninstall addons and patches early in the chain, before other packages are uninstalled.
                pdwInsertIndex = &dwExecuteActionEarlyIndex;
            }
            else if (BOOTSTRAPPER_ACTION_INSTALL == action || BOOTSTRAPPER_ACTION_MODIFY == action)
            {
                pRelatedBundle->package.requested = BOOTSTRAPPER_REQUEST_STATE_PRESENT;
            }
            else if (BOOTSTRAPPER_ACTION_REPAIR == action)
            {
                pRelatedBundle->package.requested = BOOTSTRAPPER_REQUEST_STATE_REPAIR;
            }
            break;
        case BOOTSTRAPPER_RELATION_DETECT: __fallthrough;
        case BOOTSTRAPPER_RELATION_DEPENDENT:
            break;
        default:
            hr = E_UNEXPECTED;
            ExitOnFailure1(hr, "Unexpected relation type encountered during plan: %d", pRelatedBundle->relationType);
            break;
        }

        pRelatedBundle->package.defaultRequested = pRelatedBundle->package.requested;

        int nResult = pUserExperience->pUserExperience->OnPlanRelatedBundle(pRelatedBundle->package.sczId, &pRelatedBundle->package.requested);
        hr = UserExperienceInterpretResult(pUserExperience, MB_OKCANCEL, nResult);
        ExitOnRootFailure(hr, "UX aborted plan related bundle.");

        // Log when the UX changed the bundle state so the engine doesn't get blamed for planning the wrong thing.
        if (pRelatedBundle->package.requested != pRelatedBundle->package.defaultRequested)
        {
            LogId(REPORT_STANDARD, MSG_PLANNED_BUNDLE_UX_CHANGED_REQUEST, pRelatedBundle->package.sczId, LoggingRequestStateToString(pRelatedBundle->package.requested), LoggingRequestStateToString(pRelatedBundle->package.defaultRequested));
        }

        if (BOOTSTRAPPER_REQUEST_STATE_NONE != pRelatedBundle->package.requested)
        {
            hr = ExeEnginePlanCalculatePackage(&pRelatedBundle->package, TRUE);
            ExitOnFailure1(hr, "Failed to calcuate plan for related bundle: %ls", pRelatedBundle->package.sczId);

            // Calculate package states based on reference count for addon and patch related bundles.
            if (BOOTSTRAPPER_RELATION_ADDON == pRelatedBundle->relationType || BOOTSTRAPPER_RELATION_PATCH == pRelatedBundle->relationType)
            {
                hr = DependencyPlanPackageBegin(pdwInsertIndex, &pRelatedBundle->package, pPlan, wzBundleProviderKey);
                ExitOnFailure1(hr, "Failed to plan dependency actions to unregister package: %ls", pRelatedBundle->package.sczId);

                // If uninstalling a related bundle, make sure the bundle is uninstalled after removing registration.
                if (pdwInsertIndex && BOOTSTRAPPER_ACTION_UNINSTALL == action)
                {
                    ++(*pdwInsertIndex);
                }
            }

            hr = ExeEnginePlanAddPackage(pdwInsertIndex, &pRelatedBundle->package, pPlan, pLog, pVariables, *phSyncpointEvent, FALSE);
            ExitOnFailure1(hr, "Failed to add to plan related bundle: %ls", pRelatedBundle->package.sczId);

            // Calculate package states based on reference count for addon and patch related bundles.
            if (BOOTSTRAPPER_RELATION_ADDON == pRelatedBundle->relationType || BOOTSTRAPPER_RELATION_PATCH == pRelatedBundle->relationType)
            {
                hr = DependencyPlanPackageComplete(&pRelatedBundle->package, pPlan, wzBundleProviderKey);
                ExitOnFailure1(hr, "Failed to plan dependency actions to register package: %ls", pRelatedBundle->package.sczId);
            }

            // If we are going to take any action on this package, add progress for it.
            if (BOOTSTRAPPER_ACTION_STATE_NONE != pRelatedBundle->package.execute || BOOTSTRAPPER_ACTION_STATE_NONE != pRelatedBundle->package.execute)
            {
                LoggingIncrementPackageSequence();

                ++pPlan->cExecutePackagesTotal;
                ++pPlan->cOverallProgressTicksTotal;
            }

            // If package is per-machine and is being executed, flag the plan to be per-machine as well.
            if (pRelatedBundle->package.fPerMachine)
            {
                pPlan->fPerMachine = TRUE;
            }
        }
        else if (BOOTSTRAPPER_RELATION_ADDON == pRelatedBundle->relationType || BOOTSTRAPPER_RELATION_PATCH == pRelatedBundle->relationType)
        {
            // Make sure the package is properly ref-counted even if no plan is requested.
            hr = DependencyPlanPackageBegin(pdwInsertIndex, &pRelatedBundle->package, pPlan, wzBundleProviderKey);
            ExitOnFailure1(hr, "Failed to plan dependency actions to unregister package: %ls", pRelatedBundle->package.sczId);

            hr = DependencyPlanPackageComplete(&pRelatedBundle->package, pPlan, wzBundleProviderKey);
            ExitOnFailure1(hr, "Failed to plan dependency actions to register package: %ls", pRelatedBundle->package.sczId);
        }
    }

LExit:
    ReleaseStr(sczIgnoreDependencies);

    return hr;
}

extern "C" HRESULT PlanFinalizeActions(
    __in BURN_PLAN* pPlan
    )
{
    HRESULT hr = S_OK;

    hr = RemoveUnnecessaryActions(TRUE, pPlan->rgExecuteActions, pPlan->cExecuteActions);
    ExitOnFailure(hr, "Failed to remove unnecessary execute actions.");

    hr = RemoveUnnecessaryActions(FALSE, pPlan->rgRollbackActions, pPlan->cRollbackActions);
    ExitOnFailure(hr, "Failed to remove unnecessary execute actions.");

    hr = FinalizeSlipstreamPatchActions(TRUE, pPlan->rgExecuteActions, pPlan->cExecuteActions);
    ExitOnFailure(hr, "Failed to finalize slipstream execute actions.");

    hr = FinalizeSlipstreamPatchActions(FALSE, pPlan->rgRollbackActions, pPlan->cRollbackActions);
    ExitOnFailure(hr, "Failed to finalize slipstream rollback actions.");

LExit:
    return hr;
}

extern "C" HRESULT PlanCleanPackage(
    __in BOOTSTRAPPER_ACTION action,
    __in BURN_PLAN* pPlan,
    __in BURN_PACKAGE* pPackage
    )
{
    HRESULT hr = S_OK;
    BOOL fPlanCleanPackage = FALSE;
    BURN_CLEAN_ACTION* pCleanAction = NULL;

    // The following is a complex set of logic that determines when a package should be cleaned
    // from the cache. Start by noting that we only clean if the package is being acquired or
    // already cached.
    if (pPackage->fAcquire || BURN_CACHE_STATE_PARTIAL == pPackage->cache || BURN_CACHE_STATE_COMPLETE == pPackage->cache)
    {
        // The following are all different reasons why the package should be cleaned from the cache.
        // The else-ifs are used to make the conditions easier to see (rather than have them combined
        // in one huge condition).
        if (!pPackage->fCache)  // easy, package is not supposed to stay cached.
        {
            fPlanCleanPackage = TRUE;
        }
        else if (BOOTSTRAPPER_REQUEST_STATE_ABSENT == pPackage->requested &&    // requested to be removed and
                 BOOTSTRAPPER_ACTION_STATE_UNINSTALL == pPackage->execute)      // actually being removed.
        {
            fPlanCleanPackage = TRUE;
        }
        else if (BOOTSTRAPPER_REQUEST_STATE_ABSENT == pPackage->requested &&    // requested to be removed but
                 BOOTSTRAPPER_ACTION_STATE_NONE == pPackage->execute &&         // execute is do nothing and
                 !pPackage->fDependencyManagerWasHere &&                        // dependency manager didn't change execute and
                 BOOTSTRAPPER_PACKAGE_STATE_PRESENT > pPackage->currentState)   // currently not installed.
        {
            fPlanCleanPackage = TRUE;
        }
        else if (BOOTSTRAPPER_ACTION_UNINSTALL == action &&                     // uninstalling and
                 BOOTSTRAPPER_REQUEST_STATE_NONE == pPackage->requested &&      // requested do nothing (aka: default) and
                 BOOTSTRAPPER_ACTION_STATE_NONE == pPackage->execute &&         // execute is still do nothing and
                 !pPackage->fDependencyManagerWasHere &&                        // dependency manager didn't change execute and
                 BOOTSTRAPPER_PACKAGE_STATE_PRESENT > pPackage->currentState)   // currently not installed.
        {
            fPlanCleanPackage = TRUE;
        }
    }

    if (fPlanCleanPackage)
    {
        hr = MemEnsureArraySize(reinterpret_cast<LPVOID*>(&pPlan->rgCleanActions), pPlan->cCleanActions + 1, sizeof(BURN_CLEAN_ACTION), 5);
        ExitOnFailure(hr, "Failed to grow plan's array of clean actions.");

        pCleanAction = pPlan->rgCleanActions + pPlan->cCleanActions;
        ++pPlan->cCleanActions;

        pCleanAction->pPackage = pPackage;

        pPackage->fUncache = TRUE;
    }

LExit:
    return hr;
}

extern "C" HRESULT PlanExecuteCacheSyncAndRollback(
    __in BURN_PLAN* pPlan,
    __in BURN_PACKAGE* pPackage,
    __in HANDLE hCacheEvent,
    __in BOOL fPlanPackageCacheRollback
    )
{
    HRESULT hr = S_OK;
    BURN_EXECUTE_ACTION* pAction = NULL;

    hr = PlanAppendExecuteAction(pPlan, &pAction);
    ExitOnFailure(hr, "Failed to append wait action for caching.");

    pAction->type = BURN_EXECUTE_ACTION_TYPE_WAIT_SYNCPOINT;
    pAction->syncpoint.hEvent = hCacheEvent;

    if (fPlanPackageCacheRollback)
    {
        hr = PlanAppendRollbackAction(pPlan, &pAction);
        ExitOnFailure(hr, "Failed to append rollback action.");

        pAction->type = BURN_EXECUTE_ACTION_TYPE_UNCACHE_PACKAGE;
        pAction->uncachePackage.pPackage = pPackage;

        hr = PlanExecuteCheckpoint(pPlan);
        ExitOnFailure(hr, "Failed to append execute checkpoint for cache rollback.");
    }

LExit:
    return hr;
}

extern "C" HRESULT PlanExecuteCheckpoint(
    __in BURN_PLAN* pPlan
    )
{
    HRESULT hr = S_OK;
    BURN_EXECUTE_ACTION* pAction = NULL;
    DWORD dwCheckpointId = GetNextCheckpointId();

    // execute checkpoint
    hr = PlanAppendExecuteAction(pPlan, &pAction);
    ExitOnFailure(hr, "Failed to append execute action.");

    pAction->type = BURN_EXECUTE_ACTION_TYPE_CHECKPOINT;
    pAction->checkpoint.dwId = dwCheckpointId;

    // rollback checkpoint
    hr = PlanAppendRollbackAction(pPlan, &pAction);
    ExitOnFailure(hr, "Failed to append rollback action.");

    pAction->type = BURN_EXECUTE_ACTION_TYPE_CHECKPOINT;
    pAction->checkpoint.dwId = dwCheckpointId;

LExit:
    return hr;
}

extern "C" HRESULT PlanInsertExecuteAction(
    __in DWORD dwIndex,
    __in BURN_PLAN* pPlan,
    __out BURN_EXECUTE_ACTION** ppExecuteAction
    )
{
    HRESULT hr = S_OK;

    hr = MemInsertIntoArray((void**)&pPlan->rgExecuteActions, dwIndex, 1, pPlan->cExecuteActions + 1, sizeof(BURN_EXECUTE_ACTION), 5);
    ExitOnFailure(hr, "Failed to grow plan's array of execute actions.");

    *ppExecuteAction = pPlan->rgExecuteActions + dwIndex;
    ++pPlan->cExecuteActions;

LExit:
    return hr;
}

extern "C" HRESULT PlanInsertRollbackAction(
    __in DWORD dwIndex,
    __in BURN_PLAN* pPlan,
    __out BURN_EXECUTE_ACTION** ppRollbackAction
    )
{
    HRESULT hr = S_OK;

    hr = MemInsertIntoArray((void**)&pPlan->rgRollbackActions, dwIndex, 1, pPlan->cRollbackActions + 1, sizeof(BURN_EXECUTE_ACTION), 5);
    ExitOnFailure(hr, "Failed to grow plan's array of rollback actions.");

    *ppRollbackAction = pPlan->rgRollbackActions + dwIndex;
    ++pPlan->cRollbackActions;

LExit:
    return hr;
}

extern "C" HRESULT PlanAppendExecuteAction(
    __in BURN_PLAN* pPlan,
    __out BURN_EXECUTE_ACTION** ppExecuteAction
    )
{
    HRESULT hr = S_OK;

    hr = MemEnsureArraySize((void**)&pPlan->rgExecuteActions, pPlan->cExecuteActions + 1, sizeof(BURN_EXECUTE_ACTION), 5);
    ExitOnFailure(hr, "Failed to grow plan's array of execute actions.");

    *ppExecuteAction = pPlan->rgExecuteActions + pPlan->cExecuteActions;
    ++pPlan->cExecuteActions;

LExit:
    return hr;
}

extern "C" HRESULT PlanAppendRollbackAction(
    __in BURN_PLAN* pPlan,
    __out BURN_EXECUTE_ACTION** ppRollbackAction
    )
{
    HRESULT hr = S_OK;

    hr = MemEnsureArraySize((void**)&pPlan->rgRollbackActions, pPlan->cRollbackActions + 1, sizeof(BURN_EXECUTE_ACTION), 5);
    ExitOnFailure(hr, "Failed to grow plan's array of rollback actions.");

    *ppRollbackAction = pPlan->rgRollbackActions + pPlan->cRollbackActions;
    ++pPlan->cRollbackActions;

LExit:
    return hr;
}

extern "C" HRESULT PlanKeepRegistration(
    __in BURN_PLAN* pPlan,
    __in DWORD iAfterExecutePackageAction,
    __in DWORD iBeforeRollbackPackageAction
    )
{
    HRESULT hr = S_OK;
    BURN_EXECUTE_ACTION* pAction = NULL;

    if (BURN_PLAN_INVALID_ACTION_INDEX != iAfterExecutePackageAction)
    {
        hr = PlanInsertExecuteAction(iAfterExecutePackageAction, pPlan, &pAction);
        ExitOnFailure(hr, "Failed to insert keep registration execute action.");

        pAction->type = BURN_EXECUTE_ACTION_TYPE_REGISTRATION;
        pAction->registration.fKeep = TRUE;
    }

    if (BURN_PLAN_INVALID_ACTION_INDEX != iBeforeRollbackPackageAction)
    {
        hr = PlanInsertRollbackAction(iBeforeRollbackPackageAction, pPlan, &pAction);
        ExitOnFailure(hr, "Failed to insert keep registration rollback action.");

        pAction->type = BURN_EXECUTE_ACTION_TYPE_REGISTRATION;
        pAction->registration.fKeep = FALSE;
    }

LExit:
    return hr;
}

extern "C" HRESULT PlanRemoveRegistration(
    __in BURN_PLAN* pPlan,
    __in DWORD iAfterExecutePackageAction,
    __in DWORD iAfterRollbackPackageAction
    )
{
    HRESULT hr = S_OK;
    BURN_EXECUTE_ACTION* pAction = NULL;

    if (BURN_PLAN_INVALID_ACTION_INDEX != iAfterExecutePackageAction)
    {
        hr = PlanInsertExecuteAction(iAfterExecutePackageAction, pPlan, &pAction);
        ExitOnFailure(hr, "Failed to insert remove registration execute action.");

        pAction->type = BURN_EXECUTE_ACTION_TYPE_REGISTRATION;
        pAction->registration.fKeep = FALSE;
    }

    if (BURN_PLAN_INVALID_ACTION_INDEX != iAfterRollbackPackageAction)
    {
        hr = PlanInsertRollbackAction(iAfterRollbackPackageAction, pPlan, &pAction);
        ExitOnFailure(hr, "Failed to insert remove registration rollback action.");

        pAction->type = BURN_EXECUTE_ACTION_TYPE_REGISTRATION;
        pAction->registration.fKeep = TRUE;
    }

LExit:
    return hr;
}

extern "C" HRESULT PlanRollbackBoundaryBegin(
    __in BURN_PLAN* pPlan,
    __in BURN_ROLLBACK_BOUNDARY* pRollbackBoundary
    )
{
    HRESULT hr = S_OK;
    BURN_EXECUTE_ACTION* pExecuteAction = NULL;

    // Add begin rollback boundary to execute plan.
    hr = PlanAppendExecuteAction(pPlan, &pExecuteAction);
    ExitOnFailure(hr, "Failed to append rollback boundary begin action.");

    pExecuteAction->type = BURN_EXECUTE_ACTION_TYPE_ROLLBACK_BOUNDARY;
    pExecuteAction->rollbackBoundary.pRollbackBoundary = pRollbackBoundary;

    // Add begin rollback boundary to rollback plan.
    hr = PlanAppendRollbackAction(pPlan, &pExecuteAction);
    ExitOnFailure(hr, "Failed to append rollback boundary begin action.");

    pExecuteAction->type = BURN_EXECUTE_ACTION_TYPE_ROLLBACK_BOUNDARY;
    pExecuteAction->rollbackBoundary.pRollbackBoundary = pRollbackBoundary;

LExit:
    return hr;
}

extern "C" HRESULT PlanRollbackBoundaryComplete(
    __in BURN_PLAN* pPlan
    )
{
    HRESULT hr = S_OK;
    BURN_EXECUTE_ACTION* pExecuteAction = NULL;
    DWORD dwCheckpointId = 0;

    // Add checkpoints.
    dwCheckpointId = GetNextCheckpointId();

    hr = PlanAppendExecuteAction(pPlan, &pExecuteAction);
    ExitOnFailure(hr, "Failed to append execute action.");

    pExecuteAction->type = BURN_EXECUTE_ACTION_TYPE_CHECKPOINT;
    pExecuteAction->checkpoint.dwId = dwCheckpointId;

    hr = PlanAppendRollbackAction(pPlan, &pExecuteAction);
    ExitOnFailure(hr, "Failed to append rollback action.");

    pExecuteAction->type = BURN_EXECUTE_ACTION_TYPE_CHECKPOINT;
    pExecuteAction->checkpoint.dwId = dwCheckpointId;

LExit:
    return hr;
}

/*******************************************************************
 PlanSetResumeCommand - Initializes resume command string

*******************************************************************/
extern "C" HRESULT PlanSetResumeCommand(
    __in BURN_REGISTRATION* pRegistration,
    __in BOOTSTRAPPER_ACTION action,
    __in BOOTSTRAPPER_COMMAND* pCommand,
    __in BURN_LOGGING* pLog
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczLogAppend = NULL;

    // TODO: move sczResumeCommandLine to BURN_PLAN and refactor to avoid piping the plan throughout registration

    // build the resume command-line.
    hr = StrAllocFormatted(&pRegistration->sczResumeCommandLine, L"\"%ls\"", pRegistration->sczCacheExecutablePath);
    ExitOnFailure(hr, "Failed to copy executable path to resume command-line.");

    if (pLog->sczPath)
    {
        hr = StrAllocFormatted(&sczLogAppend, L" /%ls \"%ls\"", BURN_COMMANDLINE_SWITCH_LOG_APPEND, pLog->sczPath);
        ExitOnFailure(hr, "Failed to format append log command-line for resume command-line.");

        hr = StrAllocConcat(&pRegistration->sczResumeCommandLine, sczLogAppend, 0);
        ExitOnFailure(hr, "Failed to append log command-line to resume command-line");
    }

    switch (action)
    {
    case BOOTSTRAPPER_ACTION_REPAIR:
        hr = StrAllocConcat(&pRegistration->sczResumeCommandLine, L" /repair", 0);
        break;
    case BOOTSTRAPPER_ACTION_UNINSTALL:
        hr = StrAllocConcat(&pRegistration->sczResumeCommandLine, L" /uninstall", 0);
        break;
    }
    ExitOnFailure(hr, "Failed to append action state to resume command-line");

    switch (pCommand->display)
    {
    case BOOTSTRAPPER_DISPLAY_NONE:
        hr = StrAllocConcat(&pRegistration->sczResumeCommandLine, L" /quiet", 0);
        break;
    case BOOTSTRAPPER_DISPLAY_PASSIVE:
        hr = StrAllocConcat(&pRegistration->sczResumeCommandLine, L" /passive", 0);
        break;
    }
    ExitOnFailure(hr, "Failed to append display state to resume command-line");

    switch (pCommand->restart)
    {
    case BOOTSTRAPPER_RESTART_ALWAYS:
        hr = StrAllocConcat(&pRegistration->sczResumeCommandLine, L" /forcerestart", 0);
        break;
    case BOOTSTRAPPER_RESTART_NEVER:
        hr = StrAllocConcat(&pRegistration->sczResumeCommandLine, L" /norestart", 0);
        break;
    }
    ExitOnFailure(hr, "Failed to append restart state to resume command-line");

    if (pCommand->wzCommandLine && *pCommand->wzCommandLine)
    {
        hr = StrAllocConcat(&pRegistration->sczResumeCommandLine, L" ", 0);
        ExitOnFailure(hr, "Failed to append space to resume command-line.");

        hr = StrAllocConcat(&pRegistration->sczResumeCommandLine, pCommand->wzCommandLine, 0);
        ExitOnFailure(hr, "Failed to append command-line to resume command-line.");
    }

LExit:
    ReleaseStr(sczLogAppend);

    return hr;
}


// internal function definitions
static void UninitializeCacheAction(
    __in BURN_CACHE_ACTION* pCacheAction
    )
{
    switch (pCacheAction->type)
    {
    case BURN_CACHE_ACTION_TYPE_SIGNAL_SYNCPOINT:
        ReleaseHandle(pCacheAction->syncpoint.hEvent);
        break;

    case BURN_CACHE_ACTION_TYPE_LAYOUT_BUNDLE:
        ReleaseStr(pCacheAction->bundleLayout.sczExecutableName);
        ReleaseStr(pCacheAction->bundleLayout.sczLayoutDirectory);
        ReleaseStr(pCacheAction->bundleLayout.sczUnverifiedPath);
        break;

    case BURN_CACHE_ACTION_TYPE_ACQUIRE_CONTAINER:
        ReleaseStr(pCacheAction->resolveContainer.sczUnverifiedPath);
        break;

    case BURN_CACHE_ACTION_TYPE_EXTRACT_CONTAINER:
        ReleaseStr(pCacheAction->extractContainer.sczContainerUnverifiedPath);
        ReleaseMem(pCacheAction->extractContainer.rgPayloads);
        break;

    case BURN_CACHE_ACTION_TYPE_ACQUIRE_PAYLOAD:
        ReleaseStr(pCacheAction->resolvePayload.sczUnverifiedPath);
        break;

    case BURN_CACHE_ACTION_TYPE_CACHE_PAYLOAD:
        ReleaseStr(pCacheAction->cachePayload.sczUnverifiedPath);
        break;
    }
}

static void ResetPlannedPackageState(
    __in BURN_PACKAGE* pPackage
    )
{
    // Reset package state that is a result of planning.
    pPackage->expected = BOOTSTRAPPER_PACKAGE_STATE_UNKNOWN;
    pPackage->defaultRequested = BOOTSTRAPPER_REQUEST_STATE_NONE;
    pPackage->requested = BOOTSTRAPPER_REQUEST_STATE_NONE;
    pPackage->fAcquire = FALSE;
    pPackage->fUncache = FALSE;
    pPackage->execute = BOOTSTRAPPER_ACTION_STATE_NONE;
    pPackage->rollback = BOOTSTRAPPER_ACTION_STATE_NONE;
    pPackage->dependency = BURN_DEPENDENCY_ACTION_NONE;
    pPackage->fDependencyManagerWasHere = FALSE;

    if (BURN_PACKAGE_TYPE_MSI == pPackage->type && pPackage->Msi.rgFeatures)
    {
        for (DWORD i = 0; i < pPackage->Msi.cFeatures; ++i)
        {
            BURN_MSIFEATURE* pFeature = &pPackage->Msi.rgFeatures[i];

            pFeature->execute = BOOTSTRAPPER_FEATURE_ACTION_NONE;
            pFeature->rollback = BOOTSTRAPPER_FEATURE_ACTION_NONE;
        }
    }
    else if (BURN_PACKAGE_TYPE_MSP == pPackage->type && pPackage->Msp.rgTargetProducts)
    {
        for (DWORD i = 0; i < pPackage->Msp.cTargetProductCodes; ++i)
        {
            BURN_MSPTARGETPRODUCT* pTargetProduct = &pPackage->Msp.rgTargetProducts[i];

            pTargetProduct->execute = BOOTSTRAPPER_ACTION_STATE_NONE;
            pTargetProduct->rollback = BOOTSTRAPPER_ACTION_STATE_NONE;
        }
    }
}

static HRESULT GetActionDefaultRequestState(
    __in BOOTSTRAPPER_ACTION action,
    __in BOOL fPermanent,
    __out BOOTSTRAPPER_REQUEST_STATE* pRequestState
    )
{
    HRESULT hr = S_OK;

    switch (action)
    {
    case BOOTSTRAPPER_ACTION_INSTALL:
        *pRequestState = BOOTSTRAPPER_REQUEST_STATE_PRESENT;
        break;

    case BOOTSTRAPPER_ACTION_REPAIR:
        *pRequestState = BOOTSTRAPPER_REQUEST_STATE_REPAIR;
        break;

    case BOOTSTRAPPER_ACTION_UNINSTALL:
        *pRequestState = fPermanent ? BOOTSTRAPPER_REQUEST_STATE_NONE : BOOTSTRAPPER_REQUEST_STATE_ABSENT;
        break;

    case BOOTSTRAPPER_ACTION_MODIFY:
        *pRequestState = BOOTSTRAPPER_REQUEST_STATE_NONE;
        break;

    default:
        hr = E_INVALIDARG;
        ExitOnRootFailure(hr, "Invalid action state.");
    }

LExit:
        return hr;
}

static HRESULT AddCachePackage(
    __in BURN_PLAN* pPlan,
    __in BURN_PACKAGE* pPackage,
    __out HANDLE* phSyncpointEvent
    )
{
    AssertSz(pPackage->sczCacheId && *pPackage->sczCacheId, "PlanCachePackage() expects the package to have a cache id.");

    HRESULT hr = S_OK;
    BURN_CACHE_ACTION* pCacheAction = NULL;
    DWORD dwCheckpoint = 0;
    DWORD iPackageStartAction = 0;

    BOOL fPlanned = AlreadyPlannedCachePackage(pPlan, pPackage->sczId, phSyncpointEvent);
    if (fPlanned)
    {
        ExitFunction();
    }

    // Cache checkpoints happen before the package is cached because downloading packages'
    // payloads will not roll themselves back the way installation packages rollback on
    // failure automatically.
    dwCheckpoint = GetNextCheckpointId();

    hr = AppendCacheAction(pPlan, &pCacheAction);
    ExitOnFailure(hr, "Failed to append package start action.");

    pCacheAction->type = BURN_CACHE_ACTION_TYPE_CHECKPOINT;
    pCacheAction->checkpoint.dwId = dwCheckpoint;

    hr = AppendRollbackCacheAction(pPlan, &pCacheAction);
    ExitOnFailure(hr, "Failed to append rollback cache action.");

    pCacheAction->type = BURN_CACHE_ACTION_TYPE_CHECKPOINT;
    pCacheAction->checkpoint.dwId = dwCheckpoint;

    // Plan the package start.
    hr = AppendCacheAction(pPlan, &pCacheAction);
    ExitOnFailure(hr, "Failed to append package start action.");

    pCacheAction->type = BURN_CACHE_ACTION_TYPE_PACKAGE_START;
    pCacheAction->packageStart.pPackage = pPackage;

    // Remember the index for the package start action (which is now the last in the cache
    // actions array) because we have to update this action after processing all the payloads
    // and the array may be resized later which would move a pointer around in memory.
    iPackageStartAction = pPlan->cCacheActions - 1;

    // Create a package cache rollback action.
    hr = AppendRollbackCacheAction(pPlan, &pCacheAction);
    ExitOnFailure(hr, "Failed to append rollback cache action.");

    pCacheAction->type = BURN_CACHE_ACTION_TYPE_ROLLBACK_PACKAGE;
    pCacheAction->rollbackPackage.pPackage = pPackage;

    // Add all the payload cache operations to the plan for this package.
    for (DWORD i = 0; i < pPackage->cPayloads; ++i)
    {
        BURN_PACKAGE_PAYLOAD* pPackagePayload = &pPackage->rgPayloads[i];

        hr = AppendCacheOrLayoutPayloadAction(pPlan, pPackage, iPackageStartAction, pPackagePayload->pPayload, pPackagePayload->fCached, NULL);
        ExitOnFailure(hr, "Failed to append payload cache action.");

        Assert(BURN_CACHE_ACTION_TYPE_PACKAGE_START == pPlan->rgCacheActions[iPackageStartAction].type);
        ++pPlan->rgCacheActions[iPackageStartAction].packageStart.cCachePayloads;
        pPlan->rgCacheActions[iPackageStartAction].packageStart.qwCachePayloadSizeTotal += pPackagePayload->pPayload->qwFileSize;
    }

    // Create package stop action.
    hr = AppendCacheAction(pPlan, &pCacheAction);
    ExitOnFailure(hr, "Failed to append cache action.");

    pCacheAction->type = BURN_CACHE_ACTION_TYPE_PACKAGE_STOP;
    pCacheAction->packageStop.pPackage = pPackage;

    // Update the start action with the location of the complete action.
    pPlan->rgCacheActions[iPackageStartAction].packageStart.iPackageCompleteAction = pPlan->cCacheActions - 1;

    // Create syncpoint action.
    hr = AppendCacheAction(pPlan, &pCacheAction);
    ExitOnFailure(hr, "Failed to append cache action.");

    pCacheAction->type = BURN_CACHE_ACTION_TYPE_SIGNAL_SYNCPOINT;
    pCacheAction->syncpoint.hEvent = ::CreateEventW(NULL, TRUE, FALSE, NULL);
    ExitOnNullWithLastError(pCacheAction->syncpoint.hEvent, hr, "Failed to create syncpoint event.");

    *phSyncpointEvent = pCacheAction->syncpoint.hEvent;

    ++pPlan->cOverallProgressTicksTotal;

    // If the package was not already fully cached then note that we planned the cache here. Otherwise, we only
    // did cache operations to verify the cache is valid so we did not plan the acquisition of the package.
    pPackage->fAcquire = (BURN_CACHE_STATE_COMPLETE != pPackage->cache);

LExit:
    return hr;
}

static HRESULT AddCacheSlipstreamMsps(
    __in BURN_PLAN* pPlan,
    __in BURN_PACKAGE* pPackage
    )
{
    HRESULT hr = S_OK;
    HANDLE hIgnored = NULL;

    AssertSz(BURN_PACKAGE_TYPE_MSI == pPackage->type, "Only MSI packages can have slipstream patches.");

    for (DWORD i = 0; i < pPackage->Msi.cSlipstreamMspPackages; ++i)
    {
        BURN_PACKAGE* pMspPackage = pPackage->Msi.rgpSlipstreamMspPackages[i];
        AssertSz(BURN_PACKAGE_TYPE_MSP == pMspPackage->type, "Only MSP packages can be slipstream patches.");

        hr = AddCachePackage(pPlan, pMspPackage, &hIgnored);
        ExitOnFailure1(hr, "Failed to plan slipstream MSP: %ls", pMspPackage->sczId);
    }

LExit:
    return hr;
}

static BOOL AlreadyPlannedCachePackage(
    __in BURN_PLAN* pPlan,
    __in_z LPCWSTR wzPackageId,
    __out HANDLE* phSyncpointEvent
    )
{
    BOOL fPlanned = FALSE;

    for (DWORD iCacheAction = 0; iCacheAction < pPlan->cCacheActions; ++iCacheAction)
    {
        BURN_CACHE_ACTION* pCacheAction = pPlan->rgCacheActions + iCacheAction;

        if (BURN_CACHE_ACTION_TYPE_PACKAGE_STOP == pCacheAction->type)
        {
            if (CSTR_EQUAL == ::CompareStringW(LOCALE_NEUTRAL, 0, pCacheAction->packageStop.pPackage->sczId, -1, wzPackageId, -1))
            {
                if (iCacheAction + 1 < pPlan->cCacheActions && BURN_CACHE_ACTION_TYPE_SIGNAL_SYNCPOINT == pPlan->rgCacheActions[iCacheAction + 1].type)
                {
                    *phSyncpointEvent = pPlan->rgCacheActions[iCacheAction + 1].syncpoint.hEvent;
                }

                fPlanned = TRUE;
                break;
            }
        }
    }

    return fPlanned;
}

static DWORD GetNextCheckpointId()
{
    static DWORD dwCounter = 0;
    return ++dwCounter;
}

static HRESULT AppendCacheAction(
    __in BURN_PLAN* pPlan,
    __out BURN_CACHE_ACTION** ppCacheAction
    )
{
    HRESULT hr = S_OK;

    hr = MemEnsureArraySize(reinterpret_cast<LPVOID*>(&pPlan->rgCacheActions), pPlan->cCacheActions + 1, sizeof(BURN_CACHE_ACTION), 5);
    ExitOnFailure(hr, "Failed to grow plan's array of cache actions.");

    *ppCacheAction = pPlan->rgCacheActions + pPlan->cCacheActions;
    ++pPlan->cCacheActions;

LExit:
    return hr;
}

static HRESULT AppendRollbackCacheAction(
    __in BURN_PLAN* pPlan,
    __out BURN_CACHE_ACTION** ppCacheAction
    )
{
    HRESULT hr = S_OK;

    hr = MemEnsureArraySize(reinterpret_cast<LPVOID*>(&pPlan->rgRollbackCacheActions), pPlan->cRollbackCacheActions + 1, sizeof(BURN_CACHE_ACTION), 5);
    ExitOnFailure(hr, "Failed to grow plan's array of rollback cache actions.");

    *ppCacheAction = pPlan->rgRollbackCacheActions + pPlan->cRollbackCacheActions;
    ++pPlan->cRollbackCacheActions;

LExit:
    return hr;
}

static HRESULT AppendCacheOrLayoutPayloadAction(
    __in BURN_PLAN* pPlan,
    __in_opt BURN_PACKAGE* pPackage,
    __in DWORD iPackageStartAction,
    __in BURN_PAYLOAD* pPayload,
    __in BOOL fPayloadCached,
    __in_z_opt LPCWSTR wzLayoutDirectory
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczPayloadWorkingPath = NULL;
    BURN_CACHE_ACTION* pCacheAction = NULL;
    DWORD iTryAgainAction = BURN_PLAN_INVALID_ACTION_INDEX;

    hr = CacheCalculatePayloadWorkingPath(pPlan->wzBundleId, pPayload, &sczPayloadWorkingPath);
    ExitOnFailure(hr, "Failed to calculate unverified path for payload.");

    // If the payload is in a container, ensure the container is being acquired
    // then add this payload to the list of payloads to extract already in the plan.
    if (pPayload->pContainer)
    {
        BURN_CACHE_ACTION* pPreviousPackageExtractAction = NULL;
        BURN_CACHE_ACTION* pThisPackageExtractAction = NULL;

        // If the payload is not already cached, the add it to the first extract container action in the plan. Extracting
        // all the needed payloads from the container in a single pass is the most efficient way to extract files from
        // containers. If there is not an extract container action before our package, that is okay because we'll create
        // an extract container action for our package in a second anyway.
        if (!fPayloadCached)
        {
            if (FindContainerCacheAction(BURN_CACHE_ACTION_TYPE_EXTRACT_CONTAINER, pPlan, pPayload->pContainer, 0, iPackageStartAction, &pPreviousPackageExtractAction, NULL))
            {
                hr = AddExtractPayload(pPreviousPackageExtractAction, pPackage, pPayload, sczPayloadWorkingPath);
                ExitOnFailure(hr, "Failed to add extract payload action to previous package.");
            }
        }

        // If there is already an extract container action after our package start action then try to find an acquire action
        // that is matched with it. If there is an acquire action that is our "try again" action otherwise we'll use the existing
        // extract action as the "try again" action.
        if (FindContainerCacheAction(BURN_CACHE_ACTION_TYPE_EXTRACT_CONTAINER, pPlan, pPayload->pContainer, iPackageStartAction, BURN_PLAN_INVALID_ACTION_INDEX, &pThisPackageExtractAction, &iTryAgainAction))
        {
            DWORD iAcquireAction = BURN_PLAN_INVALID_ACTION_INDEX;
            if (FindContainerCacheAction(BURN_CACHE_ACTION_TYPE_ACQUIRE_CONTAINER, pPlan, pPayload->pContainer, iPackageStartAction, iTryAgainAction, NULL, &iAcquireAction))
            {
                iTryAgainAction = iAcquireAction;
            }
        }
        else // did not find an extract container action for our package.
        {
            // Ensure there is an extract action (and maybe an acquire action) for every package that has payloads. The
            // acquire and extract action will be skipped if the payload is already cached or was added to a previous
            // package's extract action above.
            //
            // These actions always exist (even when they are likely to be skipped) so that "try again" will not
            // jump so far back in the plan that you end up extracting payloads for other packages. With these actions
            // "try again" will only retry the extraction for payloads in this package.
            hr = CreateContainerAcquireAndExtractAction(pPlan, pPayload->pContainer, iPackageStartAction, pPreviousPackageExtractAction ? TRUE : fPayloadCached, &pThisPackageExtractAction, &iTryAgainAction);
            ExitOnFailure(hr, "Failed to create container extract action.");
        }
        ExitOnFailure(hr, "Failed while searching for package's container extract action.");

        // We *always* add the payload to this package's extract action even though the extract action
        // is probably being skipped until retry if there was a previous package extract action.
        hr = AddExtractPayload(pThisPackageExtractAction, pPackage, pPayload, sczPayloadWorkingPath);
        ExitOnFailure(hr, "Failed to add extract payload to current package.");
    }
    else // add a payload acquire action to the plan.
    {
        // Try to find an existing acquire action for this payload. If one is not found,
        // we'll create it. At the same time we will change any cache/layout payload actions
        // that would "MOVE" the file to "COPY" so that our new cache/layout action below
        // can do the move.
        pCacheAction = ProcessSharedPayload(pPlan, pPayload);
        if (!pCacheAction)
        {
            hr = AppendCacheAction(pPlan, &pCacheAction);
            ExitOnFailure(hr, "Failed to append cache action to acquire payload.");

            pCacheAction->type = BURN_CACHE_ACTION_TYPE_ACQUIRE_PAYLOAD;
            pCacheAction->fSkipUntilRetried = fPayloadCached;
            pCacheAction->resolvePayload.pPackage = pPackage;
            pCacheAction->resolvePayload.pPayload = pPayload;
            hr = StrAllocString(&pCacheAction->resolvePayload.sczUnverifiedPath, sczPayloadWorkingPath, 0);
            ExitOnFailure(hr, "Failed to copy unverified path for payload to acquire.");
        }

        iTryAgainAction = static_cast<DWORD>(pCacheAction - pPlan->rgCacheActions);
        pCacheAction = NULL;
    }

    Assert(BURN_PLAN_INVALID_ACTION_INDEX != iTryAgainAction);
    Assert(BURN_CACHE_ACTION_TYPE_ACQUIRE_CONTAINER == pPlan->rgCacheActions[iTryAgainAction].type ||
           BURN_CACHE_ACTION_TYPE_EXTRACT_CONTAINER == pPlan->rgCacheActions[iTryAgainAction].type ||
           BURN_CACHE_ACTION_TYPE_ACQUIRE_PAYLOAD == pPlan->rgCacheActions[iTryAgainAction].type);

    hr = AppendCacheAction(pPlan, &pCacheAction);
    ExitOnFailure(hr, "Failed to append cache action to cache payload.");

    if (NULL == wzLayoutDirectory)
    {
        pCacheAction->type = BURN_CACHE_ACTION_TYPE_CACHE_PAYLOAD;
        pCacheAction->cachePayload.pPackage = pPackage;
        pCacheAction->cachePayload.pPayload = pPayload;
        pCacheAction->cachePayload.fMove = TRUE;
        pCacheAction->cachePayload.iTryAgainAction = iTryAgainAction;
        pCacheAction->cachePayload.sczUnverifiedPath = sczPayloadWorkingPath;
        sczPayloadWorkingPath = NULL;
    }
    else
    {
        hr = StrAllocString(&pCacheAction->layoutPayload.sczLayoutDirectory, wzLayoutDirectory, 0);
        ExitOnFailure(hr, "Failed to copy layout directory into plan.");

        pCacheAction->type = BURN_CACHE_ACTION_TYPE_LAYOUT_PAYLOAD;
        pCacheAction->layoutPayload.pPackage = pPackage;
        pCacheAction->layoutPayload.pPayload = pPayload;
        pCacheAction->layoutPayload.fMove = TRUE;
        pCacheAction->layoutPayload.iTryAgainAction = iTryAgainAction;
        pCacheAction->layoutPayload.sczUnverifiedPath = sczPayloadWorkingPath;
        sczPayloadWorkingPath = NULL;
    }

    pCacheAction = NULL;

    pPlan->qwCacheSizeTotal += pPayload->qwFileSize;

LExit:
    ReleaseStr(sczPayloadWorkingPath);

    return hr;
}

static BOOL FindContainerCacheAction(
    __in BURN_CACHE_ACTION_TYPE type,
    __in BURN_PLAN* pPlan,
    __in BURN_CONTAINER* pContainer,
    __in DWORD iSearchStart,
    __in DWORD iSearchEnd,
    __out_opt BURN_CACHE_ACTION** ppCacheAction,
    __out_opt DWORD* piCacheAction
    )
{
    BOOL fFound = FALSE; // assume we won't find what we are looking for.

    Assert(BURN_CACHE_ACTION_TYPE_ACQUIRE_CONTAINER == type || BURN_CACHE_ACTION_TYPE_EXTRACT_CONTAINER == type);

    iSearchStart = (BURN_PLAN_INVALID_ACTION_INDEX == iSearchStart) ? 0 : iSearchStart;
    iSearchEnd = (BURN_PLAN_INVALID_ACTION_INDEX == iSearchEnd) ? pPlan->cCacheActions : iSearchEnd;

    for (DWORD iSearch = iSearchStart; iSearch < iSearchEnd; ++iSearch)
    {
        BURN_CACHE_ACTION* pCacheAction = pPlan->rgCacheActions + iSearch;
        if (pCacheAction->type == type &&
            ((BURN_CACHE_ACTION_TYPE_ACQUIRE_CONTAINER == pCacheAction->type && pCacheAction->resolveContainer.pContainer == pContainer) ||
             (BURN_CACHE_ACTION_TYPE_EXTRACT_CONTAINER == pCacheAction->type && pCacheAction->extractContainer.pContainer == pContainer)))
        {
            if (ppCacheAction)
            {
                *ppCacheAction = pCacheAction;
            }

            if (piCacheAction)
            {
                *piCacheAction = iSearch;
            }

            fFound = TRUE;
            break;
        }
    }

    return fFound;
}

static HRESULT CreateContainerAcquireAndExtractAction(
    __in BURN_PLAN* pPlan,
    __in BURN_CONTAINER* pContainer,
    __in DWORD iPackageStartAction,
    __in BOOL fPayloadCached,
    __out BURN_CACHE_ACTION** ppContainerExtractAction,
    __out DWORD* piContainerTryAgainAction
    )
{
    HRESULT hr = S_OK;
    DWORD iAcquireAction = BURN_PLAN_INVALID_ACTION_INDEX;
    BURN_CACHE_ACTION* pContainerExtractAction = NULL;
    DWORD iExtractAction = BURN_PLAN_INVALID_ACTION_INDEX;
    DWORD iTryAgainAction = BURN_PLAN_INVALID_ACTION_INDEX;
    LPWSTR sczContainerWorkingPath = NULL;

    // If the container is actually attached to the executable then we will not need an acquire
    // container action.
    if (!pContainer->fActuallyAttached)
    {
        BURN_CACHE_ACTION* pAcquireContainerAction = NULL;

        // If there is no plan to acquire the container then add acquire action since we
        // can't extract stuff out of a container until we acquire the container.
        if (!FindContainerCacheAction(BURN_CACHE_ACTION_TYPE_ACQUIRE_CONTAINER, pPlan, pContainer, iPackageStartAction, BURN_PLAN_INVALID_ACTION_INDEX, &pAcquireContainerAction, &iAcquireAction))
        {
            hr = CacheCaclulateContainerWorkingPath(pPlan->wzBundleId, pContainer, &sczContainerWorkingPath);
            ExitOnFailure(hr, "Failed to calculate unverified path for container.");

            hr = AppendCacheAction(pPlan, &pAcquireContainerAction);
            ExitOnFailure(hr, "Failed to append acquire container action to plan.");

            iAcquireAction = pPlan->cCacheActions - 1;

            pAcquireContainerAction->type = BURN_CACHE_ACTION_TYPE_ACQUIRE_CONTAINER;
            pAcquireContainerAction->fSkipUntilRetried = TRUE; // we'll start by assuming the acquire is not necessary and the fPayloadCached below will set us straight if wrong.
            pAcquireContainerAction->resolveContainer.pContainer = pContainer;
            pAcquireContainerAction->resolveContainer.sczUnverifiedPath = sczContainerWorkingPath;
            sczContainerWorkingPath = NULL;
        }

        Assert(BURN_PLAN_INVALID_ACTION_INDEX != iAcquireAction);
        Assert(BURN_CACHE_ACTION_TYPE_ACQUIRE_CONTAINER == pAcquireContainerAction->type);
        Assert(pContainer == pAcquireContainerAction->resolveContainer.pContainer);
    }

    Assert((pContainer->fActuallyAttached && BURN_PLAN_INVALID_ACTION_INDEX == iAcquireAction) ||
           (!pContainer->fActuallyAttached && BURN_PLAN_INVALID_ACTION_INDEX != iAcquireAction));

    // If we do not find an action for extracting payloads from this container, create it now.
    if (!FindContainerCacheAction(BURN_CACHE_ACTION_TYPE_EXTRACT_CONTAINER, pPlan, pContainer, (BURN_PLAN_INVALID_ACTION_INDEX == iAcquireAction) ? iPackageStartAction : iAcquireAction, BURN_PLAN_INVALID_ACTION_INDEX, &pContainerExtractAction, &iExtractAction))
    {
        // Attached containers that are actually attached use the executable path for their working path.
        if (pContainer->fActuallyAttached)
        {
            Assert(BURN_PLAN_INVALID_ACTION_INDEX == iAcquireAction);

            hr = PathForCurrentProcess(&sczContainerWorkingPath, NULL);
            ExitOnFailure(hr, "Failed to get path for executing module as attached container working path.");
        }
        else // use the acquired working path as the location of the container.
        {
            Assert(BURN_PLAN_INVALID_ACTION_INDEX != iAcquireAction);

            hr = StrAllocString(&sczContainerWorkingPath, pPlan->rgCacheActions[iAcquireAction].resolveContainer.sczUnverifiedPath, 0);
            ExitOnFailure(hr, "Failed to copy container unverified path for cache action to extract container.");
        }

        hr = AppendCacheAction(pPlan, &pContainerExtractAction);
        ExitOnFailure(hr, "Failed to append cache action to extract payloads from container.");

        iExtractAction = pPlan->cCacheActions - 1;

        pContainerExtractAction->type = BURN_CACHE_ACTION_TYPE_EXTRACT_CONTAINER;
        pContainerExtractAction->fSkipUntilRetried = pContainer->fActuallyAttached; // assume we can skip the extract engine when the container is already attached and the fPayloadCached below will set us straight if wrong.
        pContainerExtractAction->extractContainer.pContainer = pContainer;
        pContainerExtractAction->extractContainer.iSkipUntilAcquiredByAction = iAcquireAction;
        pContainerExtractAction->extractContainer.sczContainerUnverifiedPath = sczContainerWorkingPath;
        sczContainerWorkingPath = NULL;
    }

    Assert(BURN_CACHE_ACTION_TYPE_EXTRACT_CONTAINER == pContainerExtractAction->type);
    Assert(BURN_PLAN_INVALID_ACTION_INDEX != iExtractAction);

    // If there is an acquire action, that is our try again action otherwise, we'll use the extract action.
    iTryAgainAction = (BURN_PLAN_INVALID_ACTION_INDEX != iAcquireAction) ? iAcquireAction : iExtractAction;

    // If the try again action thinks it can be skipped but the payload is not cached ensure
    // the action will not be skipped. That also means we need to add this container to the
    // cache size if it is not already attached.
    BURN_CACHE_ACTION* pTryAgainAction = pPlan->rgCacheActions + iTryAgainAction;
    Assert((BURN_CACHE_ACTION_TYPE_ACQUIRE_CONTAINER == pTryAgainAction->type && pContainer == pTryAgainAction->resolveContainer.pContainer) ||
           (BURN_CACHE_ACTION_TYPE_EXTRACT_CONTAINER == pTryAgainAction->type && pContainer == pTryAgainAction->extractContainer.pContainer));
    if (pTryAgainAction->fSkipUntilRetried && !fPayloadCached)
    {
        pTryAgainAction->fSkipUntilRetried = FALSE;

        if (!pContainer->fActuallyAttached)
        {
            pPlan->qwCacheSizeTotal += pContainer->qwFileSize;
        }
    }

    *ppContainerExtractAction = pContainerExtractAction;
    *piContainerTryAgainAction = iTryAgainAction;

LExit:
    ReleaseStr(sczContainerWorkingPath);

    return hr;
}

static HRESULT AddExtractPayload(
    __in BURN_CACHE_ACTION* pCacheAction,
    __in_opt BURN_PACKAGE* pPackage,
    __in BURN_PAYLOAD* pPayload,
    __in_z LPCWSTR wzPayloadWorkingPath
    )
{
    HRESULT hr = S_OK;

    Assert(BURN_CACHE_ACTION_TYPE_EXTRACT_CONTAINER == pCacheAction->type);

    hr = MemEnsureArraySize(reinterpret_cast<LPVOID*>(&pCacheAction->extractContainer.rgPayloads), pCacheAction->extractContainer.cPayloads + 1, sizeof(BURN_EXTRACT_PAYLOAD), 5);
    ExitOnFailure(hr, "Failed to grow list of payloads to extract from container.");

    BURN_EXTRACT_PAYLOAD* pExtractPayload = pCacheAction->extractContainer.rgPayloads + pCacheAction->extractContainer.cPayloads;
    pExtractPayload->pPackage = pPackage;
    pExtractPayload->pPayload = pPayload;
    hr = StrAllocString(&pExtractPayload->sczUnverifiedPath, wzPayloadWorkingPath, 0);
    ExitOnFailure(hr, "Failed to copy unverified path for payload to extract.");
    ++pCacheAction->extractContainer.cPayloads;

    pCacheAction->extractContainer.qwTotalExtractSize += pPayload->qwFileSize;

LExit:
    return hr;
}

static BURN_CACHE_ACTION* ProcessSharedPayload(
    __in BURN_PLAN* pPlan,
    __in BURN_PAYLOAD* pPayload
    )
{
    DWORD cMove = 0;
    BURN_CACHE_ACTION* pAcquireAction = NULL;

    for (DWORD i = 0; i < pPlan->cCacheActions; ++i)
    {
        BURN_CACHE_ACTION* pCacheAction = pPlan->rgCacheActions + i;

        if (BURN_CACHE_ACTION_TYPE_ACQUIRE_PAYLOAD == pCacheAction->type &&
            pCacheAction->resolvePayload.pPayload == pPayload)
        {
            AssertSz(!pAcquireAction, "There should at most one acquire cache action per payload.");
            pAcquireAction = pCacheAction;
        }
        else if (BURN_CACHE_ACTION_TYPE_CACHE_PAYLOAD == pCacheAction->type &&
                 pCacheAction->cachePayload.pPayload == pPayload &&
                 pCacheAction->cachePayload.fMove)
        {
            // Since we found a shared payload, change its operation from MOVE to COPY.
            pCacheAction->cachePayload.fMove = FALSE;

            AssertSz(1 == ++cMove, "Shared payload should be moved once and only once.");
#ifndef DEBUG
            break;
#endif
        }
        else if (BURN_CACHE_ACTION_TYPE_LAYOUT_PAYLOAD == pCacheAction->type &&
                 pCacheAction->layoutPayload.pPayload == pPayload &&
                 pCacheAction->layoutPayload.fMove)
        {
            // Since we found a shared payload, change its operation from MOVE to COPY if necessary
            pCacheAction->layoutPayload.fMove = FALSE;

            AssertSz(1 == ++cMove, "Shared payload should be moved once and only once.");
#ifndef DEBUG
            break;
#endif
        }
    }

    return pAcquireAction;
}

static HRESULT RemoveUnnecessaryActions(
    __in BOOL fExecute,
    __in BURN_EXECUTE_ACTION* rgActions,
    __in DWORD cActions
    )
{
    HRESULT hr = S_OK;
    LPCSTR szExecuteOrRollback = fExecute ? "execute" : "rollback";

    for (DWORD i = 0; i < cActions; ++i)
    {
        BURN_EXECUTE_ACTION* pAction = rgActions + i;

        // If this MSP targets a package in the chain, check the target's execute state
        // to see if this patch should be skipped.
        if (BURN_EXECUTE_ACTION_TYPE_MSP_TARGET == pAction->type && pAction->mspTarget.pChainedTargetPackage)
        {
            BOOTSTRAPPER_ACTION_STATE chainedTargetPackageAction = fExecute ? pAction->mspTarget.pChainedTargetPackage->execute : pAction->mspTarget.pChainedTargetPackage->rollback;
            if (BOOTSTRAPPER_ACTION_STATE_UNINSTALL == chainedTargetPackageAction)
            {
                LogId(REPORT_STANDARD, MSG_PLAN_SKIP_PATCH_ACTION, pAction->mspTarget.pPackage->sczId, LoggingActionStateToString(pAction->mspTarget.action), pAction->mspTarget.pChainedTargetPackage->sczId, LoggingActionStateToString(chainedTargetPackageAction), szExecuteOrRollback);
                pAction->fDeleted = TRUE;
            }
            else if (pAction->mspTarget.fSlipstream && BOOTSTRAPPER_ACTION_STATE_UNINSTALL < chainedTargetPackageAction && BOOTSTRAPPER_ACTION_STATE_UNINSTALL < pAction->mspTarget.action)
            {
                LogId(REPORT_STANDARD, MSG_PLAN_SKIP_SLIPSTREAM_ACTION, pAction->mspTarget.pPackage->sczId, LoggingActionStateToString(pAction->mspTarget.action), pAction->mspTarget.pChainedTargetPackage->sczId, LoggingActionStateToString(chainedTargetPackageAction), szExecuteOrRollback);
                pAction->fDeleted = TRUE;
            }
        }
    }

    return hr;
}

static HRESULT FinalizeSlipstreamPatchActions(
    __in BOOL fExecute,
    __in BURN_EXECUTE_ACTION* rgActions,
    __in DWORD cActions
    )
{
    HRESULT hr = S_OK;

    for (DWORD i = 0; i < cActions; ++i)
    {
        BURN_EXECUTE_ACTION* pAction = rgActions + i;

        // If this MSI package contains slipstream patches, store patch actions here.
        if (BURN_EXECUTE_ACTION_TYPE_MSI_PACKAGE == pAction->type && pAction->msiPackage.pPackage->Msi.cSlipstreamMspPackages)
        {
            BURN_PACKAGE* pPackage = pAction->msiPackage.pPackage;
            pAction->msiPackage.rgSlipstreamPatches = (BOOTSTRAPPER_ACTION_STATE*)MemAlloc(sizeof(BOOTSTRAPPER_ACTION_STATE) * pPackage->Msi.cSlipstreamMspPackages, TRUE);
            ExitOnNull(pAction->msiPackage.rgSlipstreamPatches, hr, E_OUTOFMEMORY, "Failed to allocate memory for patch actions.");

            for (DWORD j = 0; j < pPackage->Msi.cSlipstreamMspPackages; ++j)
            {
                BURN_PACKAGE* pMspPackage = pPackage->Msi.rgpSlipstreamMspPackages[j];
                AssertSz(BURN_PACKAGE_TYPE_MSP == pMspPackage->type, "Only MSP packages can be slipstream patches.");

                pAction->msiPackage.rgSlipstreamPatches[j] = fExecute ? pMspPackage->execute : pMspPackage->rollback;
                for (DWORD k = 0; k < pMspPackage->Msp.cTargetProductCodes; ++k)
                {
                    BURN_MSPTARGETPRODUCT* pTargetProduct = pMspPackage->Msp.rgTargetProducts + k;
                    if (pPackage == pTargetProduct->pChainedTargetPackage)
                    {
                        pAction->msiPackage.rgSlipstreamPatches[j] = fExecute ? pTargetProduct->execute : pTargetProduct->rollback;
                        break;
                    }
                }
            }
        }
    }

LExit:
    return hr;
}


#ifdef DEBUG

static void CacheActionLog(
    __in DWORD iAction,
    __in BURN_CACHE_ACTION* pAction,
    __in BOOL fRollback
    )
{
    LPCWSTR wzBase = fRollback ? L"   Rollback cache" : L"   Cache";
    switch (pAction->type)
    {
    case BURN_CACHE_ACTION_TYPE_ACQUIRE_CONTAINER:
        LogStringLine(REPORT_STANDARD, "%ls action[%u]: ACQUIRE_CONTAINER id: %ls, source path: %ls, working path: %ls, skip until retried: %hs", wzBase, iAction, pAction->resolveContainer.pContainer->sczId, pAction->resolveContainer.pContainer->sczSourcePath, pAction->resolveContainer.sczUnverifiedPath, LoggingBoolToString(pAction->fSkipUntilRetried));
        break;

    case BURN_CACHE_ACTION_TYPE_ACQUIRE_PAYLOAD:
        LogStringLine(REPORT_STANDARD, "%ls action[%u]: ACQUIRE_PAYLOAD package id: %ls, payload id: %ls, source path: %ls, working path: %ls, skip until retried: %hs", wzBase, iAction, pAction->resolvePayload.pPackage ? pAction->resolvePayload.pPackage->sczId : L"", pAction->resolvePayload.pPayload->sczKey, pAction->resolvePayload.pPayload->sczSourcePath, pAction->resolvePayload.sczUnverifiedPath, LoggingBoolToString(pAction->fSkipUntilRetried));
        break;

    case BURN_CACHE_ACTION_TYPE_CACHE_PAYLOAD:
        LogStringLine(REPORT_STANDARD, "%ls action[%u]: CACHE_PAYLOAD package id: %ls, payload id: %ls, working path: %ls, operation: %ls, skip until retried: %hs, retry action: %u", wzBase, iAction, pAction->cachePayload.pPackage->sczId, pAction->cachePayload.pPayload->sczKey, pAction->cachePayload.sczUnverifiedPath, pAction->cachePayload.fMove ? L"move" : L"copy", LoggingBoolToString(pAction->fSkipUntilRetried), pAction->cachePayload.iTryAgainAction);
        break;

    case BURN_CACHE_ACTION_TYPE_CHECKPOINT:
        LogStringLine(REPORT_STANDARD, "%ls action[%u]: CHECKPOINT id: %u", wzBase, iAction, pAction->checkpoint.dwId);
        break;

    case BURN_CACHE_ACTION_TYPE_EXTRACT_CONTAINER:
        LogStringLine(REPORT_STANDARD, "%ls action[%u]: EXTRACT_CONTAINER id: %ls, working path: %ls, skip until retried: %hs, skip until acquired by action: %u", wzBase, iAction, pAction->extractContainer.pContainer->sczId, pAction->extractContainer.sczContainerUnverifiedPath, LoggingBoolToString(pAction->fSkipUntilRetried), pAction->extractContainer.iSkipUntilAcquiredByAction);
        for (DWORD j = 0; j < pAction->extractContainer.cPayloads; j++)
        {
            LogStringLine(REPORT_STANDARD, "      extract package id: %ls, payload id: %ls, working path: %ls", pAction->extractContainer.rgPayloads[j].pPackage->sczId, pAction->extractContainer.rgPayloads[j].pPayload->sczKey, pAction->extractContainer.rgPayloads[j].sczUnverifiedPath);
        }
        break;

    case BURN_CACHE_ACTION_TYPE_LAYOUT_BUNDLE:
        LogStringLine(REPORT_STANDARD, "%ls action[%u]: LAYOUT_BUNDLE working path: %ls, layout directory: %ls, exe name: %ls, skip until retried: %hs", wzBase, iAction, pAction->bundleLayout.sczUnverifiedPath, pAction->bundleLayout.sczLayoutDirectory, pAction->bundleLayout.sczExecutableName, LoggingBoolToString(pAction->fSkipUntilRetried));
        break;

    case BURN_CACHE_ACTION_TYPE_LAYOUT_PAYLOAD:
        LogStringLine(REPORT_STANDARD, "%ls action[%u]: LAYOUT_PAYLOAD package id: %ls, payload id: %ls, working path: %ls, layout directory: %ls, operation: %ls, skip until retried: %hs, retry action: %u", wzBase, iAction, pAction->layoutPayload.pPackage ? pAction->layoutPayload.pPackage->sczId : L"", pAction->layoutPayload.pPayload->sczKey, pAction->layoutPayload.sczUnverifiedPath, pAction->layoutPayload.sczLayoutDirectory, pAction->layoutPayload.fMove ? L"move" : L"copy", LoggingBoolToString(pAction->fSkipUntilRetried), pAction->layoutPayload.iTryAgainAction);
        break;

    case BURN_CACHE_ACTION_TYPE_PACKAGE_START:
        LogStringLine(REPORT_STANDARD, "%ls action[%u]: PACKAGE_START id: %ls, plan index for skip: %u, payloads to cache: %u, bytes to cache: %llu, skip until retried: %hs", wzBase, iAction, pAction->packageStart.pPackage->sczId, pAction->packageStart.iPackageCompleteAction, pAction->packageStart.cCachePayloads, pAction->packageStart.qwCachePayloadSizeTotal, LoggingBoolToString(pAction->fSkipUntilRetried));
        break;

    case BURN_CACHE_ACTION_TYPE_PACKAGE_STOP:
        LogStringLine(REPORT_STANDARD, "%ls action[%u]: PACKAGE_STOP id: %ls, skip until retried: %hs", wzBase, iAction, pAction->packageStop.pPackage->sczId, LoggingBoolToString(pAction->fSkipUntilRetried));
        break;

    case BURN_CACHE_ACTION_TYPE_ROLLBACK_PACKAGE:
        LogStringLine(REPORT_STANDARD, "%ls action[%u]: ROLLBACK_PACKAGE id: %ls, skip until retried: %hs", wzBase, iAction, pAction->rollbackPackage.pPackage->sczId, LoggingBoolToString(pAction->fSkipUntilRetried));
        break;

    case BURN_CACHE_ACTION_TYPE_SIGNAL_SYNCPOINT:
        LogStringLine(REPORT_STANDARD, "%ls action[%u]: SIGNAL_SYNCPOINT event handle: 0x%x, skip until retried: %hs", wzBase, iAction, pAction->syncpoint.hEvent, LoggingBoolToString(pAction->fSkipUntilRetried));
        break;

    case BURN_CACHE_ACTION_TYPE_TRANSACTION_BOUNDARY:
        LogStringLine(REPORT_STANDARD, "%ls action[%u]: TRANSACTION_BOUNDARY id: %ls, event handle: 0x%x, vital: %ls", wzBase, iAction, pAction->rollbackBoundary.pRollbackBoundary->sczId, pAction->rollbackBoundary.hEvent, pAction->rollbackBoundary.pRollbackBoundary->fVital ? L"yes" : L"no");
        break;

    default:
        AssertSz(FALSE, "Unknown cache action type.");
        break;
    }
}

static void ExecuteActionLog(
    __in DWORD iAction,
    __in BURN_EXECUTE_ACTION* pAction,
    __in BOOL fRollback
    )
{
    LPCWSTR wzBase = fRollback ? L"   Rollback" : L"   Execute";
    switch (pAction->type)
    {
    case BURN_EXECUTE_ACTION_TYPE_CHECKPOINT:
        LogStringLine(REPORT_STANDARD, "%ls action[%u]: CHECKPOINT id: %u", wzBase, iAction, pAction->checkpoint.dwId);
        break;

    case BURN_EXECUTE_ACTION_TYPE_DEPENDENCY:
        LogStringLine(REPORT_STANDARD, "%ls action[%u]: DEPENDENCY package id: %ls, bundle provider key: %ls, action: %u", wzBase, iAction, pAction->dependency.pPackage->sczId, pAction->dependency.sczBundleProviderKey, pAction->dependency.action);
        break;

    case BURN_EXECUTE_ACTION_TYPE_EXE_PACKAGE:
        LogStringLine(REPORT_STANDARD, "%ls action[%u]: EXE_PACKAGE package id: %ls, action: %hs, ignore dependencies: %ls", wzBase, iAction, pAction->exePackage.pPackage->sczId, LoggingActionStateToString(pAction->exePackage.action), pAction->exePackage.sczIgnoreDependencies);
        break;

    case BURN_EXECUTE_ACTION_TYPE_MSI_PACKAGE:
        LogStringLine(REPORT_STANDARD, "%ls action[%u]: MSI_PACKAGE package id: %ls, action: %hs, ui level: %u, log path: %ls, logging attrib: %u", wzBase, iAction, pAction->msiPackage.pPackage->sczId, LoggingActionStateToString(pAction->msiPackage.action), pAction->msiPackage.uiLevel, pAction->msiPackage.sczLogPath, pAction->msiPackage.dwLoggingAttributes);
        for (DWORD j = 0; j < pAction->msiPackage.cPatches; ++j)
        {
            LogStringLine(REPORT_STANDARD, "      Patch[%u]: order: %u, msp package id: %ls", j, pAction->msiPackage.rgOrderedPatches->dwOrder, pAction->msiPackage.rgOrderedPatches[j].dwOrder, pAction->msiPackage.rgOrderedPatches[j].pPackage->sczId);
        }
        break;

    case BURN_EXECUTE_ACTION_TYPE_MSP_TARGET:
        LogStringLine(REPORT_STANDARD, "%ls action[%u]: MSP_TARGET package id: %ls, action: %hs, target product code: %ls, target per-machine: %ls, ui level: %u, log path: %ls", wzBase, iAction, pAction->mspTarget.pPackage->sczId, LoggingActionStateToString(pAction->mspTarget.action), pAction->mspTarget.sczTargetProductCode, pAction->mspTarget.fPerMachineTarget ? L"yes" : L"no", pAction->mspTarget.uiLevel, pAction->mspTarget.sczLogPath);
        for (DWORD j = 0; j < pAction->mspTarget.cOrderedPatches; ++j)
        {
            LogStringLine(REPORT_STANDARD, "      Patch[%u]: order: %u, msp package id: %ls", j, pAction->mspTarget.rgOrderedPatches->dwOrder, pAction->mspTarget.rgOrderedPatches[j].dwOrder, pAction->mspTarget.rgOrderedPatches[j].pPackage->sczId);
        }
        break;

    case BURN_EXECUTE_ACTION_TYPE_MSU_PACKAGE:
        LogStringLine(REPORT_STANDARD, "%ls action[%u]: MSU_PACKAGE package id: %ls, action: %hs, log path: %ls", wzBase, iAction, pAction->msuPackage.pPackage->sczId, LoggingActionStateToString(pAction->msuPackage.action), pAction->msuPackage.sczLogPath);
        break;

    case BURN_EXECUTE_ACTION_TYPE_REGISTRATION:
        LogStringLine(REPORT_STANDARD, "%ls action[%u]: REGISTRATION keep: %ls", wzBase, iAction, pAction->registration.fKeep ? L"yes" : L"no");
        break;

    case BURN_EXECUTE_ACTION_TYPE_ROLLBACK_BOUNDARY:
        LogStringLine(REPORT_STANDARD, "%ls action[%u]: ROLLBACK_BOUNDARY id: %ls, vital: %ls", wzBase, iAction, pAction->rollbackBoundary.pRollbackBoundary->sczId, pAction->rollbackBoundary.pRollbackBoundary->fVital ? L"yes" : L"no");
        break;

    case BURN_EXECUTE_ACTION_TYPE_WAIT_SYNCPOINT:
        LogStringLine(REPORT_STANDARD, "%ls action[%u]: WAIT_SYNCPOINT event handle: 0x%x", wzBase, iAction, pAction->syncpoint.hEvent);
        break;

    case BURN_EXECUTE_ACTION_TYPE_UNCACHE_PACKAGE:
        LogStringLine(REPORT_STANDARD, "%ls action[%u]: UNCACHE_PACKAGE id: %ls", wzBase, iAction, pAction->uncachePackage.pPackage->sczId);
        break;

    default:
        AssertSz(FALSE, "Unknown execute action type.");
        break;
    }
}

extern "C" void PlanDump(
    __in BURN_PLAN* pPlan
    )
{
    LogStringLine(REPORT_STANDARD, "--- Begin plan dump ---");

    LogStringLine(REPORT_STANDARD, "Plan action: %hs", LoggingBurnActionToString(pPlan->action));
    LogStringLine(REPORT_STANDARD, "     per-machine: %hs", LoggingTrueFalseToString(pPlan->fPerMachine));
    LogStringLine(REPORT_STANDARD, "     keep registration by default: %hs", LoggingTrueFalseToString(pPlan->fKeepRegistrationDefault));
    LogStringLine(REPORT_STANDARD, "     estimated size: %llu", pPlan->qwEstimatedSize);

    LogStringLine(REPORT_STANDARD, "Plan cache size: %llu", pPlan->qwCacheSizeTotal);
    for (DWORD i = 0; i < pPlan->cCacheActions; ++i)
    {
        CacheActionLog(i, pPlan->rgCacheActions + i, FALSE);
    }

    for (DWORD i = 0; i < pPlan->cRollbackCacheActions; ++i)
    {
        CacheActionLog(i, pPlan->rgRollbackCacheActions + i, TRUE);
    }

    LogStringLine(REPORT_STANDARD, "Plan execute package count: %u", pPlan->cExecutePackagesTotal);
    LogStringLine(REPORT_STANDARD, "     overall progress ticks: %u", pPlan->cOverallProgressTicksTotal);
    for (DWORD i = 0; i < pPlan->cExecuteActions; ++i)
    {
        ExecuteActionLog(i, pPlan->rgExecuteActions + i, FALSE);
    }

    for (DWORD i = 0; i < pPlan->cRollbackActions; ++i)
    {
        ExecuteActionLog(i, pPlan->rgRollbackActions + i, TRUE);
    }

    for (DWORD i = 0; i < pPlan->cCleanActions; ++i)
    {
        LogStringLine(REPORT_STANDARD, "   Clean action[%u]: CLEAN_PACKAGE package id: %ls", i, pPlan->rgCleanActions[i].pPackage->sczId);
    }

    for (DWORD i = 0; i < pPlan->cPlannedProviders; ++i)
    {
        LogStringLine(REPORT_STANDARD, "   Dependency action[%u]: PLANNED_PROVIDER key: %ls, name: %ls", i, pPlan->rgPlannedProviders[i].sczKey, pPlan->rgPlannedProviders[i].sczName);
    }

    LogStringLine(REPORT_STANDARD, "--- End plan dump ---");
}

#endif
