//-------------------------------------------------------------------------------------------------
// <copyright file="BrowsePathStep.cs" company="Microsoft">
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
//  First step in the isolated applications UI for MSI builder for ClickThrough.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Extensions.ClickThrough
{
    using System;
    using System.ComponentModel;
    using System.IO;
    using System.Text;
    using System.Windows.Forms;

    /// <summary>
    /// First step in the isolated application UI.
    /// </summary>
    public sealed partial class BrowsePathStep : UserControl
    {
        private Fabricator fabricator;
        private bool externalChange;

        /// <summary>
        /// Creates a browse path step.
        /// </summary>
        public BrowsePathStep()
        {
            this.InitializeComponent();
        }

        /// <summary>
        /// Event fired any time a change is made to the step's properties.
        /// </summary>
        public event PropertyChangedEventHandler Changed;

        /// <summary>
        /// Gets and sets the fabricator for this step.
        /// </summary>
        /// <value>Fabricator.</value>
        public Fabricator Fabricator
        {
            get { return this.fabricator; }
            set { this.fabricator = value; }
        }

        /// <summary>
        /// Gets and sets the source for this step.
        /// </summary>
        /// <value>Source from this step.</value>
        public string Source
        {
            get
            {
                return this.textBox.Text;
            }

            set
            {
                try
                {
                    this.externalChange = true;

                    if (this.textBox.Text != value)
                    {
                        this.textBox.Text = value;
                    }
                }
                finally
                {
                    this.externalChange = false;
                }
            }
        }

        /// <summary>
        /// Event handler for when the source text box is validating
        /// </summary>
        /// <param name="sender">The event sender.</param>
        /// <param name="e">The event data.</param>
        private void TextBox_Validating(object sender, CancelEventArgs e)
        {
            if (this.textBox.Text != null && this.textBox.Text != String.Empty)
            {
                if (!Directory.Exists(this.textBox.Text))
                {
                    e.Cancel = true;
                    MessageBox.Show("The specified directory does not exist.  Please, provide a valid path or browse to an existing directory.", this.fabricator.Title, MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
        }

        /// <summary>
        /// Event handler for when the source text box is validated.
        /// </summary>
        /// <param name="sender">The event sender.</param>
        /// <param name="e">The event data.</param>
        private void TextBox_Validated(object sender, EventArgs e)
        {
            if (!this.externalChange)
            {
                if (this.Changed != null)
                {
                    this.Changed(this, new PropertyChangedEventArgs("Source"));
                }
            }
        }

        /// <summary>
        /// Event handler for when the browse button is clicked.
        /// </summary>
        /// <param name="sender">Control that sent the click request.</param>
        /// <param name="e">Event arguments for click.</param>
        private void BrowseButton_Click(object sender, EventArgs e)
        {
            this.folderBrowserDialog.SelectedPath = this.textBox.Text;

            DialogResult result = this.folderBrowserDialog.ShowDialog();
            if (result == DialogResult.OK)
            {
                this.textBox.Text = this.folderBrowserDialog.SelectedPath;
                if (this.Changed != null)
                {
                    this.Changed(this, new PropertyChangedEventArgs("Source"));
                }
            }
        }
    }
}
