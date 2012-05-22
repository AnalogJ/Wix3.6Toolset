//-------------------------------------------------------------------------------------------------
// <copyright file="WixToolsSettingsPropertyPage.cs" company="Microsoft">
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
// Contains the WixToolsSettingsPropertyPage class.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.VisualStudio.PropertyPages
{
    using System;
    using System.Collections.Generic;
    using System.Runtime.InteropServices;
    using System.Text;

    /// <summary>
    /// Property page for the compiler/linker/librarian command line settings
    /// </summary>
    [ComVisible(true)]
    [Guid("37DA98B6-C5F4-4ce6-867F-553EBE590ABE")]
    internal class WixToolsSettingsPropertyPage : WixPropertyPage
    {
        // =========================================================================================
        // Constructors
        // =========================================================================================

        /// <summary>
        /// Initializes a new instance of WixToolsSettingsPropertyPage class
        /// </summary>
        public WixToolsSettingsPropertyPage()
        {
            this.PageName = WixStrings.WixToolsSettingsPropertyPageName;
        }

        // =========================================================================================
        // Methods
        // =========================================================================================

        /// <summary>
        /// Creates the controls that constitute the property page. This should be safe to re-entrancy.
        /// </summary>
        /// <returns>The newly created main control that hosts the property page.</returns>
        protected override WixPropertyPagePanel CreatePropertyPagePanel()
        {
            return new WixToolsSettingsPropertyPagePanel(this);
        }
    }
}
