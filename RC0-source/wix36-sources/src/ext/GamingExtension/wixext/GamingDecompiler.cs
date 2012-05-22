//-------------------------------------------------------------------------------------------------
// <copyright file="GamingDecompiler.cs" company="Microsoft">
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
// The decompiler for the Windows Installer XML Toolset Gaming Extension.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Extensions
{
    using System;
    using System.Collections;
    using System.Diagnostics;
    using System.Globalization;

    using Gaming = Microsoft.Tools.WindowsInstallerXml.Extensions.Serialize.Gaming;
    using Wix = Microsoft.Tools.WindowsInstallerXml.Serialize;

    /// <summary>
    /// The decompiler for the Windows Installer XML Toolset Gaming Extension.
    /// </summary>
    public sealed class GamingDecompiler : DecompilerExtension
    {
        /// <summary>
        /// Decompiles an extension table.
        /// </summary>
        /// <param name="table">The table to decompile.</param>
        public override void DecompileTable(Table table)
        {
            switch (table.Name)
            {
                case "WixGameExplorer":
                    this.DecompileWixGameExplorerTable(table);
                    break;
                default:
                    base.DecompileTable(table);
                    break;
            }
        }

        /// <summary>
        /// Decompile the WixGameExplorer table.
        /// </summary>
        /// <param name="table">The table to decompile.</param>
        private void DecompileWixGameExplorerTable(Table table)
        {
            foreach (Row row in table.Rows)
            {
                Gaming.Game game = new Gaming.Game();

                game.Id = (string)row[0];

                Wix.File file = (Wix.File)this.Core.GetIndexedElement("File", (string)row[1]);
                if (null != file)
                {
                    file.AddChild(game);
                }
                else
                {
                    this.Core.OnMessage(WixWarnings.ExpectedForeignRow(row.SourceLineNumbers, table.Name, row.GetPrimaryKey(DecompilerCore.PrimaryKeyDelimiter), "File_", (string)row[1], "File"));
                }
            }
        }
    }
}
