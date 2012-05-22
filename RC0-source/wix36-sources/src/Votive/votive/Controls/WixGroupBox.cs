//-------------------------------------------------------------------------------------------------
// <copyright file="WixGroupBox.cs" company="Microsoft">
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
// Contains the WixGroupBox class.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.VisualStudio.Controls
{
    using System;
    using System.ComponentModel;
    using System.Drawing;
    using System.Windows.Forms;
    using System.Windows.Forms.Design;

    /// <summary>
    /// Customized group box used for property page groups.
    /// </summary>
    [DefaultProperty("Text")]
    internal partial class WixGroupBox : Panel
    {
        // =========================================================================================
        // Member Variables
        // =========================================================================================

        private WixGroupLabel groupLabel;

        // =========================================================================================
        // Constructors
        // =========================================================================================

        /// <summary>
        /// Initializes a new instance of the <see cref="WixGroupBox"/> class.
        /// </summary>
        public WixGroupBox()
        {
            this.InitializeComponent();
        }

        // =========================================================================================
        // Properties
        // =========================================================================================

        /// <summary>
        /// Gets or sets the group label text.
        /// </summary>
        [Browsable(true)]
        public override string Text
        {
            get { return this.groupLabel.Text; }
            set { this.groupLabel.Text = value; }
        }

        /// <summary>
        /// Gets the space, in pixels, that is specified by default between controls.
        /// </summary>
        protected override Padding DefaultMargin
        {
            get { return new Padding(3, 12, 3, 3); }
        }

        /// <summary>
        /// Gets the internal spacing, in pixels, of the contents of a control.
        /// </summary>
        protected override Padding DefaultPadding
        {
            get { return new Padding(24, 24, 0, 0); }
        }

        // =========================================================================================
        // Methods
        // =========================================================================================

        /// <summary>
        /// Occurs when the control has resized.
        /// </summary>
        /// <param name="e">The <see cref="EventArgs"/> object that contains the event data.</param>
        protected override void OnResize(EventArgs e)
        {
            base.OnResize(e);

            if (this.groupLabel != null)
            {
                this.groupLabel.Width = this.ClientSize.Width + 3;
            }
        }

        /// <summary>
        /// Initializes the component's controls and other properties.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(WixGroupBox));
            this.groupLabel = new Microsoft.Tools.WindowsInstallerXml.VisualStudio.Controls.WixGroupLabel();
            this.SuspendLayout();

            // groupLabel
            resources.ApplyResources(this.groupLabel, "groupLabel");
            this.groupLabel.Name = "groupLabel";

            // WixGroupBox
            this.Controls.Add(this.groupLabel);
            resources.ApplyResources(this, "$this");
            this.ResumeLayout(false);
        }
    }
}
