//-------------------------------------------------------------------------------------------------
// <copyright file="HarvesterExtension.cs" company="Microsoft">
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
// The base harvester extension.  Any of these methods can be overridden to change
// the behavior of the harvester.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;

    using Wix = Microsoft.Tools.WindowsInstallerXml.Serialize;

    /// <summary>
    /// The base harvester extension.  Any of these methods can be overridden to change
    /// the behavior of the harvester.
    /// </summary>
    public abstract class HarvesterExtension
    {
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
        /// Harvest a WiX document.
        /// </summary>
        /// <param name="argument">The argument for harvesting.</param>
        /// <returns>The harvested Fragments.</returns>
        public abstract Wix.Fragment[] Harvest(string argument);
    }
}
