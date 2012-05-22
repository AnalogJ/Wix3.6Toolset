// <copyright file="ClassFactory.cpp" company="Microsoft">
//  Copyright (c) Microsoft Corporation.  All rights reserved.
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
//  Class Factory implementation.
// </summary>
//
#include "precomp.h"


class CClassFactory : public IClassFactory
{
public:
    CClassFactory() : m_cReferences(1)
    {
    }

    ~CClassFactory()
    {
    }

    HRESULT __stdcall QueryInterface(
        __in const IID& riid,
        __out void** ppvObject
        )
    {
        HRESULT hr = S_OK;

        ExitOnNull(ppvObject, hr, E_INVALIDARG, "Invalid argument ppvObject");
        *ppvObject = NULL;

       if (::IsEqualIID(IID_IClassFactory, riid))
       {
             *ppvObject = static_cast<IClassFactory*>(this);
       }
       else if (::IsEqualIID(IID_IUnknown, riid))
       {
            *ppvObject = static_cast<IUnknown*>(this);
       }
       else // no interface for requested iid
       {
            ExitFunction1(hr = E_NOINTERFACE);
       }

        AddRef();
    LExit:
        return hr;
    }


    ULONG __stdcall AddRef()
    {
        return InterlockedIncrement(&m_cReferences);
    }


    ULONG __stdcall Release()
    {
        long l = InterlockedDecrement(&m_cReferences);
        if (0 < l)
        {
            return l;
        }

        delete this;
        return 0;
    }

    HRESULT __stdcall CreateInstance(
        __in IUnknown* punkOuter,
        __in const IID& riid,
        __out void** ppvObject
        )
    {
        HRESULT hr = S_OK;
        CConnectProxy* pConnectProxy = NULL;

        ExitOnNull(ppvObject, hr, E_INVALIDARG, "Invalid ppvObject passed in.");
        *ppvObject = NULL;

        if (punkOuter)
        {
            hr = CLASS_E_NOAGGREGATION;
            ExitOnFailure(hr, "Aggregation not supported.");
        }

        hr = CConnectProxy::Create(&pConnectProxy);
        ExitOnFailure(hr, "Failed to create proxy object.");

        hr = pConnectProxy->QueryInterface(riid, ppvObject);
        ExitOnFailure(hr, "Failed to query interface on newly created proxy object.");

    LExit:
        ReleaseObject(pConnectProxy);
        return hr;
    }


    HRESULT __stdcall LockServer(
        __in BOOL bLock
        )
    {
        // have the lock look like an outstanding instance
        if (bLock)
        {
        }
        else
        {
        }

        return S_OK;
    }

private:
    long m_cReferences;
};


extern "C" HRESULT CreateClassFactory(
    __out IClassFactory** ppClassFactory
    )
{
    Assert(ppClassFactory);

    HRESULT hr = S_OK;
    CClassFactory* pFactory = NULL;

    pFactory = new CClassFactory();
    ExitOnNull(pFactory, hr, E_OUTOFMEMORY, "Failed to allocate a new CClassFactory.");

    *ppClassFactory = static_cast<IClassFactory*>(pFactory);
    pFactory = NULL;

LExit:
    ReleaseObject(pFactory);

    return hr;
}
