//-------------------------------------------------------------------------------------------------
// <copyright file="StatusChangingEventArgs.cs" company="Microsoft">
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
// New status data.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.ClickThrough
{
    using System;

    /// <summary>
    /// Delegate called when a status is changing.
    /// </summary>
    /// <param name="sender">Control that is changing status.</param>
    /// <param name="e">Data about the changing status.</param>
    public delegate void StatusChangingHandler(object sender, StatusChangingEventArgs e);

    /// <summary>
    /// New status data.
    /// </summary>
    public class StatusChangingEventArgs : EventArgs
    {
        private string message;

        /// <summary>
        /// Instantiate a new StatusChangeEventArgs class.
        /// </summary>
        public StatusChangingEventArgs()
        {
            this.message = null;
        }

        /// <summary>
        /// Instantiate a new StatusChangingEventArgs class.
        /// </summary>
        /// <param name="message">The message for the new status.</param>
        public StatusChangingEventArgs(string message)
        {
            this.message = message;
        }

        /// <summary>
        /// Gets the message for the new status.
        /// </summary>
        /// <value>The message for the new status.</value>
        public string Message
        {
            get { return this.message; }
        }
    }
}
