//-------------------------------------------------------------------------------------------------
// <copyright file="UnbinderExtension.cs" company="Microsoft">
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
// The base unbinder extension.  Any of these methods can be overridden to change
// the behavior of the unbinder.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;

    /// <summary>
    /// Base class for creating an unbinder extension.
    /// </summary>
    public abstract class UnbinderExtension
    {
        /// <summary>
        /// Called during the generation of sectionIds for an admin image.
        /// </summary>
        public virtual void GenerateSectionIds(Output output)
        {
        }
    }
}
