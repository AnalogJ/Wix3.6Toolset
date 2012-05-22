//-------------------------------------------------------------------------------------------------
// <copyright file="Harvester.cs" company="Microsoft">
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
// The Windows Installer XML Toolset harvester.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.Collections;
    using System.Diagnostics.CodeAnalysis;

    using Wix = Microsoft.Tools.WindowsInstallerXml.Serialize;

    /// <summary>
    /// The Windows Installer XML Toolset harvester.
    /// </summary>
    public sealed class Harvester
    {
        private HarvesterExtension harvesterExtension;
        private HarvesterCore core;

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
        /// Gets or sets the extension.
        /// </summary>
        /// <value>The extension.</value>
        [SuppressMessage("Microsoft.Globalization", "CA1303:DoNotPassLiteralsAsLocalizedParameters", MessageId = "System.InvalidOperationException.#ctor(System.String)")]
        public HarvesterExtension Extension
        {
            get
            {
                return this.harvesterExtension;
            }
            set
            {
                if (null != this.harvesterExtension)
                {
                    throw new InvalidOperationException(WixStrings.EXP_MultipleHarvesterExtensionsSpecified);
                }

                this.harvesterExtension = value;
            }
        }

        /// <summary>
        /// Harvest wix authoring.
        /// </summary>
        /// <param name="argument">The argument for harvesting.</param>
        /// <returns>The harvested wix authoring.</returns>
        public Wix.Wix Harvest(string argument)
        {
            if (null == argument)
            {
                throw new ArgumentNullException("argument");
            }

            if (null == this.harvesterExtension)
            {
                throw new WixException(WixErrors.HarvestTypeNotFound());
            }

            this.harvesterExtension.Core = this.core;

            Wix.Fragment[] fragments = this.harvesterExtension.Harvest(argument);
            if (null == fragments || 0 == fragments.Length)
            {
                return null;
            }

            Wix.Wix wix = new Wix.Wix();
            foreach (Wix.Fragment fragment in fragments)
            {
                wix.AddChild(fragment);
            }

            return wix;
        }
    }
}
