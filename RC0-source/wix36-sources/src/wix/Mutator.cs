//-------------------------------------------------------------------------------------------------
// <copyright file="Mutator.cs" company="Microsoft">
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
// The Windows Installer XML Toolset mutator.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.Collections;

    using Wix = Microsoft.Tools.WindowsInstallerXml.Serialize;

    /// <summary>
    /// The Windows Installer XML Toolset mutator.
    /// </summary>
    public sealed class Mutator
    {
        private HarvesterCore core;
        private SortedList extensions;
        private string extensionArgument;

        /// <summary>
        /// Instantiate a new mutator.
        /// </summary>
        public Mutator()
        {
            this.extensions = new SortedList();
        }

        /// <summary>
        /// Gets or sets the harvester core for the extension.
        /// </summary>
        /// <value>The harvester core for the extension.</value>
        public HarvesterCore Core
        {
            get { return this.core; }
            set { this.core = value; }
        }

        /// <summary>
        /// Gets or sets the value of the extension argument passed to heat.
        /// </summary>
        /// <value>The extension argument.</value>
        public string ExtensionArgument
        {
            get { return this.extensionArgument; }
            set { this.extensionArgument = value; }
        }

        /// <summary>
        /// Adds a mutator extension.
        /// </summary>
        /// <param name="mutatorExtension">The mutator extension to add.</param>
        public void AddExtension(MutatorExtension mutatorExtension)
        {
            this.extensions.Add(mutatorExtension.Sequence, mutatorExtension);
        }

        /// <summary>
        /// Mutate a WiX document.
        /// </summary>
        /// <param name="wix">The Wix document element.</param>
        /// <returns>true if mutation was successful</returns>
        public bool Mutate(Wix.Wix wix)
        {
            bool encounteredError = false;
            
            try
            {
                foreach (MutatorExtension mutatorExtension in this.extensions.Values)
                {
                    if (null == mutatorExtension.Core)
                    {
                        mutatorExtension.Core = this.core;
                    }

                    mutatorExtension.Mutate(wix);
                }
            }
            finally
            {
                encounteredError = this.core.EncounteredError;
            }

            // return the Wix document element only if mutation completed successfully
            return !encounteredError;
        }

        /// <summary>
        /// Mutate a WiX document.
        /// </summary>
        /// <param name="wixString">The Wix document as a string.</param>
        /// <returns>The mutated Wix document as a string if mutation was successful, else null.</returns>
        public string Mutate(string wixString)
        {
            bool encounteredError = false;

            try
            {
                foreach (MutatorExtension mutatorExtension in this.extensions.Values)
                {
                    if (null == mutatorExtension.Core)
                    {
                        mutatorExtension.Core = this.core;
                    }

                    wixString = mutatorExtension.Mutate(wixString);

                    if (String.IsNullOrEmpty(wixString) || this.core.EncounteredError)
                    {
                        break;
                    }
                }
            }
            finally
            {
                encounteredError = this.core.EncounteredError;
            }

            return encounteredError ? null : wixString;
        }
    }
}
