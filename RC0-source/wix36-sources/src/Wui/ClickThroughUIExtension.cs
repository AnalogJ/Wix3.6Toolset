//-------------------------------------------------------------------------------------------------
// <copyright file="ClickThroughUIExtension.cs" company="Microsoft">
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
// ClickThrough UI extension.
// </summary>
//-------------------------------------------------------------------------------------------------
namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.Windows.Forms;
    using System.Reflection;

    /// <summary>
    /// UI ClickThrough Extension.
    /// </summary>
    public abstract class ClickThroughUIExtension
    {
        /// <summary>
        /// Gets the fabricator for the extension.
        /// </summary>
        /// <value>The fabricator for the extension.</value>
        public abstract Fabricator Fabricator
        {
            get;
        }

        /// <summary>
        /// Loads a ClickThroughUIExtension from a type description string.
        /// </summary>
        /// <param name="extension">The extension type description string.</param>
        /// <returns>The loaded ClickThroughUIExtension.</returns>
        public static ClickThroughUIExtension Load(string extension)
        {
            Type extensionType;

            if (null == extension)
            {
                throw new ArgumentNullException("extension");
            }

            if (2 == extension.Split(',').Length)
            {
                extensionType = System.Type.GetType(extension);

                if (null == extensionType)
                {
                    throw new WixException(WixErrors.InvalidExtension(extension));
                }
            }
            else
            {
                try
                {
                    Assembly extensionAssembly = Assembly.Load(extension);

                    AssemblyDefaultClickThroughUIAttribute extensionAttribute = (AssemblyDefaultClickThroughUIAttribute)Attribute.GetCustomAttribute(extensionAssembly, typeof(AssemblyDefaultClickThroughUIAttribute));

                    extensionType = extensionAttribute.ExtensionType;
                }
                catch
                {
                    throw new WixException(WixErrors.InvalidExtension(extension));
                }
            }

            if (extensionType.IsSubclassOf(typeof(ClickThroughUIExtension)))
            {
                return Activator.CreateInstance(extensionType) as ClickThroughUIExtension;
            }
            else
            {
                throw new WixException(WixErrors.InvalidExtension(extension, extensionType.ToString(), typeof(ClickThroughUIExtension).ToString()));
            }
        }

        /// <summary>
        /// Gets the UI Controls for this fabricator.
        /// </summary>
        /// <returns>Array of controls that make up the steps to feed the fabricator data.</returns>
        public virtual Control[] GetControls()
        {
            return null;
        }
    }
}
