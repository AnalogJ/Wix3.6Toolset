//-------------------------------------------------------------------------------------------------
// <copyright file="WixWarningLevel.cs" company="Microsoft">
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
// Contains the WixWarningLevel enum.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.VisualStudio
{
    using System;
    using System.ComponentModel;

    /// <summary>
    /// Enumeration for the warning level used during build time
    /// </summary>
    public enum WixWarningLevel
    {
        /// <summary>
        /// No warnings at all.
        /// </summary>
        None,

        /// <summary>
        /// Only the more important warnings are shown.
        /// </summary>
        Normal,

        /// <summary>
        /// All possible warnings.
        /// </summary>
        Pedantic,
    }
}
