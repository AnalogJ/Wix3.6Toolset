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
//    Precompiled header for WiX CA
// </summary>
//-------------------------------------------------------------------------------------------------

#if _WIN32_MSI < 150
#define _WIN32_MSI 150
#endif

#include <windows.h>
#include <msiquery.h>
#include <msidefs.h>
#include <shlobj.h>
#include <richedit.h>
#include <msxml2.h>
#include <shobjidl.h>
#include <intshcut.h>
#include <sddl.h>

#include "wixstrsafe.h"
#include "wcautil.h"
#include "wcawow64.h"
#include "aclutil.h"
#include "dirutil.h"
#include "fileutil.h"
#include "memutil.h"
#include "pathutil.h"
#include "procutil.h"
#include "stierr.h"
#include "strutil.h"
#include "rmutil.h"
#include "xmlutil.h"
#include "wiutil.h"
#include "osutil.h"

#include "CustomMsiErrors.h"
#include "cost.h"
