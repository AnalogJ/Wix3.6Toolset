//--------------------------------------------------------------------------------------------------
// <copyright file="PyroTask.cs" company="Microsoft">
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
// NAnt task for the pyro tool.
// </summary>
//--------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.NAntTasks
{
    using System;
    using System.IO;

    using NAnt.Core;
    using NAnt.Core.Attributes;
    using NAnt.Core.Types;

    /// <summary>
    /// Represents the NAnt task for the &lt;pyro&gt; element in a NAnt script.
    /// </summary>
    [TaskName("pyro")]
    public class PyroTask : SingleOutputWixTask
    {
        // =========================================================================================
        // Member Variables
        // =========================================================================================

        private Transform[] transforms;

        // =========================================================================================
        // Constructors
        // =========================================================================================

        /// <summary>
        /// Initializes a new instance of the <see cref="PyroTask"/> class.
        /// </summary>
        public PyroTask()
            : base("pyro.exe")
        {
        }

        // =========================================================================================
        // Properties
        // =========================================================================================

        /// <summary>
        /// Gets or sets the list of transforms.
        /// </summary>
        [BuildElementCollection("transforms", "transform", ElementType = typeof(Transform))]
        public Transform[] Transforms
        {
            get { return this.transforms; }
            set { this.transforms = value; }
        }

        // =========================================================================================
        // Methods
        // =========================================================================================

        /// <summary>
        /// Writes all of the command-line parameters for the tool to a response file, one parameter per line.
        /// </summary>
        /// <param name="writer">The output writer.</param>
        protected override void WriteOptions(TextWriter writer)
        {
            base.WriteOptions(writer);

            foreach (Transform transform in this.Transforms)
            {
                writer.WriteLine("-t \"{0}\" \"{1}\"", transform.Baseline, transform.FilePath);
            }
        }
    }
}