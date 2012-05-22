//-------------------------------------------------------------------------------------------------
// <copyright file="setupbld.h" company="Microsoft">
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
// Setup executable builder header for ClickThrough.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


typedef struct
{
    LPCWSTR wzSourcePath;

    BOOL fPrivileged;
    BOOL fCache;
    BOOL fShowUI;
    BOOL fIgnoreFailures;
    BOOL fMinorUpgrade;
    BOOL fLink;
    BOOL fUseTransform;
    BOOL fPatch;
    BOOL fPatchForceTarget;
} CREATE_SETUP_PACKAGE;

typedef struct
{
    LPCWSTR wzSourcePath;
    DWORD dwLocaleId;
} CREATE_SETUP_TRANSFORMS;
