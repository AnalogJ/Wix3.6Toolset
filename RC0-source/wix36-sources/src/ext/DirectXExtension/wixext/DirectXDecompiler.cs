//-------------------------------------------------------------------------------------------------
// <copyright file="DirectXDecompiler.cs" company="Microsoft">
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
// The decompiler for the Windows Installer XML Toolset DirectX extension.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Extensions
{
    using System;
    using System.Text;
    using Wix = Microsoft.Tools.WindowsInstallerXml.Serialize;

    /// <summary>
    /// The Windows Installer XML Toolset DirectX Extension.
    /// </summary>
    public sealed class DirectXDecompiler : DecompilerExtension
    {
        /// <summary>
        /// Called at the beginning of the decompilation of a database.
        /// </summary>
        /// <param name="tables">The collection of all tables.</param>
        public override void InitializeDecompile(TableCollection tables)
        {
            Table propertyTable = tables["Property"];

            if (null != propertyTable)
            {
                foreach (Row row in propertyTable.Rows)
                {
                    if ("SecureCustomProperties" == row[0].ToString())
                    {
                        // if we've referenced any of the DirectX properties, add
                        // a PropertyRef to pick up the CA from the extension and then remove
                        // it from the SecureCustomExtensions property so we don't get duplicates
                        StringBuilder remainingProperties = new StringBuilder();
                        string[] secureCustomProperties = row[1].ToString().Split(';');
                        foreach (string property in secureCustomProperties)
                        {
                            if (property.StartsWith("WIX_DIRECTX_"))
                            {
                                Wix.PropertyRef propertyRef = new Wix.PropertyRef();
                                propertyRef.Id = property;
                                this.Core.RootElement.AddChild(propertyRef);
                            }
                            else
                            {
                                if (0 < remainingProperties.Length)
                                {
                                    remainingProperties.Append(";");
                                }
                                remainingProperties.Append(property);
                            }
                        }

                        row[1] = remainingProperties.ToString();
                        break;
                    }
                }
            }
        }
    }
}
