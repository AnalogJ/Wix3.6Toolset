#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="caSuffix.h" company="Microsoft">
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
//    Platform specific suffix defines/utilities.
//    Must be kept in sync with caSuffix.wxi.
// </summary>
//-------------------------------------------------------------------------------------------------

#if defined _WIN64 // x64 and ia64 get this one
#define PLATFORM_DECORATION(f) f ## L"_64"
#else
#define PLATFORM_DECORATION(f) f
#endif
