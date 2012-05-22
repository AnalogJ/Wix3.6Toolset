//-------------------------------------------------------------------------------------------------
// <copyright file="precomp.h" company="Microsoft">
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
//    Precompiled header for theme viewer.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include <windows.h>
#include <gdiplus.h>
#include <msiquery.h>
#include <objbase.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <stdlib.h>
#include <strsafe.h>

#include "dutil.h"
#include "memutil.h"
#include "dirutil.h"
#include "fileutil.h"
#include "locutil.h"
#include "logutil.h"
#include "pathutil.h"
#include "resrutil.h"
#include "shelutil.h"
#include "strutil.h"
#include "thmutil.h"

#include "resource.h"

struct HANDLE_THEME
{
    DWORD cReferences;
    THEME* pTheme;
};

enum WM_THMVWR
{
    WM_THMVWR_SHOWPAGE = WM_APP,
    WM_THMVWR_PARSE_FILE,
    WM_THMVWR_NEW_THEME,
    WM_THMVWR_THEME_LOAD_ERROR,
};

extern "C" HRESULT DisplayStart(
    __in HINSTANCE hInstance,
    __in HWND hWnd,
    __out HANDLE *phThread,
    __out DWORD* pdwThreadId
    );
extern "C" HRESULT LoadStart(
    __in_z LPCWSTR wzThemePath,
    __in HWND hWnd,
    __out HANDLE* phThread
    );

extern "C" HRESULT AllocHandleTheme(
    __in THEME* pTheme,
    __out HANDLE_THEME** ppHandle
    );
extern "C" void IncrementHandleTheme(
    __in HANDLE_THEME* pHandle
    );
extern "C" void DecrementHandleTheme(
    __in HANDLE_THEME* pHandle
    );
