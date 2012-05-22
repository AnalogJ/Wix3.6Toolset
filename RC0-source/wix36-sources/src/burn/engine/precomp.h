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
//    Precompiled header for setup chainer/bootstrapper core.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


// In "development builds" have all the ExitXxx macros log.
#ifdef BURN_DEVELOPMENT_BUILD
#define ExitTrace LogErrorString
#define ExitTrace1 LogErrorString
#define ExitTrace2 LogErrorString
#define ExitTrace3 LogErrorString
#endif

#include <wixver.h>

#include <windows.h>
#include <aclapi.h>
#include <Bits.h>
#include <math.h>
#include <msiquery.h>
#include <sddl.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <softpub.h>
#include <strsafe.h>
#include <intsafe.h>
#include <mscat.h>
#include <lmcons.h>

#include <dutil.h>
#include <aclutil.h>
#include <buffutil.h>
#include <cabutil.h>
#include <certutil.h>
#include <cryputil.h>
#include <dirutil.h>
#include <fileutil.h>
#include <logutil.h>
#include <memutil.h>
#include <osutil.h>
#include <pathutil.h>
#include <procutil.h>
#include <regutil.h>
#include <resrutil.h>
#include <shelutil.h>
#include <srputil.h>
#include <strutil.h>
#include <svcutil.h>
#include <userutil.h>
#include <wiutil.h>
#include <wuautil.h>
#include <xmlutil.h>
#include <dictutil.h>
#include <deputil.h>

#include "IBootstrapperEngine.h"
#include "IBootstrapperApplication.h"

#include "platform.h"
#include "variant.h"
#include "variable.h"
#include "condition.h"
#include "search.h"
#include "section.h"
#include "container.h"
#include "catalog.h"
#include "payload.h"
#include "cabextract.h"
#include "userexperience.h"
#include "package.h"
#include "registration.h"
#include "detect.h"
#include "plan.h"
#include "logging.h"
#include "pipe.h"
#include "core.h"
#include "cache.h"
#include "downloadengine.h"
#include "apply.h"
#include "exeengine.h"
#include "msiengine.h"
#include "mspengine.h"
#include "msuengine.h"
#include "dependency.h"
#include "elevation.h"
#include "embedded.h"
#include "manifest.h"
#include "splashscreen.h"
#include "uithread.h"
#include "bitsengine.h"
#include "netfxchainer.h"

#include "EngineForApplication.h"
#include "engine.messages.h"
