//-------------------------------------------------------------------------------------------------
// <copyright file="WixToolsSettingsPropertyPagePanel.cs" company="Microsoft">
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
// Contains the WixToolsSettingsPropertyPagePanel class.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.VisualStudio.PropertyPages
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Drawing;
    using System.Data;
    using System.Text;
    using System.Windows.Forms;

    /// <summary>
    /// Property page contents for the Tools Settings page
    /// </summary>
    internal partial class WixToolsSettingsPropertyPagePanel : WixPropertyPagePanel
    {
        /// <summary>
        /// Initializes a new instance of the WixToolSettingsPropertyPagePanel class.
        /// </summary>
        /// <param name="parentPropertyPage">The parent property page to which this is bound</param>
        public WixToolsSettingsPropertyPagePanel(WixPropertyPage parentPropertyPage)
            : base(parentPropertyPage)
        {
            this.InitializeComponent();

            this.suppressIceCheckbox.Tag = WixProjectFileConstants.SuppressValidation;
            this.specificIceTextBox.Tag = WixProjectFileConstants.SuppressIces;
            this.compilerTextBox.Tag = WixProjectFileConstants.CompilerAdditionalOptions;
            this.linkerTextBox.Tag = WixProjectFileConstants.LinkerAdditionalOptions;
            this.librarianTextBox.Tag = WixProjectFileConstants.LibAdditionalOptions;

            this.suppressIceCheckbox.CheckStateChanged += delegate
            {
                this.specificIceTextBox.Enabled = (this.suppressIceCheckbox.CheckState != CheckState.Checked);
            };
        }

        /// <summary>
        /// Binds the properties from the MSBuild project file to the controls on the property page.
        /// </summary>
        protected internal override void BindProperties()
        {
            base.BindProperties();

            WixOutputType projectType = this.ParentPropertyPage.ProjectMgr.OutputType;
            this.linkerLibrarianLabel.Text = (projectType == WixOutputType.Library ? WixStrings.Librarian : WixStrings.Linker);
            this.linkerTextBox.Visible = (projectType != WixOutputType.Library);
            this.librarianTextBox.Visible = (projectType == WixOutputType.Library);
            this.suppressIceCheckbox.Enabled = (projectType != WixOutputType.Library && projectType != WixOutputType.Bundle);
            this.specificIceTextBox.Enabled = (projectType != WixOutputType.Library && projectType != WixOutputType.Bundle);
        }
    }
}
