//-------------------------------------------------------------------------------------------------
// <copyright file="BundleChainAttributes.cs" company="Microsoft">
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
// Bit flags for a bundle chain in the Windows Installer Xml toolset.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;

    /// <summary>
    /// Attributes available for a bundle chain.
    /// </summary>
    [Flags]
    internal enum BundleChainAttributes : int
    {
        None = 0x0,
        DisableRollback = 0x1,
        DisableSystemRestore = 0x2,
        ParallelCache = 0x4,
    }
}
