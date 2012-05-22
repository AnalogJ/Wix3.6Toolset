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

#include <windows.h>
#include <msiquery.h>
#include <strsafe.h>
#include <mscoree.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <shlwapi.h>
#include <sys/stat.h>
#include <malloc.h>
#include <fdi.h>
#include <msiquery.h>
#import <mscorlib.tlb> raw_interfaces_only rename("ReportEvent", "CorReportEvent")
using namespace mscorlib;
