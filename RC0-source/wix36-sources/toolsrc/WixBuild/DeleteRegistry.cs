//-------------------------------------------------------------------------------------------------
// <copyright file="DeleteRegistry.cs" company="Microsoft">
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
// Contains the DeleteRegistry class.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.WixBuild
{
    using System;
    using System.IO;
    using System.Security;
    using System.Security.AccessControl;
    using Microsoft.Build.Framework;
    using Microsoft.Build.Utilities;
    using Microsoft.Win32;

    /// <summary>
    /// MSBuild task for deleting a key from the registry.
    /// </summary>
    public class DeleteRegistry : RegistryBase
    {
        // =========================================================================================
        // Member Variables
        // =========================================================================================

        // =========================================================================================
        // Constructors
        // =========================================================================================

        /// <summary>
        /// Initializes a new instance of the <see cref="DeleteRegistry"/> class.
        /// </summary>
        public DeleteRegistry()
        {
        }

        // =========================================================================================
        // Properties
        // =========================================================================================

        // =========================================================================================
        // Methods
        // =========================================================================================

        /// <summary>
        /// Executes the task by deleting a key from the registry.
        /// </summary>
        /// <returns><see langword="true"/> if the task successfully executed; otherwise, <see langword="false"/>.</returns>
        public override bool Execute()
        {
            // get the registry hive
            RegistryKey hiveKey = this.HiveRegistryKey;
            if (hiveKey == null)
            {
                return false;
            }

            try
            {
                hiveKey.DeleteSubKeyTree(this.Key);
                this.Log.LogMessage(MessageImportance.Normal, @"Deleted registry key at '{0}\{1}\{2}'.", hiveKey.Name, this.Key, this.Name);
            }
            catch (Exception e)
            {
                if (e is SecurityException || e is UnauthorizedAccessException || e is IOException)
                {
                    this.Log.LogErrorFromException(e);
                    return false;
                }
                else
                {
                    throw;
                }
            }

            return true;
        }
    }
}
