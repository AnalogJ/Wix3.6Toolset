//-------------------------------------------------------------------------------------------------
// <copyright file="ProjectPropertyArgumentException.cs" company="Microsoft">
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
// Contains the ProjectPropertyArgumentException class.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.VisualStudio
{
    using System;
    using System.Runtime.Serialization;
    using System.ComponentModel;

    /// <summary>
    /// Exception thrown when an invalid property is entered on the project property pages
    /// </summary>
    [Serializable]
    internal class ProjectPropertyArgumentException : ArgumentException
    {
        // =========================================================================================
        // Constructors
        // =========================================================================================

        /// <summary>
        /// Constructor for the ProjectPropertyArgumentException
        /// </summary>
        /// <param name="message">Error message associated with the exception</param>
        public ProjectPropertyArgumentException(string message)
            : base(message)
        {
        }

        /// <summary>
        /// Creates a new deserialized exception instance.
        /// </summary>
        /// <param name="info">Serialization info.</param>
        /// <param name="context">Streaming context.</param>
        protected ProjectPropertyArgumentException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }
}
