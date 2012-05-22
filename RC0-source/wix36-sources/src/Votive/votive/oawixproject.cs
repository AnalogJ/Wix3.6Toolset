//-------------------------------------------------------------------------------------------------
// <copyright file="OAWixProject.cs" company="Microsoft">
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
// Contains the OAWixProject class.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.VisualStudio
{
    using EnvDTE;
    using Microsoft.VisualStudio.Package.Automation;
    using Microsoft.VisualStudio.Shell.Interop;
    using System;
    using System.Runtime.InteropServices;
 
    /// <summary>
    /// Represents automation object corresponding to a WiX project.
    /// </summary>
    [CLSCompliant(false), ComVisible(true)]
    [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Interoperability", "CA1409:ComVisibleTypesShouldBeCreatable")]
    public class OAWixProject : OAProject
    {
        // =========================================================================================
        // Member variables
        // =========================================================================================

        /// <summary>
        /// Properties associated with the WiX project.
        /// </summary>
        private OAProperties properties;

        // =========================================================================================
        // Constructors
        // =========================================================================================

        /// <summary>
        /// Initializes a new instance of the <see cref="OAWixProject"/> class.
        /// </summary>
        /// <param name="wixProject">The node to which this project belongs.</param>
        public OAWixProject(WixProjectNode wixProject)
            : base(wixProject)
        {
            if (wixProject != null)
            {
                this.properties = new OAProperties(wixProject.NodeProperties);
            }
        }

        /// <summary>
        /// Properties of the project
        /// </summary>
        /// <value>Collection of all project properties</value>
        public override EnvDTE.Properties Properties
        {
            get
            {
                return this.properties;
            }
        }
    }
}