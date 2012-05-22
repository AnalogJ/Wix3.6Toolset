#pragma once
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
//    dutil precompiled header.
// </summary>
//-------------------------------------------------------------------------------------------------

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#ifndef _WIN32_MSI
#define _WIN32_MSI 200
#endif

#define JET_VERSION 0x0501

#include <windows.h>
#include <windowsx.h>
#include <intsafe.h>
#include <strsafe.h>
#include <wininet.h>
#include <msi.h>
#include <msiquery.h>
#include <psapi.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <gdiplus.h>
#include <Tlhelp32.h>
#include <lm.h>
#include <Iads.h>
#include <activeds.h>
#include <richedit.h>
#include <stddef.h>
#include <esent.h>
#include <ahadmin.h>
#include <SRRestorePtAPI.h>
#include <userenv.h>
#include <WinIoCtl.h>
#include <wtsapi32.h>
#include <wuapi.h>
#include <commctrl.h>

#include "dutil.h"
#include "aclutil.h"
#include "atomutil.h"
#include "buffutil.h"
#include "cabcutil.h"
#include "cabutil.h"
#include "conutil.h"
#include "cryputil.h"
#include "eseutil.h"
#include "dirutil.h"
#include "fileutil.h"
#include "gdiputil.h"
#include "dictutil.h"
#include "iis7util.h"
#include "inetutil.h"
#include "iniutil.h"
#include "jsonutil.h"
#include "locutil.h"
#include "logutil.h"
#include "memutil.h"  // NOTE: almost everying is inlined so there is a small .cpp file
//#include "metautil.h" - see metautil.cpp why this *must* be commented out
#include "osutil.h"
#include "pathutil.h"
#include "perfutil.h"
#include "procutil.h"
#include "regutil.h"
#include "resrutil.h"
#include "reswutil.h"
#include "rmutil.h"
#include "rssutil.h"
#include "apuputil.h" // NOTE: this must come after atomutil.h and rssutil.h since it uses them.
//#include "sqlutil.h" - see sqlutil.cpp why this *must* be commented out
#include "shelutil.h"
#include "srputil.h"
#include "strutil.h"
#include "timeutil.h"
#include "timeutil.h"
#include "thmutil.h"
#include "uriutil.h"
#include "userutil.h"
#include <comutil.h>  // This header is needed for msxml2.h to compile correctly
#include <msxml2.h>   // This file is needed to include xmlutil.h
#include "wiutil.h"
#include "wuautil.h"
#include "xmlutil.h"

