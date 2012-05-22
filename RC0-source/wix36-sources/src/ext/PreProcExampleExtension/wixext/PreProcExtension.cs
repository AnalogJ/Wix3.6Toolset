//-------------------------------------------------------------------------------------------------
// <copyright file="PreProcExtension.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
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
// A simple preprocessor extension.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Extensions
{
    using System;
    using System.Collections;
    using System.IO;

    /// <summary>
    /// The example preprocessor extension.
    /// </summary>
    public sealed class PreProcExtension : PreprocessorExtension
    {
        /// <summary>
        /// Instantiate a new PreProcExtension.
        /// </summary>
        public PreProcExtension()
        {
        }

        /// <summary>
        /// Gets the variable prefixes for this extension.
        /// </summary>
        /// <value>The variable prefixes for this extension.</value>
        public override string[] Prefixes
        {
            get
            {
                string[] prefixes = new string[1];

                prefixes[0] = "abc";

                return prefixes;
            }
        }

        /// <summary>
        /// Preprocesses a parameter.
        /// </summary>
        /// <param name="name">Name of parameter that matches extension.</param>
        /// <returns>The value of the parameter after processing.</returns>
        /// <remarks>By default this method will cause an error if its called.</remarks>
        public override string PreprocessParameter(string name)
        {
            if ("SampleVariable" == name)
            {
                return "SampleValue";
            }

            return null;
        }
    }
}
