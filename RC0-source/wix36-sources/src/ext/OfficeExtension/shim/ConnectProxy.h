// <copyright file="ConnectProxy.h" company="Microsoft">
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
//  Connection proxy header.
// </summary>
//
#pragma once

class CConnectProxy : public CUnknownImpl<IDTExtensibility2>
{
public:
    static HRESULT Create(CConnectProxy** ppNewProxy);

public: // IDispatch
    HRESULT __stdcall GetTypeInfoCount(
        __out unsigned int FAR* pctinfo
        );

    HRESULT __stdcall GetTypeInfo(
        __in unsigned int iTInfo,
        __in LCID lcid,
        __out ITypeInfo FAR* FAR*  ppTInfo
        );

    HRESULT __stdcall GetIDsOfNames(
        __in REFIID  riid, 
        __in OLECHAR FAR* FAR*  rgszNames,
        __in unsigned int  cNames,
        __in LCID lcid,
        __in DISPID FAR*  rgDispId
        );

    HRESULT __stdcall Invoke(
        __in DISPID dispIdMember,
        __in REFIID riid,
        __in LCID lcid,
        __in WORD wFlags,
        __in DISPPARAMS FAR* pDispParams,
        __out VARIANT FAR* pVarResult,
        __out EXCEPINFO FAR* pExcepInfo,
        __out unsigned int FAR* puArgErr
        );

public: // IDTExtensibility2
    HRESULT __stdcall OnConnection(
        __in IDispatch* Application,
        __in ext_ConnectMode ConnectMode,
        __in IDispatch* AddInInst,
        __in SAFEARRAY** custom
        );

    HRESULT __stdcall OnDisconnection(
        __in ext_DisconnectMode RemoveMode,
        __in SAFEARRAY** custom
        );
    
    HRESULT __stdcall OnAddInsUpdate(
        __in SAFEARRAY** custom
        );

    HRESULT __stdcall OnStartupComplete(
        __in SAFEARRAY** custom
        );

    HRESULT __stdcall OnBeginShutdown(
        __in SAFEARRAY** custom
        );

protected:
    CConnectProxy();

    ~CConnectProxy();

    HRESULT Initialize();

protected:
    LPWSTR m_pwzAppId;
    ITypeInfo* m_pTypeInfo;

    BOOL m_fInstanceCreated;
    IDTExtensibility2* m_pConnect; // cached pointer to managed add-in
};
