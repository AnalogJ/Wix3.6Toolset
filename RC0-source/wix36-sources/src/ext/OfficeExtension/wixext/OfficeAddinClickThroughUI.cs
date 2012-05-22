//-------------------------------------------------------------------------------------------------
// <copyright file="OfficeAddinClickThroughUI.cs" company="Microsoft">
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
// Office addin ClickThrough UI extension.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Extensions
{
    using System;
    using System.Windows.Forms;

    using Microsoft.Tools.WindowsInstallerXml.Extensions.OfficeAddin;

    /// <summary>
    /// Creates a new isolated app ClickThrough UI extension.
    /// </summary>
    public sealed class OfficeAddinClickThroughUI : ClickThroughUIExtension
    {
        private OfficeAddinFabricator fabricator;
        private Control[] controls;

        /// <summary>
        /// Creates a new OfficeAddinClickThroughConsole.
        /// </summary>
        public OfficeAddinClickThroughUI()
        {
            this.fabricator = new OfficeAddinFabricator();
        }

        /// <summary>
        /// Gets the fabrictor for this extension.
        /// </summary>
        /// <value>Fabricator for Office Add-in.</value>
        public override Fabricator Fabricator
        {
            get { return this.fabricator; }
        }

        /// <summary>
        /// Gets the UI Controls for this fabricator.
        /// </summary>
        /// <returns>Array of controls that make up the steps to feed the fabricator data.</returns>
        public override Control[] GetControls()
        {
            if (null == this.controls)
            {
                BrowsePathStep step1 = new BrowsePathStep();
                step1.Fabricator = this.fabricator;

                PickEntryStep step2 = new PickEntryStep();
                step2.Fabricator = this.fabricator;

                PackageInfoStep step3 = new PackageInfoStep();
                step3.Fabricator = this.fabricator;

                PickOfficeAppsStep step4 = new PickOfficeAppsStep();
                step4.Fabricator = this.fabricator;

                FeedStep step5 = new FeedStep();
                step5.Fabricator = this.fabricator;

                UpdateInfoStep step6 = new UpdateInfoStep();
                step6.Fabricator = this.fabricator;

                BuildStep step7 = new BuildStep();
                step7.Fabricator = this.fabricator;

                this.controls = new Control[] { step1, step2, step3, step4, step5, step6, step7 };
            }

            return this.controls;
        }
    }
}
