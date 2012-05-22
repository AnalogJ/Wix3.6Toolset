//-------------------------------------------------------------------------------------------------
// <copyright file="CloseApps.cpp" company="Microsoft">
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
//    Code to close applications via custom actions when the installer cannot.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

#define PROCESS_CLOSE_WAIT_TIME 5000

// WixCloseApplication     Target      Description     Condition       Attributes      Sequence

// structs
LPCWSTR wzQUERY_CLOSEAPPS = L"SELECT `WixCloseApplication`, `Target`, `Description`, `Condition`, `Attributes`, `Property` FROM `WixCloseApplication` ORDER BY `Sequence`";
enum eQUERY_CLOSEAPPS { QCA_ID = 1, QCA_TARGET, QCA_DESCRIPTION, QCA_CONDITION, QCA_ATTRIBUTES, QCA_PROPERTY };

// CloseApplication.Attributes
enum CLOSEAPP_ATTRIBUTES
{
    CLOSEAPP_ATTRIBUTE_NONE = 0,
    CLOSEAPP_ATTRIBUTE_CLOSEMESSAGE = 1,
    CLOSEAPP_ATTRIBUTE_REBOOTPROMPT = 2,
    CLOSEAPP_ATTRIBUTE_ELEVATEDCLOSEMESSAGE = 4,
};


/******************************************************************
 EnumCloseWindowsProc - callback function which sends WM_CLOSE if the
 current window matches the passed in process ID

******************************************************************/
BOOL CALLBACK EnumCloseWindowsProc(HWND hwnd, LPARAM lParam)
{
    DWORD dwProcessId = 0;

    ::GetWindowThreadProcessId(hwnd, &dwProcessId);

    // check if the process Id is the one we're looking for
    if (dwProcessId != lParam)
        return TRUE;

    WcaLog(LOGMSG_VERBOSE, "Sending close message to process id 0x%x", dwProcessId);

    ::PostMessageW(hwnd, WM_CLOSE, 0, 0);

    WcaLog(LOGMSG_VERBOSE, "Result 0x%x", ::GetLastError());

    // so we know we succeeded
    ::SetLastError(ERROR_SUCCESS);

    // A process may have more than one top-level window, continue searching through all windows
    return TRUE;
}

/******************************************************************
 SendProcessCloseMessage - helper function to enumerate the top-level 
 windows and send WM_CLOSE to all matching a process ID

******************************************************************/
void SendProcessCloseMessage(DWORD dwProcessId)
{
    DWORD dwLastError;

    WcaLog(LOGMSG_VERBOSE, "Attempting to send close message to process id 0x%x", dwProcessId);

    if (::EnumWindows(EnumCloseWindowsProc, dwProcessId))
        return;

    dwLastError = GetLastError();
    if (dwLastError != ERROR_SUCCESS)
    {
        WcaLog(LOGMSG_VERBOSE, "CloseApp enumeration error: 0x%x", dwLastError);
    }
}

/******************************************************************
 SendApplicationCloseMessage - helper function to iterate through the 
 processes for the specified application and send all
 applicable process Ids a WM_CLOSE message

******************************************************************/
void SendApplicationCloseMessage(__in LPCWSTR wzApplication)
{
    DWORD *prgProcessIds = NULL;
    DWORD cProcessIds = 0, iProcessId;
    HRESULT hr = S_OK;

    WcaLog(LOGMSG_VERBOSE, "Checking App: %ls ", wzApplication);

    hr = ProcFindAllIdsFromExeName(wzApplication, &prgProcessIds, &cProcessIds);

    if (SUCCEEDED(hr) && 0 < cProcessIds)
    {
        WcaLog(LOGMSG_VERBOSE, "App: %ls found running, %d processes, attempting to send close message.", wzApplication, cProcessIds);

        for (iProcessId = 0; iProcessId < cProcessIds; ++iProcessId)
        {
            SendProcessCloseMessage(prgProcessIds[iProcessId]);
        }
        
        ProcWaitForIds(prgProcessIds, cProcessIds, PROCESS_CLOSE_WAIT_TIME);
    }

    ReleaseMem(prgProcessIds);
}

/******************************************************************
 SetRunningProcessProperty - helper function that sets the specified
 property if there are any instances of the specified executable
 running. Useful to show custom UI to ask for shutdown.
******************************************************************/
void SetRunningProcessProperty(
    __in LPCWSTR wzApplication,
    __in LPCWSTR wzProperty
    )
{
    DWORD *prgProcessIds = NULL;
    DWORD cProcessIds = 0;
    HRESULT hr = S_OK;

    WcaLog(LOGMSG_VERBOSE, "Checking App: %ls ", wzApplication);

    hr = ProcFindAllIdsFromExeName(wzApplication, &prgProcessIds, &cProcessIds);

    if (SUCCEEDED(hr) && 0 < cProcessIds)
    {
        WcaLog(LOGMSG_VERBOSE, "App: %ls found running, %d processes, setting '%ls' property.", wzApplication, cProcessIds, wzProperty);
        WcaSetIntProperty(wzProperty, cProcessIds);
    }

    ReleaseMem(prgProcessIds);
}

/******************************************************************
 WixCloseApplications - entry point for WixCloseApplications Custom Action

 called as Type 1 CustomAction (binary DLL) from Windows Installer 
 in InstallExecuteSequence before InstallFiles
******************************************************************/
extern "C" UINT __stdcall WixCloseApplications(
    __in MSIHANDLE hInstall
    )
{
    //AssertSz(FALSE, "debug WixCloseApplications");
    HRESULT hr = S_OK;
    UINT er = ERROR_SUCCESS;

    LPWSTR pwzData = NULL;
    LPWSTR pwzId = NULL;
    LPWSTR pwzTarget = NULL;
    LPWSTR pwzDescription = NULL;
    LPWSTR pwzCondition = NULL;
    LPWSTR pwzProperty = NULL;
    DWORD dwAttributes = 0;
    MSICONDITION condition = MSICONDITION_NONE;

    DWORD cCloseApps = 0;

    PMSIHANDLE hView = NULL;
    PMSIHANDLE hRec = NULL;
    MSIHANDLE hListboxTable = NULL;
    MSIHANDLE hListboxColumns = NULL;

    LPWSTR pwzCustomActionData = NULL;
    DWORD cchCustomActionData = 0;

    //
    // initialize
    //
    hr = WcaInitialize(hInstall, "WixCloseApplications");
    ExitOnFailure(hr, "failed to initialize");

    //
    // loop through all the objects to be secured
    //
    hr = WcaOpenExecuteView(wzQUERY_CLOSEAPPS, &hView);
    ExitOnFailure(hr, "failed to open view on WixCloseApplication table");
    while (S_OK == (hr = WcaFetchRecord(hView, &hRec)))
    {
        hr = WcaGetRecordString(hRec, QCA_ID, &pwzId);
        ExitOnFailure(hr, "failed to get id from WixCloseApplication table");

        hr = WcaGetRecordString(hRec, QCA_CONDITION, &pwzCondition);
        ExitOnFailure(hr, "failed to get condition from WixCloseApplication table");

        if (pwzCondition && *pwzCondition)
        {
            condition = ::MsiEvaluateConditionW(hInstall, pwzCondition);
            if (MSICONDITION_ERROR == condition)
            {
                hr = E_INVALIDARG;
                ExitOnFailure1(hr, "failed to process condition for WixCloseApplication '%ls'", pwzId);
            }
            else if (MSICONDITION_FALSE == condition)
            {
                continue; // skip processing this target
            }
        }

        hr = WcaGetRecordFormattedString(hRec, QCA_TARGET, &pwzTarget);
        ExitOnFailure(hr, "failed to get target from WixCloseApplication table");

        hr = WcaGetRecordFormattedString(hRec, QCA_DESCRIPTION, &pwzDescription);
        ExitOnFailure(hr, "failed to get description from WixCloseApplication table");

        hr = WcaGetRecordInteger(hRec, QCA_ATTRIBUTES, reinterpret_cast<int*>(&dwAttributes));
        ExitOnFailure(hr, "failed to get attributes from WixCloseApplication table");

        hr = WcaGetRecordFormattedString(hRec, QCA_PROPERTY, &pwzProperty);
        ExitOnFailure(hr, "failed to get property from WixCloseApplication table");

        //
        // send WM_CLOSE to currently running applications
        //
        if (dwAttributes & CLOSEAPP_ATTRIBUTE_CLOSEMESSAGE)
        {
            SendApplicationCloseMessage(pwzTarget);
        }

        //
        // Pass the targets to the deferred action in case the app comes back
        // even if we close it now.
        //
        if ((dwAttributes & CLOSEAPP_ATTRIBUTE_REBOOTPROMPT) || (dwAttributes & CLOSEAPP_ATTRIBUTE_ELEVATEDCLOSEMESSAGE))
        {
            hr = WcaWriteStringToCaData(pwzTarget, &pwzCustomActionData);
            ExitOnFailure(hr, "failed to add target data to CustomActionData");

            hr = WcaWriteIntegerToCaData(dwAttributes, &pwzCustomActionData);
            ExitOnFailure(hr, "failed to add attribute data to CustomActionData");
        }

        if (pwzProperty && *pwzProperty)
        {
            SetRunningProcessProperty(pwzTarget, pwzProperty);
        }

        ++cCloseApps;
    }

    // if we looped through all records all is well
    if (E_NOMOREITEMS == hr)
    {
        hr = S_OK;
    }
    ExitOnFailure(hr, "failed while looping through all apps to close");

    //
    // Do the UI dance now.
    //
    /*

    TODO: Do this eventually

    if (cCloseApps)
    {
        while (TRUE)
        {
            for (DWORD i = 0; i < cCloseApps; ++i)
            {
                hr = WcaAddTempRecord(&hListboxTable, &hListboxColumns, L"ListBox", NULL, 0, 4, L"FileInUseProcess", i, target, description);
                if (FAILED(hr))
                {
                }
            }
        }
    }
    */

    //
    // schedule the custom action and add to progress bar
    //
    if (pwzCustomActionData && *pwzCustomActionData)
    {
        Assert(0 < cCloseApps);

        hr = WcaDoDeferredAction(L"WixCloseApplicationsDeferred", pwzCustomActionData, cCloseApps * COST_CLOSEAPP);
        ExitOnFailure(hr, "failed to schedule WixCloseApplicationsDeferred action");
    }

LExit:
    if (hListboxColumns)
    {
        ::MsiCloseHandle(hListboxColumns);
    }
    if (hListboxTable)
    {
        ::MsiCloseHandle(hListboxTable);
    }

    ReleaseStr(pwzCustomActionData);
    ReleaseStr(pwzData);
    ReleaseStr(pwzProperty);
    ReleaseStr(pwzCondition);
    ReleaseStr(pwzDescription);
    ReleaseStr(pwzTarget);
    ReleaseStr(pwzId);

    if (FAILED(hr))
        er = ERROR_INSTALL_FAILURE;
    return WcaFinalize(er);
}


/******************************************************************
 WixCloseApplicationsDeferred - entry point for 
                                WixCloseApplicationsDeferred Custom Action
                                called as Type 1025 CustomAction 
                                (deferred binary DLL)

 NOTE: deferred CustomAction since it modifies the machine
 NOTE: CustomActionData == wzTarget\tdwAttributes\t...
******************************************************************/
extern "C" UINT __stdcall WixCloseApplicationsDeferred(
    __in MSIHANDLE hInstall
    )
{
//    AssertSz(FALSE, "debug WixCloseApplicationsDeferred");
    HRESULT hr = S_OK;
    DWORD er = ERROR_SUCCESS;

    LPWSTR pwz = NULL;
    LPWSTR pwzData = NULL;
    LPWSTR pwzTarget = NULL;
    DWORD dwAttributes = 0;

    DWORD *prgProcessIds = NULL;
    DWORD cProcessIds = 0;

    //
    // initialize
    //
    hr = WcaInitialize(hInstall, "WixCloseApplicationsDeferred");
    ExitOnFailure(hr, "failed to initialize");

    hr = WcaGetProperty(L"CustomActionData", &pwzData);
    ExitOnFailure(hr, "failed to get CustomActionData");

    WcaLog(LOGMSG_TRACEONLY, "CustomActionData: %ls", pwzData);

    pwz = pwzData;

    //
    // loop through all the passed in data
    //
    while (pwz && *pwz)
    {
        hr = WcaReadStringFromCaData(&pwz, &pwzTarget);
        ExitOnFailure(hr, "failed to process CustomActionData");
        hr = WcaReadIntegerFromCaData(&pwz, reinterpret_cast<int*>(&dwAttributes));
        ExitOnFailure(hr, "failed to processCustomActionData");

        WcaLog(LOGMSG_VERBOSE, "Checking for App: %ls Attributes: %d", pwzTarget, dwAttributes);

        //
        // send WM_CLOSE to currently running applications
        //
        if (dwAttributes & CLOSEAPP_ATTRIBUTE_ELEVATEDCLOSEMESSAGE)
        {
            SendApplicationCloseMessage(pwzTarget);
        }

        //
        // If we find that an app that we need closed is still runing, require a
        // reboot and bail.  Keep iterating through the list in case other apps set 
        // CLOSEAPP_ATTRIBUTE_ELEVATEDCLOSEMESSAGE.  
        // Since the close here happens async the process may still be running, 
        // resulting in a false positive reboot message.
        //
        ProcFindAllIdsFromExeName(pwzTarget, &prgProcessIds, &cProcessIds);
        if ((0 < cProcessIds) && (dwAttributes & CLOSEAPP_ATTRIBUTE_REBOOTPROMPT))
        {
            WcaLog(LOGMSG_VERBOSE, "App: %ls found running, requiring a reboot.", pwzTarget);

            WcaDeferredActionRequiresReboot();
        }

        hr = WcaProgressMessage(COST_CLOSEAPP, FALSE);
        ExitOnFailure(hr, "failed to send progress message");
    }

LExit:
    ReleaseMem(prgProcessIds);

    ReleaseStr(pwzTarget);
    ReleaseStr(pwzData);

    if (FAILED(hr))
    {
        er = ERROR_INSTALL_FAILURE;
    }
    return WcaFinalize(er);
}
