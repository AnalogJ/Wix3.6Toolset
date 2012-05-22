//-------------------------------------------------------------------------------------------------
// <copyright file="ProgressEventArgs.cs" company="Microsoft">
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
// Porgress event arguments for ClickThrough.
// </summary>
//-------------------------------------------------------------------------------------------------
namespace Microsoft.Tools.WindowsInstallerXml.ClickThrough
{
    using System;
    using System.ComponentModel;

    [Serializable]
    public delegate void ProgressEventHandler(object sender, ProgressEventArgs e);

    /// <summary>
    /// Summary description for ProgressEventArgs.
    /// </summary>
    sealed public class ProgressEventArgs : CancelEventArgs
    {
        private int current;
        private int upperBound;
        private string message;

        /// <summary>
        /// Creates a new progress event argument.
        /// </summary>
        /// <param name="current">Current progress.</param>
        /// <param name="upperBound">Upper bound of progress.</param>
        /// <param name="message">Optional message to display with progress.</param>
        public ProgressEventArgs(int current, int upperBound, string message)
        {
            this.current = current;
            this.upperBound = upperBound;
            this.message = message;
        }

        /// <summary>
        /// Gets or sets the current progress.
        /// </summary>
        public int Current
        {
            get { return this.current; }
            set { this.current = value; }
        }

        /// <summary>
        /// Gets or sets the upperbound of the progress.
        /// </summary>
        public int UpperBound
        {
            get { return this.upperBound; }
            set { this.upperBound = value; }
        }

        /// <summary>
        /// Gets or sets the progress message.
        /// </summary>
        public string Message
        {
            get { return this.message; }
            set { this.message = value; }
        }
    }
}
