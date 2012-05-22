//-------------------------------------------------------------------------------------------------
// <copyright file="userexperience.h" company="Microsoft">
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

const DWORD MB_RETRYTRYAGAIN = 0xF;


// structs

typedef struct _BURN_USER_EXPERIENCE
{
    BOOL fSplashScreen;
    BURN_PAYLOADS payloads;

    HMODULE hUXModule;
    IBootstrapperApplication* pUserExperience;
    LPWSTR sczTempDirectory;

    CRITICAL_SECTION csEngineActive;    // Changing the engine active state in the user experience must be
                                        // syncronized through this critical section.
                                        // Note: The engine must never do a UX callback while in this critical section.

    BOOL fEngineActive;                 // Indicates that the engine is currently active with one of the execution
                                        // steps (detect, plan, apply), and cannot accept requests from the UX.
                                        // This flag should be cleared by the engine prior to UX callbacks that
                                        // allows altering of the engine state.

    HRESULT hrApplyError;               // Tracks is an error occurs during apply that requires the cache or
                                        // execute threads to bail.

    DWORD dwExitCode;                   // Exit code returned by the user experience for the engine overall.
} BURN_USER_EXPERIENCE;


// functions

HRESULT UserExperienceParseFromXml(
    __in BURN_USER_EXPERIENCE* pUserExperience,
    __in IXMLDOMNode* pixnBundle
    );
void UserExperienceUninitialize(
    __in BURN_USER_EXPERIENCE* pUserExperience
    );
HRESULT UserExperienceLoad(
    __in BURN_USER_EXPERIENCE* pUserExperience,
    __in IBootstrapperEngine* pEngine,
    __in BOOTSTRAPPER_COMMAND* pCommand
    );
HRESULT UserExperienceUnload(
    __in BURN_USER_EXPERIENCE* pUserExperience
    );
HRESULT UserExperienceEnsureWorkingFolder(
    __in LPCWSTR wzBundleId,
    __deref_out_z LPWSTR* psczUserExperienceWorkingFolder
    );
HRESULT UserExperienceRemove(
    __in BURN_USER_EXPERIENCE* pUserExperience
    );
HRESULT UserExperienceActivateEngine(
    __in BURN_USER_EXPERIENCE* pUserExperience,
    __out_opt BOOL* pfActivated
    );
void UserExperienceDeactivateEngine(
    __in BURN_USER_EXPERIENCE* pUserExperience
    );
HRESULT UserExperienceEnsureEngineInactive(
    __in BURN_USER_EXPERIENCE* pUserExperience
    );
void UserExperienceExecuteReset(
    __in BURN_USER_EXPERIENCE* pUserExperience
    );
void UserExperienceExecutePhaseComplete(
    __in BURN_USER_EXPERIENCE* pUserExperience,
    __in HRESULT hrResult
    );
HRESULT UserExperienceInterpretResult(
    __in BURN_USER_EXPERIENCE* pUserExperience,
    __in DWORD dwAllowedResults,
    __in int nResult
    );
int UserExperienceCheckExecuteResult(
    __in BURN_USER_EXPERIENCE* pUserExperience,
    __in BOOL fRollback,
    __in DWORD dwAllowedResults,
    __in int nResult
    );
HRESULT UserExperienceInterpretExecuteResult(
    __in BURN_USER_EXPERIENCE* pUserExperience,
    __in BOOL fRollback,
    __in DWORD dwAllowedResults,
    __in int nResult
    );
#if defined(__cplusplus)
}
#endif
