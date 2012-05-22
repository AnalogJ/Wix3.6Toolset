//-------------------------------------------------------------------------------------------------
// <copyright file="CommandLineOption.cs" company="Microsoft">
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
// Command line option used by console tools.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    /// <summary>
    /// A command line option.
    /// </summary>
    public struct CommandLineOption
    {
        public string Option;
        public string Description;
        public int AdditionalArguments;

        /// <summary>
        /// Instantiates a new BuilderCommandLineOption.
        /// </summary>
        /// <param name="option">The option name.</param>
        /// <param name="description">The description of the option.</param>
        /// <param name="additionalArguments">Count of additional arguments to require after this switch.</param>
        public CommandLineOption(string option, string description, int additionalArguments)
        {
            this.Option = option;
            this.Description = description;
            this.AdditionalArguments = additionalArguments;
        }
    }
}
