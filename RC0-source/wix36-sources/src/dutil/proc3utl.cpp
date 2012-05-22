//-------------------------------------------------------------------------------------------------
// <copyright file="proc3utl.cpp" company="Microsoft">
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
//    Process helper functions that require Wtsapi32.DLL.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

static HRESULT GetActiveSessionUserToken(
    __out HANDLE *phToken
    );


/********************************************************************
 ProcExecuteAsInteractiveUser() - runs process as currently logged in
                                  user.
*******************************************************************/
extern "C" HRESULT DAPI ProcExecuteAsInteractiveUser(
    __in_z LPCWSTR wzExecutablePath,
    __in_z LPCWSTR wzCommandLine,
    __out HANDLE *phProcess
    )
{
    HRESULT hr = S_OK;
    HANDLE hToken = NULL;
    LPVOID pEnvironment = NULL;
    LPWSTR sczFullCommandLine = NULL;
    STARTUPINFOW si = { };
    PROCESS_INFORMATION pi = { };

    hr = GetActiveSessionUserToken(&hToken);
    ExitOnFailure(hr, "Failed to get active session user token.");

    if (!::CreateEnvironmentBlock(&pEnvironment, hToken, FALSE))
    {
        ExitWithLastError(hr, "Failed to create environment block for UI process.");
    }

    hr = StrAllocFormatted(&sczFullCommandLine, L"\"%ls\" %ls", wzExecutablePath, wzCommandLine);
    ExitOnFailure(hr, "Failed to allocate full command-line.");

    si.cb = sizeof(si);
    if (!::CreateProcessAsUserW(hToken, wzExecutablePath, sczFullCommandLine, NULL, NULL, FALSE, CREATE_UNICODE_ENVIRONMENT, pEnvironment, NULL, &si, &pi))
    {
        ExitWithLastError1(hr, "Failed to create UI process: %ls", sczFullCommandLine);
    }

    *phProcess = pi.hProcess;
    pi.hProcess = NULL;

LExit:
    ReleaseHandle(pi.hThread);
    ReleaseHandle(pi.hProcess);
    ReleaseStr(sczFullCommandLine);

    if (pEnvironment)
    {
        ::DestroyEnvironmentBlock(pEnvironment);
    }

    ReleaseHandle(hToken);

    return hr;
}


static HRESULT GetActiveSessionUserToken(
    __out HANDLE *phToken
    )
{
    HRESULT hr = S_OK;
    PWTS_SESSION_INFO pSessionInfo = NULL;
    DWORD cSessions = 0;
    DWORD dwSessionId = 0;
    BOOL fSessionFound = FALSE;
    HANDLE hToken = NULL;

    // Loop through the sessions looking for the active one.
    if (!::WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pSessionInfo, &cSessions))
    {
        ExitWithLastError(hr, "Failed to enumerate sessions.");
    }

    for (DWORD i = 0; i < cSessions; ++i)
    {
        if (WTSActive == pSessionInfo[i].State)
        {
            dwSessionId = pSessionInfo[i].SessionId;
            fSessionFound = TRUE;

            break;
        }
    }

    if (!fSessionFound)
    {
        ExitFunction1(hr = E_NOTFOUND);
    }

    // Get the user token from the active session.
    if (!::WTSQueryUserToken(dwSessionId, &hToken))
    {
        ExitWithLastError(hr, "Failed to get active session user token.");
    }

    *phToken = hToken;
    hToken = NULL;

LExit:
    ReleaseHandle(hToken);

    if (pSessionInfo)
    {
        ::WTSFreeMemory(pSessionInfo);
    }

    return hr;
}
