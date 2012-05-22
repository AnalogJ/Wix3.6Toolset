//-------------------------------------------------------------------------------------------------
// <copyright file="IncludedFileEventHandler.cs" company="Microsoft">
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
// Included file event handler and event args.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.Text;

    /// <summary>
    /// Included file event handler delegate.
    /// </summary>
    /// <param name="sender">Sender of the message.</param>
    /// <param name="ea">Arguments for the included file event.</param>
    public delegate void IncludedFileEventHandler(object sender, IncludedFileEventArgs e);

    /// <summary>
    /// Event args for included file event.
    /// </summary>
    public class IncludedFileEventArgs : EventArgs
    {
        private SourceLineNumberCollection sourceLineNumbers;
        private string fullName;

        /// <summary>
        /// Creates a new IncludedFileEventArgs.
        /// </summary>
        /// <param name="sourceLineNumbers">Source line numbers for the included file.</param>
        /// <param name="fullName">The full path of the included file.</param>
        public IncludedFileEventArgs(SourceLineNumberCollection sourceLineNumbers, string fullName)
        {
            this.sourceLineNumbers = sourceLineNumbers;
            this.fullName = fullName;
        }

        /// <summary>
        /// Gets the full path of the included file.
        /// </summary>
        /// <value>The full path of the included file.</value>
        public string FullName
        {
            get { return this.fullName; }
        }

        /// <summary>
        /// Gets the source line numbers.
        /// </summary>
        /// <value>The source line numbers.</value>
        public SourceLineNumberCollection SourceLineNumbers
        {
            get { return this.sourceLineNumbers; }
        }
    }
}
