//-------------------------------------------------------------------------------------------------
// <copyright file="OfficeAddinClickThroughConsole.cs" company="Microsoft">
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
// Office Addin ClickThrough console extension.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Extensions
{
    using System;
    using System.IO;

    /// <summary>
    /// Creates a new isolated app ClickThrough console extension.
    /// </summary>
    public sealed class OfficeAddinClickThroughConsole : ClickThroughConsoleExtension
    {
        private OfficeAddinFabricator fabricator;

        /// <summary>
        /// Creates a new OfficeAddinClickThroughConsole.
        /// </summary>
        public OfficeAddinClickThroughConsole()
        {
            this.fabricator = new OfficeAddinFabricator();
        }

        /// <summary>
        /// Gets the fabrictor for this extension.
        /// </summary>
        /// <value>Fabricator for IsolatedApp Add-in.</value>
        public override Fabricator Fabricator
        {
            get { return this.fabricator; }
        }

        /// <summary>
        /// Gets the supported command line types for this extension.
        /// </summary>
        /// <value>The supported command line types for this extension.</value>
        public override CommandLineOption[] CommandLineTypes
        {
            get
            {
                return new CommandLineOption[]
                {
                    new CommandLineOption("officeaddin", "build an Office Addin", 0),
                    new CommandLineOption("src", "specify the source to write the .wxs file out to.", 1),
                };
            }
        }

        /// <summary>
        /// Parse the command line options for this extension.
        /// </summary>
        /// <param name="args">The option arguments.</param>
        public override void ParseOptions(string[] args)
        {
            string filePath = null;

            // parse the options
            for (int i = 0; i < args.Length; ++i)
            {
                string arg = args[i];
                if (null == arg || 0 == arg.Length) // skip blank arguments
                {
                    continue;
                }

                if ('-' == arg[0] || '/' == arg[0])
                {
                    string parameter = arg.Substring(1);

                    if ("src" == parameter)
                    {
                        if (!CommandLine.IsValidArg(args, ++i))
                        {
                            throw new ArgumentException(String.Concat("-", parameter), "args");
                        }

                        this.fabricator.SaveSourceFile = Path.GetFullPath(args[i]);
                    }
                }
                else if (filePath == null)
                {
                    filePath = arg;
                }
                else
                {
                    throw new ArgumentException("Unexpected argument.");
                }
            }

            if (filePath != null)
            {
                this.Fabricator.Open(filePath);
            }
        }
    }
}
