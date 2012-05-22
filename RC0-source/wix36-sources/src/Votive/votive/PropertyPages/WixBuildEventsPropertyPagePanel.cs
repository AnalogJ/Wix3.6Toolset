//-------------------------------------------------------------------------------------------------
// <copyright file="WixBuildEventsPropertyPagePanel.cs" company="Microsoft">
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
// Contains the WixBuildEventsPropertyPagePanel class.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.VisualStudio.PropertyPages
{
    using System;
    using System.Diagnostics;
    using System.Text;
    using System.Windows.Forms;
    using Microsoft.Tools.WindowsInstallerXml.VisualStudio.Forms;

    /// <summary>
    /// Property page contents for the Candle Settings page.
    /// </summary>
    internal partial class WixBuildEventsPropertyPagePanel : WixPropertyPagePanel
    {
        // =========================================================================================
        // Member Variables
        // =========================================================================================

        private WixBuildEventEditorForm editorForm;

        // =========================================================================================
        // Constructors
        // =========================================================================================

        /// <summary>
        /// Initializes a new instance of the <see cref="WixBuildEventsPropertyPagePanel"/> class.
        /// </summary>
        /// <param name="parentPropertyPage">The parent property page to which this is bound.</param>
        public WixBuildEventsPropertyPagePanel(WixPropertyPage parentPropertyPage)
            : base(parentPropertyPage)
        {
            this.InitializeComponent();

            this.editorForm = new WixBuildEventEditorForm(parentPropertyPage.Site);

            // hook up the form to both editors
            this.preBuildEditor.Initialize(parentPropertyPage.ProjectMgr, this.editorForm);
            this.postBuildEditor.Initialize(parentPropertyPage.ProjectMgr, this.editorForm);

            this.preBuildEditor.TextBox.Tag = WixProjectFileConstants.PreBuildEvent;
            this.postBuildEditor.TextBox.Tag = WixProjectFileConstants.PostBuildEvent;
            this.runPostBuildComboBox.Tag = WixProjectFileConstants.RunPostBuildEvent;
        }
    }
}
