//-------------------------------------------------------------------------------------------------
// <copyright file="plan.h" company="Microsoft">
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

#pragma once


#if defined(__cplusplus)
extern "C" {
#endif


// constants

const DWORD BURN_PLAN_INVALID_ACTION_INDEX = 0x80000000;

enum BURN_CACHE_ACTION_TYPE
{
    BURN_CACHE_ACTION_TYPE_NONE,
    BURN_CACHE_ACTION_TYPE_CHECKPOINT,
    BURN_CACHE_ACTION_TYPE_LAYOUT_BUNDLE,
    BURN_CACHE_ACTION_TYPE_PACKAGE_START,
    BURN_CACHE_ACTION_TYPE_PACKAGE_STOP,
    BURN_CACHE_ACTION_TYPE_ROLLBACK_PACKAGE,
    BURN_CACHE_ACTION_TYPE_SIGNAL_SYNCPOINT,
    BURN_CACHE_ACTION_TYPE_ACQUIRE_CONTAINER,
    BURN_CACHE_ACTION_TYPE_EXTRACT_CONTAINER,
    BURN_CACHE_ACTION_TYPE_ACQUIRE_PAYLOAD,
    BURN_CACHE_ACTION_TYPE_CACHE_PAYLOAD,
    BURN_CACHE_ACTION_TYPE_LAYOUT_PAYLOAD,
    BURN_CACHE_ACTION_TYPE_TRANSACTION_BOUNDARY,
};

enum BURN_EXECUTE_ACTION_TYPE
{
    BURN_EXECUTE_ACTION_TYPE_NONE,
    BURN_EXECUTE_ACTION_TYPE_CHECKPOINT,
    BURN_EXECUTE_ACTION_TYPE_WAIT_SYNCPOINT,
    BURN_EXECUTE_ACTION_TYPE_UNCACHE_PACKAGE,
    BURN_EXECUTE_ACTION_TYPE_EXE_PACKAGE,
    BURN_EXECUTE_ACTION_TYPE_MSI_PACKAGE,
    BURN_EXECUTE_ACTION_TYPE_MSP_TARGET,
    BURN_EXECUTE_ACTION_TYPE_MSU_PACKAGE,
    BURN_EXECUTE_ACTION_TYPE_SERVICE_STOP,
    BURN_EXECUTE_ACTION_TYPE_SERVICE_START,
    BURN_EXECUTE_ACTION_TYPE_DEPENDENCY,
    BURN_EXECUTE_ACTION_TYPE_ROLLBACK_BOUNDARY,
    BURN_EXECUTE_ACTION_TYPE_REGISTRATION,
};

enum BURN_CLEAN_ACTION_TYPE
{
    BURN_CLEAN_ACTION_TYPE_NONE,
    BURN_CLEAN_ACTION_TYPE_BUNDLE,
    BURN_CLEAN_ACTION_TYPE_PACKAGE,
};


// structs

typedef struct _BURN_EXTRACT_PAYLOAD
{
    BURN_PACKAGE* pPackage;
    BURN_PAYLOAD* pPayload;
    LPWSTR sczUnverifiedPath;
} BURN_EXTRACT_PAYLOAD;

typedef struct _BURN_CACHE_ACTION
{
    BURN_CACHE_ACTION_TYPE type;
    BOOL fSkipUntilRetried;
    union
    {
        struct
        {
            DWORD dwId;
        } checkpoint;
        struct
        {
            LPWSTR sczExecutableName;
            LPWSTR sczLayoutDirectory;
            LPWSTR sczUnverifiedPath;
            DWORD64 qwBundleSize;
        } bundleLayout;
        struct
        {
            BURN_PACKAGE* pPackage;
            DWORD cCachePayloads;
            DWORD64 qwCachePayloadSizeTotal;
            DWORD iPackageCompleteAction;
        } packageStart;
        struct
        {
            BURN_PACKAGE* pPackage;
        } packageStop;
        struct
        {
            BURN_PACKAGE* pPackage;
        } rollbackPackage;
        struct
        {
            HANDLE hEvent;
        } syncpoint;
        struct
        {
            BURN_CONTAINER* pContainer;
            LPWSTR sczUnverifiedPath;
        } resolveContainer;
        struct
        {
            BURN_CONTAINER* pContainer;
            DWORD64 qwTotalExtractSize;
            DWORD iSkipUntilAcquiredByAction;
            LPWSTR sczContainerUnverifiedPath;

            BURN_EXTRACT_PAYLOAD* rgPayloads;
            DWORD cPayloads;
        } extractContainer;
        struct
        {
            BURN_PACKAGE* pPackage;
            BURN_PAYLOAD* pPayload;
            LPWSTR sczUnverifiedPath;
        } resolvePayload;
        struct
        {
            BURN_PACKAGE* pPackage;
            BURN_PAYLOAD* pPayload;
            DWORD iTryAgainAction;
            DWORD cTryAgainAttempts;
            LPWSTR sczUnverifiedPath;
            BOOL fMove;
        } cachePayload;
        struct
        {
            BURN_PACKAGE* pPackage;
            BURN_PAYLOAD* pPayload;
            DWORD iTryAgainAction;
            DWORD cTryAgainAttempts;
            LPWSTR sczLayoutDirectory;
            LPWSTR sczUnverifiedPath;
            BOOL fMove;
        } layoutPayload;
        struct
        {
            BURN_ROLLBACK_BOUNDARY* pRollbackBoundary;
            HANDLE hEvent;
        } rollbackBoundary;
    };
} BURN_CACHE_ACTION;

typedef struct _BURN_ORDERED_PATCHES
{
    DWORD dwOrder;
    BURN_PACKAGE* pPackage;
} BURN_ORDERED_PATCHES;

typedef struct _BURN_EXECUTE_ACTION
{
    BURN_EXECUTE_ACTION_TYPE type;
    BOOL fDeleted; // used to skip an action after it was planned since deleting actions out of the plan is too hard.
    union
    {
        struct
        {
            DWORD dwId;
        } checkpoint;
        struct
        {
            HANDLE hEvent;
        } syncpoint;
        struct
        {
            BURN_PACKAGE* pPackage;
        } uncachePackage;
        struct
        {
            BURN_PACKAGE* pPackage;
            BOOTSTRAPPER_ACTION_STATE action;
            LPWSTR sczIgnoreDependencies;
        } exePackage;
        struct
        {
            BURN_PACKAGE* pPackage;
            LPWSTR sczLogPath;
            DWORD dwLoggingAttributes;
            INSTALLUILEVEL uiLevel;
            BOOTSTRAPPER_ACTION_STATE action;

            BOOTSTRAPPER_FEATURE_ACTION* rgFeatures;
            BOOTSTRAPPER_ACTION_STATE* rgSlipstreamPatches;

            BURN_ORDERED_PATCHES* rgOrderedPatches;
            DWORD cPatches;
        } msiPackage;
        struct
        {
            BURN_PACKAGE* pPackage;
            LPWSTR sczTargetProductCode;
            BURN_PACKAGE* pChainedTargetPackage;
            BOOL fSlipstream;
            BOOL fPerMachineTarget;
            LPWSTR sczLogPath;
            INSTALLUILEVEL uiLevel;
            BOOTSTRAPPER_ACTION_STATE action;

            BURN_ORDERED_PATCHES* rgOrderedPatches;
            DWORD cOrderedPatches;
        } mspTarget;
        struct
        {
            BURN_PACKAGE* pPackage;
            LPWSTR sczLogPath;
            BOOTSTRAPPER_ACTION_STATE action;
        } msuPackage;
        struct
        {
            LPWSTR sczServiceName;
        } service;
        struct
        {
            BOOL fKeep;
        } registration;
        struct
        {
            BURN_ROLLBACK_BOUNDARY* pRollbackBoundary;
        } rollbackBoundary;
        struct
        {
            BURN_PACKAGE* pPackage;
            LPWSTR sczBundleProviderKey;
            BURN_DEPENDENCY_ACTION action;
        } dependency;
    };
} BURN_EXECUTE_ACTION;

typedef struct _BURN_CLEAN_ACTION
{
    BURN_PACKAGE* pPackage;
} BURN_CLEAN_ACTION;

typedef struct _BURN_PLAN
{
    BOOTSTRAPPER_ACTION action;
    LPWSTR wzBundleId; // points directly into parent the ENGINE_STATE.
    BOOL fPerMachine;
    BOOL fKeepRegistrationDefault;

    DWORD64 qwCacheSizeTotal;

    DWORD64 qwEstimatedSize;

    DWORD cExecutePackagesTotal;
    DWORD cOverallProgressTicksTotal;

    BURN_CACHE_ACTION* rgCacheActions;
    DWORD cCacheActions;

    BURN_CACHE_ACTION* rgRollbackCacheActions;
    DWORD cRollbackCacheActions;

    BURN_EXECUTE_ACTION* rgExecuteActions;
    DWORD cExecuteActions;

    BURN_EXECUTE_ACTION* rgRollbackActions;
    DWORD cRollbackActions;

    BURN_CLEAN_ACTION* rgCleanActions;
    DWORD cCleanActions;

    DEPENDENCY* rgPlannedProviders;
    UINT cPlannedProviders;
} BURN_PLAN;


// functions

void PlanReset(
    __in BURN_PLAN* pPlan,
    __in BURN_PACKAGES* pPackages
    );
void PlanUninitializeExecuteAction(
    __in BURN_EXECUTE_ACTION* pExecuteAction
    );
HRESULT PlanSetVariables(
    __in BOOTSTRAPPER_ACTION action,
    __in BURN_VARIABLES* pVariables
    );
HRESULT PlanDefaultPackageRequestState(
    __in BURN_PACKAGE_TYPE packageType,
    __in BOOTSTRAPPER_PACKAGE_STATE currentState,
    __in BOOL fPermanent,
    __in BOOTSTRAPPER_ACTION action,
    __in BURN_VARIABLES* pVariables,
    __in_z_opt LPCWSTR wzInstallCondition,
    __in BOOTSTRAPPER_RELATION_TYPE relationType,
    __out BOOTSTRAPPER_REQUEST_STATE* pRequestState
    );
HRESULT PlanLayoutBundle(
    __in BURN_PLAN* pPlan,
    __in_z LPCWSTR wzExecutableName,
    __in DWORD64 qwBundleSize,
    __in BURN_VARIABLES* pVariables,
    __in BURN_PAYLOADS* pPayloads,
    __out_z LPWSTR* psczLayoutDirectory
    );
HRESULT PlanLayoutPackage(
    __in BURN_PLAN* pPlan,
    __in BURN_PACKAGE* pPackage,
    __in_z LPCWSTR wzLayoutDirectory
    );
HRESULT PlanExecutePackage(
    __in BOOTSTRAPPER_DISPLAY display,
    __in BURN_USER_EXPERIENCE* pUserExperience,
    __in BURN_PLAN* pPlan,
    __in BURN_PACKAGE* pPackage,
    __in BURN_LOGGING* pLog,
    __in BURN_VARIABLES* pVariables,
    __in_z LPCWSTR wzBundleProviderKey,
    __inout HANDLE* phSyncpointEvent
    );
HRESULT PlanRelatedBundles(
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
    );
HRESULT PlanFinalizeActions(
    __in BURN_PLAN* pPlan
    );
HRESULT PlanCleanPackage(
    __in BOOTSTRAPPER_ACTION action,
    __in BURN_PLAN* pPlan,
    __in BURN_PACKAGE* pPackage
    );
HRESULT PlanExecuteCacheSyncAndRollback(
    __in BURN_PLAN* pPlan,
    __in BURN_PACKAGE* pPackage,
    __in HANDLE hCacheEvent,
    __in BOOL fPlanPackageCacheRollback
    );
HRESULT PlanExecuteCheckpoint(
    __in BURN_PLAN* pPlan
    );
HRESULT PlanInsertExecuteAction(
    __in DWORD dwIndex,
    __in BURN_PLAN* pPlan,
    __out BURN_EXECUTE_ACTION** ppExecuteAction
    );
HRESULT PlanInsertRollbackAction(
    __in DWORD dwIndex,
    __in BURN_PLAN* pPlan,
    __out BURN_EXECUTE_ACTION** ppRollbackAction
    );
HRESULT PlanAppendExecuteAction(
    __in BURN_PLAN* pPlan,
    __out BURN_EXECUTE_ACTION** ppExecuteAction
    );
HRESULT PlanAppendRollbackAction(
    __in BURN_PLAN* pPlan,
    __out BURN_EXECUTE_ACTION** ppExecuteAction
    );
HRESULT PlanKeepRegistration(
    __in BURN_PLAN* pPlan,
    __in DWORD iAfterExecutePackageAction,
    __in DWORD iBeforeRollbackPackageAction
    );
HRESULT PlanRemoveRegistration(
    __in BURN_PLAN* pPlan,
    __in DWORD iAfterExecutePackageAction,
    __in DWORD iAfterRollbackPackageAction
    );
HRESULT PlanRollbackBoundaryBegin(
    __in BURN_PLAN* pPlan,
    __in BURN_ROLLBACK_BOUNDARY* pRollbackBoundary
    );
HRESULT PlanRollbackBoundaryComplete(
    __in BURN_PLAN* pPlan
    );
HRESULT PlanSetResumeCommand(
    __in BURN_REGISTRATION* pRegistration,
    __in BOOTSTRAPPER_ACTION action,
    __in BOOTSTRAPPER_COMMAND* pCommand,
    __in BURN_LOGGING* pLog
    );
//HRESULT AppendExecuteWaitAction(
//    __in BURN_PLAN* pPlan,
//    __in HANDLE hEvent
//    );
//HRESULT AppendExecuteCheckpointAction(
//    __in BURN_PLAN* pPlan,
//    __in DWORD dwId
//    );

#ifdef DEBUG
void PlanDump(
    __in BURN_PLAN* pPlan
    );
#endif

#if defined(__cplusplus)
}
#endif
