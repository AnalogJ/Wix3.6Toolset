//-------------------------------------------------------------------------------------------------
// <copyright file="IfDefEventHandler.cs" company="Microsoft">
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
// Preprocessor ifdef/ifndef event handler and event args.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.Text;

    public delegate void IfDefEventHandler(object sender, IfDefEventArgs e);

    public class IfDefEventArgs : EventArgs
    {
        private SourceLineNumberCollection sourceLineNumbers;
        private bool isIfDef;
        private bool isDefined;
        private string variableName;

        public IfDefEventArgs(SourceLineNumberCollection sourceLineNumbers, bool isIfDef, bool isDefined, string variableName)
        {
            this.sourceLineNumbers = sourceLineNumbers;
            this.isIfDef = isIfDef;
            this.isDefined = isDefined;
            this.variableName = variableName;
        }

        public SourceLineNumberCollection SourceLineNumbers
        {
            get { return this.sourceLineNumbers; }
        }

        public bool IsDefined
        {
            get { return this.isDefined; }
        }

        public bool IsIfDef
        {
            get { return this.isIfDef; }
        }

        public string VariableName
        {
            get { return this.variableName; }
        }
    }
}
