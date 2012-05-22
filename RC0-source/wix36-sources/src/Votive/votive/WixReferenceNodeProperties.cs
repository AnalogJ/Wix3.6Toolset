//-------------------------------------------------------------------------------------------------
// <copyright file="WixReferenceNodeProperties.cs" company="Microsoft">
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
// Contains the WixReferenceNodeProperties class.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.VisualStudio
{
    using System;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Diagnostics.CodeAnalysis;
    using System.Runtime.InteropServices;
    using Microsoft.VisualStudio.Package;

    /// <summary>
    /// Represents Wixlib reference node properties.
    /// </summary>
    /// <remarks>This class must be public and marked as ComVisible in order for the DispatchWrapper to work correctly.</remarks>
    [CLSCompliant(false)]
    [ComVisible(true)]
    [Guid("C39AF44F-FBBB-4457-ABB4-4731B2D29B40")]
    [SuppressMessage("Microsoft.Interoperability", "CA1409:ComVisibleTypesShouldBeCreatable")]
    public abstract class WixReferenceNodeProperties : ReferenceNodeProperties
    {
        // =========================================================================================
        // Constructors
        // =========================================================================================

        /// <summary>
        /// Initializes a new instance of the <see cref="WixReferenceNodeProperties"/> class.
        /// </summary>
        /// <param name="node">The node that contains the properties to expose via the Property Browser.</param>
        protected WixReferenceNodeProperties(ReferenceNode node)
            : base(node)
        {
        }

        // =========================================================================================
        // Properties
        // =========================================================================================

        /// <summary>
        /// Overridden to suppress browsability - copy local is meaningless for WiX references.
        /// </summary>
        /// <value>Flag indicating whether the referenced item should be copied to the local directory when the project builds.</value>
        [Browsable(false)]
        public override bool CopyToLocal
        {
            get
            {
                return false;
            }

            set
            {
                base.CopyToLocal = false;
            }
        }

        // =========================================================================================
        // Methods
        // =========================================================================================

        /// <summary>
        /// Creates a custom property descriptor for the node properties, which affects the behavior
        /// of the property grid.
        /// </summary>
        /// <param name="propertyDescriptor">The <see cref="PropertyDescriptor"/> to wrap.</param>
        /// <returns>A custom <see cref="PropertyDescriptor"/> object.</returns>
        [SuppressMessage("Microsoft.Naming", "CA1725:ParameterNamesShouldMatchBaseDeclaration", MessageId = "0#", Justification = "In the 2005 SDK, it's called p and in the 2008 SDK it's propertyDescriptor")]
        public override DesignPropertyDescriptor CreateDesignPropertyDescriptor(PropertyDescriptor propertyDescriptor)
        {
            return new WixDesignPropertyDescriptor(propertyDescriptor);
        }
    }
}
