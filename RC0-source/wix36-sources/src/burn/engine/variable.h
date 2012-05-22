//-------------------------------------------------------------------------------------------------
// <copyright file="variable.h" company="Microsoft">
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
//    Variable management functions for Burn.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


#if defined(__cplusplus)
extern "C" {
#endif


// constants

const LPCWSTR VARIABLE_DATE = L"Date";
const LPCWSTR VARIABLE_LOGONUSER = L"LogonUser";
const LPCWSTR VARIABLE_INSTALLERNAME = L"InstallerName";
const LPCWSTR VARIABLE_INSTALLERVERSION = L"InstallerVersion";


// typedefs

typedef HRESULT (*PFN_INITIALIZEVARIABLE)(
    __in DWORD_PTR dwpData,
    __inout BURN_VARIANT* pValue
    );


// structs

typedef struct _BURN_VARIABLE
{
    LPWSTR sczName;
    BURN_VARIANT Value;
    BOOL fHidden;
    BOOL fPersisted;

    // used for late initialization of built-in variables
    BOOL fBuiltIn;
    PFN_INITIALIZEVARIABLE pfnInitialize;
    DWORD_PTR dwpInitializeData;
} BURN_VARIABLE;

typedef struct _BURN_VARIABLES
{
    CRITICAL_SECTION csAccess;
    DWORD dwMaxVariables;
    DWORD cVariables;
    BURN_VARIABLE* rgVariables;
} BURN_VARIABLES;


// function declarations

HRESULT VariableInitialize(
    __in BURN_VARIABLES* pVariables
    );
HRESULT VariablesParseFromXml(
    __in BURN_VARIABLES* pVariables,
    __in IXMLDOMNode* pixnBundle
    );
void VariablesUninitialize(
    __in BURN_VARIABLES* pVariables
    );
void VariablesDump(
    __in BURN_VARIABLES* pVariables
    );
HRESULT VariableGetNumeric(
    __in BURN_VARIABLES* pVariables,
    __in_z LPCWSTR wzVariable,
    __out LONGLONG* pllValue
    );
HRESULT VariableGetString(
    __in BURN_VARIABLES* pVariables,
    __in_z LPCWSTR wzVariable,
    __out_z LPWSTR* psczValue
    );
HRESULT VariableGetVersion(
    __in BURN_VARIABLES* pVariables,
    __in_z LPCWSTR wzVariable,
    __in DWORD64* pqwValue
    );
HRESULT VariableGetVariant(
    __in BURN_VARIABLES* pVariables,
    __in_z LPCWSTR wzVariable,
    __in BURN_VARIANT* pValue
    );
HRESULT VariableGetFormatted(
    __in BURN_VARIABLES* pVariables,
    __in_z LPCWSTR wzVariable,
    __out_z LPWSTR* psczValue
    );
HRESULT VariableSetNumeric(
    __in BURN_VARIABLES* pVariables,
    __in_z LPCWSTR wzVariable,
    __in LONGLONG llValue,
    __in BOOL fOverwriteBuiltIn
    );
HRESULT VariableSetString(
    __in BURN_VARIABLES* pVariables,
    __in_z LPCWSTR wzVariable,
    __in_z_opt LPCWSTR wzValue,
    __in BOOL fOverwriteBuiltIn
    );
HRESULT VariableSetVersion(
    __in BURN_VARIABLES* pVariables,
    __in_z LPCWSTR wzVariable,
    __in DWORD64 qwValue,
    __in BOOL fOverwriteBuiltIn
    );
HRESULT VariableSetVariant(
    __in BURN_VARIABLES* pVariables,
    __in_z LPCWSTR wzVariable,
    __in BURN_VARIANT* pVariant,
    __in BOOL fOverwriteBuiltIn
    );
HRESULT VariableFormatString(
    __in BURN_VARIABLES* pVariables,
    __in_z LPCWSTR wzIn,
    __out_z_opt LPWSTR* psczOut,
    __out_opt DWORD* pcchOut
    );
HRESULT VariableFormatStringObfuscated(
    __in BURN_VARIABLES* pVariables,
    __in_z LPCWSTR wzIn,
    __out_z_opt LPWSTR* psczOut,
    __out_opt DWORD* pcchOut
    );
HRESULT VariableEscapeString(
    __in_z LPCWSTR wzIn,
    __out_z LPWSTR* psczOut
    );
HRESULT VariableSerialize(
    __in BURN_VARIABLES* pVariables,
    __in BOOL fPersisting,
    __inout BYTE** ppbBuffer,
    __inout SIZE_T* piBuffer
    );
HRESULT VariableDeserialize(
    __in BURN_VARIABLES* pVariables,
    __in_bcount(cbBuffer) BYTE* pbBuffer,
    __in SIZE_T cbBuffer,
    __inout SIZE_T* piBuffer
    );

#if defined(__cplusplus)
}
#endif
