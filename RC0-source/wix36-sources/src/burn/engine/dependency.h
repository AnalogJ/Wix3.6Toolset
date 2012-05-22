//-------------------------------------------------------------------------------------------------
// <copyright file="dependency.h" company="Microsoft">
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

#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

// constants

const LPCWSTR DEPENDENCY_IGNOREDEPENDENCIES = L"IGNOREDEPENDENCIES";


// function declarations

/********************************************************************
 DependencyParseProvidersFromXml - Parses dependency information
  from the manifest for the specified package.

*********************************************************************/
HRESULT DependencyParseProvidersFromXml(
    __in BURN_PACKAGE* pPackage,
    __in IXMLDOMNode* pixnPackage
    );

/********************************************************************
 DependencyPlanInitialize - Initializes the plan.

*********************************************************************/
HRESULT DependencyPlanInitialize(
    __in const BURN_ENGINE_STATE* pEngineState,
    __in BURN_PLAN* pPlan
    );

/********************************************************************
 DependencyAllocIgnoreDependencies - Allocates the dependencies to
  ignore as a semicolon-delimited string.

*********************************************************************/
HRESULT DependencyAllocIgnoreDependencies(
    __in const BURN_PLAN *pPlan,
    __out_z LPWSTR* psczIgnoreDependencies
    );

/********************************************************************
 DependencyPlanPackageBegin - Updates the dependency registration
  action depending on the calculated state for the package.

*********************************************************************/
HRESULT DependencyPlanPackageBegin(
    __in_opt DWORD *pdwInsertSequence,
    __in BURN_PACKAGE* pPackage,
    __in BURN_PLAN* pPlan,
    __in_z LPCWSTR wzBundleProviderKey
    );

/********************************************************************
 DependencyPlanPackageComplete - Updates the dependency registration
  action depending on the planned action for the package.

*********************************************************************/
HRESULT DependencyPlanPackageComplete(
    __in BURN_PACKAGE* pPackage,
    __in BURN_PLAN* pPlan,
    __in_z LPCWSTR wzBundleProviderKey
    );

/********************************************************************
 DependencyExecuteAction - Registers or unregisters dependency
  information for the package contained within the action.

*********************************************************************/
HRESULT DependencyExecuteAction(
    __in const BURN_EXECUTE_ACTION* pAction,
    __in BOOL fPerMachine
    );

/********************************************************************
 DependencyRegisterBundle - Registers the bundle dependency provider.

*********************************************************************/
HRESULT DependencyRegisterBundle(
    __in const BURN_REGISTRATION* pRegistration
    );

/********************************************************************
 DependencyRegisterPackage - Registers each dependency provider
  defined for the package (if not imported from the package itself).

 Note: Returns S_OK if the package is non-vital.
*********************************************************************/
HRESULT DependencyRegisterPackage(
    __in const BURN_PACKAGE* pPackage
    );

/********************************************************************
 DependencyUnregisterBundle - Removes the bundle dependency provider.

 Note: Does not check for existing dependents before removing the key.
*********************************************************************/
HRESULT DependencyUnregisterBundle(
    __in const BURN_REGISTRATION* pRegistration
    );

/********************************************************************
 DependencyUnregisterPackage - Removes each dependency provider
  for the package (if not imported from the package itself).

 Note: Returns S_OK if the package is non-vital.

 Note: Does not check for existing dependents before removing the key.
*********************************************************************/
HRESULT DependencyUnregisterPackage(
    __in const BURN_PACKAGE* pPackage
    );

#if defined(__cplusplus)
}
#endif
