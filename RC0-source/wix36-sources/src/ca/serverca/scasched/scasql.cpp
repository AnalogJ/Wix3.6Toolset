//-------------------------------------------------------------------------------------------------
// <copyright file="scasql.cpp" company="Microsoft">
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
//    SQL functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

// prototypes
static HRESULT ConfigureSqlData(
    __in SCA_ACTION saAction
    );


/********************************************************************
InstallSqlData - CUSTOM ACTION ENTRY POINT for installing
                 SQL data

********************************************************************/
extern "C" UINT __stdcall InstallSqlData(
    __in MSIHANDLE hInstall
    )
{
    HRESULT hr = S_OK;
    UINT er = ERROR_SUCCESS;

    // initialize
    hr = WcaInitialize(hInstall, "InstallSqlData");
    ExitOnFailure(hr, "Failed to initialize");

    hr = ConfigureSqlData(SCA_ACTION_INSTALL);

LExit:
    er = SUCCEEDED(hr) ? ERROR_SUCCESS : ERROR_INSTALL_FAILURE;
    return WcaFinalize(er);
}


/********************************************************************
UninstallSqlData - CUSTOM ACTION ENTRY POINT for uninstalling
                   SQL data

********************************************************************/
extern "C" UINT __stdcall UninstallSqlData(
    __in MSIHANDLE hInstall
    )
{
    HRESULT hr = S_OK;
    UINT er = ERROR_SUCCESS;

    // initialize
    hr = WcaInitialize(hInstall, "UninstallCertificates");
    ExitOnFailure(hr, "Failed to initialize");

    hr = ConfigureSqlData(SCA_ACTION_UNINSTALL);

LExit:
    er = SUCCEEDED(hr) ? ERROR_SUCCESS : ERROR_INSTALL_FAILURE;
    return WcaFinalize(er);
}


static HRESULT ConfigureSqlData(
    __in SCA_ACTION saAction
    )
{
    //AssertSz(FALSE, "debug ConfigureSqlData()");
    HRESULT hr = S_OK;

    SCA_DB* psdList = NULL;
    SCA_SQLSTR* psssList = NULL;

    // check for the prerequsite tables
    if (S_OK != WcaTableExists(L"SqlDatabase"))
    {
        WcaLog(LOGMSG_VERBOSE, "skipping SQL CustomAction, no SqlDatabase table");
        ExitFunction1(hr = S_FALSE);
    }

    // read tables
    hr = ScaDbsRead(&psdList, saAction);
    ExitOnFailure(hr, "failed to read SqlDatabase table");

    hr = ScaSqlStrsRead(&psssList, saAction);
    ExitOnFailure(hr, "failed to read SqlStrings table");

    hr = ScaSqlStrsReadScripts(&psssList, saAction);
    ExitOnFailure(hr, "failed to read SqlScripts table");

    if (SCA_ACTION_UNINSTALL == saAction)
    {
        // do uninstall actions (order is important!)
        hr = ScaSqlStrsUninstall(psdList, psssList);
        ExitOnFailure(hr, "failed to execute uninstall SQL strings");

        hr = ScaDbsUninstall(psdList);
        ExitOnFailure(hr, "failed to uninstall databases");
    }
    else
    {
        // do install actions (order is important!)
        hr = ScaDbsInstall(psdList);
        ExitOnFailure(hr, "failed to install databases");

        hr = ScaSqlStrsInstall(psdList, psssList);
        ExitOnFailure(hr, "failed to execute install SQL strings, length may be too long, try add GO to break up");
    }

LExit:
    if (psssList)
        ScaSqlStrsFreeList(psssList);

    if (psdList)
        ScaDbsFreeList(psdList);

    return hr;
}
