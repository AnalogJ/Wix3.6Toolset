//-------------------------------------------------------------------------------------------------
// <copyright file="WixLibraryReferenceNode.cs" company="Microsoft">
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
// Contains the WixLibraryReferenceNode class.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.VisualStudio
{
    using System;
    using System.IO;
    using Microsoft.VisualStudio.Package;
    using Microsoft.VisualStudio.Shell;
    using Microsoft.Tools.WindowsInstallerXml.Build.Tasks;

    /// <summary>
    /// Represents a Wixlib reference node.
    /// </summary>
    [CLSCompliant(false)]
    public class WixLibraryReferenceNode : WixReferenceNode
    {
        // =========================================================================================
        // Constructors
        // =========================================================================================

        /// <summary>
        /// Initializes a new instance of the <see cref="WixLibraryReferenceNode"/> class.
        /// </summary>
        /// <param name="root">The root <see cref="WixProjectNode"/> that contains this node.</param>
        /// <param name="element">The element that contains MSBuild properties.</param>
        public WixLibraryReferenceNode(WixProjectNode root, ProjectElement element)
            : base(root, element)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="WixLibraryReferenceNode"/> class.
        /// </summary>
        /// <param name="root">The root <see cref="WixProjectNode"/> that contains this node.</param>
        /// <param name="referencePath">The path to the wixlib reference file.</param>
        public WixLibraryReferenceNode(WixProjectNode root, string referencePath)
            : base(root, referencePath, WixProjectFileConstants.WixLibrary)
        {
        }

        // =========================================================================================
        // Methods
        // =========================================================================================

        /// <summary>
        /// Validates that a reference can be added.
        /// </summary>
        /// <param name="errorHandler">A CannotAddReferenceErrorMessage delegate to show the error message.</param>
        /// <returns>true if the reference can be added.</returns>
        protected override bool CanAddReference(out CannotAddReferenceErrorMessage errorHandler)
        {
            if (!base.CanAddReference(out errorHandler))
            {
                return false;
            }

            errorHandler = null;
            if (!WixReferenceValidator.IsValidWixLibrary(this.Url))
            {
                errorHandler = new CannotAddReferenceErrorMessage(this.ShowInvalidWixReferenceMessage);
                return false;
            }

            return true;
        }

        /// <summary>
        /// Creates an object derived from <see cref="NodeProperties"/> that will be used to expose
        /// properties specific for this object to the property browser.
        /// </summary>
        /// <returns>A new <see cref="WixLibraryReferenceNodeProperties"/> object.</returns>
        protected override NodeProperties CreatePropertiesObject()
        {
            return new WixLibraryReferenceNodeProperties(this);
        }
    }
}
