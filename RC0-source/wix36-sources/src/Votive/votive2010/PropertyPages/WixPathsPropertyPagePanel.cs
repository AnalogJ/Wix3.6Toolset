//-------------------------------------------------------------------------------------------------
// <copyright file="WixPathsPropertyPagePanel.cs" company="Microsoft">
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
// Contains the WixPathsPropertyPagePanel class.
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
    /// Property page contents for the Paths property page
    /// </summary>
    /// 
    internal partial class WixPathsPropertyPagePanel : WixPropertyPagePanel
    {
        // =========================================================================================
        // Constructors
        // =========================================================================================

        /// <summary>
        /// Initilizes a new instance of the WixPathsPropertyPagePanel class
        /// </summary>
        /// <param name="parentPropertyPage">The parent property page to which this is bound</param>
        public WixPathsPropertyPagePanel(WixPropertyPage parentPropertyPage)
            : base(parentPropertyPage)
        {
            this.InitializeComponent();

            this.referencePathsFoldersSelector.Tag = WixProjectFileConstants.ReferencePaths;
            this.includePathsFolderSelector.Tag = WixProjectFileConstants.IncludeSearchPaths;
        }
    }
}
