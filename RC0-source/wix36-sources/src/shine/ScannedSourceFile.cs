//-------------------------------------------------------------------------------------------------
// <copyright file="ScannedSourceFile.cs" company="Microsoft">
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
// Source file found by the Windows Installer Xml toolset scanner.
// </summary>
//-------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Text;

namespace Microsoft.Tools.WindowsInstallerXml
{
    public class ScannedSourceFile
    {
        public ScannedSourceFile(string path)
            : this(path, null)
        {
        }

        public ScannedSourceFile(string path, IDictionary<string, string> preprocessorDefines)
        {
            StringBuilder keyBuilder = new StringBuilder();
            keyBuilder.Append(path.ToLowerInvariant());

            this.Path = path;

            this.PreprocessorDefines = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
            if (null != preprocessorDefines)
            {
                foreach (KeyValuePair<string, string> kvp in preprocessorDefines)
                {
                    this.PreprocessorDefines.Add(kvp.Key, kvp.Value);
                }
            }

            this.Key = ScannedSourceFile.CalculateKey(this.Path, this.PreprocessorDefines);
            this.SourceProjects = new List<ScannedProject>();
            this.TargetSymbols = new List<ScannedSymbol>();
        }

        public string Key { get; private set; }

        public string Path { get; private set; }

        public IDictionary<string, string> PreprocessorDefines { get; private set; }

        public IList<ScannedProject> SourceProjects { get; private set; }

        public IList<ScannedSymbol> TargetSymbols { get; private set; }

        public static string CalculateKey(string path, IDictionary<string, string> preprocessorDefines)
        {
            StringBuilder keyBuilder = new StringBuilder();
            keyBuilder.Append(path.ToLowerInvariant());

            if (null != preprocessorDefines)
            {
                foreach (KeyValuePair<string, string> kvp in preprocessorDefines)
                {
                    keyBuilder.AppendFormat(";{0}={1}", kvp.Key, kvp.Value);
                }
            }

            return keyBuilder.ToString();
        }
    }
}
