//--------------------------------------------------------------------------------------------------
// <copyright file="WixLocalizedControlTextAttribute.cs" company="Microsoft">
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
// Contains the WixLocalizedControlTextAttribute class.
// </summary>
//--------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.VisualStudio
{
    using System;
    using System.ComponentModel;

    /// <summary>
    /// Attribute to denote the localized text that should be displayed on the control for the
    /// property page settings.
    /// </summary>
    [AttributeUsage(AttributeTargets.Property)]
    public sealed class WixLocalizedControlTextAttribute : Attribute
    {
        // =========================================================================================
        // Member Variables
        // =========================================================================================

        private string id;

        // =========================================================================================
        // Constructors
        // =========================================================================================

        /// <summary>
        /// Initializes a new instance of the <see cref="WixLocalizedControlTextAttribute"/> class.
        /// </summary>
        /// <param name="controlTextId">The string identifier to get.</param>
        public WixLocalizedControlTextAttribute(string controlTextId)
        {
            this.id = controlTextId;
        }

        // =========================================================================================
        // Properties
        // =========================================================================================

        /// <summary>
        /// Gets the identifier for the associated control or control's label.
        /// </summary>
        /// <value>The identifier for the associated control or control's label.</value>
        public string ControlTextId
        {
            get { return this.id; }
        }

        /// <summary>
        /// Gets the text to display for the associated control or control's label.
        /// </summary>
        /// <value>The text to display for the associated control or control's label.</value>
        public string ControlText
        {
            get
            {
                return WixStrings.ResourceManager.GetString(this.id);
            }
        }
    }
}