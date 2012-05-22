//-------------------------------------------------------------------------------------------------
// <copyright file="WixGenericMessageEventArgs.cs" company="Microsoft">
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
// Generic event args for message events.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.Resources;

    /// <summary>
    /// Generic event args for message events.
    /// </summary>
    public class WixGenericMessageEventArgs : MessageEventArgs
    {
        private GenericResourceManager resourceManager;

        /// <summary>
        /// Creates a new generc message event arg.
        /// </summary>
        /// <param name="sourceLineNumbers">Source line numbers for the message.</param>
        /// <param name="id">Id for the message.</param>
        /// <param name="level">Level for the message.</param>
        /// <param name="format">Format message for arguments.</param>
        /// <param name="messageArgs">Arguments for the format string.</param>
        public WixGenericMessageEventArgs(SourceLineNumberCollection sourceLineNumbers, int id, MessageLevel level, string format, params object[] messageArgs)
            : base(sourceLineNumbers, id, format, messageArgs)
        {
            this.resourceManager = new GenericResourceManager();

            this.Level = level;
        }

        /// <summary>
        /// Gets the resource manager for this event args.
        /// </summary>
        /// <value>The resource manager for this event args.</value>
        public override ResourceManager ResourceManager
        {
            get { return this.resourceManager; }
        }

        /// <summary>
        /// Private resource manager to return our format message as the "localized" string untouched.
        /// </summary>
        private class GenericResourceManager : ResourceManager
        {
            /// <summary>
            /// Passes the "resource name" through as the format string.
            /// </summary>
            /// <param name="name">Format message that is passed in as the resource name.</param>
            /// <returns>The name.</returns>
            public override string GetString(string name)
            {
                return name;
            }
        }
    }
}