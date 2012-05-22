//-------------------------------------------------------------------------------------------------
// <copyright file="Librarian.cs" company="Microsoft">
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
// Core librarian tool.
// </summary>
//-------------------------------------------------------------------------------------------------
namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.Collections;
    using System.Collections.Specialized;

    using Microsoft.Tools.WindowsInstallerXml.Msi;

    /// <summary>
    /// Core librarian tool.
    /// </summary>
    public sealed class Librarian : IMessageHandler
    {
        private TableDefinitionCollection tableDefinitions;
        private bool encounteredError;
        private bool showPedanticMessages;

        /// <summary>
        /// Instantiate a new Librarian class.
        /// </summary>
        public Librarian()
        {
            this.tableDefinitions = Installer.GetTableDefinitions();
        }

        /// <summary>
        /// Event for messages.
        /// </summary>
        public event MessageEventHandler Message;

        /// <summary>
        /// Gets or sets the option to show pedantic messages.
        /// </summary>
        /// <value>The option to show pedantic messages.</value>
        public bool ShowPedanticMessages
        {
            get { return this.showPedanticMessages; }
            set { this.showPedanticMessages = value; }
        }

        /// <summary>
        /// Gets table definitions used by this librarian.
        /// </summary>
        /// <value>Table definitions.</value>
        public TableDefinitionCollection TableDefinitions
        {
            get { return this.tableDefinitions; }
        }

        /// <summary>
        /// Adds an extension.
        /// </summary>
        /// <param name="extension">The extension to add.</param>
        public void AddExtension(WixExtension extension)
        {
            if (null != extension.TableDefinitions)
            {
                foreach (TableDefinition tableDefinition in extension.TableDefinitions)
                {
                    if (!this.tableDefinitions.Contains(tableDefinition.Name))
                    {
                        this.tableDefinitions.Add(tableDefinition);
                    }
                    else
                    {
                        throw new WixException(WixErrors.DuplicateExtensionTable(extension.GetType().ToString(), tableDefinition.Name));
                    }
                }
            }
        }

        /// <summary>
        /// Create a library by combining several intermediates (objects).
        /// </summary>
        /// <param name="sections">The sections to combine into a library.</param>
        /// <returns>Returns the new library.</returns>
        public Library Combine(SectionCollection sections)
        {
            Library library = new Library();

            library.Sections.AddRange(sections);

            // check for multiple entry sections and duplicate symbols
            this.Validate(library);

            return (this.encounteredError ? null : library);
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

            if (null != this.Message)
            {
                this.Message(this, e);
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

        /// <summary>
        /// Validate that a library contains one entry section and no duplicate symbols.
        /// </summary>
        /// <param name="library">Library to validate.</param>
        private void Validate(Library library)
        {
            Section entrySection;
            SymbolCollection allSymbols;

            library.Sections.FindEntrySectionAndLoadSymbols(false, this, OutputType.Unknown, out entrySection, out allSymbols);

            foreach (Section section in library.Sections)
            {
                section.ResolveReferences(OutputType.Unknown, allSymbols, null, null, this);
            }
        }
    }
}
