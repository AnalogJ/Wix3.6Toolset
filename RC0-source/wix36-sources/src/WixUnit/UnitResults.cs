//-------------------------------------------------------------------------------------------------
// <copyright file="UnitResults.cs" company="Microsoft">
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
// Container for the results from a single unit test run.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Unit
{
    using System;
    using System.Collections;
    using System.Xml;

    /// <summary>
    /// Container for the results from a single unit test run.
    /// </summary>
    internal sealed class UnitResults
    {
        private ArrayList errors;
        private ArrayList output;
        private ArrayList outputFiles;

        /// <summary>
        /// Instantiate a new UnitResults.
        /// </summary>
        public UnitResults()
        {
            this.errors = new ArrayList();
            this.output = new ArrayList();
            this.outputFiles = new ArrayList();
        }

        /// <summary>
        /// Gets the ArrayList of error strings.
        /// </summary>
        /// <value>The ArrayList of error strings.</value>
        public ArrayList Errors
        {
            get { return this.errors; }
        }

        /// <summary>
        /// Gets the ArrayList of output strings.
        /// </summary>
        /// <value>The ArrayList of output strings.</value>
        public ArrayList Output
        {
            get { return this.output; }
        }

        /// <summary>
        /// Gets the ArrayList of output files.
        /// </summary>
        /// <value>The ArrayList of output files.</value>
        public ArrayList OutputFiles
        {
            get { return this.outputFiles; }
        }
    }
}
