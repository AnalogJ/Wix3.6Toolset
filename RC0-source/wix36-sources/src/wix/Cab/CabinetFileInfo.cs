//-------------------------------------------------------------------------------------------------
// <copyright file="CabinetFileInfo.cs" company="Microsoft">
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
// Contains properties for a file inside a cabinet
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;

    /// <summary>
    /// Properties of a file in a cabinet.
    /// </summary>
    internal sealed class CabinetFileInfo
    {
        private string fileId;
        private ushort date;
        private ushort time;

        /// <summary>
        /// Constructs CabinetFileInfo
        /// </summary>
        /// <param name="fileId">File Id</param>
        /// <param name="date">Last modified date (MS-DOS time)</param>
        /// <param name="time">Last modified time (MS-DOS time)</param>
        public CabinetFileInfo(string fileId, ushort date, ushort time)
        {
            this.fileId = fileId;
            this.date = date;
            this.time = time;
        }

        /// <summary>
        /// Gets the file Id of the file.
        /// </summary>
        /// <value>file Id</value>
        public string FileId
        {
            get { return this.fileId; }
        }

        /// <summary>
        /// Gets modified date (DOS format).
        /// </summary>
        public ushort Date
        {
            get { return this.date; }
        }

        /// <summary>
        /// Gets modified time (DOS format).
        /// </summary>
        public ushort Time
        {
            get { return this.time; }
        }
    }
}
