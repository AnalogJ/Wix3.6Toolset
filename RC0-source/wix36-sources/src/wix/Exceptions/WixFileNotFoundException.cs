//-------------------------------------------------------------------------------------------------
// <copyright file="WixFileNotFoundException.cs" company="Microsoft">
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
// WixException thrown when a file cannot be found.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;

    /// <summary>
    /// WixException thrown when a file cannot be found.
    /// </summary>
    [Serializable]
    public sealed class WixFileNotFoundException : WixException
    {
        /// <summary>
        /// Instantiate a new WixFileNotFoundException.
        /// </summary>
        /// <param name="file">The file that could not be found.</param>
        public WixFileNotFoundException(string file) : this(null, file, null)
        {
        }

        /// <summary>
        /// Instantiate a new WixFileNotFoundException.
        /// </summary>
        /// <param name="sourceLineNumbers">Source line information pertaining to the file that cannot be found.</param>
        /// <param name="file">The file that could not be found.</param>
        public WixFileNotFoundException(SourceLineNumberCollection sourceLineNumbers, string file) :
            base(WixErrors.FileNotFound(sourceLineNumbers, file))
        {
        }

        /// <summary>
        /// Instantiate a new WixFileNotFoundException.
        /// </summary>
        /// <param name="file">The file that could not be found.</param>
        /// <param name="fileType">The type of file that cannot be found.</param>
        public WixFileNotFoundException(string file, string fileType) : this(null, file, fileType)
        {
        }

        /// <summary>
        /// Instantiate a new WixFileNotFoundException.
        /// </summary>
        /// <param name="sourceLineNumbers">Source line information pertaining to the file that cannot be found.</param>
        /// <param name="file">The file that could not be found.</param>
        /// <param name="fileType">The type of file that cannot be found.</param>
        public WixFileNotFoundException(SourceLineNumberCollection sourceLineNumbers, string file, string fileType) :
            base(WixErrors.FileNotFound(sourceLineNumbers, file, fileType))
        {
        }
    }
}
