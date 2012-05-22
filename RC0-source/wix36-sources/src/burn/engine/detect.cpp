//-------------------------------------------------------------------------------------------------
// <copyright file="detect.cpp" company="Microsoft">
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


// function definitions

extern "C" void DetectReset(
    __in BURN_PACKAGES* pPackages
    )
{
    for (DWORD iPackage = 0; iPackage < pPackages->cPackages; ++iPackage)
    {
        BURN_PACKAGE* pPackage = pPackages->rgPackages + iPackage;

        pPackage->currentState = BOOTSTRAPPER_PACKAGE_STATE_UNKNOWN;

        pPackage->cache = BURN_CACHE_STATE_NONE;
        for (DWORD iPayload = 0; iPayload < pPackage->cPayloads; ++iPayload)
        {
            BURN_PACKAGE_PAYLOAD* pPayload = pPackage->rgPayloads + iPayload;
            pPayload->fCached = FALSE;
        }

        if (BURN_PACKAGE_TYPE_MSI == pPackage->type)
        {
            for (DWORD iFeature = 0; iFeature < pPackage->Msi.cFeatures; ++iFeature)
            {
                BURN_MSIFEATURE* pFeature = pPackage->Msi.rgFeatures + iFeature;

                pFeature->currentState = BOOTSTRAPPER_FEATURE_STATE_UNKNOWN;
            }
        }
        else if (BURN_PACKAGE_TYPE_MSP == pPackage->type)
        {
            ReleaseNullMem(pPackage->Msp.rgTargetProducts);
            pPackage->Msp.cTargetProductCodes = 0;
        }
    }

    for (DWORD iPatchInfo = 0; iPatchInfo < pPackages->cPatchInfo; ++iPatchInfo)
    {
        MSIPATCHSEQUENCEINFOW* pPatchInfo = pPackages->rgPatchInfo + iPatchInfo;
        pPatchInfo->dwOrder = 0;
        pPatchInfo->uStatus = 0;
    }
}
