//-------------------------------------------------------------------------------------------------
// <copyright file="BinderExtension.cs" company="Microsoft">
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
// The base binder extension.  Any of these methods can be overridden to perform binding tasks at
// various stages during the binding process.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;

    /// <summary>
    /// Base class for creating an binder extension.
    /// </summary>
    public abstract class BinderExtension
    {
        private BinderCore binderCore;

        /// <summary>
        /// Gets or sets the binder core for the extension.
        /// </summary>
        /// <value>Binder core for the extension.</value>
        public BinderCore Core
        {
            get { return this.binderCore; }
            set { this.binderCore = value; }
        }

        /// <summary>
        /// Called before transform binding occurs.
        /// </summary>
        public virtual void TransformInitialize(Output transform)
        {
        }

        /// <summary>
        /// Called after all changes to the transform occur and right before the transform is bound into an mst.
        /// </summary>
        public virtual void TransformFinalize(Output transform)
        {
        }

        /// <summary>
        /// Called before database binding occurs.
        /// </summary>
        public virtual void DatabaseInitialize(Output output)
        {
        }

        /// <summary>
        /// Called after all output changes occur and right before the output is bound into its final format.
        /// </summary>
        public virtual void DatabaseFinalize(Output output)
        {
        }

        /// <summary>
        /// Called before bundle binding occurs.
        /// </summary>
        public virtual void BundleInitialize(Output bundle)
        {
        }

        /// <summary>
        /// Called after all output changes occur and right before the output is bound into its final format.
        /// </summary>
        public virtual void BundleFinalize(Output bundle)
        {
        }
    }
}