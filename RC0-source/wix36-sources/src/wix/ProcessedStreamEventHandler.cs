//-------------------------------------------------------------------------------------------------
// <copyright file="ProcessedStreamEventHandler.cs" company="Microsoft">
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
// Preprocessed stream event handler and event args.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.IO;
    using System.Text;

    /// <summary>
    /// Preprocessed output stream event handler delegate.
    /// </summary>
    /// <param name="sender">Sender of the message.</param>
    /// <param name="ea">Arguments for the preprocessed stream event.</param>
    public delegate void ProcessedStreamEventHandler(object sender, ProcessedStreamEventArgs e);

    /// <summary>
    /// Event args for preprocessed stream event.
    /// </summary>
    public class ProcessedStreamEventArgs : EventArgs
    {
        private string sourceFile;
        private Stream processed;

        /// <summary>
        /// Creates a new ProcessedStreamEventArgs.
        /// </summary>
        /// <param name="sourceFile">Source file that is preprocessed.</param>
        /// <param name="processed">Preprocessed output stream.</param>
        public ProcessedStreamEventArgs(string sourceFile, Stream processed)
        {
            this.sourceFile = sourceFile;
            this.processed = processed;
        }

        /// <summary>
        /// Gets the full path of the source file.
        /// </summary>
        /// <value>The full path of the source file.</value>
        public string SourceFile
        {
            get { return this.sourceFile; }
        }

        /// <summary>
        /// Gets the preprocessed output stream.
        /// </summary>
        /// <value>The the preprocessed output stream.</value>
        public Stream Processed
        {
            get { return this.processed; }
        }
    }
}
