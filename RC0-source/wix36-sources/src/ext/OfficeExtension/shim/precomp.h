// <copyright file="precomp.h" company="Microsoft">
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

#include <windows.h>

#include <mscoree.h>
#import <mscorlib.tlb> auto_rename raw_interfaces_only high_property_prefixes("_get","_put","_putref") rename_namespace("ClrNamespace") // for _AppDomain.  Used to communicate with the default app domain from unmanaged code
#import "libid:AC0714F2-3D04-11D1-AE7D-00A0C90F26F4" raw_interfaces_only named_guids rename_namespace("AddinNamespace") //The following #import imports the MSADDNDR.dl typelib which we need for IDTExtensibility2.

using namespace ClrNamespace;
using namespace AddinNamespace;

#include "wixstrsafe.h"
#include "dutil.h"
#include "memutil.h"
#include "resrutil.h"
#include "strutil.h"
#include "UnknownImpl.h"

#include "appsynup.h"

#include "ClrLoader.h"
#include "ConnectProxy.h"
#include "ClassFactory.h"
#include "UpdateThread.h"

#include "resource.h"