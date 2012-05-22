//--------------------------------------------------------------------------------------------------
// <copyright file="RunPostBuildEvent.cs" company="Microsoft">
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
// Contains the RunPostBuildEvent class.
// </summary>
//--------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.VisualStudio
{
    using System;

    /// <summary>
    /// Enumerates the values of the RunPostBuildEvent MSBuild property.
    /// </summary>
    public enum RunPostBuildEvent
    {
        /// <summary>
        /// The post-build event is always run.
        /// </summary>
        Always,

        /// <summary>
        /// The post-build event is only run when the build succeeds.
        /// </summary>
        OnBuildSuccess,

        /// <summary>
        /// The post-build event is only run if the project's output is updated.
        /// </summary>
        OnOutputUpdated,
    }
}
