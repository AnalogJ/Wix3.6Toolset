//-------------------------------------------------------------------------------------------------
// <copyright file="ResolvedVariableEventHandler.cs" company="Microsoft">
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
// Resolved variable event handler and event args.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.Text;

    public delegate void ResolvedVariableEventHandler(object sender, ResolvedVariableEventArgs e);

    public class ResolvedVariableEventArgs : EventArgs
    {
        private SourceLineNumberCollection sourceLineNumbers;
        private string variableName;
        private string variableValue;

        public ResolvedVariableEventArgs(SourceLineNumberCollection sourceLineNumbers, string variableName, string variableValue)
        {
            this.sourceLineNumbers = sourceLineNumbers;
            this.variableName = variableName;
            this.variableValue = variableValue;
        }

        public SourceLineNumberCollection SourceLineNumbers
        {
            get { return this.sourceLineNumbers; }
        }

        public string VariableName
        {
            get { return this.variableName; }
        }

        public string VariableValue
        {
            get { return this.variableValue; }
        }
    }
}
