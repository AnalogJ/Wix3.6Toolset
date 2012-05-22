//-------------------------------------------------------------------------------------------------
// <copyright file="apply.h" company="Microsoft">
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
//    Apply phase functions.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


#ifdef __cplusplus
extern "C" {
#endif


enum GENERIC_EXECUTE_MESSAGE_TYPE
{
    GENERIC_EXECUTE_MESSAGE_NONE,
    GENERIC_EXECUTE_MESSAGE_ERROR,
    GENERIC_EXECUTE_MESSAGE_PROGRESS,
    GENERIC_EXECUTE_MESSAGE_FILES_IN_USE,
};

typedef struct _GENERIC_EXECUTE_MESSAGE
{
    GENERIC_EXECUTE_MESSAGE_TYPE type;
    DWORD dwAllowedResults;

    union
    {
        struct
        {
            DWORD dwErrorCode;
            LPCWSTR wzMessage;
        } error;
        struct
        {
            DWORD dwPercentage;
        } progress;
        struct
        {
            DWORD cFiles;
            LPCWSTR* rgwzFiles;
        } filesInUse;
    };
} GENERIC_EXECUTE_MESSAGE;


typedef int (*PFN_GENERICMESSAGEHANDLER)(
    __in GENERIC_EXECUTE_MESSAGE* pMessage,
    __in LPVOID pvContext
    );


void ApplyReset(
    __in BURN_USER_EXPERIENCE* pUX,
    __in BURN_PACKAGES* pPackages
    );
HRESULT ApplyLock(
    __in BOOL fPerMachine,
    __out HANDLE* phLock
    );
HRESULT ApplyRegister(
    __in BURN_ENGINE_STATE* pEngineState
    );
HRESULT ApplyUnregister(
    __in BURN_ENGINE_STATE* pEngineState,
    __in BOOL fRollback,
    __in BOOL fSuspend,
    __in BOOTSTRAPPER_APPLY_RESTART restart
    );
HRESULT ApplyCache(
    __in BURN_USER_EXPERIENCE* pUX,
    __in BURN_VARIABLES* pVariables,
    __in BURN_PLAN* pPlan,
    __in HANDLE hPipe,
    __inout DWORD* pcOverallProgressTicks,
    __out BOOL* pfRollback
    );
HRESULT ApplyExecute(
    __in BURN_ENGINE_STATE* pEngineState,
    __in_opt HWND hwndParent,
    __in_opt HANDLE hCacheThread,
    __inout DWORD* pcOverallProgressTicks,
    __out BOOL* pfKeepRegistration,
    __out BOOL* pfRollback,
    __out BOOL* pfSuspend,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    );
void ApplyClean(
    __in BURN_USER_EXPERIENCE* pUX,
    __in BURN_PLAN* pPlan,
    __in HANDLE hPipe
    );


#ifdef __cplusplus
}
#endif
