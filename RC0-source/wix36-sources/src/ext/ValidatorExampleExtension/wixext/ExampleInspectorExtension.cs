//-------------------------------------------------------------------------------------------------
// <copyright file="ExampleInspectorExtension.cs" company="Microsoft">
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
// The Windows Installer XML Toolset Inspector Example Extension.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Extensions
{
    using System;
    using Microsoft.Tools.WindowsInstallerXml;

    /// <summary>
    /// The Windows Installer XML Toolset Example InspectorExtension.
    /// </summary>
    public sealed class ExampleInspectorExtension : InspectorExtension
    {
        public override void InspectIntermediate(Intermediate output)
        {
            foreach (Section section in output.Sections)
            {
                Table fileTable = section.Tables["File"];
                if (null != fileTable && 0 < fileTable.Rows.Count)
                {
                    Row fileRow = fileTable.Rows[0];
                    this.Core.OnMessage(ValidationWarnings.ExampleWarning(fileRow.SourceLineNumbers));

                    return;
                }
            }

            this.Core.OnMessage(ValidationErrors.ExampleError());
        }

        public override void InspectOutput(Output output)
        {
            Table fileTable = output.Tables["File"];
            if (null != fileTable && 0 < fileTable.Rows.Count)
            {
                Row fileRow = fileTable.Rows[0];
                this.Core.OnMessage(ValidationWarnings.ExampleWarning(fileRow.SourceLineNumbers));

                return;
            }

            this.Core.OnMessage(ValidationErrors.ExampleError());
        }

        public override void InspectTransform(Output transform)
        {
            Table fileTable = transform.Tables["File"];
            if (null != fileTable && 0 < fileTable.Rows.Count)
            {
                Row fileRow = fileTable.Rows[0];
                this.Core.OnMessage(ValidationWarnings.ExampleWarning(fileRow.SourceLineNumbers));

                return;
            }

            this.Core.OnMessage(ValidationErrors.ExampleError());
        }

        public override void InspectPatch(Output patch)
        {
            foreach (SubStorage transform in patch.SubStorages)
            {
                // Skip patch transforms.
                if (transform.Name.StartsWith("#"))
                {
                    continue;
                }

                Table fileTable = transform.Data.Tables["File"];
                if (null != fileTable && 0 < fileTable.Rows.Count)
                {
                    Row fileRow = fileTable.Rows[0];
                    this.Core.OnMessage(ValidationWarnings.AnotherExampleWarning(fileRow.SourceLineNumbers));

                    return;
                }
            }

            this.Core.OnMessage(ValidationErrors.ExampleError());
        }
    }
}
