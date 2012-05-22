//-------------------------------------------------------------------------------------------------
// <copyright file="search.h" company="Microsoft">
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
//    Module: Search
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


#if defined(__cplusplus)
extern "C" {
#endif


// constants

enum BURN_SEARCH_TYPE
{
    BURN_SEARCH_TYPE_NONE,
    BURN_SEARCH_TYPE_DIRECTORY,
    BURN_SEARCH_TYPE_FILE,
    BURN_SEARCH_TYPE_REGISTRY,
    BURN_SEARCH_TYPE_MSI_COMPONENT,
    BURN_SEARCH_TYPE_MSI_PRODUCT,
    BURN_SEARCH_TYPE_MSI_FEATURE,
};

enum BURN_DIRECTORY_SEARCH_TYPE
{
    BURN_DIRECTORY_SEARCH_TYPE_NONE,
    BURN_DIRECTORY_SEARCH_TYPE_EXISTS,
};

enum BURN_FILE_SEARCH_TYPE
{
    BURN_FILE_SEARCH_TYPE_NONE,
    BURN_FILE_SEARCH_TYPE_EXISTS,
    BURN_FILE_SEARCH_TYPE_VERSION,
};

enum BURN_REGISTRY_SEARCH_TYPE
{
    BURN_REGISTRY_SEARCH_TYPE_NONE,
    BURN_REGISTRY_SEARCH_TYPE_EXISTS,
    BURN_REGISTRY_SEARCH_TYPE_VALUE,
};

enum BURN_MSI_COMPONENT_SEARCH_TYPE
{
    BURN_MSI_COMPONENT_SEARCH_TYPE_NONE,
    BURN_MSI_COMPONENT_SEARCH_TYPE_KEYPATH,
    BURN_MSI_COMPONENT_SEARCH_TYPE_STATE,
    BURN_MSI_COMPONENT_SEARCH_TYPE_DIRECTORY,
};

enum BURN_MSI_PRODUCT_SEARCH_TYPE
{
    BURN_MSI_PRODUCT_SEARCH_TYPE_NONE,
    BURN_MSI_PRODUCT_SEARCH_TYPE_VERSION,
    BURN_MSI_PRODUCT_SEARCH_TYPE_LANGUAGE,
    BURN_MSI_PRODUCT_SEARCH_TYPE_STATE,
    BURN_MSI_PRODUCT_SEARCH_TYPE_ASSIGNMENT,
};

enum BURN_MSI_FEATURE_SEARCH_TYPE
{
    BURN_MSI_FEATURE_SEARCH_TYPE_NONE,
    BURN_MSI_FEATURE_SEARCH_TYPE_STATE,
};


// structs

typedef struct _BURN_SEARCH
{
    LPWSTR sczKey;
    LPWSTR sczVariable;
    LPWSTR sczCondition;

    BURN_SEARCH_TYPE Type;
    union
    {
        struct
        {
            BURN_DIRECTORY_SEARCH_TYPE Type;
            LPWSTR sczPath;
        } DirectorySearch;
        struct
        {
            BURN_FILE_SEARCH_TYPE Type;
            LPWSTR sczPath;
        } FileSearch;
        struct
        {
            BURN_REGISTRY_SEARCH_TYPE Type;
            BURN_VARIANT_TYPE VariableType;
            HKEY hRoot;
            LPWSTR sczKey;
            LPWSTR sczValue;
            BOOL fWin64;
            BOOL fExpandEnvironment;
        } RegistrySearch;
        struct
        {
            BURN_MSI_COMPONENT_SEARCH_TYPE Type;
            LPWSTR sczProductCode;
            LPWSTR sczComponentId;
        } MsiComponentSearch;
        struct
        {
            BURN_MSI_PRODUCT_SEARCH_TYPE Type;
            LPWSTR sczProductCode;
        } MsiProductSearch;
        struct
        {
            BURN_MSI_FEATURE_SEARCH_TYPE Type;
            LPWSTR sczProductCode;
            LPWSTR sczFeatureId;
        } MsiFeatureSearch;
    };
} BURN_SEARCH;

typedef struct _BURN_SEARCHES
{
    BURN_SEARCH* rgSearches;
    DWORD cSearches;
} BURN_SEARCHES;


// function declarations

HRESULT SearchesParseFromXml(
    __in BURN_SEARCHES* pSearches,
    __in IXMLDOMNode* pixnBundle
    );
HRESULT SearchesExecute(
    __in BURN_SEARCHES* pSearches,
    __in BURN_VARIABLES* pVariables
    );
void SearchesUninitialize(
    __in BURN_SEARCHES* pSearches
    );


#if defined(__cplusplus)
}
#endif
