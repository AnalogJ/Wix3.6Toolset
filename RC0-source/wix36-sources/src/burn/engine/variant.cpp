//-------------------------------------------------------------------------------------------------
// <copyright file="variant.cpp" company="Microsoft">
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

#include "precomp.h"


// function definitions

extern "C" void BVariantUninitialize(
    __in BURN_VARIANT* pVariant
    )
{
    if (BURN_VARIANT_TYPE_STRING == pVariant->Type)
    {
        ReleaseStr(pVariant->sczValue);
    }
    memset(pVariant, 0, sizeof(BURN_VARIANT));
}

extern "C" HRESULT BVariantGetNumeric(
    __in BURN_VARIANT* pVariant,
    __out LONGLONG* pllValue
    )
{
    HRESULT hr = S_OK;

    switch (pVariant->Type)
    {
    case BURN_VARIANT_TYPE_NUMERIC:
        *pllValue = pVariant->llValue;
        break;
    case BURN_VARIANT_TYPE_STRING:
        hr = StrStringToInt64(pVariant->sczValue, 0, pllValue);
        if (FAILED(hr))
        {
            hr = DISP_E_TYPEMISMATCH;
        }
        break;
    case BURN_VARIANT_TYPE_VERSION:
        *pllValue = (LONGLONG)pVariant->qwValue;
        break;
    default:
        hr = E_INVALIDARG;
        break;
    }

    return hr;
}

extern "C" HRESULT BVariantGetString(
    __in BURN_VARIANT* pVariant,
    __out_z LPWSTR* psczValue
    )
{
    HRESULT hr = S_OK;

    switch (pVariant->Type)
    {
    case BURN_VARIANT_TYPE_NUMERIC:
        hr = StrAllocFormatted(psczValue, L"%I64d", pVariant->llValue);
        ExitOnFailure(hr, "Failed to convert int64 to string.");
        break;
    case BURN_VARIANT_TYPE_STRING:
        hr = StrAllocString(psczValue, pVariant->sczValue, 0);
        ExitOnFailure(hr, "Failed to copy value.");
        break;
    case BURN_VARIANT_TYPE_VERSION:
        hr = StrAllocFormatted(psczValue, L"%hu.%hu.%hu.%hu",
            (WORD)(pVariant->qwValue >> 48),
            (WORD)(pVariant->qwValue >> 32),
            (WORD)(pVariant->qwValue >> 16),
            (WORD)pVariant->qwValue);
        ExitOnFailure(hr, "Failed to convert version to string.");
        break;
    default:
        hr = E_INVALIDARG;
        break;
    }

LExit:
    return hr;
}

extern "C" HRESULT BVariantGetVersion(
    __in BURN_VARIANT* pVariant,
    __out DWORD64* pqwValue
    )
{
    HRESULT hr = S_OK;

    switch (pVariant->Type)
    {
    case BURN_VARIANT_TYPE_NUMERIC:
        *pqwValue = (DWORD64)pVariant->llValue;
        break;
    case BURN_VARIANT_TYPE_STRING:
        hr = FileVersionFromStringEx(pVariant->sczValue, 0, pqwValue);
        if (FAILED(hr))
        {
            hr = DISP_E_TYPEMISMATCH;
        }
        break;
    case BURN_VARIANT_TYPE_VERSION:
        *pqwValue = pVariant->qwValue;
        break;
    default:
        hr = E_INVALIDARG;
        break;
    }

    return hr;
}

extern "C" HRESULT BVariantSetNumeric(
    __in BURN_VARIANT* pVariant,
    __in LONGLONG llValue
    )
{
    HRESULT hr = S_OK;

    if (BURN_VARIANT_TYPE_STRING == pVariant->Type)
    {
        ReleaseStr(pVariant->sczValue);
    }
    memset(pVariant, 0, sizeof(BURN_VARIANT));
    pVariant->llValue = llValue;
    pVariant->Type = BURN_VARIANT_TYPE_NUMERIC;

    return hr;
}

extern "C" HRESULT BVariantSetString(
    __in BURN_VARIANT* pVariant,
    __in_z LPCWSTR wzValue,
    __in DWORD_PTR cchValue
    )
{
    HRESULT hr = S_OK;

    if (BURN_VARIANT_TYPE_STRING != pVariant->Type)
    {
        memset(pVariant, 0, sizeof(BURN_VARIANT));
    }
    hr = StrAllocString(&pVariant->sczValue, wzValue, cchValue);
    ExitOnFailure(hr, "Failed to copy string.");
    pVariant->Type = BURN_VARIANT_TYPE_STRING;

LExit:
    return hr;
}

extern "C" HRESULT BVariantSetVersion(
    __in BURN_VARIANT* pVariant,
    __in DWORD64 qwValue
    )
{
    HRESULT hr = S_OK;

    if (BURN_VARIANT_TYPE_STRING == pVariant->Type)
    {
        ReleaseStr(pVariant->sczValue);
    }
    memset(pVariant, 0, sizeof(BURN_VARIANT));
    pVariant->qwValue = qwValue;
    pVariant->Type = BURN_VARIANT_TYPE_VERSION;

    return hr;
}

extern "C" HRESULT BVariantCopy(
    __in BURN_VARIANT* pSource,
    __out BURN_VARIANT* pTarget
    )
{
    HRESULT hr = S_OK;

    // If the variant is changing types then reset the variant.
    if (pTarget->Type != pSource->Type)
    {
        BVariantUninitialize(pTarget);
    }

    switch (pSource->Type)
    {
    case BURN_VARIANT_TYPE_NONE:
        pTarget->Type = BURN_VARIANT_TYPE_NONE;
        break;
    case BURN_VARIANT_TYPE_NUMERIC:
        pTarget->llValue = pSource->llValue;
        pTarget->Type = BURN_VARIANT_TYPE_NUMERIC;
        break;
    case BURN_VARIANT_TYPE_STRING:
        hr = StrAllocString(&pTarget->sczValue, pSource->sczValue, 0);
        ExitOnFailure(hr, "Failed to copy variant value.");
        pTarget->Type = BURN_VARIANT_TYPE_STRING;
        break;
    case BURN_VARIANT_TYPE_VERSION:
        pTarget->qwValue = pSource->qwValue;
        pTarget->Type = BURN_VARIANT_TYPE_VERSION;
        break;
    default:
        hr = E_INVALIDARG;
    }

LExit:
    return hr;
}

extern "C" HRESULT BVariantChangeType(
    __in BURN_VARIANT* pVariant,
    __in BURN_VARIANT_TYPE type
    )
{
    HRESULT hr = S_OK;
    BURN_VARIANT variant = { };

    if (pVariant->Type == type)
    {
        ExitFunction(); // variant already is of the requested type
    }

    switch (type)
    {
    case BURN_VARIANT_TYPE_NONE:
        hr = S_OK;
        break;
    case BURN_VARIANT_TYPE_NUMERIC:
        hr = BVariantGetNumeric(pVariant, &variant.llValue);
        break;
    case BURN_VARIANT_TYPE_STRING:
        hr = BVariantGetString(pVariant, &variant.sczValue);
        break;
    case BURN_VARIANT_TYPE_VERSION:
        hr = BVariantGetVersion(pVariant, &variant.qwValue);
        break;
    default:
        ExitFunction1(hr = E_INVALIDARG);
    }
    ExitOnFailure(hr, "Failed to copy variant value.");
    variant.Type = type;

    BVariantUninitialize(pVariant);
    memcpy_s(pVariant, sizeof(BURN_VARIANT), &variant, sizeof(BURN_VARIANT));

LExit:
    return hr;
}
