//-------------------------------------------------------------------------------------------------
// <copyright file="WixInstallerPropertyPage.cs" company="Microsoft">
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
// Contains the WixInstallerPropertyPage class.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.VisualStudio.PropertyPages
{
    using System;
    using System.Globalization;
    using System.Runtime.InteropServices;
    using System.Windows.Forms;
    using Microsoft.VisualStudio;
    using Microsoft.VisualStudio.Package;

    /// <summary>
    /// Property page for the general WiX project configuration-independent build settings.
    /// </summary>
    [ComVisible(true)]
    [Guid("3C50BD5E-0E85-4306-A0A8-5B39CCB07DA0")]
    internal class WixInstallerPropertyPage : WixPropertyPage
    {
        // =========================================================================================
        // Constructors
        // =========================================================================================

        /// <summary>
        /// Initializes a new instance of the <see cref="WixInstallerPropertyPage"/> class.
        /// </summary>
        public WixInstallerPropertyPage()
        {
            this.PageName = WixStrings.WixInstallerPropertyPageName;
        }

        // =========================================================================================
        // Methods
        // =========================================================================================

        /// <summary>
        /// Gets a project property.
        /// </summary>
        /// <param name="propertyName">The name of the property to get.</param>
        /// <returns>
        /// Value of the property, or null if the property is unset or inconsistent across configurations.
        /// </returns>
        public override string GetProperty(string propertyName)
        {
            string value = base.GetProperty(propertyName);
            
            if (propertyName == WixProjectFileConstants.OutputType && value != null)
            {
                try
                {
                    WixOutputType outputType = (WixOutputType)Enum.Parse(typeof(WixOutputType), value, true);
                    value = ((int)outputType).ToString(CultureInfo.InvariantCulture);
                }
                catch (ArgumentException)
                {
                    value = null;
                }
            }

            return value;
        }

        /// <summary>
        /// Sets a project property.
        /// </summary>
        /// <param name="propertyName">Name of the property to set.</param>
        /// <param name="value">Value of the property.</param>
        public override void SetProperty(string propertyName, string value)
        {
            if (propertyName == WixProjectFileConstants.OutputType)
            {
                WixOutputType outputType = (WixOutputType)Int32.Parse(value, CultureInfo.InvariantCulture);
                if (Enum.IsDefined(typeof(WixOutputType), outputType) && outputType != this.ProjectMgr.OutputType)
                {
                    base.SetProperty(propertyName, outputType.ToString());
                    this.IsDirty = true;
                    this.ProjectMgr.OnOutputTypeChanged();
                }
            }
            else
            {
                base.SetProperty(propertyName, value);
            }
        }

        /// <summary>
        /// Creates the controls that constitute the property page. This should be safe to re-entrancy.
        /// </summary>
        /// <returns>The newly created main control that hosts the property page.</returns>
        protected override WixPropertyPagePanel CreatePropertyPagePanel()
        {
            return new WixInstallerPropertyPagePanel(this);
        }
     }
}