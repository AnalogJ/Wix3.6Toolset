//-------------------------------------------------------------------------------------------------
// <copyright file="SupportedFrameworkElementCollection.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
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
// Handler for the collection of supportedFramework configuration section.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Bootstrapper
{
    using System;
    using System.Configuration;
    using System.Diagnostics.CodeAnalysis;

    /// <summary>
    /// Handler for the supportedFramework collection.
    /// </summary>
    [SuppressMessage("Microsoft.Design", "CA1010:CollectionsShouldImplementGenericInterface")]
    [ConfigurationCollection(typeof(SupportedFrameworkElement), AddItemName = "supportedFramework", CollectionType = ConfigurationElementCollectionType.BasicMap)]
    public sealed class SupportedFrameworkElementCollection : ConfigurationElementCollection
    {
        public override ConfigurationElementCollectionType CollectionType
        {
            get { return ConfigurationElementCollectionType.BasicMap; }
        }

        protected override string ElementName
        {
            get { return "supportedFramework"; }
        }

        protected override ConfigurationElement CreateNewElement()
        {
            return new SupportedFrameworkElement();
        }

        protected override object GetElementKey(ConfigurationElement element)
        {
            return (element as SupportedFrameworkElement).Version;
        }
    }
}
