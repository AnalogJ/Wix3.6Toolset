//-------------------------------------------------------------------------------------------------
// <copyright file="scawebappext7.cpp" company="Microsoft">
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
//    IIS Web Application Extension functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

HRESULT ScaWebAppExtensionsWrite7(
    __in_z LPCWSTR wzWebName,
    __in_z LPCWSTR wzRootOfWeb,
    __in SCA_WEB_APPLICATION_EXTENSION* pswappextList
    )
{
    HRESULT hr = S_OK;
    SCA_WEB_APPLICATION_EXTENSION* pswappext = NULL;

    if (!pswappextList)
    {
        ExitFunction1(hr = S_OK);
    }

    //create the Extension for this vdir application
    //all go to same web/root location tag
    hr = ScaWriteConfigID(IIS_APPEXT_BEGIN);
    ExitOnFailure(hr, "Failed to write webappext begin id");
    hr = ScaWriteConfigString(wzWebName);                //site name key
    ExitOnFailure(hr, "Failed to write app web key");
    hr = ScaWriteConfigString(wzRootOfWeb);               //app path key
    ExitOnFailure(hr, "Failed to write app web key");

    pswappext = pswappextList;

    while (pswappext)
    {
        //create the Extension for this vdir application
        hr = ScaWriteConfigID(IIS_APPEXT);
        ExitOnFailure(hr, "Failed to write webappext begin id");

        if (*pswappext->wzExtension)
        {
            hr = ScaWriteConfigString(pswappext->wzExtension);
        }
        else   // blank means "*" (all)
        {
            hr = ScaWriteConfigString(L"*");
        }
        ExitOnFailure(hr, "Failed to write extension");

        hr = ScaWriteConfigString(pswappext->wzExecutable);
        ExitOnFailure(hr, "Failed to write extension executable");

        hr = ScaWriteConfigString(pswappext->wzVerbs);
        ExitOnFailure(hr, "Failed to write extension verbs");

        pswappext = pswappext->pswappextNext;
    }

    hr = ScaWriteConfigID(IIS_APPEXT_END);
    ExitOnFailure(hr, "Failed to write webappext begin id");

LExit:
    return hr;
}

