//-------------------------------------------------------------------------------------------------
// <copyright file="BundlePackageAttributes.cs" company="Microsoft">
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
// Bit flags for a bundle package in the Windows Installer Xml toolset.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;

    /// <summary>
    /// Attributes available for a bundle package.
    /// </summary>
    [Flags]
    internal enum BundlePackageAttributes : int
    {
        None = 0x0,
        Permanent = 0x1,
        Visible = 0x2,
    }
}
