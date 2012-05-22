//--------------------------------------------------------------------------------------------------
// <copyright file="WixDesignPropertyDescriptor.cs" company="Microsoft">
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
// Contains the WixDesignPropertyDescriptor class.
// </summary>
//--------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.VisualStudio
{
    using System;
    using System.ComponentModel;
    using Microsoft.VisualStudio.Package;

    /// <summary>
    /// Subclasses <see cref="DesignPropertyDescriptor"/> to allow for non-bolded text in the property grid.
    /// </summary>
    [CLSCompliant(false)]
    public class WixDesignPropertyDescriptor : DesignPropertyDescriptor
    {
        // =========================================================================================
        // Constructors
        // =========================================================================================

        /// <summary>
        /// Initializes a new instance of the <see cref="WixDesignPropertyDescriptor"/> class.
        /// </summary>
        /// <param name="propertyDescriptor">The <see cref="PropertyDescriptor"/> to wrap.</param>
        public WixDesignPropertyDescriptor(PropertyDescriptor propertyDescriptor)
            : base(propertyDescriptor)
        {
        }

        // =========================================================================================
        // Methods
        // =========================================================================================

        /// <summary>
        /// By returning false here, we're always going to show the property as non-bolded.
        /// </summary>
        /// <param name="component">The component to check.</param>
        /// <returns>Always returns false.</returns>
        public override bool ShouldSerializeValue(object component)
        {
            return false;
        }
    }
}