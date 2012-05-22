//--------------------------------------------------------------------------------------------------
// <copyright file="WixLocalizedDescriptionAttribute.cs" company="Microsoft">
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
// Contains the WixLocalizedDescritpionAttribute class.
// </summary>
//--------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.VisualStudio
{
    using System;
    using System.ComponentModel;

    /// <summary>
    /// Subclasses <see cref="DescriptionAttribute"/> to allow for localized strings retrieved
    /// from the resource assembly.
    /// </summary>
    [AttributeUsage(AttributeTargets.All)]
    public sealed class WixLocalizedDescriptionAttribute : DescriptionAttribute
    {
        // =========================================================================================
        // Member Variables
        // =========================================================================================

        private bool initialized;

        // =========================================================================================
        // Constructors
        // =========================================================================================

        /// <summary>
        /// Initializes a new instance of the <see cref="WixLocalizedDescriptionAttribute"/> class.
        /// </summary>
        /// <param name="descriptionId">The string identifier to get.</param>
        public WixLocalizedDescriptionAttribute(string descriptionId)
            : base(descriptionId)
        {
        }

        // =========================================================================================
        // Properties
        // =========================================================================================

        /// <summary>
        /// Gets the identifier for the description.
        /// </summary>
        /// <value>The identifier for the description.</value>
        public string DescriptionId
        {
            get { return this.DescriptionValue; }
        }

        /// <summary>
        /// Gets the description stored in this attribute.
        /// </summary>
        /// <value>The description stored in this attribute.</value>
        public override string Description
        {
            get
            {
                if (!this.initialized)
                {
                    string localizedDescription = WixStrings.ResourceManager.GetString(this.DescriptionValue);
                    if (localizedDescription != null)
                    {
                        this.DescriptionValue = localizedDescription;
                    }

                    this.initialized = true;
                }

                return this.DescriptionValue;
            }
        }
    }
}