//--------------------------------------------------------------------------------------------------
// <copyright file="WixLocalizedDisplayNameAttribute.cs" company="Microsoft">
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
// Contains the WixLocalizedDisplayNameAttribute class.
// </summary>
//--------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.VisualStudio
{
    using System;
    using System.ComponentModel;

    /// <summary>
    /// Subclasses <see cref="DisplayNameAttribute"/> to allow for localized strings retrieved
    /// from the resource assembly.
    /// </summary>
    [AttributeUsage(AttributeTargets.Event | AttributeTargets.Property | AttributeTargets.Method | AttributeTargets.Class)]
    public sealed class WixLocalizedDisplayNameAttribute : DisplayNameAttribute
    {
        // =========================================================================================
        // Member Variables
        // =========================================================================================

        private bool initialized;

        // =========================================================================================
        // Constructors
        // =========================================================================================

        /// <summary>
        /// Initializes a new instance of the <see cref="WixLocalizedDisplayNameAttribute"/> class.
        /// </summary>
        /// <param name="displayNameId">The string identifier to get.</param>
        public WixLocalizedDisplayNameAttribute(string displayNameId)
            : base(displayNameId)
        {
        }

        // =========================================================================================
        // Properties
        // =========================================================================================

        /// <summary>
        /// Gets the identifier for the display name.
        /// </summary>
        /// <value>The identifier for the display name.</value>
        public string DisplayNameId
        {
            get { return this.DisplayNameValue; }
        }

        /// <summary>
        /// Gets the display name for a property, event, or public void method that takes no
        /// arguments stored in this attribute.
        /// </summary>
        /// <value>The display name for a property, event, or public void method with no arguments.</value>
        public override string DisplayName
        {
            get
            {
                if (!this.initialized)
                {
                    string localizedString = WixStrings.ResourceManager.GetString(this.DisplayNameValue);
                    if (localizedString != null)
                    {
                        this.DisplayNameValue = localizedString;
                    }

                    this.initialized = true;
                }

                return this.DisplayNameValue;
            }
        }
    }
}