//-------------------------------------------------------------------------------------------------
// <copyright file="RefreshGeneratedFile.cs" company="Microsoft">
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
// Build task to help in converting project references to HeatProject items.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Build.Tasks
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.Globalization;
    using System.IO;
    using System.Xml;
    using Microsoft.Build.Framework;
    using Microsoft.Build.Utilities;

    /// <summary>
    /// This task assigns Culture metadata to files based on the value of the Culture attribute on the
    /// WixLocalization element inside the file.
    /// </summary>
    public class ConvertReferences : Task
    {
        private string projectOutputGroups;
        private ITaskItem[] projectReferences;
        private ITaskItem[] harvestItems;

        /// <summary>
        /// The total list of cabs in this database
        /// </summary>
        [Output]
        public ITaskItem[] HarvestItems
        {
            get { return this.harvestItems; }
        }

        /// <summary>
        /// The project output groups to harvest.
        /// </summary>
        [Required]
        public string ProjectOutputGroups
        {
            get { return this.projectOutputGroups; }
            set { this.projectOutputGroups = value; }
        }

        /// <summary>
        /// All the project references in the project.
        /// </summary>
        [Required]
        public ITaskItem[] ProjectReferences
        {
            get { return this.projectReferences; }
            set { this.projectReferences = value; }
        }

        /// <summary>
        /// Gets a complete list of external cabs referenced by the given installer database file.
        /// </summary>
        /// <returns>True upon completion of the task execution.</returns>
        public override bool Execute()
        {
            List<ITaskItem> newItems = new List<ITaskItem>();

            foreach(ITaskItem item in this.ProjectReferences)
            {
                Dictionary<string, string> newItemMetadeta = new Dictionary<string, string>();

                if (!String.IsNullOrEmpty(item.GetMetadata(Common.DoNotHarvest)))
                {
                    continue;
                }

                string refTargetDir = item.GetMetadata("RefTargetDir");
                if (!String.IsNullOrEmpty(refTargetDir))
                {
                    newItemMetadeta.Add("DirectoryIds", refTargetDir);
                }

                string refName = item.GetMetadata("Name");
                if (!String.IsNullOrEmpty(refName))
                {
                    newItemMetadeta.Add("ProjectName", refName);
                }

                newItemMetadeta.Add("ProjectOutputGroups", this.ProjectOutputGroups);

                ITaskItem newItem = new TaskItem(item.ItemSpec, newItemMetadeta);
                newItems.Add(newItem);
            }

            this.harvestItems = newItems.ToArray();

            return true;
        }
    }
}
