//-------------------------------------------------------------------------------------------------
// <copyright file="ConnectToFeature.cs" company="Microsoft">
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
// Object that connects things (components/modules) to features.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.Diagnostics;
    using System.Collections;
    using System.Collections.Specialized;
    using System.Globalization;
    using System.Xml;

    /// <summary>
    /// Object that connects things (components/modules) to features.
    /// </summary>
    public sealed class ConnectToFeature
    {
        private Section section;
        private string childId;

        private string primaryFeature;
        private bool explicitPrimaryFeature;
        private StringCollection connectFeatures;

        /// <summary>
        /// Creates a new connect to feature.
        /// </summary>
        /// <param name="section">Section this connect belongs to.</param>
        /// <param name="childId">Id of the child.</param>
        public ConnectToFeature(Section section, string childId) :
            this(section, childId, null, false)
        {
        }

        /// <summary>
        /// Creates a new connect to feature.
        /// </summary>
        /// <param name="section">Section this connect belongs to.</param>
        /// <param name="childId">Id of the child.</param>
        /// <param name="primaryFeature">Sets the primary feature for the connection.</param>
        /// <param name="explicitPrimaryFeature">Sets if this is explicit primary.</param>
        public ConnectToFeature(Section section, string childId, string primaryFeature, bool explicitPrimaryFeature)
        {
            this.section = section;
            this.childId = childId;

            this.primaryFeature = primaryFeature;
            this.explicitPrimaryFeature = explicitPrimaryFeature;

            this.connectFeatures = new StringCollection();
        }

        /// <summary>
        /// Gets the section.
        /// </summary>
        /// <value>Section.</value>
        public Section Section
        {
            get { return this.section; }
        }

        /// <summary>
        /// Gets the child identifier.
        /// </summary>
        /// <value>The child identifier.</value>
        public string ChildId
        {
            get { return this.childId; }
        }

        /// <summary>
        /// Gets or sets if the flag for if the primary feature was set explicitly.
        /// </summary>
        /// <value>The flag for if the primary feature was set explicitly.</value>
        public bool IsExplicitPrimaryFeature
        {
            get { return this.explicitPrimaryFeature; }
            set { this.explicitPrimaryFeature = value; }
        }

        /// <summary>
        /// Gets or sets the primary feature.
        /// </summary>
        /// <value>The primary feature.</value>
        public string PrimaryFeature
        {
            get { return this.primaryFeature; }
            set { this.primaryFeature = value; }
        }

        /// <summary>
        /// Gets the features connected to.
        /// </summary>
        /// <value>Features connected to.</value>
        public StringCollection ConnectFeatures
        {
            get { return this.connectFeatures; }
        }
    }
}
