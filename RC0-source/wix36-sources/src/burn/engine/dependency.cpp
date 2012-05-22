//-------------------------------------------------------------------------------------------------
// <copyright file="dependency.cpp" company="Microsoft">
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
//    Dependency functions for Burn.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

// constants

#define INITIAL_STRINGDICT_SIZE 48
const LPCWSTR vcszIgnoreDependenciesDelim = L";";


// internal function declarations

static HRESULT DependencySplitIgnoreDependencies(
    __in_z LPCWSTR wzIgnoreDependencies,
    __deref_inout_ecount_opt(*pcDependencies) DEPENDENCY** prgDependencies,
    __inout LPUINT pcDependencies
    );

static HRESULT DependencyJoinIgnoreDependencies(
    __out_z LPWSTR* psczIgnoreDependencies,
    __in_ecount(cDependencies) const DEPENDENCY* rgDependencies,
    __in UINT cDependencies
    );

static HRESULT DependencyGetIgnoredDependents(
    __in const BURN_PACKAGE* pPackage,
    __in const BURN_PLAN* pPlan,
    __in_z LPCWSTR wzBundleProviderKey,
    __deref_inout STRINGDICT_HANDLE* psdIgnoredDependents
    );

static void DependencyCalculatePlan(
    __in const BURN_PACKAGE* pPackage,
    __in const BOOTSTRAPPER_ACTION action,
    __out BURN_DEPENDENCY_ACTION* pDependencyExecuteAction,
    __out BURN_DEPENDENCY_ACTION* pDependencyRollbackAction
    );

static HRESULT DependencyPlanActions(
    __in_opt DWORD *pdwInsertSequence,
    __in const BURN_PACKAGE* pPackage,
    __in BURN_PLAN* pPlan,
    __in_z LPCWSTR wzBundleProviderKey,
    __in const BURN_DEPENDENCY_ACTION dependencyExecuteAction,
    __in const BURN_DEPENDENCY_ACTION dependencyRollbackAction
    );

static HRESULT DependencyRegisterPackageDependency(
    __in_z LPCWSTR wzDependentProviderKey,
    __in BOOL fPerMachine,
    __in const BURN_PACKAGE* pPackage
    );

static HRESULT DependencyUnregisterPackageDependency(
    __in_z LPCWSTR wzDependentProviderKey,
    __in BOOL fPerMachine,
    __in const BURN_PACKAGE* pPackage
    );


// functions

extern "C" HRESULT DependencyParseProvidersFromXml(
    __in BURN_PACKAGE* pPackage,
    __in IXMLDOMNode* pixnPackage
    )
{
    HRESULT hr = S_OK;
    IXMLDOMNodeList* pixnNodes = NULL;
    DWORD cNodes = 0;
    IXMLDOMNode* pixnNode = NULL;

    // Select dependency provider nodes.
    hr = XmlSelectNodes(pixnPackage, L"Provides", &pixnNodes);
    ExitOnFailure(hr, "Failed to select dependency provider nodes.");

    // Get dependency provider node count.
    hr = pixnNodes->get_length((long*)&cNodes);
    ExitOnFailure(hr, "Failed to get the dependency provider node count.");

    if (!cNodes)
    {
        ExitFunction1(hr = S_OK);
    }

    // Allocate memory for dependency provider pointers.
    pPackage->rgDependencyProviders = (BURN_DEPENDENCY_PROVIDER*)MemAlloc(sizeof(BURN_DEPENDENCY_PROVIDER) * cNodes, TRUE);
    ExitOnNull(pPackage->rgDependencyProviders, hr, E_OUTOFMEMORY, "Failed to allocate memory for dependency providers.");

    pPackage->cDependencyProviders = cNodes;

    // Parse dependency provider elements.
    for (DWORD i = 0; i < cNodes; i++)
    {
        BURN_DEPENDENCY_PROVIDER* pDependencyProvider = &pPackage->rgDependencyProviders[i];

        hr = XmlNextElement(pixnNodes, &pixnNode, NULL);
        ExitOnFailure(hr, "Failed to get the next dependency provider node.");

        // @Key
        hr = XmlGetAttributeEx(pixnNode, L"Key", &pDependencyProvider->sczKey);
        ExitOnFailure(hr, "Failed to get the Key attribute.");

        // @Version
        hr = XmlGetAttributeEx(pixnNode, L"Version", &pDependencyProvider->sczVersion);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to get the Version attribute.");
        }

        // @DisplayName
        hr = XmlGetAttributeEx(pixnNode, L"DisplayName", &pDependencyProvider->sczDisplayName);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to get the DisplayName attribute.");
        }

        // @Imported
        hr = XmlGetYesNoAttribute(pixnNode, L"Imported", &pDependencyProvider->fImported);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to get the Imported attribute.");
        }
        else
        {
            pDependencyProvider->fImported = FALSE;
            hr = S_OK;
        }

        // Prepare next iteration.
        ReleaseNullObject(pixnNode);
    }

    hr = S_OK;

LExit:
    ReleaseObject(pixnNode);
    ReleaseObject(pixnNodes);

    return hr;
}

extern "C" HRESULT DependencyPlanInitialize(
    __in const BURN_ENGINE_STATE* pEngineState,
    __in BURN_PLAN* pPlan
    )
{
    HRESULT hr = S_OK;

    // The current bundle provider key should always be ignored for dependency checks.
    hr = DepDependencyArrayAlloc(&pPlan->rgPlannedProviders, &pPlan->cPlannedProviders, pEngineState->registration.sczProviderKey, NULL);
    ExitOnFailure(hr, "Failed to add the bundle provider key to the list of dependencies to ignore.");

    // Add the list of dependencies to ignore to the plan.
    if (pEngineState->sczIgnoreDependencies)
    {
        // TODO: After adding enumeration to STRINGDICT, a single STRINGDICT_HANDLE can be used everywhere.
        hr = DependencySplitIgnoreDependencies(pEngineState->sczIgnoreDependencies, &pPlan->rgPlannedProviders, &pPlan->cPlannedProviders);
        ExitOnFailure(hr, "Failed to split the list of dependencies to ignore.");
    }

LExit:
    return hr;
}

extern "C" HRESULT DependencyAllocIgnoreDependencies(
    __in const BURN_PLAN *pPlan,
    __out_z LPWSTR* psczIgnoreDependencies
    )
{
    HRESULT hr = S_OK;

    // Join the list of dependencies to ignore for each related bundle.
    if (0 < pPlan->cPlannedProviders)
    {
        hr = DependencyJoinIgnoreDependencies(psczIgnoreDependencies, pPlan->rgPlannedProviders, pPlan->cPlannedProviders);
        ExitOnFailure(hr, "Failed to join the list of dependencies to ignore.");
    }

LExit:
    return hr;
}

extern "C" HRESULT DependencyPlanPackageBegin(
    __in_opt DWORD *pdwInsertSequence,
    __in BURN_PACKAGE* pPackage,
    __in BURN_PLAN* pPlan,
    __in_z LPCWSTR wzBundleProviderKey
    )
{
    HRESULT hr = S_OK;
    STRINGDICT_HANDLE sdIgnoredDependents = NULL;
    DEPENDENCY* rgDependents = NULL;
    UINT cDependents = 0;
    HKEY hkHive = pPackage->fPerMachine ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
    BURN_DEPENDENCY_ACTION dependencyExecuteAction = BURN_DEPENDENCY_ACTION_NONE;
    BURN_DEPENDENCY_ACTION dependencyRollbackAction = BURN_DEPENDENCY_ACTION_NONE;
    pPackage->dependency = BURN_DEPENDENCY_ACTION_NONE;

    // Make sure the package defines at least one provider.
    if (0 == pPackage->cDependencyProviders)
    {
        LogId(REPORT_VERBOSE, MSG_DEPENDENCY_PACKAGE_SKIP_NOPROVIDERS, pPackage->sczId);
        ExitFunction();
    }

    // If we're uninstalling the package, check if any dependents are registered.
    if (BOOTSTRAPPER_ACTION_STATE_UNINSTALL == pPackage->execute)
    {
        // Build up a list of dependents to ignore, including the current bundle.
        hr = DependencyGetIgnoredDependents(pPackage, pPlan, wzBundleProviderKey, &sdIgnoredDependents);
        ExitOnFailure(hr, "Failed to build the list of ignored dependents.");

        // Skip the dependency check if "ALL" was authored for IGNOREDEPENDENCIES.
        hr = DictKeyExists(sdIgnoredDependents, L"ALL");
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to check if \"ALL\" was set in IGNOREDEPENDENCIES.");
        }
        else
        {
            for (DWORD i = 0; i < pPackage->cDependencyProviders; ++i)
            {
                const BURN_DEPENDENCY_PROVIDER* pProvider = &pPackage->rgDependencyProviders[i];

                hr = DepCheckDependents(hkHive, pProvider->sczKey, 0, sdIgnoredDependents, &rgDependents, &cDependents);
                if (E_FILENOTFOUND != hr || pPackage->fVital)
                {
                    ExitOnFailure1(hr, "Failed dependents check on package provider: %ls", pProvider->sczKey);
                }
                else
                {
                    hr = S_OK;
                }
            }
        }
    }

    // Calculate the dependency actions before the package itself is planned.
    DependencyCalculatePlan(pPackage, pPlan->action, &dependencyExecuteAction, &dependencyRollbackAction);

    // If dependents were found, change the action to not uninstall the package.
    if (0 < cDependents)
    {
        LogId(REPORT_STANDARD, MSG_DEPENDENCY_PACKAGE_HASDEPENDENTS, pPackage->sczId, cDependents);

        for (DWORD i = 0; i < cDependents; ++i)
        {
            const DEPENDENCY* pDependency = &rgDependents[i];
            LogId(REPORT_VERBOSE, MSG_DEPENDENCY_PACKAGE_DEPENDENT, pDependency->sczKey, LoggingStringOrUnknownIfNull(pDependency->sczName));
        }

        pPackage->fDependencyManagerWasHere = TRUE;
        pPackage->execute = BOOTSTRAPPER_ACTION_STATE_NONE;
        pPackage->rollback = BOOTSTRAPPER_ACTION_STATE_NONE;
    }

    // If the package will be removed, add its providers to the growing list in the plan.
    if (BOOTSTRAPPER_ACTION_STATE_UNINSTALL == pPackage->execute)
    {
        for (DWORD i = 0; i < pPackage->cDependencyProviders; ++i)
        {
            const BURN_DEPENDENCY_PROVIDER* pProvider = &pPackage->rgDependencyProviders[i];

            hr = DepDependencyArrayAlloc(&pPlan->rgPlannedProviders, &pPlan->cPlannedProviders, pProvider->sczKey, NULL);
            ExitOnFailure1(hr, "Failed to add the package provider key \"%ls\" to the planned list.", pProvider->sczKey);
        }
    }

    // If the dependency execution action is to unregister, add the dependency actions to the plan.
    if (BURN_DEPENDENCY_ACTION_UNREGISTER == dependencyExecuteAction)
    {
        hr = DependencyPlanActions(pdwInsertSequence, pPackage, pPlan, wzBundleProviderKey, dependencyExecuteAction, dependencyRollbackAction);
        ExitOnFailure1(hr, "Failed to plan the dependency actions for package: %ls", pPackage->sczId);

        // Pass the action back to skip post-package planning.
        pPackage->dependency = dependencyExecuteAction;
    }

LExit:
    ReleaseDependencyArray(rgDependents, cDependents);
    ReleaseDict(sdIgnoredDependents);

    return hr;
}

extern "C" HRESULT DependencyPlanPackageComplete(
    __in BURN_PACKAGE* pPackage,
    __in BURN_PLAN* pPlan,
    __in_z LPCWSTR wzBundleProviderKey
    )
{
    HRESULT hr = S_OK;
    BURN_DEPENDENCY_ACTION dependencyExecuteAction = BURN_DEPENDENCY_ACTION_NONE;
    BURN_DEPENDENCY_ACTION dependencyRollbackAction = BURN_DEPENDENCY_ACTION_NONE;

    // If the dependency action is already planned, there's nothing to do.
    if (BURN_DEPENDENCY_ACTION_NONE != pPackage->dependency)
    {
        ExitFunction();
    }

    // Make sure the package defines at least one provider.
    if (0 == pPackage->cDependencyProviders)
    {
        // Already logged the dependency plan will be skipped.
        ExitFunction();
    }

    DependencyCalculatePlan(pPackage, pPlan->action, &dependencyExecuteAction, &dependencyRollbackAction);

    // If the dependency execution action is to register, add the dependency actions to the plan.
    if (BURN_DEPENDENCY_ACTION_REGISTER == dependencyExecuteAction)
    {
        hr = DependencyPlanActions(NULL, pPackage, pPlan, wzBundleProviderKey, dependencyExecuteAction, dependencyRollbackAction);
        ExitOnFailure1(hr, "Failed to plan the dependency actions for package: %ls", pPackage->sczId);

        pPackage->dependency = dependencyExecuteAction;
    }

LExit:
    return hr;
}

extern "C" HRESULT DependencyExecuteAction(
    __in const BURN_EXECUTE_ACTION* pAction,
    __in BOOL fPerMachine
    )
{
    AssertSz(BURN_EXECUTE_ACTION_TYPE_DEPENDENCY == pAction->type, "Execute action type not supported by this function.");

    HRESULT hr = S_OK;
    const BURN_PACKAGE* pPackage = pAction->dependency.pPackage;

    // Register or unregister the bundle as a dependent of each package dependency provider.
    if (BURN_DEPENDENCY_ACTION_REGISTER == pAction->dependency.action)
    {
        hr = DependencyRegisterPackageDependency(pAction->dependency.sczBundleProviderKey, fPerMachine, pPackage);
        ExitOnFailure(hr, "Failed to register the dependency on the package provider.");
    }
    else if (BURN_DEPENDENCY_ACTION_UNREGISTER == pAction->dependency.action)
    {
        hr = DependencyUnregisterPackageDependency(pAction->dependency.sczBundleProviderKey, fPerMachine, pPackage);
        ExitOnFailure(hr, "Failed to unregister the dependency from the package provider.");
    }

LExit:
    if (!pPackage->fVital)
    {
        hr = S_OK;
    }

    return hr;
}

extern "C" HRESULT DependencyRegisterBundle(
    __in const BURN_REGISTRATION* pRegistration
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczVersion = NULL;

    hr = FileVersionToStringEx(pRegistration->qwVersion, &sczVersion);
    ExitOnFailure(hr, "Failed to format the registration version string.");

    LogId(REPORT_VERBOSE, MSG_DEPENDENCY_BUNDLE_REGISTER, pRegistration->sczProviderKey, sczVersion);

    // Register the bundle provider key.
    hr = DepRegisterDependency(pRegistration->hkRoot, pRegistration->sczProviderKey, sczVersion, pRegistration->sczDisplayName, 0);
    ExitOnFailure(hr, "Failed to register the bundle dependency provider.");

    // Register each dependent related bundle.
    for (DWORD i = 0; i < pRegistration->relatedBundles.cRelatedBundles; ++i)
    {
        const BURN_RELATED_BUNDLE* pRelatedBundle = pRegistration->relatedBundles.rgRelatedBundles + i;

        if (BOOTSTRAPPER_RELATION_DEPENDENT == pRelatedBundle->relationType)
        {
            for (DWORD j = 0; j < pRelatedBundle->package.cDependencyProviders; ++j)
            {
                const BURN_DEPENDENCY_PROVIDER* pProvider = pRelatedBundle->package.rgDependencyProviders + j;

                LogId(REPORT_VERBOSE, MSG_DEPENDENCY_PACKAGE_REGISTER_DEPENDENCY, pRegistration->sczProviderKey, pProvider->sczKey, pRelatedBundle->package.sczId);

                // Register all dependent related bundles as non-vital.
                hr = DepRegisterDependent(pRegistration->hkRoot, pRegistration->sczProviderKey, pProvider->sczKey, NULL, NULL, 0);
                if (E_FILENOTFOUND != hr)
                {
                    ExitOnFailure1(hr, "Failed to register the dependency on package dependency provider: %ls", pProvider->sczKey);
                }
            }
        }
    }

LExit:
    ReleaseStr(sczVersion);

    return hr;
}

extern "C" HRESULT DependencyRegisterPackage(
    __in const BURN_PACKAGE* pPackage
    )
{
    HRESULT hr = S_OK;
    HKEY hkRoot = pPackage->fPerMachine ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;

    if (pPackage->rgDependencyProviders)
    {
        for (DWORD i = 0; i < pPackage->cDependencyProviders; ++i)
        {
            const BURN_DEPENDENCY_PROVIDER* pProvider = &pPackage->rgDependencyProviders[i];

            if (!pProvider->fImported)
            {
                LogId(REPORT_VERBOSE, MSG_DEPENDENCY_PACKAGE_REGISTER, pProvider->sczKey, pProvider->sczVersion, pPackage->sczId);

                hr = DepRegisterDependency(hkRoot, pProvider->sczKey, pProvider->sczVersion, pProvider->sczDisplayName, 0);
                ExitOnFailure1(hr, "Failed to register the package dependency provider: %ls", pProvider->sczKey);
            }
        }
    }

LExit:
    if (!pPackage->fVital)
    {
        hr = S_OK;
    }

    return hr;
}

extern "C" HRESULT DependencyUnregisterBundle(
    __in const BURN_REGISTRATION* pRegistration
    )
{
    HRESULT hr = S_OK;

    LogId(REPORT_VERBOSE, MSG_DEPENDENCY_BUNDLE_UNREGISTER, pRegistration->sczProviderKey);

    // Remove the bundle provider key.
    hr = DepUnregisterDependency(pRegistration->hkRoot, pRegistration->sczProviderKey);
    if (E_FILENOTFOUND != hr)
    {
        ExitOnFailure(hr, "Failed to remove the bundle dependency provider.");
    }
    else
    {
        hr = S_OK;
    }

LExit:
    return hr;
}

extern "C" HRESULT DependencyUnregisterPackage(
    __in const BURN_PACKAGE* pPackage
    )
{
    HRESULT hr = S_OK;
    HKEY hkRoot = pPackage->fPerMachine ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;

    if (pPackage->rgDependencyProviders)
    {
        for (DWORD i = 0; i < pPackage->cDependencyProviders; ++i)
        {
            const BURN_DEPENDENCY_PROVIDER* pProvider = &pPackage->rgDependencyProviders[i];

            if (!pProvider->fImported)
            {
                LogId(REPORT_VERBOSE, MSG_DEPENDENCY_PACKAGE_UNREGISTER, pProvider->sczKey, pPackage->sczId);

                hr = DepUnregisterDependency(hkRoot, pProvider->sczKey);
                if (E_FILENOTFOUND != hr)
                {
                    ExitOnFailure1(hr, "Failed to remove the package dependency provider: %ls", pProvider->sczKey);
                }
                else
                {
                    hr = S_OK;
                }
            }
        }
    }

LExit:
    if (!pPackage->fVital)
    {
        hr = S_OK;
    }

    return hr;
}


// internal functions

/********************************************************************
 DependencyParseIgnoreDependencies - Splits a semicolon-delimited
  string into a list of unique dependencies to ignore.

*********************************************************************/
static HRESULT DependencySplitIgnoreDependencies(
    __in_z LPCWSTR wzIgnoreDependencies,
    __deref_inout_ecount_opt(*pcDependencies) DEPENDENCY** prgDependencies,
    __inout LPUINT pcDependencies
    )
{
    HRESULT hr = S_OK;
    LPWSTR wzContext = NULL;
    STRINGDICT_HANDLE sdIgnoreDependencies = NULL;

    // Create a dictionary to hold unique dependencies.
    hr = DictCreateStringList(&sdIgnoreDependencies, INITIAL_STRINGDICT_SIZE, DICT_FLAG_CASEINSENSITIVE);
    ExitOnFailure(hr, "Failed to create the string dictionary.");

    // Parse through the semicolon-delimited tokens and add to the array.
    for (LPCWSTR wzToken = ::wcstok_s(const_cast<LPWSTR>(wzIgnoreDependencies), vcszIgnoreDependenciesDelim, &wzContext); wzToken; wzToken = ::wcstok_s(NULL, vcszIgnoreDependenciesDelim, &wzContext))
    {
        hr = DictKeyExists(sdIgnoreDependencies, wzToken);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to check the dictionary of unique dependencies.");
        }
        else
        {
            hr = DepDependencyArrayAlloc(prgDependencies, pcDependencies, wzToken, NULL);
            ExitOnFailure1(hr, "Failed to add \"%ls\" to the list of dependencies to ignore.", wzToken);

            hr = DictAddKey(sdIgnoreDependencies, wzToken);
            ExitOnFailure1(hr, "Failed to add \"%ls\" to the string dictionary.", wzToken);
        }
    }

LExit:
    ReleaseDict(sdIgnoreDependencies);

    return hr;
}

/********************************************************************
 DependencyJoinIgnoreDependencies - Joins a list of dependencies
  to ignore into a semicolon-delimited string of unique values.

*********************************************************************/
static HRESULT DependencyJoinIgnoreDependencies(
    __out_z LPWSTR* psczIgnoreDependencies,
    __in_ecount(cDependencies) const DEPENDENCY* rgDependencies,
    __in UINT cDependencies
    )
{
    HRESULT hr = S_OK;
    STRINGDICT_HANDLE sdIgnoreDependencies = NULL;

    // Make sure we pass back an empty string if there are no dependencies.
    if (0 == cDependencies)
    {
        ExitFunction();
    }

    // Create a dictionary to hold unique dependencies.
    hr = DictCreateStringList(&sdIgnoreDependencies, INITIAL_STRINGDICT_SIZE, DICT_FLAG_CASEINSENSITIVE);
    ExitOnFailure(hr, "Failed to create the string dictionary.");

    for (UINT i = 0; i < cDependencies; ++i)
    {
        const DEPENDENCY* pDependency = &rgDependencies[i];

        hr = DictKeyExists(sdIgnoreDependencies, pDependency->sczKey);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to check the dictionary of unique dependencies.");
        }
        else
        {
            if (0 < i)
            {
                hr = StrAllocConcat(psczIgnoreDependencies, vcszIgnoreDependenciesDelim, 1);
                ExitOnFailure(hr, "Failed to append the string delimiter.");
            }

            hr = StrAllocConcat(psczIgnoreDependencies, pDependency->sczKey, 0);
            ExitOnFailure1(hr, "Failed to append the key \"%ls\".", pDependency->sczKey);

            hr = DictAddKey(sdIgnoreDependencies, pDependency->sczKey);
            ExitOnFailure1(hr, "Failed to add \"%ls\" to the string dictionary.", pDependency->sczKey);
        }
    }

LExit:
    ReleaseDict(sdIgnoreDependencies);

    return hr;
}

/********************************************************************
 DependencyGetIgnoredDependents - Combines the current bundle's
  provider key, packages' provider keys that are being uninstalled,
  and any ignored dependencies authored for packages into a string
  list to pass to deputil.

*********************************************************************/
static HRESULT DependencyGetIgnoredDependents(
    __in const BURN_PACKAGE* pPackage,
    __in const BURN_PLAN* pPlan,
    __in LPCWSTR wzBundleProviderKey,
    __deref_inout STRINGDICT_HANDLE* psdIgnoredDependents
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczIgnoreDependencies = NULL;
    LPWSTR wzContext = NULL;

    // Create the dictionary and add the bundle provider key initially.
    hr = DictCreateStringList(psdIgnoredDependents, INITIAL_STRINGDICT_SIZE, DICT_FLAG_CASEINSENSITIVE);
    ExitOnFailure(hr, "Failed to create the string dictionary.");

    hr = DictAddKey(*psdIgnoredDependents, wzBundleProviderKey);
    ExitOnFailure1(hr, "Failed to add the bundle provider key \"%ls\" to the list of ignored dependencies.", wzBundleProviderKey);

    // Add previously planned package providers to the dictionary.
    for (DWORD i = 0; i < pPlan->cPlannedProviders; ++i)
    {
        const DEPENDENCY* pDependency = &pPlan->rgPlannedProviders[i];

        hr = DictAddKey(*psdIgnoredDependents, pDependency->sczKey);
        ExitOnFailure1(hr, "Failed to add the package provider key \"%ls\" to the list of ignored dependencies.", pDependency->sczKey);
    }

    // Get the IGNOREDEPENDENCIES property if defined.
    hr = PackageGetProperty(pPackage, DEPENDENCY_IGNOREDEPENDENCIES, &sczIgnoreDependencies);
    if (E_NOTFOUND != hr)
    {
        ExitOnFailure1(hr, "Failed to get the package property: %ls", DEPENDENCY_IGNOREDEPENDENCIES);

        // Parse through the semicolon-delimited tokens and add to the string dictionary.
        for (LPCWSTR wzToken = ::wcstok_s(sczIgnoreDependencies, vcszIgnoreDependenciesDelim, &wzContext); wzToken; wzToken = ::wcstok_s(NULL, vcszIgnoreDependenciesDelim, &wzContext))
        {
            hr = DictAddKey(*psdIgnoredDependents, wzToken);
            ExitOnFailure1(hr, "Failed to add the provider key \"%ls\" to the list of ignored dependencies.", wzToken);
        }
    }
    else
    {
        hr = S_OK;
    }

LExit:
    ReleaseStr(sczIgnoreDependencies);

    return hr;
}

/********************************************************************
 DependencyCalculatePlan - Calculates the dependency execute and
  rollback actions for a package.

*********************************************************************/
static void DependencyCalculatePlan(
    __in const BURN_PACKAGE* pPackage,
    __in const BOOTSTRAPPER_ACTION action,
    __out BURN_DEPENDENCY_ACTION* pDependencyExecuteAction,
    __out BURN_DEPENDENCY_ACTION* pDependencyRollbackAction
    )
{
    switch (action)
    {
    case BOOTSTRAPPER_ACTION_UNINSTALL:
        // Always remove the dependency when uninstalling a bundle even if the package is absent.
        *pDependencyExecuteAction = BURN_DEPENDENCY_ACTION_UNREGISTER;
        break;
    case BOOTSTRAPPER_ACTION_INSTALL:
        // Always remove the dependency during rollback when installing a bundle.
        *pDependencyRollbackAction = BURN_DEPENDENCY_ACTION_UNREGISTER;
        __fallthrough;
    case BOOTSTRAPPER_ACTION_MODIFY: __fallthrough;
    case BOOTSTRAPPER_ACTION_REPAIR:
        switch (pPackage->execute)
        {
        case BOOTSTRAPPER_ACTION_STATE_NONE:
            // Register the bundle with the package if requested but already installed.
            switch (pPackage->requested)
            {
            case BOOTSTRAPPER_REQUEST_STATE_PRESENT: __fallthrough;
            case BOOTSTRAPPER_REQUEST_STATE_REPAIR:
                switch (pPackage->currentState)
                {
                case BOOTSTRAPPER_PACKAGE_STATE_PRESENT: __fallthrough;
                case BOOTSTRAPPER_PACKAGE_STATE_SUPERSEDED:
                    *pDependencyExecuteAction = BURN_DEPENDENCY_ACTION_REGISTER;
                    break;
                }
            }
            break;
        case BOOTSTRAPPER_ACTION_STATE_UNINSTALL:
            *pDependencyExecuteAction = BURN_DEPENDENCY_ACTION_UNREGISTER;
            break;
        case BOOTSTRAPPER_ACTION_STATE_INSTALL: __fallthrough;
        case BOOTSTRAPPER_ACTION_STATE_MODIFY: __fallthrough;
        case BOOTSTRAPPER_ACTION_STATE_REPAIR: __fallthrough;
        case BOOTSTRAPPER_ACTION_STATE_MINOR_UPGRADE: __fallthrough;
        case BOOTSTRAPPER_ACTION_STATE_MAJOR_UPGRADE: __fallthrough;
        case BOOTSTRAPPER_ACTION_STATE_PATCH:
            *pDependencyExecuteAction = BURN_DEPENDENCY_ACTION_REGISTER;
            break;
        }
        break;
    }

    switch (*pDependencyExecuteAction)
    {
    case BURN_DEPENDENCY_ACTION_REGISTER:
        switch (pPackage->currentState)
        {
        case BOOTSTRAPPER_PACKAGE_STATE_OBSOLETE: __fallthrough;
        case BOOTSTRAPPER_PACKAGE_STATE_ABSENT: __fallthrough;
        case BOOTSTRAPPER_PACKAGE_STATE_CACHED:
            *pDependencyRollbackAction = BURN_DEPENDENCY_ACTION_UNREGISTER;
            break;
        }
        break;
    case BURN_DEPENDENCY_ACTION_UNREGISTER:
        switch (pPackage->currentState)
        {
        case BOOTSTRAPPER_PACKAGE_STATE_PRESENT: __fallthrough;
        case BOOTSTRAPPER_PACKAGE_STATE_SUPERSEDED:
            *pDependencyRollbackAction = BURN_DEPENDENCY_ACTION_REGISTER;
            break;
        }
    }
}

/********************************************************************
 DependencyPlanActions - Adds the dependency execute and rollback
  actions to the plan.

*********************************************************************/
static HRESULT DependencyPlanActions(
    __in_opt DWORD *pdwInsertSequence,
    __in const BURN_PACKAGE* pPackage,
    __in BURN_PLAN* pPlan,
    __in_z LPCWSTR wzBundleProviderKey,
    __in const BURN_DEPENDENCY_ACTION dependencyExecuteAction,
    __in const BURN_DEPENDENCY_ACTION dependencyRollbackAction
    )
{
    HRESULT hr = S_OK;
    BURN_EXECUTE_ACTION* pAction = NULL;

    // Add the execute plan.
    if (BURN_DEPENDENCY_ACTION_NONE != dependencyExecuteAction)
    {
        if (NULL != pdwInsertSequence)
        {
            hr = PlanInsertExecuteAction(*pdwInsertSequence, pPlan, &pAction);
            ExitOnFailure(hr, "Failed to insert execute action.");
        }
        else
        {
            hr = PlanAppendExecuteAction(pPlan, &pAction);
            ExitOnFailure(hr, "Failed to append execute action.");
        }

        pAction->type = BURN_EXECUTE_ACTION_TYPE_DEPENDENCY;
        pAction->dependency.pPackage = const_cast<BURN_PACKAGE*>(pPackage);
        pAction->dependency.action = dependencyExecuteAction;

        hr = StrAllocString(&pAction->dependency.sczBundleProviderKey, wzBundleProviderKey, 0);
        ExitOnFailure(hr, "Failed to copy the bundle dependency provider.");
    }

    // Add the rollback plan.
    if (BURN_DEPENDENCY_ACTION_NONE != dependencyRollbackAction)
    {
        hr = PlanAppendRollbackAction(pPlan, &pAction);
        ExitOnFailure(hr, "Failed to append rollback action.");

        pAction->type = BURN_EXECUTE_ACTION_TYPE_DEPENDENCY;
        pAction->dependency.pPackage = const_cast<BURN_PACKAGE*>(pPackage);
        pAction->dependency.action = dependencyRollbackAction;

        hr = StrAllocString(&pAction->dependency.sczBundleProviderKey, wzBundleProviderKey, 0);
        ExitOnFailure(hr, "Failed to copy the bundle dependency provider.");
    }

LExit:
    return hr;
}

/********************************************************************
 DependencyRegisterPackageDependency - Registers the provider key
  as a dependent of a package.

*********************************************************************/
static HRESULT DependencyRegisterPackageDependency(
    __in_z LPCWSTR wzDependentProviderKey,
    __in BOOL fPerMachine,
    __in const BURN_PACKAGE* pPackage
    )
{
    HRESULT hr = S_OK;
    HKEY hkRoot = fPerMachine ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;

    // Do not register a dependency on a package in a different install context.
    if (fPerMachine != pPackage->fPerMachine)
    {
        LogId(REPORT_STANDARD, MSG_DEPENDENCY_PACKAGE_SKIP_WRONGSCOPE, pPackage->sczId, LoggingPerMachineToString(fPerMachine), LoggingPerMachineToString(pPackage->fPerMachine));
        ExitFunction1(hr = S_OK);
    }

    if (pPackage->rgDependencyProviders)
    {
        for (DWORD i = 0; i < pPackage->cDependencyProviders; ++i)
        {
            const BURN_DEPENDENCY_PROVIDER* pProvider = &pPackage->rgDependencyProviders[i];

            LogId(REPORT_VERBOSE, MSG_DEPENDENCY_PACKAGE_REGISTER_DEPENDENCY, wzDependentProviderKey, pProvider->sczKey, pPackage->sczId);

            hr = DepRegisterDependent(hkRoot, pProvider->sczKey, wzDependentProviderKey, NULL, NULL, 0);
            if (E_FILENOTFOUND != hr || pPackage->fVital)
            {
                ExitOnFailure1(hr, "Failed to register the dependency on package dependency provider: %ls", pProvider->sczKey);
            }
            else
            {
                LogId(REPORT_VERBOSE, MSG_DEPENDENCY_PACKAGE_SKIP_MISSING, pProvider->sczKey, pPackage->sczId);
                hr = S_OK;
            }
        }
    }

LExit:
    return hr;
}

/********************************************************************
 DependencyUnregisterPackageDependency - Unregisters the provider key
  as a dependent of a package.

*********************************************************************/
static HRESULT DependencyUnregisterPackageDependency(
    __in_z LPCWSTR wzDependentProviderKey,
    __in BOOL fPerMachine,
    __in const BURN_PACKAGE* pPackage
    )
{
    HRESULT hr = S_OK;
    HKEY hkRoot = fPerMachine ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;

    // Should be no registration to remove since we don't write keys across contexts.
    if (fPerMachine != pPackage->fPerMachine)
    {
        LogId(REPORT_STANDARD, MSG_DEPENDENCY_PACKAGE_SKIP_WRONGSCOPE, pPackage->sczId, LoggingPerMachineToString(fPerMachine), LoggingPerMachineToString(pPackage->fPerMachine));
        ExitFunction1(hr = S_OK);
    }

    // Loop through each package provider and remove the bundle dependency key.
    if (pPackage->rgDependencyProviders)
    {
        for (DWORD i = 0; i < pPackage->cDependencyProviders; ++i)
        {
            const BURN_DEPENDENCY_PROVIDER* pProvider = &pPackage->rgDependencyProviders[i];

            LogId(REPORT_VERBOSE, MSG_DEPENDENCY_PACKAGE_UNREGISTER_DEPENDENCY, wzDependentProviderKey, pProvider->sczKey, pPackage->sczId);

            hr = DepUnregisterDependent(hkRoot, pProvider->sczKey, wzDependentProviderKey);
            if (E_FILENOTFOUND != hr)
            {
                ExitOnFailure1(hr, "Failed to remove the dependency from package dependency provider: %ls", pProvider->sczKey);
            }
            else
            {
                hr = S_OK;
            }
        }
    }

LExit:
    return hr;
}
