//-------------------------------------------------------------------------------------------------
// <copyright file="HeatCore.cs" company="Microsoft">
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
// The Windows Installer XML Toolset Harvester application core.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Tools
{
    using System;
    using System.Reflection;
    using Microsoft.Tools.WindowsInstallerXml;

    using Wix = Microsoft.Tools.WindowsInstallerXml.Serialize;

    /// <summary>
    /// The Windows Installer XML Toolset Harvester application core.
    /// </summary>
    public class HeatCore
    {
        private bool encounteredError;
        private Harvester harvester;
        private Mutator mutator;

        /// <summary>
        /// Instantiates a new HeatCore.
        /// </summary>
        /// <param name="messageHandler">The message handler for the core.</param>
        public HeatCore(MessageEventHandler messageHandler)
        {
            this.MessageHandler = messageHandler;
            this.harvester = new Harvester();
            this.mutator = new Mutator();
        }

        /// <summary>
        /// Event for messages.
        /// </summary>
        private event MessageEventHandler MessageHandler;

        /// <summary>
        /// Gets whether the mutator core encountered an error while processing.
        /// </summary>
        /// <value>Flag if core encountered an error during processing.</value>
        public bool EncounteredError
        {
            get { return this.encounteredError; }
        }

        /// <summary>
        /// Gets the harvester.
        /// </summary>
        /// <value>The harvester.</value>
        public Harvester Harvester
        {
            get { return this.harvester; }
        }

        /// <summary>
        /// Gets the mutator.
        /// </summary>
        /// <value>The mutator.</value>
        public Mutator Mutator
        {
            get { return this.mutator; }
        }

        /// <summary>
        /// Sends a message to the message delegate if there is one.
        /// </summary>
        /// <param name="mea">Message event arguments.</param>
        public void OnMessage(MessageEventArgs mea)
        {
            if (mea is WixErrorEventArgs)
            {
                this.encounteredError = true;
            }

            if (null != this.MessageHandler)
            {
                this.MessageHandler(this, mea);
                if (MessageLevel.Error == mea.Level)
                {
                    this.encounteredError = true;
                }
            }
        }
    }
}
