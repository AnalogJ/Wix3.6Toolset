//-------------------------------------------------------------------------------------------------
// <copyright file="FabricatorCore.cs" company="Microsoft">
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
// The Windows Installer XML Toolset fabricator core.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.Reflection;

    /// <summary>
    /// The Windows Installer XML Toolset Fabricator core.
    /// </summary>
    public class FabricatorCore
    {
        private bool encounteredError;

        /// <summary>
        /// Instantiates a new FabricatorCore.
        /// </summary>
        /// <param name="messageHandler">The message handler for the core.</param>
        public FabricatorCore(MessageEventHandler messageHandler)
        {
            this.Message = messageHandler;
        }

        /// <summary>
        /// Event for messages.
        /// </summary>
        private event MessageEventHandler Message;

        /// <summary>
        /// Gets whether the fabricator core encountered an error while processing.
        /// </summary>
        /// <value>Flag if core encountered an error during processing.</value>
        public bool EncounteredError
        {
            get { return this.encounteredError; }
        }

        /// <summary>
        /// Gets the event handler used for messages.
        /// </summary>
        /// <value>Message handler.</value>
        public MessageEventHandler MessageEventHandler
        {
            get { return this.Message; }
        }

        /// <summary>
        /// Sends a message to the message delegate if there is one.
        /// </summary>
        /// <param name="mea">Message event arguments.</param>
        public void OnMessage(MessageEventArgs mea)
        {
            WixErrorEventArgs errorEventArgs = mea as WixErrorEventArgs;

            if (null != errorEventArgs)
            {
                this.encounteredError = true;
            }

            if (null != this.Message)
            {
                this.Message(this, mea);
                if (MessageLevel.Error == mea.Level)
                {
                    this.encounteredError = true;
                }
            }
            else if (null != errorEventArgs)
            {
                throw new WixException(errorEventArgs);
            }
        }
    }
}
