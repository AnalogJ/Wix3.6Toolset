//-------------------------------------------------------------------------------------------------
// <copyright file="HeatTask.cs" company="Microsoft">
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
// Build task to execute the harvester of the Windows Installer Xml toolset.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Build.Tasks
{
    using System;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Text;

    using Microsoft.Build.Framework;
    using Microsoft.Build.Utilities;

    /// <summary>
    /// A base MSBuild task to run the WiX harvester.
    /// Specific harvester tasks should extend this class.
    /// </summary>
    public abstract class HeatTask : WixToolTask
    {
        private const string HeatToolName = "Heat.exe";

        private bool autogenerageGuids;
        private bool generateGuidsNow;
        private ITaskItem outputFile;
        private bool suppressFragments;
        private bool suppressUniqueIds;
        private string[] transforms;

        public bool AutogenerateGuids
        {
            get { return this.autogenerageGuids; }
            set { this.autogenerageGuids = value; }
        }

        public bool GenerateGuidsNow
        {
            get { return this.generateGuidsNow; }
            set { this.generateGuidsNow = value; }
        }

        [Required]
        [Output]
        public ITaskItem OutputFile
        {
            get { return this.outputFile; }
            set { this.outputFile = value; }
        }

        public bool SuppressFragments
        {
            get { return this.suppressFragments; }
            set { this.suppressFragments = value; }
        }

        public bool SuppressUniqueIds
        {
            get { return this.suppressUniqueIds; }
            set { this.suppressUniqueIds = value; }
        }

        public string[] Transforms
        {
            get { return this.transforms; }
            set { this.transforms = value; }
        }

        /// <summary>
        /// Get the name of the executable.
        /// </summary>
        /// <remarks>The ToolName is used with the ToolPath to get the location of heat.exe.</remarks>
        /// <value>The name of the executable.</value>
        protected override string ToolName
        {
            get { return HeatToolName; }
        }

        /// <summary>
        /// Gets the name of the heat operation performed by the task.
        /// </summary>
        /// <remarks>This is the first parameter passed on the heat.exe command-line.</remarks>
        /// <value>The name of the heat operation performed by the task.</value>
        protected abstract string OperationName
        {
            get;
        }

        /// <summary>
        /// Get the path to the executable. 
        /// </summary>
        /// <remarks>GetFullPathToTool is only called when the ToolPath property is not set (see the ToolName remarks above).</remarks>
        /// <returns>The full path to the executable or simply heat.exe if it's expected to be in the system path.</returns>
        protected override string GenerateFullPathToTool()
        {
            // If there's not a ToolPath specified, it has to be in the system path.
            if (String.IsNullOrEmpty(this.ToolPath))
            {
                return HeatToolName;
            }

            return Path.Combine(Path.GetFullPath(this.ToolPath), HeatToolName);
        }

        /// <summary>
        /// Builds a command line from options in this task.
        /// </summary>
        protected override void BuildCommandLine(WixCommandLineBuilder commandLineBuilder)
        {
            base.BuildCommandLine(commandLineBuilder);

            commandLineBuilder.AppendIfTrue("-ag", this.AutogenerateGuids);
            commandLineBuilder.AppendIfTrue("-gg", this.GenerateGuidsNow);
            commandLineBuilder.AppendIfTrue("-nologo", this.NoLogo);
            commandLineBuilder.AppendIfTrue("-sfrag", this.SuppressFragments);
            commandLineBuilder.AppendIfTrue("-suid", this.SuppressUniqueIds);
            commandLineBuilder.AppendArrayIfNotNull("-sw", this.SuppressSpecificWarnings);
            commandLineBuilder.AppendArrayIfNotNull("-t ", this.Transforms);
            commandLineBuilder.AppendTextIfNotNull(this.AdditionalOptions);
            commandLineBuilder.AppendSwitchIfNotNull("-out ", this.OutputFile);
        }
    }
}
