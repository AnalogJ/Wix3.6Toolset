//-------------------------------------------------------------------------------------------------
// <copyright file="StepPanel.cs" company="Microsoft">
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
//   Panel for a step in ClickThrough UI.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Tools.ClickThrough
{
    using System;
    using System.ComponentModel;
    using System.Drawing;
    using System.Windows.Forms;

    /// <summary>
    /// Step panel displayed in the work page.
    /// </summary>
    internal sealed class StepPanel : TableLayoutPanel
    {
        private int step;
        private Control containedControl;

        private Label label;
        private PictureBox pictureBox;

        /// <summary>
        /// Creates a new step panel.
        /// </summary>
        public StepPanel()
        {
            this.pictureBox = new PictureBox();
            this.label = new Label();
        }

        /// <summary>
        /// Gets and sets the step of this panel.
        /// </summary>
        public int Step
        {
            get { return this.step; }
            set { this.step = value; }
        }

        /// <summary>
        /// Gets and sets the contained control in the panel.
        /// </summary>
        public Control ContainedControl
        {
            get { return this.containedControl; }
            set { this.containedControl = value; }
        }

        /// <summary>
        /// Initializes the panel.
        /// </summary>
        public void InitializeComponent()
        {
            ComponentResourceManager resources = new ComponentResourceManager(typeof(StepPictures));

            this.SuspendLayout();
            ((ISupportInitialize)this.pictureBox).BeginInit();

            this.containedControl.SuspendLayout();

            this.pictureBox.Image = ((System.Drawing.Image)(resources.GetObject(String.Concat("circle", this.step))));
            this.pictureBox.Location = new System.Drawing.Point(0, 0);
            this.pictureBox.Name = String.Concat("pictureBoxStepPanel", this.step);
            this.SetRowSpan(this.pictureBox, 3);
            this.pictureBox.Size = new System.Drawing.Size(80, 80);

            this.label.Name = String.Concat("labelStepPanel", this.step);
            this.label.Size = new System.Drawing.Size(296, 13);
            this.label.Text = (string)this.containedControl.Tag;

            this.AutoSize = true;
            this.Dock = System.Windows.Forms.DockStyle.Left;
            this.Location = new System.Drawing.Point(0, 0);
            this.Margin = new System.Windows.Forms.Padding(0);
            this.MinimumSize = new System.Drawing.Size(160, 80);
            this.Name = String.Concat("stepPanel", this.step);

            this.ColumnCount = 2;
            this.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
            this.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());

            this.RowCount = 3;
            this.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 10F));
            this.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.RowStyles.Add(new System.Windows.Forms.RowStyle());

            this.Controls.Add(this.pictureBox, 0, 0);
            this.Controls.Add(this.label, 1, 1);
            this.Controls.Add(this.containedControl, 1, 2);

            Graphics graphics = this.label.CreateGraphics();
            SizeF textSize = graphics.MeasureString(this.label.Text, SystemFonts.DefaultFont);

            if (textSize.Width > this.Width)
            {
                int lines = ((int)(textSize.Width + 1) / this.containedControl.Width) + 1;
                this.label.Height *= lines;
                this.label.Width = this.containedControl.Width;
            }

            ((ISupportInitialize)this.pictureBox).EndInit();

            this.containedControl.ResumeLayout(false);
            this.containedControl.PerformLayout();

            this.ResumeLayout(false);
            this.PerformLayout();
        }
    }
}
