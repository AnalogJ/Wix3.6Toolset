//-------------------------------------------------------------------------------------------------
// <copyright file="variant.h" company="Microsoft">
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
//
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


#if defined(__cplusplus)
extern "C" {
#endif


// constants

enum BURN_VARIANT_TYPE
{
    BURN_VARIANT_TYPE_NONE,
    BURN_VARIANT_TYPE_NUMERIC,
    BURN_VARIANT_TYPE_STRING,
    BURN_VARIANT_TYPE_VERSION,
};


// struct

typedef struct _BURN_VARIANT
{
    union
    {
        LONGLONG llValue;
        DWORD64 qwValue;
        LPWSTR sczValue;
    };
    BURN_VARIANT_TYPE Type;
} BURN_VARIANT;


// function declarations

void BVariantUninitialize(
    __in BURN_VARIANT* pVariant
    );
HRESULT BVariantGetNumeric(
    __in BURN_VARIANT* pVariant,
    __out LONGLONG* pllValue
    );
HRESULT BVariantGetString(
    __in BURN_VARIANT* pVariant,
    __out_z LPWSTR* psczValue
    );
HRESULT BVariantGetVersion(
    __in BURN_VARIANT* pVariant,
    __out DWORD64* pqwValue
    );
HRESULT BVariantSetNumeric(
    __in BURN_VARIANT* pVariant,
    __in LONGLONG llValue
    );
HRESULT BVariantSetString(
    __in BURN_VARIANT* pVariant,
    __in_z LPCWSTR wzValue,
    __in DWORD_PTR cchValue
    );
HRESULT BVariantSetVersion(
    __in BURN_VARIANT* pVariant,
    __in DWORD64 qwValue
    );
HRESULT BVariantCopy(
    __in BURN_VARIANT* pSource,
    __out BURN_VARIANT* pTarget
    );
HRESULT BVariantChangeType(
    __in BURN_VARIANT* pVariant,
    __in BURN_VARIANT_TYPE type
    );

#if defined(__cplusplus)
}
#endif
