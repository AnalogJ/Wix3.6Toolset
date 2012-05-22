// <copyright file="ClrLoader.cpp" company="Microsoft">
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
//  CLR Loader header.
// </summary>
//
#pragma once

extern "C" void WINAPI ClrLoaderInitialize();

extern "C" void WINAPI ClrLoaderUninitialize();

extern "C" HRESULT WINAPI ClrLoaderCreateInstance(
    __in_opt LPCWSTR wzClrVersion,
    __in LPCWSTR wzAssemblyName,
    __in LPCWSTR wzClassName,
    __in const IID &riid,
    __in void ** ppvObject
    );

extern "C" void WINAPI ClrLoaderDestroyInstance();
