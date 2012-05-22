//-------------------------------------------------------------------------------------------------
// <copyright file="container.h" company="Microsoft">
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


// typedefs

//typedef HRESULT (*PFN_EXTRACTOPEN)(
//    __in HANDLE hFile,
//    __in DWORD64 qwOffset,
//    __in DWORD64 qwSize,
//    __out void** ppCookie
//    );
//typedef HRESULT (*PFN_EXTRACTNEXTSTREAM)(
//    __in void* pCookie,
//    __inout_z LPWSTR* psczStreamName
//    );
//typedef HRESULT (*PFN_EXTRACTSTREAMTOFILE)(
//    __in void* pCookie,
//    __in_z LPCWSTR wzFileName
//    );
//typedef HRESULT (*PFN_EXTRACTSTREAMTOBUFFER)(
//    __in void* pCookie,
//    __out BYTE** ppbBuffer,
//    __out SIZE_T* pcbBuffer
//    );
//typedef HRESULT (*PFN_EXTRACTCLOSE)(
//    __in void* pCookie
//    );


// constants

enum BURN_CONTAINER_TYPE
{
    BURN_CONTAINER_TYPE_NONE,
    BURN_CONTAINER_TYPE_CABINET,
    BURN_CONTAINER_TYPE_SEVENZIP,
};

enum BURN_CAB_OPERATION
{
    BURN_CAB_OPERATION_NONE,
    BURN_CAB_OPERATION_NEXT_STREAM,
    BURN_CAB_OPERATION_STREAM_TO_FILE,
    BURN_CAB_OPERATION_STREAM_TO_BUFFER,
    BURN_CAB_OPERATION_SKIP_STREAM,
    BURN_CAB_OPERATION_CLOSE,
};


// structs

typedef struct _BURN_DOWNLOAD_SOURCE
{
    LPWSTR sczUrl;
    LPWSTR sczUser;
    LPWSTR sczPassword;
} BURN_DOWNLOAD_SOURCE;

typedef struct _BURN_CONTAINER
{
    LPWSTR sczId;
    BURN_CONTAINER_TYPE type;
    BOOL fPrimary;
    BOOL fAttached;
    DWORD dwAttachedIndex;
    DWORD64 qwFileSize;
    LPWSTR sczHash;
    LPWSTR sczFilePath;         // relative path to container.
    LPWSTR sczSourcePath;
    BURN_DOWNLOAD_SOURCE downloadSource;

    DWORD64 qwAttachedOffset;
    BOOL fActuallyAttached;     // indicates whether an attached container is attached or missing.

    //LPWSTR* rgsczPayloads;
    //DWORD cPayloads;
} BURN_CONTAINER;

typedef struct _BURN_CONTAINERS
{
    BURN_CONTAINER* rgContainers;
    DWORD cContainers;
} BURN_CONTAINERS;

typedef struct _BURN_CONTAINER_CONTEXT
{
    HANDLE hFile;
    DWORD64 qwOffset;
    DWORD64 qwSize;

    //PFN_EXTRACTOPEN pfnExtractOpen;
    //PFN_EXTRACTNEXTSTREAM pfnExtractNextStream;
    //PFN_EXTRACTSTREAMTOFILE pfnExtractStreamToFile;
    //PFN_EXTRACTSTREAMTOBUFFER pfnExtractStreamToBuffer;
    //PFN_EXTRACTCLOSE pfnExtractClose;
    //void* pCookie;
    BURN_CONTAINER_TYPE type;
    union
    {
        struct
        {
            LPWSTR sczFile;

            HANDLE hThread;
            HANDLE hBeginOperationEvent;
            HANDLE hOperationCompleteEvent;

            BURN_CAB_OPERATION operation;
            HRESULT hrError;

            LPWSTR* psczStreamName;
            LPCWSTR wzTargetFile;
            HANDLE hTargetFile;
            BYTE* pbTargetBuffer;
            DWORD cbTargetBuffer;
            DWORD iTargetBuffer;
        } Cabinet;
    };

} BURN_CONTAINER_CONTEXT;


// functions

HRESULT ContainersParseFromXml(
    __in BURN_SECTION* pSection,
    __in BURN_CONTAINERS* pContainers,
    __in IXMLDOMNode* pixnBundle
    );
void ContainersUninitialize(
    __in BURN_CONTAINERS* pContainers
    );
HRESULT ContainerOpenUX(
    __in BURN_SECTION* pSection,
    __in BURN_CONTAINER_CONTEXT* pContext
    );
HRESULT ContainerOpen(
    __in BURN_CONTAINER_CONTEXT* pContext,
    __in BURN_CONTAINER* pContainer,
    __in_z LPCWSTR wzFilePath
    );
HRESULT ContainerNextStream(
    __inout BURN_CONTAINER_CONTEXT* pContext,
    __inout_z LPWSTR* psczStreamName
    );
HRESULT ContainerStreamToFile(
    __in BURN_CONTAINER_CONTEXT* pContext,
    __in_z LPCWSTR wzFileName
    );
HRESULT ContainerStreamToBuffer(
    __in BURN_CONTAINER_CONTEXT* pContext,
    __out BYTE** ppbBuffer,
    __out SIZE_T* pcbBuffer
    );
HRESULT ContainerSkipStream(
    __in BURN_CONTAINER_CONTEXT* pContext
    );
HRESULT ContainerClose(
    __in BURN_CONTAINER_CONTEXT* pContext
    );
HRESULT ContainerFindById(
    __in BURN_CONTAINERS* pContainers,
    __in_z LPCWSTR wzId,
    __out BURN_CONTAINER** ppContainer
    );


#if defined(__cplusplus)
}
#endif
