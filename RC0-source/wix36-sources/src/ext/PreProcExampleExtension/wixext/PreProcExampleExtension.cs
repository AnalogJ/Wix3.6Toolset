//-------------------------------------------------------------------------------------------------
// <copyright file="PreProcExampleExtension.cs" company="Microsoft">
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
// The Windows Installer XML Toolset PreProcesses Example Extension.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Extensions
{
    using System;
    using System.Reflection;

    /// <summary>
    /// The Windows Installer XML Toolset PreProcessor Example Extension.
    /// </summary>
    public sealed class PreProcExampleExtension : WixExtension
    {
        private PreProcExtension extension;

        /// <summary>
        /// Gets the optional preprocessor extension.
        /// </summary>
        /// <value>The optional preprocessor extension.</value>
        public override PreprocessorExtension PreprocessorExtension
        {
            get
            {
                if (null == this.extension)
                {
                    this.extension = new PreProcExtension();
                }

                return this.extension;
            }
        }
    }
}
