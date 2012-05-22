//-------------------------------------------------------------------------------------------------
// <copyright file="downloadengine.cpp" company="Microsoft">
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
//    Setup chainer/bootstrapper download engine for WiX toolset.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"
#include <wininet.h>
#include <inetutil.h>
#include <uriutil.h>

static const DWORD64 BURN_DOWNLOAD_ENGINE_TWO_GIGABYTES = DWORD64(2) * 1024 * 1024 * 1024;
static LPCWSTR BURN_DOWNLOAD_ENGINE_ACCEPT_TYPES[] = { L"*/*", NULL };


// internal function declarations

static HRESULT InitializeResume(
    __in LPCWSTR wzDestinationPath,
    __out LPWSTR* psczResumePath,
    __out HANDLE* phResumeFile,
    __out DWORD64* pdw64ResumeOffset
    );
static HRESULT GetResourceMetadata(
    __in HINTERNET hSession,
    __inout_z LPWSTR* psczUrl,
    __in_z_opt LPCWSTR wzUser,
    __in_z_opt LPCWSTR wzPassword,
    __out DWORD64* pdw64ResourceSize,
    __out FILETIME* pftResourceCreated
    );
static HRESULT DownloadResource(
    __in HINTERNET hSession,
    __inout_z LPWSTR* psczUrl,
    __in_z_opt LPCWSTR wzUser,
    __in_z_opt LPCWSTR wzPassword,
    __in_z LPCWSTR wzDestinationPath,
    __in DWORD64 dw64AuthoredResourceLength,
    __in DWORD64 dw64ResourceLength,
    __in DWORD64 dw64ResumeOffset,
    __in HANDLE hResumeFile,
    __in BURN_CACHE_CALLBACK* pCallback
    );
static HRESULT AllocateRangeRequestHeader(
    __in DWORD64 dw64ResumeOffset,
    __in DWORD64 dw64ResourceLength,
    __deref_out_z LPWSTR* psczHeader
    );
static HRESULT WriteToFile(
    __in HINTERNET hUrl,
    __in HANDLE hPayloadFile,
    __inout DWORD64* pdw64ResumeOffset,
    __in HANDLE hResumeFile,
    __in DWORD64 dw64ResourceLength,
    __in LPBYTE pbData,
    __in DWORD cbData,
    __in BURN_CACHE_CALLBACK* pCallback
    );
static HRESULT UpdateResumeOffset(
    __inout DWORD64* pdw64ResumeOffset,
    __in HANDLE hResumeFile,
    __in DWORD cbData
    );
static HRESULT MakeRequest(
    __in HINTERNET hSession,
    __inout_z LPWSTR* psczSourceUrl,
    __in_z_opt LPCWSTR wzMethod,
    __in_z_opt LPCWSTR wzHeaders,
    __in_z_opt LPCWSTR wzUser,
    __in_z_opt LPCWSTR wzPassword,
    __out HINTERNET* phConnect,
    __out HINTERNET* phUrl,
    __out BOOL* pfRangeRequestsAccepted
    );
static HRESULT OpenRequest(
    __in HINTERNET hConnect,
    __in_z_opt LPCWSTR wzMethod,
    __in INTERNET_SCHEME scheme,
    __in_z LPCWSTR wzResource,
    __in_z_opt LPCWSTR wzQueryString,
    __in_z_opt LPCWSTR wzHeader,
    __out HINTERNET* phUrl
    );
static HRESULT SendRequest(
    __in HINTERNET hUrl,
    __inout_z LPWSTR* psczUrl,
    __out BOOL* pfRetry,
    __out BOOL* pfRangesAccepted
    );


// function definitions

extern "C" HRESULT WininetDownloadUrl(
    __in BURN_CACHE_CALLBACK* pCallback,
    __in BURN_DOWNLOAD_SOURCE* pDownloadSource,
    __in DWORD64 dw64AuthoredDownloadSize,
    __in LPCWSTR wzDestinationPath
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczUrl = NULL;
    HINTERNET hSession = NULL;
    LPWSTR sczResumePath = NULL;
    HANDLE hResumeFile = INVALID_HANDLE_VALUE;
    DWORD64 dw64ResumeOffset = 0;
    DWORD64 dw64Size = 0;
    FILETIME ftCreated = { };

    // Copy the download source into a working variable to handle redirects then
    // open the internet session.
    hr = StrAllocString(&sczUrl, pDownloadSource->sczUrl, 0);
    ExitOnFailure(hr, "Failed to copy download source URL.");

    hSession = ::InternetOpenW(L"Burn", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    ExitOnNullWithLastError(hSession, hr, "Failed to open internet session");

    // Get the resource size and creation time from the internet.
    hr = GetResourceMetadata(hSession, &sczUrl, pDownloadSource->sczUser, pDownloadSource->sczPassword, &dw64Size, &ftCreated);
    ExitOnFailure1(hr, "Failed to get size and time for URL: %ls", sczUrl);

    // Ignore failure to initialize resume because we will fall back to full download then
    // download.
    InitializeResume(wzDestinationPath, &sczResumePath, &hResumeFile, &dw64ResumeOffset);

    hr = DownloadResource(hSession, &sczUrl, pDownloadSource->sczUser, pDownloadSource->sczPassword, wzDestinationPath, dw64AuthoredDownloadSize, dw64Size, dw64ResumeOffset, hResumeFile, pCallback);
    ExitOnFailure1(hr, "Failed to download URL: %ls", sczUrl);

    // Cleanup the resume file because we successfully downloaded the whole file.
    if (sczResumePath && *sczResumePath)
    {
        ::DeleteFileW(sczResumePath);
    }

LExit:
    ReleaseFileHandle(hResumeFile);
    ReleaseStr(sczResumePath);
    ReleaseInternet(hSession);
    ReleaseStr(sczUrl);

    return hr;
}


// internal helper functions

static HRESULT InitializeResume(
    __in LPCWSTR wzDestinationPath,
    __out LPWSTR* psczResumePath,
    __out HANDLE* phResumeFile,
    __out DWORD64* pdw64ResumeOffset
    )
{
    HRESULT hr = S_OK;
    HANDLE hResumeFile = INVALID_HANDLE_VALUE;
    DWORD cbTotalReadResumeData = 0;
    DWORD cbReadData = 0;

    *pdw64ResumeOffset = 0;

    hr = CacheGetResumePath(wzDestinationPath, psczResumePath);
    ExitOnFailure1(hr, "Failed to calculate resume path from working path: %ls", wzDestinationPath);

    hResumeFile = ::CreateFileW(*psczResumePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == hResumeFile)
    {
        ExitWithLastError1(hr, "Failed to create resume file: %ls", *psczResumePath);
    }

    do
    {
        if (!::ReadFile(hResumeFile, reinterpret_cast<BYTE*>(pdw64ResumeOffset) + cbTotalReadResumeData, sizeof(DWORD64) - cbTotalReadResumeData, &cbReadData, NULL))
        {
            ExitWithLastError1(hr, "Failed to read resume file: %ls", *psczResumePath);
        }
        cbTotalReadResumeData += cbReadData;
    } while (cbReadData && sizeof(DWORD64) > cbTotalReadResumeData);

    // Start over if we couldn't get a resume offset.
    if (cbTotalReadResumeData != sizeof(DWORD64))
    {
        *pdw64ResumeOffset = 0;
    }

    *phResumeFile = hResumeFile;
    hResumeFile = INVALID_HANDLE_VALUE;

LExit:
    ReleaseFileHandle(hResumeFile);
    return hr;
}

static HRESULT GetResourceMetadata(
    __in HINTERNET hSession,
    __inout_z LPWSTR* psczUrl,
    __in_z_opt LPCWSTR wzUser,
    __in_z_opt LPCWSTR wzPassword,
    __out DWORD64* pdw64ResourceSize,
    __out FILETIME* pftResourceCreated
    )
{
    HRESULT hr = S_OK;
    BOOL fRangeRequestsAccepted = TRUE;
    HINTERNET hConnect = NULL;
    HINTERNET hUrl = NULL;
    LONGLONG llLength = 0;

    hr = MakeRequest(hSession, psczUrl, L"HEAD", NULL, wzUser, wzPassword, &hConnect, &hUrl, &fRangeRequestsAccepted);
    ExitOnFailure1(hr, "Failed to connect to URL: %ls", *psczUrl);

    hr = InternetGetSizeByHandle(hUrl, &llLength);
    if (FAILED(hr))
    {
        llLength = 0;
        hr = S_OK;
    }

    *pdw64ResourceSize = llLength;

    // Get the last modified time from the server, we'll use that as our downloaded time here. If
    // the server time isn't available then use the local system time.
    hr = InternetGetCreateTimeByHandle(hUrl, pftResourceCreated);
    if (FAILED(hr))
    {
        ::GetSystemTimeAsFileTime(pftResourceCreated);
        hr = S_OK;
    }

LExit:
    ReleaseInternet(hUrl);
    ReleaseInternet(hConnect);
    return hr;
}

static HRESULT DownloadResource(
    __in HINTERNET hSession,
    __inout_z LPWSTR* psczUrl,
    __in_z_opt LPCWSTR wzUser,
    __in_z_opt LPCWSTR wzPassword,
    __in_z LPCWSTR wzDestinationPath,
    __in DWORD64 dw64AuthoredResourceLength,
    __in DWORD64 dw64ResourceLength,
    __in DWORD64 dw64ResumeOffset,
    __in HANDLE hResumeFile,
    __in_opt BURN_CACHE_CALLBACK* pCallback
    )
{
    HRESULT hr = S_OK;
    HANDLE hPayloadFile = INVALID_HANDLE_VALUE;
    DWORD cbMaxData = 64 * 1024; // 64 KB
    BYTE* pbData = NULL;
    BOOL fRangeRequestsAccepted = TRUE;
    LPWSTR sczRangeRequestHeader = NULL;
    HINTERNET hConnect = NULL;
    HINTERNET hUrl = NULL;
    LONGLONG llLength = 0;

    hPayloadFile = ::CreateFileW(wzDestinationPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == hPayloadFile)
    {
        ExitWithLastError1(hr, "Failed to create download destination file: %ls", wzDestinationPath);
    }

    // Allocate a memory block on a page boundary in case we want to do optimal writing.
    pbData = static_cast<BYTE*>(::VirtualAlloc(NULL, cbMaxData, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
    ExitOnNullWithLastError(pbData, hr, "Failed to allocate buffer to download files into.");

    // Let's try downloading the file assuming that range requests are accepted. If range requests
    // are not supported we'll have to start over and accept the fact that we only get one shot
    // downloading the file however big it is. Hopefully, not more than 2 GB since wininet doesn't
    // like files that big.
    while (fRangeRequestsAccepted && (0 == dw64ResourceLength || dw64ResumeOffset < dw64ResourceLength))
    {
        hr = AllocateRangeRequestHeader(dw64ResumeOffset, 0 == dw64ResourceLength ? dw64AuthoredResourceLength : dw64ResourceLength, &sczRangeRequestHeader);
        ExitOnFailure(hr, "Failed to allocate range request header.");

        ReleaseNullInternet(hConnect);
        ReleaseNullInternet(hUrl);

        hr = MakeRequest(hSession, psczUrl, L"GET", sczRangeRequestHeader, wzUser, wzPassword, &hConnect, &hUrl, &fRangeRequestsAccepted);
        ExitOnFailure1(hr, "Failed to request URL for download: %ls", *psczUrl);

        // If we didn't get the size of the resource from the initial "HEAD" request
        // then let's try to get the size from this "GET" request.
        if (0 == dw64ResourceLength)
        {
            hr = InternetGetSizeByHandle(hUrl, &llLength);
            if (SUCCEEDED(hr))
            {
                dw64ResourceLength = llLength;
            }
            else // server didn't tell us the resource length.
            {
                // Fallback to the authored size of the resource. However, since we
                // don't really know the size on the server, don't try to use
                // range requests either.
                dw64ResourceLength = dw64AuthoredResourceLength;
                fRangeRequestsAccepted = FALSE;
            }
        }

        // If we just tried to do a range request and found out that it isn't supported, start over.
        if (!fRangeRequestsAccepted)
        {
            // TODO: log a message that the server did not accept range requests.
            dw64ResumeOffset = 0;
        }

        hr = WriteToFile(hUrl, hPayloadFile, &dw64ResumeOffset, hResumeFile, dw64ResourceLength, pbData, cbMaxData, pCallback);
        ExitOnFailure1(hr, "Failed while reading from internet and writing to: %ls", wzDestinationPath);
    }

LExit:
    ReleaseInternet(hUrl);
    ReleaseInternet(hConnect);
    ReleaseStr(sczRangeRequestHeader);
    if (pbData)
    {
        ::VirtualFree(pbData, 0, MEM_RELEASE);
    }
    ReleaseFileHandle(hPayloadFile);

    return hr;
}

static HRESULT AllocateRangeRequestHeader(
    __in DWORD64 dw64ResumeOffset,
    __in DWORD64 dw64ResourceLength,
    __deref_out_z LPWSTR* psczHeader
    )
{
    HRESULT hr = S_OK;

    // If the remaining length is less that 2GB we'll be able to ask for everything.
    DWORD64 dw64RemainingLength = dw64ResourceLength - dw64ResumeOffset;
    if (BURN_DOWNLOAD_ENGINE_TWO_GIGABYTES > dw64RemainingLength)
    {
        // If we have a resume offset, let's download everything from there. Otherwise, we'll
        // just get everything with no headers in the way.
        if (0 < dw64ResumeOffset)
        {
            hr = StrAllocFormatted(psczHeader, L"Range: bytes=%I64u-", dw64ResumeOffset);
            ExitOnFailure(hr, "Failed to add range read header.");
        }
        else
        {
            ReleaseNullStr(*psczHeader);
        }
    }
    else // we'll have to download in chunks.
    {
        hr = StrAllocFormatted(psczHeader, L"Range: bytes=%I64u-%I64u", dw64ResumeOffset, dw64ResumeOffset + dw64RemainingLength - 1);
        ExitOnFailure(hr, "Failed to add range read header.");
    }

LExit:
    return hr;
}

static HRESULT WriteToFile(
    __in HINTERNET hUrl,
    __in HANDLE hPayloadFile,
    __inout DWORD64* pdw64ResumeOffset,
    __in HANDLE hResumeFile,
    __in DWORD64 dw64ResourceLength,
    __in LPBYTE pbData,
    __in DWORD cbData,
    __in_opt BURN_CACHE_CALLBACK* pCallback
    )
{
    HRESULT hr = S_OK;
    DWORD cbReadData = 0;

    hr = FileSetPointer(hPayloadFile, *pdw64ResumeOffset, NULL, FILE_BEGIN);
    ExitOnFailure(hr, "Failed to seek to start point in file.");

    do
    {
        // Read bits from the internet.
        if (!::InternetReadFile(hUrl, static_cast<void*>(pbData), cbData, &cbReadData))
        {
            ExitWithLastError(hr, "Failed while reading from internet.");
        }

        // Write bits to disk (if there are any).
        if (cbReadData)
        {
            DWORD cbTotalWritten = 0;
            DWORD cbWritten = 0;
            do
            {
                if (!::WriteFile(hPayloadFile, pbData + cbTotalWritten, cbReadData - cbTotalWritten, &cbWritten, NULL))
                {
                    ExitWithLastError(hr, "Failed to write data from internet.");
                }

                cbTotalWritten += cbWritten;
            } while (cbWritten && cbTotalWritten < cbReadData);

            // Ignore failure from updating resume file as this doesn't mean the download cannot succeed.
            UpdateResumeOffset(pdw64ResumeOffset, hResumeFile, cbTotalWritten);

            if (pCallback && pCallback->pfnProgress)
            {
                hr = CacheSendProgressCallback(pCallback, *pdw64ResumeOffset, dw64ResourceLength, hPayloadFile);
                ExitOnFailure(hr, "UX aborted on cache progress.");
            }
        }
    } while (cbReadData);

LExit:
    return hr;
}

static HRESULT UpdateResumeOffset(
    __inout DWORD64* pdw64ResumeOffset,
    __in HANDLE hResumeFile,
    __in DWORD cbData
    )
{
    HRESULT hr = S_OK;

    *pdw64ResumeOffset += cbData;

    if (INVALID_HANDLE_VALUE != hResumeFile)
    {
        DWORD cbTotalWrittenResumeData = 0;
        DWORD cbWrittenResumeData = 0;

        hr = FileSetPointer(hResumeFile, 0, NULL, FILE_BEGIN);
        ExitOnFailure(hr, "Failed to seek to start point in file.");

        do
        {
            // Ignore failure to write to the resume file as that should not prevent the download from happening.
            if (!::WriteFile(hResumeFile, pdw64ResumeOffset + cbTotalWrittenResumeData, sizeof(DWORD64) - cbTotalWrittenResumeData, &cbWrittenResumeData, NULL))
            {
                ExitOnFailure(hr, "Failed to seek to write to file.");
            }

            cbTotalWrittenResumeData += cbWrittenResumeData;
        } while (cbWrittenResumeData && sizeof(DWORD64) > cbTotalWrittenResumeData);
    }

LExit:
    return hr;
}

static HRESULT MakeRequest(
    __in HINTERNET hSession,
    __inout_z LPWSTR* psczSourceUrl,
    __in_z_opt LPCWSTR wzMethod,
    __in_z_opt LPCWSTR wzHeaders,
    __in_z_opt LPCWSTR wzUser,
    __in_z_opt LPCWSTR wzPassword,
    __out HINTERNET* phConnect,
    __out HINTERNET* phUrl,
    __out BOOL* pfRangeRequestsAccepted
    )
{
    HRESULT hr = S_OK;
    HINTERNET hConnect = NULL;
    HINTERNET hUrl = NULL;
    URI_INFO uri = { };

    // Try to open the URL.
    BOOL fRetry;
    do
    {
        fRetry = FALSE;

        // If the URL was opened close it, so we can reopen it again.
        ReleaseInternet(hUrl);
        ReleaseInternet(hConnect);

        // Open the url.
        hr = UriCrackEx(*psczSourceUrl, &uri);
        ExitOnFailure(hr, "Failed to break URL into server and resource parts.");

        hConnect = ::InternetConnectW(hSession, uri.sczHostName, uri.port, (wzUser && *wzUser) ? wzUser : uri.sczUser, (wzPassword && *wzPassword) ? wzPassword : uri.sczPassword, INTERNET_SCHEME_FTP == uri.scheme ? INTERNET_SERVICE_FTP : INTERNET_SERVICE_HTTP, 0, 0);
        ExitOnNullWithLastError1(hConnect, hr, "Failed to connect to URL: %ls", *psczSourceUrl);

        // Best effort set the proxy username and password, if they were provided.
        if ((wzUser && *wzUser) && (wzPassword && *wzPassword))
        {
            if (::InternetSetOptionW(hConnect, INTERNET_OPTION_PROXY_USERNAME, (LPVOID)wzUser, lstrlenW(wzUser)))
            {
                ::InternetSetOptionW(hConnect, INTERNET_OPTION_PROXY_PASSWORD, (LPVOID)wzPassword, lstrlenW(wzPassword));
            }
        }

        hr = OpenRequest(hConnect, wzMethod, uri.scheme, uri.sczPath, uri.sczQueryString, wzHeaders, &hUrl);
        ExitOnFailure1(hr, "Failed to open internet URL: %ls", *psczSourceUrl);

        hr = SendRequest(hUrl, psczSourceUrl, &fRetry, pfRangeRequestsAccepted);
        ExitOnFailure1(hr, "Failed to send request to URL: %ls", *psczSourceUrl);
    } while (fRetry);

    // Okay, we're all ready to start downloading. Update the connection information.
    *phConnect = hConnect;
    hConnect = NULL;
    *phUrl = hUrl;
    hUrl = NULL;

LExit:
    UriInfoUninitialize(&uri);
    ReleaseInternet(hUrl);
    ReleaseInternet(hConnect);

    return hr;
}

static HRESULT OpenRequest(
    __in HINTERNET hConnect,
    __in_z_opt LPCWSTR wzMethod,
    __in INTERNET_SCHEME scheme,
    __in_z LPCWSTR wzResource,
    __in_z_opt LPCWSTR wzQueryString,
    __in_z_opt LPCWSTR wzHeader,
    __out HINTERNET* phUrl
    )
{
    HRESULT hr = S_OK;
    DWORD dwRequestFlags = INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_NO_UI | INTERNET_FLAG_RELOAD;
    LPWSTR sczResource = NULL;
    HINTERNET hUrl = NULL;

    if (INTERNET_SCHEME_HTTPS == scheme)
    {
        dwRequestFlags |= INTERNET_FLAG_SECURE;
    }

    // Allocate the resource name.
    hr = StrAllocString(&sczResource, wzResource, 0);
    ExitOnFailure(hr, "Failed to allocate string for resource URI.");

    if (wzQueryString && *wzQueryString)
    {
        hr = StrAllocConcat(&sczResource, wzQueryString, 0);
        ExitOnFailure(hr, "Failed to append query strong to resource from URI.");
    }

    // Open the request and add the header if provided.
    hUrl = ::HttpOpenRequestW(hConnect, wzMethod, sczResource, NULL, NULL, BURN_DOWNLOAD_ENGINE_ACCEPT_TYPES, dwRequestFlags, NULL);
    ExitOnNullWithLastError(hUrl, hr, "Failed to open internet request.");

    if (wzHeader && *wzHeader)
    {
        if (!::HttpAddRequestHeadersW(hUrl, wzHeader, static_cast<DWORD>(-1), HTTP_ADDREQ_FLAG_COALESCE))
        {
            ExitWithLastError(hr, "Failed to add header to HTTP request.");
        }
    }

    *phUrl = hUrl;
    hUrl = NULL;

LExit:
    ReleaseInternet(hUrl);
    ReleaseStr(sczResource);
    return hr;
}

static HRESULT SendRequest(
    __in HINTERNET hUrl,
    __inout_z LPWSTR* psczUrl,
    __out BOOL* pfRetry,
    __out BOOL* pfRangesAccepted
    )
{
    HRESULT hr = S_OK;
    LONG lCode = 0;

    if (!::HttpSendRequestW(hUrl, NULL, 0, NULL, 0))
    {
        ExitWithLastError1(hr, "Failed to send request to URL: %ls", *psczUrl);
    }

    // Check the http status code.
    hr = InternetQueryInfoNumber(hUrl, HTTP_QUERY_STATUS_CODE, &lCode);
    ExitOnFailure1(hr, "Failed to get HTTP status code for URL: %ls", *psczUrl);

    switch (lCode)
    {
    case 200: // OK but range requests don't work.
        *pfRangesAccepted = FALSE;
        hr = S_OK;
        break;

    case 206: // Partial content means that range requests work!
        *pfRangesAccepted = TRUE;
        hr = S_OK;
        break;

    // redirection cases
    case 301: __fallthrough; // file moved
    case 302: __fallthrough; // temporary
    case 303: // redirect method
        hr = InternetQueryInfoString(hUrl, HTTP_QUERY_CONTENT_LOCATION, psczUrl);
        ExitOnFailure1(hr, "Failed to get redirect url: %ls", *psczUrl);

        *pfRetry = TRUE;
        break;

    // error cases
    case 400: // bad request
        hr = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);
        break;

    case 401: __fallthrough; // unauthorized
    case 407: __fallthrough; // proxy unauthorized
    case 403: // access denied
        hr = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
        break;

    case 404: // file not found
    case 410: // gone
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        break;

    case 405: // method not allowed
        hr = HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
        break;

    case 408: __fallthrough; // request timedout
    case 504: // gateway timeout
        hr = HRESULT_FROM_WIN32(WAIT_TIMEOUT);
        break;

    case 414: // request URI too long
        hr = CO_E_PATHTOOLONG;
        break;

    case 502: __fallthrough; // server (through a gateway) was not found
    case 503: // server unavailable
        hr = HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);
        break;

    case 418: // I'm a teapot.
    default:
        {
            hr = E_UNEXPECTED;
#ifdef DEBUG
            CHAR sz[INTERNET_MAX_URL_LENGTH];
            StringCchPrintfA(sz, countof(sz), "unhandled HTTP status %d, unknown status code for URL: %ls", lCode, *psczUrl);
            AssertSz(FALSE, sz);
#endif
        }
        break;
    }

LExit:
    return hr;
}
