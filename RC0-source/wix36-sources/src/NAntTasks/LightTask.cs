//--------------------------------------------------------------------------------------------------
// <copyright file="LightTask.cs" company="Microsoft">
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
// NAnt task for the light linker.
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
    /// Represents the NAnt task for the &lt;light&gt; element in a NAnt script.
    /// </summary>
    [TaskName("light")]
    public class LightTask : SingleOutputWixTask
    {
        #region Member Variables
        //==========================================================================================
        // Member Variables
        //==========================================================================================

        private string cultures;
        private FileSet localizations;
        private string suppressICEs;
        private bool suppressPdb;
        private bool reuseCab;
        private string cabCachePath;
        private bool fileVersions;
        #endregion

        #region Constructors
        //==========================================================================================
        // Constructors
        //==========================================================================================

        /// <summary>
        /// Initializes a new instance of the <see cref="LightTask"/> class.
        /// </summary>
        public LightTask() : base("light.exe")
        {
            this.localizations = new FileSet();
        }
        #endregion

        #region Properties
        //==========================================================================================
        // Properties
        //==========================================================================================

        /// <summary>
        /// Gets or sets the cultures to use.
        /// </summary>
        [TaskAttribute("cultures")]
        public string Cultures
        {
            get { return this.cultures; }
            set { this.cultures = value; }
        }

        /// <summary>
        /// Gets or sets the localization files to use.
        /// </summary>
        [BuildElement("localizations")]
        public FileSet Localizations
        {
            get { return this.localizations; }
            set { this.localizations = value; }
        }

        /// <summary>
        /// Gets or sets the option to suppress particular ICEs.
        /// </summary>
        [TaskAttribute("suppressices")]
        public string SuppressICEs
        {
            get { return this.suppressICEs; }
            set { this.suppressICEs = value; }
        }

        /// <summary>
        /// Gets or sets the option to suppress build the pdb file.
        /// </summary>
        [BooleanValidator]
        [TaskAttribute("suppresspdb")]
        public bool SuppressPdb
        {
            get { return this.suppressPdb; }
            set { this.suppressPdb = value; }
        }
 
        /// <summary>
        /// Gets or sets the option to reuse the cached cab file.
        /// </summary>
        [BooleanValidator]
        [TaskAttribute("reusecab")]
        public bool ReuseCab
        {
            get { return this.reuseCab; }
            set { this.reuseCab = value; }
        }
 
        /// <summary>
        /// Gets or sets the option to suppress particular ICEs.
        /// </summary>
        [TaskAttribute("cabcache")]
        public string CabCachePath
        {
            get { return this.cabCachePath; }
            set { this.cabCachePath = value; }
        }
 
        /// <summary>
        /// Gets or sets the option to add a 'fileVersion' entry to the MsiAssemblyName table.
        /// </summary>
        [BooleanValidator]
        [TaskAttribute("fileversions")]
        public bool FileVersions
        {
            get { return fileVersions; }
            set { fileVersions = value; }
        }
        #endregion

        #region Methods
        //==========================================================================================
        // Methods
        //==========================================================================================

        /// <summary>
        /// Writes all of the command-line parameters for the tool to a response file, one parameter per line.
        /// </summary>
        /// <param name="writer">The output writer.</param>
        protected override void WriteOptions(TextWriter writer)
        {
            base.WriteOptions(writer);

            if (null != this.cultures)
            {
                writer.WriteLine("-cultures:{0}", this.cultures);
            }

            foreach (string fileName in this.localizations.FileNames)
            {
                writer.WriteLine("-loc \"{0}\"", fileName);
            }

            if (this.suppressICEs != null)
            {
                foreach (string suppressICE in this.suppressICEs.Split(';'))
                {
                    writer.WriteLine("-sice:{0}", suppressICE);
                }
            }

            if (this.suppressPdb)
            {
                writer.WriteLine("-spdb");
            }

            if (this.reuseCab)
            {
                writer.WriteLine("-reusecab");
            }

            if (null != this.cabCachePath)
            {
                writer.WriteLine("-cc {0}", Utility.QuotePathIfNeeded(this.cabCachePath));
            }

            if (this.fileVersions)
            {
                writer.WriteLine("-fv");
            }
        }
        #endregion
    }
}