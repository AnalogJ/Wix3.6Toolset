//-------------------------------------------------------------------------------------------------
// <copyright file="HostSection.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
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
// Handler for the Host configuration section.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Bootstrapper
{
    using System;
    using System.Configuration;

    /// <summary>
    /// Handler for the Host configuration section.
    /// </summary>
    public sealed class HostSection : ConfigurationSection
    {
        private static readonly ConfigurationProperty assemblyNameProperty = new ConfigurationProperty("assemblyName", typeof(string), null, ConfigurationPropertyOptions.IsRequired);
        private static readonly ConfigurationProperty supportedFrameworksProperty = new ConfigurationProperty("", typeof(SupportedFrameworkElementCollection), null, ConfigurationPropertyOptions.IsDefaultCollection);

        /// <summary>
        /// Creates a new instance of the <see cref="HostSection"/> class.
        /// </summary>
        public HostSection()
        {
        }

        /// <summary>
        /// Gets the name of the assembly that contians the <see cref="BootstrapperApplication"/> child class.
        /// </summary>
        /// <remarks>
        /// The assembly specified by this name must contain the <see cref="BootstrapperApplicationAttribute"/> to identify
        /// the type of the <see cref="BootstrapperApplication"/> child class.
        /// </remarks>
        [ConfigurationProperty("assemblyName", IsRequired = true)]
        public string AssemblyName
        {
            get { return (string)base[assemblyNameProperty]; }
            set { base[assemblyNameProperty] = value; }
        }

        /// <summary>
        /// Gets the <see cref="SupportedFrameworkElementCollection"/> of supported frameworks for the host configuration.
        /// </summary>
        [ConfigurationProperty("", IsDefaultCollection = true)]
        [ConfigurationCollection(typeof(SupportedFrameworkElement))]
        public SupportedFrameworkElementCollection SupportedFrameworks
        {
            get { return (SupportedFrameworkElementCollection)base[supportedFrameworksProperty]; }
        }
    }
}
