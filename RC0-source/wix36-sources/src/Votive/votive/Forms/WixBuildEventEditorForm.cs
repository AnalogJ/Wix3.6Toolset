//--------------------------------------------------------------------------------------------------
// <copyright file="WixBuildEventEditorForm.cs" company="Microsoft">
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
// Contains the WixBuildEventEditorForm class.
// </summary>
//--------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.VisualStudio.Forms
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Windows.Forms;

    /// <summary>
    /// Advanced editor form for build events.
    /// </summary>
    internal partial class WixBuildEventEditorForm : Form
    {
        // =========================================================================================
        // Member Variables
        // =========================================================================================
        private static readonly List<string> DelayedExpansionProperties = new List<string>(new string[]
            {
                "TargetPath",
                "TargetPdbPath"
            });

        private IServiceProvider serviceProvider;

        // =========================================================================================
        // Constructors
        // =========================================================================================

        /// <summary>
        /// Initializes a new instance of the <see cref="WixBuildEventEditorForm"/> class.
        /// </summary>
        /// <param name="serviceProvider">The service provider object provided by the IDE hosting the project.</param>
        public WixBuildEventEditorForm(IServiceProvider serviceProvider)
        {
            this.serviceProvider = serviceProvider;
            this.InitializeComponent();

            this.HelpRequested += new HelpEventHandler(this.WixBuildEventEditorForm_HelpRequested);

            this.Font = WixHelperMethods.GetDialogFont();
        }

        // =========================================================================================
        // Properties
        // =========================================================================================

        /// <summary>
        /// Gets or sets the editor's text.
        /// </summary>
        public string EditorText
        {
            get { return this.contentTextBox.Text; }
            set { this.contentTextBox.Text = value; }
        }

        // =========================================================================================
        // Methods
        // =========================================================================================

        /// <summary>
        /// Sets up the macros list view.
        /// </summary>
        /// <param name="buildMacros">The collection of build macros to add to the list.</param>
        public void InitializeMacroList(WixBuildMacroCollection buildMacros)
        {
            this.macrosListView.Items.Clear();

            foreach (WixBuildMacroCollection.MacroNameValuePair pair in buildMacros)
            {
                ListViewItem item = new ListViewItem(new string[] { pair.MacroName, pair.Value }, 0);
                this.macrosListView.Items.Add(item);
            }

            this.macrosListView.AutoResizeColumns(ColumnHeaderAutoResizeStyle.ColumnContent);
        }

        /// <summary>
        /// This method gets called whenever the dialog box is shown.
        /// </summary>
        /// <param name="e">Contains information about the event raised.</param>
        protected override void OnShown(EventArgs e)
        {
            // Set focus and cursor to text box.
            this.contentTextBox.Focus();
            this.contentTextBox.SelectionStart = this.contentTextBox.Text.Length;

            // Disable insert button as no macro is selected by default.
            this.insertButton.Enabled = false;
            base.OnShown(e);
        }

        /// <summary>
        /// Inserts the specified macro name into the text box at the current text selection.
        /// </summary>
        /// <param name="item">The <see cref="ListViewItem"/> that contains the macro name.</param>
        private void InsertMacro(ListViewItem item)
        {
            string macroName = item.Text;

            if (DelayedExpansionProperties.Contains(macroName))
            {
                this.contentTextBox.SelectedText = "!(" + macroName + ")";
            }
            else
            {
                this.contentTextBox.SelectedText = "$(" + macroName + ")";
            }

            this.contentTextBox.Focus();
        }

        // =========================================================================================
        // Event Handlers
        // =========================================================================================

        private void OnInsertButtonClick(object sender, EventArgs e)
        {
            if (this.macrosListView.SelectedItems.Count > 0)
            {
                this.InsertMacro(this.macrosListView.SelectedItems[0]);
            }
        }

        private void OnMacrosListViewMouseDoubleClick(object sender, MouseEventArgs e)
        {
            ListViewHitTestInfo hitTestInfo = this.macrosListView.HitTest(e.Location);

            if (hitTestInfo.Item != null)
            {
                this.InsertMacro(hitTestInfo.Item);
            }
        }

        private void OnMacrosListViewSelectedIndexChanged(object sender, EventArgs e)
        {
            this.insertButton.Enabled = this.macrosListView.SelectedItems.Count > 0;
        }

        private void WixBuildEventEditorForm_HelpRequested(object sender, HelpEventArgs hlpevent)
        {
            Microsoft.VisualStudio.VSHelp.Help help = this.serviceProvider.GetService(typeof(Microsoft.VisualStudio.VSHelp.Help)) as Microsoft.VisualStudio.VSHelp.Help;

            if (help != null)
            {
                help.DisplayTopicFromF1Keyword("cs.ProjectPropertiesBuildEventsBuilder");
            }

            hlpevent.Handled = true;
        }
    }
}