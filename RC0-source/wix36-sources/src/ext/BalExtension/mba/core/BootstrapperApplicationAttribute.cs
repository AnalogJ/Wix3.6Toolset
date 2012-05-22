//-------------------------------------------------------------------------------------------------
// <copyright file="BootstrapperApplicationAttribute.cs" company="Microsoft">
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
// Identifies the bootstrapper application class.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Bootstrapper
{
    using System;

    /// <summary>
    /// Identifies the bootstrapper application class.
    /// </summary>
    /// <remarks>
    /// This required assembly attribute identifies the bootstrapper application class.
    /// </remarks>
    [AttributeUsage(AttributeTargets.Assembly, AllowMultiple = false)]
    public sealed class BootstrapperApplicationAttribute : Attribute
    {
        private Type bootstrapperApplicationType;

        /// <summary>
        /// Creates a new instance of the <see cref="BootstrapperApplicationAttribute"/> class.
        /// </summary>
        /// <param name="bootstrapperApplicationType">The <see cref="Type"/> of the user experience, or null for the default user experience.</param>
        public BootstrapperApplicationAttribute(Type bootstrapperApplicationType)
        {
            this.bootstrapperApplicationType = bootstrapperApplicationType;
        }

        /// <summary>
        /// Gets the type of the bootstrapper application class to create.
        /// </summary>
        public Type BootstrapperApplicationType
        {
            get { return this.bootstrapperApplicationType; }
        }
    }
}
