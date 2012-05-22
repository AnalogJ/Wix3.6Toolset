//-------------------------------------------------------------------------------------------------
// <copyright file="WixNotOutputException.cs" company="Microsoft">
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
// Exception thrown when trying to create an output from a file that is not an output file.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;

    /// <summary>
    /// Exception thrown when trying to create an output from a file that is not an output file.
    /// </summary>
    [Serializable]
    public sealed class WixNotOutputException : WixException
    {
        /// <summary>
        /// Instantiate a new WixNotOutputException.
        /// </summary>
        /// <param name="error">Localized error information.</param>
        public WixNotOutputException(WixErrorEventArgs error)
            : base(error)
        {
        }
    }
}
