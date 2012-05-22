//-------------------------------------------------------------------------------------------------
// <copyright file="AssemblyHarvester.cs" company="Microsoft">
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
// Harvest WiX authoring from an assembly file.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Extensions
{
    using System;
    using System.Reflection;
    using System.Runtime.InteropServices;

    using Wix = Microsoft.Tools.WindowsInstallerXml.Serialize;

    /// <summary>
    /// Harvest WiX authoring from an assembly file.
    /// </summary>
    public sealed class AssemblyHarvester
    {
        /// <summary>
        /// Harvest the registry values written by RegisterAssembly.
        /// </summary>
        /// <param name="path">The file to harvest registry values from.</param>
        /// <returns>The harvested registry values.</returns>
        public Wix.RegistryValue[] HarvestRegistryValues(string path)
        {
            RegistrationServices regSvcs = new RegistrationServices();
            Assembly assembly = Assembly.LoadFrom(path);

            // must call this before overriding registry hives to prevent binding failures
            // on exported types during RegisterAssembly
            assembly.GetExportedTypes();

            using (RegistryHarvester registryHarvester = new RegistryHarvester(true))
            {
                regSvcs.RegisterAssembly(assembly, AssemblyRegistrationFlags.SetCodeBase);

                return registryHarvester.HarvestRegistry();
            }
        }
    }
}
