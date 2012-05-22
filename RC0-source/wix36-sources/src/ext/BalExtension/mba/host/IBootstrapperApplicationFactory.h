//-------------------------------------------------------------------------------------------------
// <copyright file="IBootstrapperApplicationFactory.h" company="Microsoft">
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
// Class interface for the managed BootstrapperApplicationFactory class.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include "precomp.h"

DECLARE_INTERFACE_IID_(IBootstrapperApplicationFactory, IUnknown, "2965A12F-AC7B-43A0-85DF-E4B2168478A4")
{
    STDMETHOD(Create)(
        __in IBootstrapperEngine* pEngine,
        __in const BOOTSTRAPPER_COMMAND *pCommand,
        __out IBootstrapperApplication **ppApplication
        );
};
