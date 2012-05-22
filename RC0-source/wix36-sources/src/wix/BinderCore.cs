//-------------------------------------------------------------------------------------------------
// <copyright file="BinderCore.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
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
// The base binder extension.  Any of these methods can be overridden to change
// the behavior of the binder.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics.CodeAnalysis;
    using Microsoft.Tools.WindowsInstallerXml.Msi;

    /// <summary>
    /// Core class for the binder.
    /// </summary>
    public sealed class BinderCore : IMessageHandler
    {
        private bool encounteredError;
        private TableDefinitionCollection tableDefinitions;

        /// <summary>
        /// Event for messages.
        /// </summary>
        private event MessageEventHandler MessageHandler;

        /// <summary>
        /// Constructor for binder core.
        /// </summary>
        /// <param name="messageHandler">The message handler.</param>
        internal BinderCore(MessageEventHandler messageHandler)
        {
            this.tableDefinitions = Installer.GetTableDefinitions();
            this.MessageHandler = messageHandler;
        }

        /// <summary>
        /// Gets whether the binder core encountered an error while processing.
        /// </summary>
        /// <value>Flag if core encountered an error during processing.</value>
        public bool EncounteredError
        {
            get { return this.encounteredError; }
            set { this.encounteredError = value; }
        }

        /// <summary>
        /// Gets the table definitions used by the Binder.
        /// </summary>
        /// <value>Table definitions used by the binder.</value>
        public TableDefinitionCollection TableDefinitions
        {
            get { return this.tableDefinitions; }
        }

        /// <summary>
        /// Generate an identifier by hashing data from the row.
        /// </summary>
        /// <param name="prefix">Three letter or less prefix for generated row identifier.</param>
        /// <param name="args">Information to hash.</param>
        /// <returns>The generated identifier.</returns>
        [SuppressMessage("Microsoft.Globalization", "CA1303:DoNotPassLiteralsAsLocalizedParameters", MessageId = "System.InvalidOperationException.#ctor(System.String)")]
        public string GenerateIdentifier(string prefix, params string[] args)
        {
            // Backward compatibility not required for new code.
            return Common.GenerateIdentifier(prefix, true, args);
        }

        /// <summary>
        /// Sends a message to the message delegate if there is one.
        /// </summary>
        /// <param name="mea">Message event arguments.</param>
        public void OnMessage(MessageEventArgs e)
        {
            WixErrorEventArgs errorEventArgs = e as WixErrorEventArgs;

            if (null != errorEventArgs)
            {
                this.encounteredError = true;
            }

            if (null != this.MessageHandler)
            {
                this.MessageHandler(this, e);
                if (MessageLevel.Error == e.Level)
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