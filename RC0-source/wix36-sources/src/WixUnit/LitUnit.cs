//-------------------------------------------------------------------------------------------------
// <copyright file="LitUnit.cs" company="Microsoft">
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
// The Windows Installer XML Lit unit tester.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Unit
{
    using System;
    using System.Collections;
    using System.IO;
    using System.Text;
    using System.Xml;

    /// <summary>
    /// The Windows Installer XML Lit unit tester.
    /// </summary>
    internal sealed class LitUnit
    {
        /// <summary>
        /// Private constructor to prevent instantiation of static class.
        /// </summary>
        private LitUnit()
        {
        }

        /// <summary>
        /// Run a Lit unit test.
        /// </summary>
        /// <param name="element">The unit test element.</param>
        /// <param name="previousUnitResults">The previous unit test results.</param>
        /// <param name="args">The command arguments passed to WixUnit.</param>
        public static void RunUnitTest(XmlElement element, UnitResults previousUnitResults, ICommandArgs args)
        {
            string arguments = element.GetAttribute("Arguments");
            string expectedErrors = element.GetAttribute("ExpectedErrors");
            string expectedWarnings = element.GetAttribute("ExpectedWarnings");
            string extensions = element.GetAttribute("Extensions");
            string tempDirectory = element.GetAttribute("TempDirectory");
            string toolsDirectory = element.GetAttribute("ToolsDirectory");

            string toolFile = Path.Combine(toolsDirectory, "lit.exe");
            StringBuilder commandLine = new StringBuilder(arguments);

            // handle extensions
            if (!String.IsNullOrEmpty(extensions))
            {
                foreach (string extension in extensions.Split(';'))
                {
                    commandLine.AppendFormat(" -ext \"{0}\"", extension);
                }
            }

            // handle any previous outputs
            foreach (string databaseFile in previousUnitResults.OutputFiles)
            {
                commandLine.AppendFormat(" \"{0}\"", databaseFile);
            }
            previousUnitResults.OutputFiles.Clear();

            string outputFile = Path.Combine(tempDirectory, "library.wixlib");
            commandLine.AppendFormat(" -out \"{0}\"", outputFile);
            previousUnitResults.OutputFiles.Add(outputFile);

            // run the tool
            ArrayList output = ToolUtility.RunTool(toolFile, commandLine.ToString());
            previousUnitResults.Errors.AddRange(ToolUtility.GetErrors(output, expectedErrors, expectedWarnings));
            previousUnitResults.Output.AddRange(output);
        }
    }
}
