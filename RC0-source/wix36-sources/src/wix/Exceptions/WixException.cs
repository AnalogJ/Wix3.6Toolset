//-------------------------------------------------------------------------------------------------
// <copyright file="WixException.cs" company="Microsoft">
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
// Base class for all Wix exceptions.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;

    /// <summary>
    /// Base class for all WiX exceptions.
    /// </summary>
    [Serializable]
    public class WixException : Exception
    {
        [NonSerialized]
        private WixErrorEventArgs error;

        /// <summary>
        /// Instantiate a new WixException with a given WixError.
        /// </summary>
        /// <param name="error">The localized error information.</param>
        public WixException(WixErrorEventArgs error)
        {
            this.error = error;
        }

        /// <summary>
        /// Gets the error message.
        /// </summary>
        /// <value>The error message.</value>
        public WixErrorEventArgs Error
        {
            get { return this.error; }
        }
    }
}
