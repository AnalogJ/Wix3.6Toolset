//-------------------------------------------------------------------------------------------------
// <copyright file="precomp.h" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
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
// Pre-compiled header.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include <windows.h>
#include <msiquery.h>
#include <mscoree.h>

#import <mscorlib.tlb> raw_interfaces_only rename("ReportEvent", "mscorlib_ReportEvent")

#include <dutil.h>
#include <pathutil.h>
#include <regutil.h>
#include <strutil.h>
#include <xmlutil.h>

#include "IBootstrapperEngine.h"
#include "IBootstrapperApplication.h"
#include "IBootstrapperApplicationFactory.h"

#include "balutil.h"
