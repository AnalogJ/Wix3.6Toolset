//-------------------------------------------------------------------------------------------------
// <copyright file="VerboseEventArgs.cs" company="Microsoft">
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
// Event arguments for verbose messages.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;

    /// <summary>
    /// Event arguments for verbose messages.
    /// </summary>
    public sealed class VerboseEventArgs : EventArgs
    {
        private string message;

        /// <summary>
        /// VerboseEventArgs Constructor.
        /// </summary>
        /// <param name="message">Verbose message content.</param>
        public VerboseEventArgs(string message)
        {
            this.message = message;
        }

        /// <summary>
        /// Getter for the message content.
        /// </summary>
        /// <value>The message content.</value>
        public string Message
        {
            get { return this.message; }
        }
    }
}