//-------------------------------------------------------------------------------------------------
// <copyright file="SupportedFrameworkElement.cs" company="Microsoft">
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
// Handler for the supportedFramework element.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Bootstrapper
{
    using System;
    using System.Configuration;

    /// <summary>
    /// Handler for the supportedFramework configuration section.
    /// </summary>
    public sealed class SupportedFrameworkElement : ConfigurationElement
    {
        private static readonly ConfigurationProperty versionProperty = new ConfigurationProperty("version", typeof(string), null, ConfigurationPropertyOptions.IsRequired);
        private static readonly ConfigurationProperty runtimeVersionProperty = new ConfigurationProperty("runtimeVersion", typeof(string));

        /// <summary>
        /// Creates a new instance of the <see cref="SupportedFrameworkElement"/> class.
        /// </summary>
        public SupportedFrameworkElement()
        {
        }

        /// <summary>
        /// Gets the version of the supported framework.
        /// </summary>
        /// <remarks>
        /// The assembly specified by this name must contain a value matching the NETFX version registry key under
        /// "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\NET Framework Setup\NDP".
        /// </remarks>
        [ConfigurationProperty("version", IsRequired = true)]
        public string Version
        {
            get { return (string)base[versionProperty]; }
            set { base[versionProperty] = value; }
        }

        /// <summary>
        /// Gets the runtime version required by this supported framework.
        /// </summary>
        [ConfigurationProperty("runtimeVersion", IsRequired = false)]
        public string RuntimeVersion
        {
            get { return (string)base[runtimeVersionProperty]; }
            set { base[runtimeVersionProperty] = value; }
        }
    }
}
