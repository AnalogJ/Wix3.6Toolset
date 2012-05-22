//-------------------------------------------------------------------------------------------------
// <copyright file="PickOfficeAppsStep.cs" company="Microsoft">
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
// Pick the Office Addin UI for MSI builder for ClickThrough.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Extensions.OfficeAddin
{
    using System;
    using System.ComponentModel;
    using System.Drawing;
    using System.Text;
    using System.Windows.Forms;

    /// <summary>
    /// Pick the Office Addin UI for MSI builder for ClickThrough.
    /// </summary>
    internal partial class PickOfficeAppsStep : UserControl
    {
        private OfficeAddinFabricator fabricator;
        private bool externalChange;

        /// <summary>
        /// Create a new PickOfficeAppsStep.
        /// </summary>
        public PickOfficeAppsStep()
        {
            this.InitializeComponent();
        }

        /// <summary>
        /// Gets and sets the fabricator for this step.
        /// </summary>
        /// <value>Fabricator.</value>
        public OfficeAddinFabricator Fabricator
        {
            get
            {
                return this.fabricator;
            }

            set
            {
                if (this.fabricator != null)
                {
                    this.fabricator.Changed -= this.FabricatorProperty_Changed;
                    this.fabricator.Opened -= this.FabricatorOpened_Changed;
                }

                this.fabricator = value;
                this.fabricator.Changed += this.FabricatorProperty_Changed;
                this.fabricator.Opened += this.FabricatorOpened_Changed;
            }
        }

        /// <summary>
        /// Event handler for when any check box is validated.
        /// </summary>
        /// <param name="sender">The event sender.</param>
        /// <param name="e">The event data.</param>
        private void AnyCheckBox_Validated(object sender, EventArgs e)
        {
            if (!this.externalChange)
            {
                if (this.excel2003CheckBox.Checked)
                {
                    this.fabricator.AddExtendedOfficeApplication(OfficeAddinFabricator.OfficeApplications.Excel2003);
                }
                else
                {
                    this.fabricator.RemoveExtendedOfficeApplication(OfficeAddinFabricator.OfficeApplications.Excel2003);
                }

                if (this.outlook2003CheckBox.Checked)
                {
                    this.fabricator.AddExtendedOfficeApplication(OfficeAddinFabricator.OfficeApplications.Outlook2003);
                }
                else
                {
                    this.fabricator.RemoveExtendedOfficeApplication(OfficeAddinFabricator.OfficeApplications.Outlook2003);
                }

                if (this.powerPoint2003CheckBox.Checked)
                {
                    this.fabricator.AddExtendedOfficeApplication(OfficeAddinFabricator.OfficeApplications.PowerPoint2003);
                }
                else
                {
                    this.fabricator.RemoveExtendedOfficeApplication(OfficeAddinFabricator.OfficeApplications.PowerPoint2003);
                }

                if (this.word2003CheckBox.Checked)
                {
                    this.fabricator.AddExtendedOfficeApplication(OfficeAddinFabricator.OfficeApplications.Word2003);
                }
                else
                {
                    this.fabricator.RemoveExtendedOfficeApplication(OfficeAddinFabricator.OfficeApplications.Word2003);
                }
            }
        }

        /// <summary>
        /// Event handler for when the fabricator is re-opened ("model" for the "view") changes.
        /// </summary>
        /// <param name="sender">The event sender.</param>
        /// <param name="e">The event data.</param>
        private void FabricatorOpened_Changed(object sender, EventArgs e)
        {
            if (sender == this.fabricator)
            {
                try
                {
                    this.externalChange = true;
                    this.RepopulateCheckBoxes();
                }
                finally
                {
                    this.externalChange = false;
                }
            }
        }

        /// <summary>
        /// Event handler for when the fabricator's property ("model" for the "view") changes.
        /// </summary>
        /// <param name="sender">The event sender.</param>
        /// <param name="e">The event data.</param>
        private void FabricatorProperty_Changed(object sender, PropertyChangedEventArgs e)
        {
            if (sender == this.fabricator)
            {
                try
                {
                    this.externalChange = true;
                    switch (e.PropertyName)
                    {
                        case "SupportedOfficeApplications":
                            this.RepopulateCheckBoxes();
                            break;
                    }
                }
                finally
                {
                    this.externalChange = false;
                }
            }
        }

        /// <summary>
        /// Resets the values of the check boxes based on the model (the "fabricator").
        /// </summary>
        private void RepopulateCheckBoxes()
        {
            this.excel2003CheckBox.Checked = false;
            this.outlook2003CheckBox.Checked = false;
            this.powerPoint2003CheckBox.Checked = false;
            this.word2003CheckBox.Checked = false;

            foreach (OfficeAddinFabricator.OfficeApplications app in this.fabricator.SupportedOfficeApplications)
            {
                switch (app)
                {
                    case OfficeAddinFabricator.OfficeApplications.Excel2003:
                        this.excel2003CheckBox.Checked = true;
                        break;
                    case OfficeAddinFabricator.OfficeApplications.Outlook2003:
                        this.outlook2003CheckBox.Checked = true;
                        break;
                    case OfficeAddinFabricator.OfficeApplications.PowerPoint2003:
                        this.powerPoint2003CheckBox.Checked = true;
                        break;
                    case OfficeAddinFabricator.OfficeApplications.Word2003:
                        this.word2003CheckBox.Checked = true;
                        break;
                }
            }
        }
    }
}
