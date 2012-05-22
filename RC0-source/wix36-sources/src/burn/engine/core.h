//-------------------------------------------------------------------------------------------------
// <copyright file="core.h" company="Microsoft">
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
//
//    Setup chainer/bootstrapper core for WiX toolset.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


#if defined(__cplusplus)
extern "C" {
#endif


// constants

const LPCWSTR BURN_COMMANDLINE_SWITCH_ELEVATED = L"burn.elevated";
const LPCWSTR BURN_COMMANDLINE_SWITCH_EMBEDDED = L"burn.embedded";
const LPCWSTR BURN_COMMANDLINE_SWITCH_RUNONCE = L"burn.runonce";
const LPCWSTR BURN_COMMANDLINE_SWITCH_LOG_APPEND = L"burn.log.append";
const LPCWSTR BURN_COMMANDLINE_SWITCH_UNELEVATED = L"burn.unelevated";
const LPCWSTR BURN_COMMANDLINE_SWITCH_RELATED_DETECT = L"burn.related.detect";
const LPCWSTR BURN_COMMANDLINE_SWITCH_RELATED_UPGRADE = L"burn.related.upgrade";
const LPCWSTR BURN_COMMANDLINE_SWITCH_RELATED_ADDON = L"burn.related.addon";
const LPCWSTR BURN_COMMANDLINE_SWITCH_RELATED_PATCH = L"burn.related.patch";
const LPCWSTR BURN_COMMANDLINE_SWITCH_DISABLE_UNELEVATE = L"burn.disable.unelevate";
const LPCWSTR BURN_COMMANDLINE_SWITCH_IGNOREDEPENDENCIES = L"burn.ignoredependencies";
const LPCWSTR BURN_COMMANDLINE_SWITCH_PREFIX = L"burn.";

const LPCWSTR BURN_BUNDLE_LAYOUT_DIRECTORY = L"WixBundleLayoutDirectory";
const LPCWSTR BURN_BUNDLE_ACTION = L"WixBundleAction";
const LPCWSTR BURN_BUNDLE_INSTALLED = L"WixBundleInstalled";
const LPCWSTR BURN_BUNDLE_ELEVATED = L"WixBundleElevated";
const LPCWSTR BURN_BUNDLE_PROVIDER_KEY = L"WixBundleProviderKey";
const LPCWSTR BURN_BUNDLE_TAG = L"WixBundleTag";

// The following constants must stay in sync with src\wix\Binder.cs
const LPCWSTR BURN_BUNDLE_NAME = L"WixBundleName";
const LPCWSTR BURN_BUNDLE_ORIGINAL_SOURCE = L"WixBundleOriginalSource";
const LPCWSTR BURN_BUNDLE_LAST_USED_SOURCE = L"WixBundleLastUsedSource";


// enums

enum BURN_MODE
{
    BURN_MODE_NORMAL,
    BURN_MODE_ELEVATED,
    BURN_MODE_EMBEDDED,
    BURN_MODE_RUNONCE,
};

enum BURN_ELEVATION_STATE
{
    BURN_ELEVATION_STATE_UNELEVATED,
    BURN_ELEVATION_STATE_UNELEVATED_EXPLICITLY,
    BURN_ELEVATION_STATE_ELEVATED,
    BURN_ELEVATION_STATE_ELEVATED_EXPLICITLY,
};

enum BURN_AU_PAUSE_ACTION
{
    BURN_AU_PAUSE_ACTION_NONE,
    BURN_AU_PAUSE_ACTION_IFELEVATED,
    BURN_AU_PAUSE_ACTION_IFELEVATED_NORESUME,
};


// structs

typedef struct _BURN_ENGINE_STATE
{
    // synchronization
    CRITICAL_SECTION csActive; // Any call from the UX that reads or alters the engine state
                               // needs to be syncronized through this critical section.
                               // Note: The engine must never do a UX callback while in this critical section.

    // UX flow control
    //BOOL fSuspend;             // Is TRUE when UX made Suspend() call on core.
    //BOOL fForcedReboot;        // Is TRUE when UX made Reboot() call on core.
    //BOOL fCancelled;           // Is TRUE when UX return cancel on UX OnXXX() methods.
    //BOOL fReboot;              // Is TRUE when UX confirms OnRestartRequried().
    BOOL fRestart;               // Set TRUE when UX returns IDRESTART during Apply().

    // engine data
    BOOTSTRAPPER_COMMAND command;
    BURN_SECTION section;
    BURN_VARIABLES variables;
    BURN_CONDITION condition;
    BURN_SEARCHES searches;
    BURN_USER_EXPERIENCE userExperience;
    BURN_REGISTRATION registration;
    BURN_CONTAINERS containers;
    BURN_CATALOGS catalogs;
    BURN_PAYLOADS payloads;
    BURN_PACKAGES packages;

    HWND hMessageWindow;
    HANDLE hMessageWindowThread;

    BOOL fDisableRollback;
    BOOL fDisableSystemRestore;
    BOOL fParallelCacheAndExecute;

    BURN_LOGGING log;

    BURN_PLAN plan;

    BURN_MODE mode;
    BURN_AU_PAUSE_ACTION automaticUpdates;

    BURN_ELEVATION_STATE elevationState;
    DWORD dwElevatedLoggingTlsId;

    LPWSTR sczBundleEngineWorkingPath;
    BURN_PIPE_CONNECTION companionConnection;
    BURN_PIPE_CONNECTION embeddedConnection;

    BURN_RESUME_MODE resumeMode;
    BOOL fDisableUnelevate;

    LPWSTR sczIgnoreDependencies;
} BURN_ENGINE_STATE;


// function declarations

HRESULT CoreInitialize(
    __in_z_opt LPCWSTR wzCommandLine,
    __in BURN_ENGINE_STATE* pEngineState
    );
HRESULT CoreSerializeEngineState(
    __in BURN_ENGINE_STATE* pEngineState,
    __inout BYTE** ppbBuffer,
    __inout SIZE_T* piBuffer
    );
HRESULT CoreQueryRegistration(
    __in BURN_ENGINE_STATE* pEngineState
    );
//HRESULT CoreDeserializeEngineState(
//    __in BURN_ENGINE_STATE* pEngineState,
//    __in_bcount(cbBuffer) BYTE* pbBuffer,
//    __in SIZE_T cbBuffer
//    );
HRESULT CoreDetect(
    __in BURN_ENGINE_STATE* pEngineState
    );
HRESULT CorePlan(
    __in BURN_ENGINE_STATE* pEngineState,
    __in BOOTSTRAPPER_ACTION action
    );
HRESULT CoreElevate(
    __in BURN_ENGINE_STATE* pEngineState,
    __in_opt HWND hwndParent
    );
HRESULT CoreApply(
    __in BURN_ENGINE_STATE* pEngineState,
    __in_opt HWND hwndParent
    );
HRESULT CoreQuit(
    __in BURN_ENGINE_STATE* pEngineState,
    __in int nExitCode
    );
HRESULT CoreSaveEngineState(
    __in BURN_ENGINE_STATE* pEngineState
    );

#if defined(__cplusplus)
}
#endif
