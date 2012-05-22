//-------------------------------------------------------------------------------------------------
// <copyright file="mspengine.cpp" company="Microsoft">
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

#include "precomp.h"


// constants


// structs


// internal function declarations

static void DeterminePatchChainedTarget(
    __in BURN_PACKAGES* pPackages,
    __in BURN_PACKAGE* pMspPackage,
    __in LPCWSTR wzTargetProductCode,
    __out BURN_PACKAGE** ppChainedTargetPackage,
    __out BOOL* pfSlipstreamed
    );
static HRESULT AddDetectedTargetProduct(
    __in BURN_PACKAGES* pPackages,
    __in BURN_PACKAGE* pPackage,
    __in MSIINSTALLCONTEXT context,
    __in DWORD dwOrder,
    __in_z LPCWSTR wzProductCode
    );
static HRESULT PlanTargetProduct(
    __in BOOTSTRAPPER_DISPLAY display,
    __in BOOL fRollback,
    __in BURN_PLAN* pPlan,
    __in BURN_LOGGING* pLog,
    __in BURN_VARIABLES* pVariables,
    __in BOOTSTRAPPER_ACTION_STATE actionState,
    __in BURN_PACKAGE* pPackage,
    __in BURN_MSPTARGETPRODUCT* pTargetProduct
    );


// function definitions

extern "C" HRESULT MspEngineParsePackageFromXml(
    __in IXMLDOMNode* pixnMspPackage,
    __in BURN_PACKAGE* pPackage
    )
{
    HRESULT hr = S_OK;

    // @PatchCode
    hr = XmlGetAttributeEx(pixnMspPackage, L"PatchCode", &pPackage->Msp.sczPatchCode);
    ExitOnFailure(hr, "Failed to get @PatchCode.");

    // @PatchXml
    hr = XmlGetAttributeEx(pixnMspPackage, L"PatchXml", &pPackage->Msp.sczApplicabilityXml);
    ExitOnFailure(hr, "Failed to get @PatchXml.");

    // @DisplayInternalUI
    hr = XmlGetYesNoAttribute(pixnMspPackage, L"DisplayInternalUI", &pPackage->Msp.fDisplayInternalUI);
    ExitOnFailure(hr, "Failed to get @DisplayInternalUI.");

    // Read properties.
    hr = MsiEngineParsePropertiesFromXml(pixnMspPackage, &pPackage->Msp.rgProperties, &pPackage->Msp.cProperties);
    ExitOnFailure(hr, "Failed to parse properties from XML.");

LExit:

    return hr;
}

extern "C" void MspEnginePackageUninitialize(
    __in BURN_PACKAGE* pPackage
    )
{
    ReleaseStr(pPackage->Msp.sczPatchCode);
    ReleaseStr(pPackage->Msp.sczApplicabilityXml);

    // free properties
    if (pPackage->Msp.rgProperties)
    {
        for (DWORD i = 0; i < pPackage->Msp.cProperties; ++i)
        {
            BURN_MSIPROPERTY* pProperty = &pPackage->Msp.rgProperties[i];

            ReleaseStr(pProperty->sczId);
            ReleaseStr(pProperty->sczValue);
            ReleaseStr(pProperty->sczRollbackValue);
        }
        MemFree(pPackage->Msp.rgProperties);
    }

    // free target products
    ReleaseMem(pPackage->Msp.rgTargetProducts);

    // clear struct
    memset(&pPackage->Msp, 0, sizeof(pPackage->Msp));
}

extern "C" HRESULT MspEngineDetectInitialize(
    __in BURN_PACKAGES* pPackages
    )
{
    AssertSz(pPackages->cPatchInfo, "MspEngineDetectInitialize() should only be called if there are MSP packages.");

    static MSIINSTALLCONTEXT rgContexts[] = { MSIINSTALLCONTEXT_USERUNMANAGED, MSIINSTALLCONTEXT_USERMANAGED, MSIINSTALLCONTEXT_MACHINE };

    HRESULT hr = S_OK;
    DWORD iProduct = 0;
    WCHAR wzProductCode[MAX_GUID_CHARS + 1];

#ifdef DEBUG
    // All patch info should be initialized to zero.
    for (DWORD i = 0; i < pPackages->cPatchInfo; ++i)
    {
        BURN_PACKAGE* pPackage = pPackages->rgPatchInfoToPackage[i];
        Assert(!pPackage->Msp.cTargetProductCodes);
        Assert(!pPackage->Msp.rgTargetProducts);
    }
#endif

    // Loop through all products on the machine, testing the collective patch applicability
    // against each product in all contexts. Store the result with the appropriate patch package.
    do
    {
        hr = WiuEnumProducts(iProduct, wzProductCode);
        if (SUCCEEDED(hr))
        {
            for (DWORD i = 0; i < countof(rgContexts); ++i)
            {
                hr = WiuDeterminePatchSequence(wzProductCode, NULL, rgContexts[i], pPackages->rgPatchInfo, pPackages->cPatchInfo);
                if (SUCCEEDED(hr))
                {
                    for (DWORD iPatchInfo = 0; iPatchInfo < pPackages->cPatchInfo; ++iPatchInfo)
                    {
                        if (ERROR_SUCCESS == pPackages->rgPatchInfo[iPatchInfo].uStatus)
                        {
                            BURN_PACKAGE* pMspPackage = pPackages->rgPatchInfoToPackage[iPatchInfo];
                            Assert(BURN_PACKAGE_TYPE_MSP == pMspPackage->type);

                            // Note that we do add superseded and obsolete MSP packages. Package Detect and Plan will sort them out later.
                            hr = AddDetectedTargetProduct(pPackages, pMspPackage, rgContexts[i], pPackages->rgPatchInfo[iPatchInfo].dwOrder, wzProductCode);
                            ExitOnFailure1(hr, "Failed to add target product code to package: %ls", pMspPackage->sczId);
                        }
                        // TODO: should we log something for this error case?
                    }
                }
                // TODO: should we log something for this error case?
            }

            hr = S_OK; // always reset so we test all possible target products.
        }
        else if (E_BADCONFIGURATION == hr)
        {
            // Skip this product and continue.
            LogId(REPORT_STANDARD, MSG_DETECT_BAD_PRODUCT_CONFIGURATION, wzProductCode);

            hr = S_OK;
        }

        ++iProduct;

    } while (SUCCEEDED(hr));

    if (E_NOMOREITEMS == hr)
    {
        hr = S_OK;
    }
    ExitOnFailure(hr, "Failed to test patches applicability against all products on the machine.");

LExit:
    return hr;
}

extern "C" HRESULT MspEngineDetectPackage(
    __in BURN_PACKAGE* pPackage,
    __in BURN_USER_EXPERIENCE* pUserExperience
    )
{
    HRESULT hr = S_OK;
    int nResult = IDOK;
    LPWSTR sczState = NULL;

    if (0 == pPackage->Msp.cTargetProductCodes)
    {
        pPackage->currentState = BOOTSTRAPPER_PACKAGE_STATE_ABSENT;
    }
    else
    {
        // Start the package state at the the highest state then loop through all the
        // target product codes and end up setting the current state to the lowest
        // package state applied to the the target product codes.
        pPackage->currentState = BOOTSTRAPPER_PACKAGE_STATE_SUPERSEDED;

        for (DWORD i = 0; i < pPackage->Msp.cTargetProductCodes; ++i)
        {
            BURN_MSPTARGETPRODUCT* pTargetProduct = pPackage->Msp.rgTargetProducts + i;

            hr = WiuGetPatchInfoEx(pPackage->Msp.sczPatchCode, pTargetProduct->wzTargetProductCode, NULL, pTargetProduct->context, INSTALLPROPERTY_PATCHSTATE, &sczState);
            if (SUCCEEDED(hr))
            {
                switch (*sczState)
                {
                case '1':
                    pTargetProduct->patchPackageState = BOOTSTRAPPER_PACKAGE_STATE_PRESENT;
                    break;

                case '2':
                    pTargetProduct->patchPackageState = BOOTSTRAPPER_PACKAGE_STATE_SUPERSEDED;
                    break;

                case '4':
                    pTargetProduct->patchPackageState = BOOTSTRAPPER_PACKAGE_STATE_OBSOLETE;
                    break;

                default:
                    pTargetProduct->patchPackageState = BOOTSTRAPPER_PACKAGE_STATE_ABSENT;
                    break;
                }
            }
            else if (HRESULT_FROM_WIN32(ERROR_UNKNOWN_PATCH) == hr)
            {
                pTargetProduct->patchPackageState = BOOTSTRAPPER_PACKAGE_STATE_ABSENT;
                hr = S_OK;
            }
            ExitOnFailure2(hr, "Failed to get patch information for patch code: %ls, target product code: %ls", pPackage->Msp.sczPatchCode, pTargetProduct->wzTargetProductCode);

            if (pPackage->currentState > pTargetProduct->patchPackageState)
            {
                pPackage->currentState = pTargetProduct->patchPackageState;
            }

            nResult = pUserExperience->pUserExperience->OnDetectTargetMsiPackage(pPackage->sczId, pTargetProduct->wzTargetProductCode, pTargetProduct->patchPackageState);
            hr = UserExperienceInterpretResult(pUserExperience, MB_OKCANCEL, nResult);
            ExitOnRootFailure(hr, "UX aborted detect target MSI package.");
        }
    }

LExit:
    ReleaseStr(sczState);

    return hr;
}

//
// PlanCalculate - calculates the execute and rollback state for the requested package state.
//
extern "C" HRESULT MspEnginePlanCalculatePackage(
    __in BURN_PACKAGE* pPackage,
    __in BURN_USER_EXPERIENCE* pUserExperience
    )
{
    HRESULT hr = S_OK;

    for (DWORD i = 0; i < pPackage->Msp.cTargetProductCodes; ++i)
    {
        BURN_MSPTARGETPRODUCT* pTargetProduct = pPackage->Msp.rgTargetProducts + i;

        BOOTSTRAPPER_REQUEST_STATE requested = pPackage->requested;
        BOOTSTRAPPER_ACTION_STATE execute = BOOTSTRAPPER_ACTION_STATE_NONE;
        BOOTSTRAPPER_ACTION_STATE rollback = BOOTSTRAPPER_ACTION_STATE_NONE;

        int nResult = pUserExperience->pUserExperience->OnPlanTargetMsiPackage(pPackage->sczId, pTargetProduct->wzTargetProductCode, &requested);
        hr = UserExperienceInterpretResult(pUserExperience, MB_OKCANCEL, nResult);
        ExitOnRootFailure(hr, "UX aborted plan target MSI package.");

        // Calculate the execute action.
        switch (pTargetProduct->patchPackageState)
        {
        case BOOTSTRAPPER_PACKAGE_STATE_PRESENT:
            switch (requested)
            {
            case BOOTSTRAPPER_REQUEST_STATE_REPAIR:
                // If the patch targets a package in the chain and the target package is already being
                // repaired or uninstalled, don't do anything to the patch (because the target package
                // will do the operation for us).
                if (pTargetProduct->pChainedTargetPackage &&
                    (BOOTSTRAPPER_ACTION_STATE_REPAIR == pTargetProduct->pChainedTargetPackage->execute ||
                     BOOTSTRAPPER_ACTION_STATE_UNINSTALL == pTargetProduct->pChainedTargetPackage->execute))
                {
                    execute = BOOTSTRAPPER_ACTION_STATE_NONE;
                }
                else
                {
                    execute = BOOTSTRAPPER_ACTION_STATE_INSTALL;
                }
                break;

            case BOOTSTRAPPER_REQUEST_STATE_ABSENT: __fallthrough;
            case BOOTSTRAPPER_REQUEST_STATE_CACHE:
                // If the patch is not uninstallable or the patch targets a package in the chain and
                // the target package is already being uninstalled, don't do anything to the patch
                // (because the target package will do the operation for us).
                if (!pPackage->fUninstallable ||
                    (pTargetProduct->pChainedTargetPackage && BOOTSTRAPPER_ACTION_STATE_UNINSTALL == pTargetProduct->pChainedTargetPackage->execute))
                {
                    execute = BOOTSTRAPPER_ACTION_STATE_NONE;
                }
                else
                {
                    execute = BOOTSTRAPPER_ACTION_STATE_UNINSTALL;
                }
                break;

            default:
                execute = BOOTSTRAPPER_ACTION_STATE_NONE;
                break;
            }
            break;

        case BOOTSTRAPPER_PACKAGE_STATE_ABSENT:
            switch (requested)
            {
            case BOOTSTRAPPER_REQUEST_STATE_PRESENT: __fallthrough;
            case BOOTSTRAPPER_REQUEST_STATE_REPAIR:
                execute = BOOTSTRAPPER_ACTION_STATE_INSTALL;
                break;

            default:
                execute = BOOTSTRAPPER_ACTION_STATE_NONE;
                break;
            }
            break;
        }

        // Calculate the rollback action if there is an execute action.
        if (BOOTSTRAPPER_ACTION_STATE_NONE != execute)
        {
            switch (BOOTSTRAPPER_PACKAGE_STATE_UNKNOWN != pPackage->expected ? pPackage->expected : pPackage->currentState)
            {
            case BOOTSTRAPPER_PACKAGE_STATE_PRESENT:
                switch (requested)
                {
                case BOOTSTRAPPER_REQUEST_STATE_ABSENT:
                    rollback = BOOTSTRAPPER_ACTION_STATE_INSTALL;
                    break;

                default:
                    rollback = BOOTSTRAPPER_ACTION_STATE_NONE;
                    break;
                }
                break;

            case BOOTSTRAPPER_PACKAGE_STATE_ABSENT: __fallthrough;
            case BOOTSTRAPPER_PACKAGE_STATE_CACHED:
                switch (requested)
                {
                case BOOTSTRAPPER_REQUEST_STATE_PRESENT: __fallthrough;
                case BOOTSTRAPPER_REQUEST_STATE_REPAIR:
                    rollback = pPackage->fUninstallable ? BOOTSTRAPPER_ACTION_STATE_UNINSTALL : BOOTSTRAPPER_ACTION_STATE_NONE;
                    break;

                default:
                    rollback = BOOTSTRAPPER_ACTION_STATE_NONE;
                    break;
                }
                break;

            default:
                rollback = BOOTSTRAPPER_ACTION_STATE_NONE;
                break;
            }
        }

        pTargetProduct->execute = execute;
        pTargetProduct->rollback = rollback;

        // The highest aggregate action state found will be returned.
        if (pPackage->execute < execute)
        {
            pPackage->execute = execute;
        }

        if (pPackage->rollback < rollback)
        {
            pPackage->rollback = rollback;
        }
    }

LExit:

    return hr;
}

//
// PlanAdd - adds the calculated execute and rollback actions for the package.
//
extern "C" HRESULT MspEnginePlanAddPackage(
    __in_opt DWORD* /*pdwInsertSequence*/,
    __in BOOTSTRAPPER_DISPLAY display,
    __in BURN_PACKAGE* pPackage,
    __in BURN_PLAN* pPlan,
    __in BURN_LOGGING* pLog,
    __in BURN_VARIABLES* pVariables,
    __in_opt HANDLE hCacheEvent,
    __in BOOL fPlanPackageCacheRollback
    )
{
    HRESULT hr = S_OK;

    // TODO: need to handle the case where this patch adds itself to an earlier patch's list of target products. That would
    //       essentially bump this patch earlier in the plan and we need to make sure this patch is downloaded.
    // add wait for cache
    if (hCacheEvent)
    {
        hr = PlanExecuteCacheSyncAndRollback(pPlan, pPackage, hCacheEvent, fPlanPackageCacheRollback);
        ExitOnFailure(hr, "Failed to plan package cache syncpoint");
    }

    for (DWORD i = 0; i < pPackage->Msp.cTargetProductCodes; ++i)
    {
        BURN_MSPTARGETPRODUCT* pTargetProduct = pPackage->Msp.rgTargetProducts + i;

        if (BOOTSTRAPPER_ACTION_STATE_NONE != pTargetProduct->execute)
        {
            hr = PlanTargetProduct(display, FALSE, pPlan, pLog, pVariables, pTargetProduct->execute, pPackage, pTargetProduct);
            ExitOnFailure(hr, "Failed to plan target product.");
        }

        if (BOOTSTRAPPER_ACTION_STATE_NONE != pTargetProduct->rollback)
        {
            hr = PlanTargetProduct(display, TRUE, pPlan, pLog, pVariables, pTargetProduct->rollback, pPackage, pTargetProduct);
            ExitOnFailure(hr, "Failed to plan rollack target product.");
        }
    }

LExit:

    return hr;
}

extern "C" HRESULT MspEngineExecutePackage(
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
    INSTALLUILEVEL uiLevel = pExecuteAction->mspTarget.pPackage->Msp.fDisplayInternalUI ? INSTALLUILEVEL_DEFAULT : static_cast<INSTALLUILEVEL>(INSTALLUILEVEL_NONE | INSTALLUILEVEL_SOURCERESONLY);
    WIU_MSI_EXECUTE_CONTEXT context = { };
    WIU_RESTART restart = WIU_RESTART_NONE;

    LPWSTR sczCachedDirectory = NULL;
    LPWSTR sczMspPath = NULL;
    LPWSTR sczPatches = NULL;
    LPWSTR sczProperties = NULL;
    LPWSTR sczObfuscatedProperties = NULL;

    // default to "verbose" logging
    DWORD dwLogMode = WIU_LOG_DEFAULT | INSTALLLOGMODE_VERBOSE;

    // get cached MSP paths
    for (DWORD i = 0; i < pExecuteAction->mspTarget.cOrderedPatches; ++i)
    {
        LPCWSTR wzAppend = NULL;
        BURN_PACKAGE* pMspPackage = pExecuteAction->mspTarget.rgOrderedPatches[i].pPackage;
        AssertSz(BURN_PACKAGE_TYPE_MSP == pMspPackage->type, "Invalid package type added to ordered patches.");

        if (BOOTSTRAPPER_ACTION_STATE_INSTALL == pExecuteAction->mspTarget.action)
        {
            hr = CacheGetCompletedPath(pMspPackage->fPerMachine, pMspPackage->sczCacheId, &sczCachedDirectory);
            ExitOnFailure1(hr, "Failed to get cached path for MSP package: %ls", pMspPackage->sczId);

            hr = PathConcat(sczCachedDirectory, pMspPackage->rgPayloads[0].pPayload->sczFilePath, &sczMspPath);
            ExitOnFailure(hr, "Failed to build MSP path.");

            wzAppend = sczMspPath;
        }
        else // uninstall
        {
            wzAppend = pMspPackage->Msp.sczPatchCode;
        }

        if (NULL != sczPatches)
        {
            hr = StrAllocConcat(&sczPatches, L";", 0);
            ExitOnFailure(hr, "Failed to semi-colon delimit patches.");
        }

        hr = StrAllocConcat(&sczPatches, wzAppend, 0);
        ExitOnFailure(hr, "Failed to append patch.");
    }

    // Wire up the external UI handler and logging.
    hr = WiuInitializeExternalUI(pfnMessageHandler, uiLevel, hwndParent, pvContext, fRollback, &context);
    ExitOnFailure(hr, "Failed to initialize external UI handler.");

    //if (BURN_LOGGING_LEVEL_DEBUG == logLevel)
    //{
    //    dwLogMode | INSTALLLOGMODE_EXTRADEBUG;
    //}

    if (pExecuteAction->mspTarget.sczLogPath && *pExecuteAction->mspTarget.sczLogPath)
    {
        hr = WiuEnableLog(dwLogMode, pExecuteAction->mspTarget.sczLogPath, 0);
        ExitOnFailure2(hr, "Failed to enable logging for package: %ls to: %ls", pExecuteAction->mspTarget.pPackage->sczId, pExecuteAction->mspTarget.sczLogPath);
    }

    // set up properties
    hr = MsiEngineConcatProperties(pExecuteAction->mspTarget.pPackage->Msp.rgProperties, pExecuteAction->mspTarget.pPackage->Msp.cProperties, pVariables, fRollback, &sczProperties, FALSE);
    ExitOnFailure(hr, "Failed to add properties to argument string.");

    hr = MsiEngineConcatProperties(pExecuteAction->mspTarget.pPackage->Msp.rgProperties, pExecuteAction->mspTarget.pPackage->Msp.cProperties, pVariables, fRollback, &sczObfuscatedProperties, TRUE);
    ExitOnFailure(hr, "Failed to add properties to obfuscated argument string.");

    LogId(REPORT_STANDARD, MSG_APPLYING_PATCH_PACKAGE, pExecuteAction->mspTarget.pPackage->sczId, LoggingActionStateToString(pExecuteAction->mspTarget.action), sczMspPath, sczObfuscatedProperties, pExecuteAction->mspTarget.sczTargetProductCode);

    //
    // Do the actual action.
    //
    switch (pExecuteAction->mspTarget.action)
    {
    case BOOTSTRAPPER_ACTION_STATE_INSTALL:
        hr = StrAllocConcat(&sczProperties, L" PATCH=\"", 0);
        ExitOnFailure(hr, "Failed to add PATCH property on install.");

        hr = StrAllocConcat(&sczProperties, sczPatches, 0);
        ExitOnFailure(hr, "Failed to add patches to PATCH property on install.");

        hr = StrAllocConcat(&sczProperties, L"\" REBOOT=ReallySuppress", 0);
        ExitOnFailure(hr, "Failed to add reboot suppression property on install.");

        hr = WiuConfigureProductEx(pExecuteAction->mspTarget.sczTargetProductCode, INSTALLLEVEL_DEFAULT, INSTALLSTATE_DEFAULT, sczProperties, &restart);
        ExitOnFailure(hr, "Failed to install MSP package.");

        hr = DependencyRegisterPackage(pExecuteAction->mspTarget.pPackage);
        ExitOnFailure(hr, "Failed to register the package dependency providers.");
        break;

    case BOOTSTRAPPER_ACTION_STATE_UNINSTALL:
        hr = StrAllocConcat(&sczProperties, L" REBOOT=ReallySuppress", 0);
        ExitOnFailure(hr, "Failed to add reboot suppression property on uninstall.");

        // Ignore all dependencies, since the Burn engine already performed the check.
        hr = StrAllocFormatted(&sczProperties, L"%ls %ls=ALL", sczProperties, DEPENDENCY_IGNOREDEPENDENCIES);
        ExitOnFailure(hr, "Failed to add the list of dependencies to ignore to the properties.");

        hr = WiuRemovePatches(sczPatches, pExecuteAction->mspTarget.sczTargetProductCode, sczProperties, &restart);
        ExitOnFailure(hr, "Failed to uninstall MSP package.");

        hr = DependencyUnregisterPackage(pExecuteAction->mspTarget.pPackage);
        ExitOnFailure(hr, "Failed to unregister the package dependency providers.");
        break;
    }

LExit:
    WiuUninitializeExternalUI(&context);

    ReleaseStr(sczCachedDirectory);
    ReleaseStr(sczMspPath);
    ReleaseStr(sczProperties);
    ReleaseStr(sczObfuscatedProperties);
    ReleaseStr(sczPatches);

    switch (restart)
    {
        case WIU_RESTART_NONE:
            *pRestart = BOOTSTRAPPER_APPLY_RESTART_NONE;
            break;

        case WIU_RESTART_REQUIRED:
            *pRestart = BOOTSTRAPPER_APPLY_RESTART_REQUIRED;
            break;

        case WIU_RESTART_INITIATED:
            *pRestart = BOOTSTRAPPER_APPLY_RESTART_INITIATED;
            break;
    }

    return hr;
}

extern "C" void MspEngineSlipstreamUpdateState(
    __in BURN_PACKAGE* pPackage,
    __in BOOTSTRAPPER_ACTION_STATE execute,
    __in BOOTSTRAPPER_ACTION_STATE rollback
    )
{
    Assert(BURN_PACKAGE_TYPE_MSP == pPackage->type);

    // If the dependency manager set our state then that means something else
    // is dependent on our package. That trumps whatever the slipstream update
    // state might set.
    if (!pPackage->fDependencyManagerWasHere)
    {
        // The highest aggregate action state found will be returned.
        if (pPackage->execute < execute)
        {
            pPackage->execute = execute;
        }

        if (pPackage->rollback < rollback)
        {
            pPackage->rollback = rollback;
        }
    }
}


// internal helper functions

static HRESULT AddDetectedTargetProduct(
    __in BURN_PACKAGES* pPackages,
    __in BURN_PACKAGE* pPackage,
    __in MSIINSTALLCONTEXT context,
    __in DWORD dwOrder,
    __in_z LPCWSTR wzProductCode
    )
{
    HRESULT hr = S_OK;

    hr = MemEnsureArraySize(reinterpret_cast<LPVOID*>(&pPackage->Msp.rgTargetProducts), pPackage->Msp.cTargetProductCodes + 1, sizeof(BURN_MSPTARGETPRODUCT), 5);
    ExitOnFailure(hr, "Failed to ensure enough target product codes were allocated.");

    hr = ::StringCchCopyW(pPackage->Msp.rgTargetProducts[pPackage->Msp.cTargetProductCodes].wzTargetProductCode, countof(pPackage->Msp.rgTargetProducts[pPackage->Msp.cTargetProductCodes].wzTargetProductCode), wzProductCode);
    ExitOnFailure(hr, "Failed to copy target product code.");

    DeterminePatchChainedTarget(pPackages, pPackage, wzProductCode, 
                                &pPackage->Msp.rgTargetProducts[pPackage->Msp.cTargetProductCodes].pChainedTargetPackage,
                                &pPackage->Msp.rgTargetProducts[pPackage->Msp.cTargetProductCodes].fSlipstream);

    pPackage->Msp.rgTargetProducts[pPackage->Msp.cTargetProductCodes].context = context;
    pPackage->Msp.rgTargetProducts[pPackage->Msp.cTargetProductCodes].dwOrder = dwOrder;
    ++pPackage->Msp.cTargetProductCodes;

LExit:
    return hr;
}

static void DeterminePatchChainedTarget(
    __in BURN_PACKAGES* pPackages,
    __in BURN_PACKAGE* pMspPackage,
    __in LPCWSTR wzTargetProductCode,
    __out BURN_PACKAGE** ppChainedTargetPackage,
    __out BOOL* pfSlipstreamed
    )
{
    BURN_PACKAGE* pTargetMsiPackage = NULL;
    BOOL fSlipstreamed = FALSE;

    for (DWORD iPackage = 0; iPackage < pPackages->cPackages; ++iPackage)
    {
        BURN_PACKAGE* pPackage = pPackages->rgPackages + iPackage;

        if (BURN_PACKAGE_TYPE_MSI == pPackage->type && CSTR_EQUAL == ::CompareStringW(LOCALE_NEUTRAL, 0, wzTargetProductCode, -1, pPackage->Msi.sczProductCode, -1))
        {
            pTargetMsiPackage = pPackage;

            for (DWORD j = 0; j < pPackage->Msi.cSlipstreamMspPackages; ++j)
            {
                BURN_PACKAGE* pSlipstreamMsp = pPackage->Msi.rgpSlipstreamMspPackages[j];
                if (pSlipstreamMsp == pMspPackage)
                {
                    AssertSz(!fSlipstreamed, "An MSP should only show up as a slipstreamed patch in an MSI once.");
                    fSlipstreamed = TRUE;
                    break;
                }
            }

            break;
        }
    }

    *ppChainedTargetPackage = pTargetMsiPackage;
    *pfSlipstreamed = fSlipstreamed;

    return;
}

static HRESULT PlanTargetProduct(
    __in BOOTSTRAPPER_DISPLAY display,
    __in BOOL fRollback,
    __in BURN_PLAN* pPlan,
    __in BURN_LOGGING* pLog,
    __in BURN_VARIABLES* pVariables,
    __in BOOTSTRAPPER_ACTION_STATE actionState,
    __in BURN_PACKAGE* pPackage,
    __in BURN_MSPTARGETPRODUCT* pTargetProduct
    )
{
    HRESULT hr = S_OK;
    BURN_EXECUTE_ACTION* rgActions = fRollback ? pPlan->rgRollbackActions : pPlan->rgExecuteActions;
    DWORD cActions = fRollback ? pPlan->cRollbackActions : pPlan->cExecuteActions;
    BURN_EXECUTE_ACTION* pAction = NULL;

    // Try to find another MSP action with the exact same action (install or uninstall) targeting
    // the same product in the same machine context (per-user or per-machine).
    for (DWORD i = 0; i < cActions; ++i)
    {
        pAction = rgActions + i;

        if (BURN_EXECUTE_ACTION_TYPE_MSP_TARGET == pAction->type &&
            pAction->mspTarget.action == actionState &&
            pAction->mspTarget.fPerMachineTarget == (MSIINSTALLCONTEXT_MACHINE == pTargetProduct->context) &&
            CSTR_EQUAL == ::CompareStringW(LOCALE_NEUTRAL, 0, pAction->mspTarget.sczTargetProductCode, -1, pTargetProduct->wzTargetProductCode, -1))
        {
            break;
        }

        pAction = NULL;
    }

    // If we didn't find an MSP target action already updating the product, create a new action.
    if (!pAction)
    {
        if (fRollback)
        {
            hr = PlanAppendRollbackAction(pPlan, &pAction);
        }
        else
        {
            hr = PlanAppendExecuteAction(pPlan, &pAction);
        }
        ExitOnFailure(hr, "Failed to plan action for target product.");

        pAction->type = BURN_EXECUTE_ACTION_TYPE_MSP_TARGET;
        pAction->mspTarget.action = actionState;
        pAction->mspTarget.pPackage = pPackage;
        pAction->mspTarget.fPerMachineTarget = (MSIINSTALLCONTEXT_MACHINE == pTargetProduct->context);
        pAction->mspTarget.uiLevel = MsiEngineCalculateInstallLevel(pPackage->Msp.fDisplayInternalUI, display);
        pAction->mspTarget.pChainedTargetPackage = pTargetProduct->pChainedTargetPackage;
        pAction->mspTarget.fSlipstream = pTargetProduct->fSlipstream;
        hr = StrAllocString(&pAction->mspTarget.sczTargetProductCode, pTargetProduct->wzTargetProductCode, 0);
        ExitOnFailure(hr, "Failed to copy target product code.");

        // If this is a per-machine target product, then the plan needs to be per-machine as well.
        if (pAction->mspTarget.fPerMachineTarget)
        {
            pPlan->fPerMachine = TRUE;
        }

        LoggingSetPackageVariable(pPackage, pAction->mspTarget.sczTargetProductCode, fRollback, pLog, pVariables, &pAction->mspTarget.sczLogPath); // ignore errors.
    }

    // Add our target product to the array and sort based on their order determined during detection.
    hr = MemEnsureArraySize(reinterpret_cast<LPVOID*>(&pAction->mspTarget.rgOrderedPatches), pAction->mspTarget.cOrderedPatches + 1, sizeof(BURN_ORDERED_PATCHES), 2);
    ExitOnFailure(hr, "Failed grow array of ordered patches.");

    pAction->mspTarget.rgOrderedPatches[pAction->mspTarget.cOrderedPatches].dwOrder = pTargetProduct->dwOrder;
    pAction->mspTarget.rgOrderedPatches[pAction->mspTarget.cOrderedPatches].pPackage = pPackage;
    ++pAction->mspTarget.cOrderedPatches;

    // Insertion sort to keep the patches ordered.
    for (DWORD i = pAction->mspTarget.cOrderedPatches - 1; i > 0; --i)
    {
        if (pAction->mspTarget.rgOrderedPatches[i].dwOrder < pAction->mspTarget.rgOrderedPatches[i - 1].dwOrder)
        {
            BURN_ORDERED_PATCHES temp = pAction->mspTarget.rgOrderedPatches[i - 1];
            pAction->mspTarget.rgOrderedPatches[i - 1] = pAction->mspTarget.rgOrderedPatches[i];
            pAction->mspTarget.rgOrderedPatches[i] = temp;
        }
        else // no swap necessary, we're done.
        {
            break;
        }
    }

LExit:
    return hr;
}
