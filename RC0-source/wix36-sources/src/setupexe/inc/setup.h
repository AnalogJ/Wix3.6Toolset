//-------------------------------------------------------------------------------------------------
// <copyright file="setup.h" company="Microsoft">
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

#define SETUP_MAX_FILE_LENGTH                        64

#define SETUP_RESOURCE_IDS_PRODUCTNAME               51001
#define SETUP_RESOURCE_IDS_LICENSETEXT               51002
#define SETUP_RESOURCE_IDB_BACKGROUND                51003

#define SETUP_INSTALL_CHAIN_ALLUSERS                0x0001 // package installs to ALLUSERS
#define SETUP_INSTALL_CHAIN_PRIVILEGED              0x0002 // package requires elevated privileges to install
#define SETUP_INSTALL_CHAIN_CACHE                   0x0004 // cache the package before installing
#define SETUP_INSTALL_CHAIN_SHOW_UI                 0x0008 // show the UI of the embedded MSI
#define SETUP_INSTALL_CHAIN_IGNORE_FAILURES         0x0010 // ignore any install failures
#define SETUP_INSTALL_CHAIN_MINOR_UPGRADE_ALLOWED   0x0020 // the embedded MSI can do a minor upgrade
#define SETUP_INSTALL_CHAIN_LINK                    0x0040 // embedded resource is only a link to package
#define SETUP_INSTALL_CHAIN_USE_TRANSFORM           0x0080 // Uses the transforms included in setup
#define SETUP_INSTALL_CHAIN_PATCH                   0x0100 // database is a patch
#define SETUP_INSTALL_CHAIN_PATCH_FORCE_TARGET      0x0200 // do not ignore ERROR_PATCH_TARGET_NOT_FOUND

typedef struct
{
    CHAR szSource[12];
    WCHAR wzFilename[SETUP_MAX_FILE_LENGTH];
    GUID guidProductCode;
    DWORD dwVersionMajor;
    DWORD dwVersionMinor;
    DWORD dwAttributes; // any combination of the #define SETUP_INSTALL_CHAIN_
    // DWORD rgHash[8];
} SETUP_INSTALL_PACKAGE;

typedef struct
{
    CHAR szTransform[12];
    DWORD dwLocaleId;
} SETUP_INSTALL_TRANSFORM;

typedef struct
{
    BYTE dwRevision;
    BYTE cPackages;
    // Total number of transforms in package
    BYTE cTransforms;
    __field_ecount(cPackages) SETUP_INSTALL_PACKAGE rgPackages[1];
    // __field_ecount(cTransforms) SETUP_INSTALL_TRANSFORM rgTransforms[1];
} SETUP_INSTALL_CHAIN;
