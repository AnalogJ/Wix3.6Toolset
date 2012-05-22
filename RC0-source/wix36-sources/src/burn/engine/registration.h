//-------------------------------------------------------------------------------------------------
// <copyright file="registration.h" company="Microsoft">
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


enum BURN_MODE;
struct _BURN_LOGGING;
typedef _BURN_LOGGING BURN_LOGGING;

// constants

enum BURN_RESUME_MODE
{
    BURN_RESUME_MODE_NONE,
    BURN_RESUME_MODE_ACTIVE,
    BURN_RESUME_MODE_SUSPEND,
    BURN_RESUME_MODE_ARP,
    BURN_RESUME_MODE_REBOOT_PENDING,
};

enum BURN_REGISTRATION_MODIFY_TYPE
{
    BURN_REGISTRATION_MODIFY_ENABLED,
    BURN_REGISTRATION_MODIFY_DISABLE,
    BURN_REGISTRATION_MODIFY_DISABLE_BUTTON,
};


// structs

typedef struct _BURN_UPDATE_REGISTRATION
{
    BOOL fRegisterUpdate;
    LPWSTR sczManufacturer;
    LPWSTR sczDepartment;
    LPWSTR sczProductFamily;
    LPWSTR sczName;
    LPWSTR sczClassification;
} BURN_UPDATE_REGISTRATION;

typedef struct _BURN_RELATED_BUNDLE
{
    BOOTSTRAPPER_RELATION_TYPE relationType;

    DWORD64 qwVersion;

    BURN_PACKAGE package;
} BURN_RELATED_BUNDLE;

typedef struct _BURN_RELATED_BUNDLES
{
    BURN_RELATED_BUNDLE* rgRelatedBundles;
    DWORD cRelatedBundles;
} BURN_RELATED_BUNDLES;

typedef struct _BURN_REGISTRATION
{
    BOOL fPerMachine;
    BOOL fRegisterArp;
    BOOL fInstalled;
    LPWSTR sczId;
    LPWSTR sczTag;

    LPWSTR *rgsczDetectCodes;
    DWORD cDetectCodes;

    LPWSTR *rgsczUpgradeCodes;
    DWORD cUpgradeCodes;

    LPWSTR *rgsczAddonCodes;
    DWORD cAddonCodes;

    LPWSTR *rgsczPatchCodes;
    DWORD cPatchCodes;

    DWORD64 qwVersion;
    LPWSTR sczProviderKey;
    LPWSTR sczExecutableName;

    // paths
    HKEY hkRoot;
    LPWSTR sczRegistrationKey;
    LPWSTR sczCacheExecutablePath;
    LPWSTR sczResumeCommandLine;
    LPWSTR sczStateFile;

    // ARP registration
    LPWSTR sczDisplayName;
    LPWSTR sczDisplayVersion;
    LPWSTR sczPublisher;
    LPWSTR sczHelpLink;
    LPWSTR sczHelpTelephone;
    LPWSTR sczAboutUrl;
    LPWSTR sczUpdateUrl;
    LPWSTR sczParentDisplayName;
    LPWSTR sczComments;
    //LPWSTR sczReadme; // TODO: this would be a file path
    LPWSTR sczContact;
    //DWORD64 qwEstimatedSize; // TODO: size should come from disk cost calculation
    BURN_REGISTRATION_MODIFY_TYPE modify;
    BOOL fNoRemoveDefined;
    BOOL fNoRemove;

    // Update registration
    BURN_UPDATE_REGISTRATION update;

    // Related
    BURN_RELATED_BUNDLES relatedBundles;
} BURN_REGISTRATION;


// functions

HRESULT RegistrationParseFromXml(
    __in BURN_REGISTRATION* pRegistration,
    __in IXMLDOMNode* pixnBundle
    );
void RegistrationUninitialize(
    __in BURN_REGISTRATION* pRegistration
    );
HRESULT RegistrationSetVariables(
    __in BURN_REGISTRATION* pRegistration,
    __in BURN_VARIABLES* pVariables
    );
HRESULT RegistrationDetectInstalled(
    __in BURN_REGISTRATION* pRegistration,
    __out BOOL* pfInstalled
    );
HRESULT RegistrationDetectResumeType(
    __in BURN_REGISTRATION* pRegistration,
    __out BOOTSTRAPPER_RESUME_TYPE* pResumeType
    );
HRESULT RegistrationDetectRelatedBundles(
    __in BOOL fElevated,
    __in_opt BURN_USER_EXPERIENCE* pUX,
    __in BURN_REGISTRATION* pRegistration,
    __in_opt BOOTSTRAPPER_COMMAND* pCommand
    );
HRESULT RegistrationSessionBegin(
    __in_z LPCWSTR wzEngineWorkingPath,
    __in BURN_REGISTRATION* pRegistration,
    __in BURN_VARIABLES* pVariables,
    __in BURN_USER_EXPERIENCE* pUserExperience,
    __in BOOTSTRAPPER_ACTION action,
    __in DWORD64 qwEstimatedSize,
    __in BOOL fPerMachineProcess
    );
HRESULT RegistrationSessionResume(
    __in BURN_REGISTRATION* pRegistration,
    __in BOOL fPerMachineProcess
    );
HRESULT RegistrationSessionEnd(
    __in BURN_REGISTRATION* pRegistration,
    __in BOOL fKeepRegistration,
    __in BOOL fSuspend,
    __in BOOTSTRAPPER_APPLY_RESTART restart,
    __in BOOL fPerMachineProcess,
    __out_opt BURN_RESUME_MODE* pResumeMode
    );
HRESULT RegistrationSaveState(
    __in BURN_REGISTRATION* pRegistration,
    __in_bcount_opt(cbBuffer) BYTE* pbBuffer,
    __in_opt DWORD cbBuffer
    );
HRESULT RegistrationLoadState(
    __in BURN_REGISTRATION* pRegistration,
    __out_bcount(*pcbBuffer) BYTE** ppbBuffer,
    __out DWORD* pcbBuffer
    );


#if defined(__cplusplus)
}
#endif
