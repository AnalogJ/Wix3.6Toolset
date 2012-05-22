//-------------------------------------------------------------------------------------------------
// <copyright file="WixMediaTemplateRow.cs" company="Microsoft">
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
// Specialization of a row for the MediaTeplate table.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.Globalization;
    using System.Text;

    /// <summary>
    /// Specialization of a row for the MediaTemplate table.
    /// </summary>
    public sealed class WixMediaTemplateRow : Row
    {
        private Cab.CompressionLevel compressionLevel;
        private bool hasExplicitCompressionLevel;

        /// <summary>
        /// Creates a MediaTemplate row that belongs to a table.
        /// </summary>
        /// <param name="sourceLineNumbers">Original source lines for this row.</param>
        /// <param name="table">Table this MediaTeplate row belongs to and should get its column definitions from.</param>
        public WixMediaTemplateRow(SourceLineNumberCollection sourceLineNumbers, Table table)
            : base(sourceLineNumbers, table)
        {
            // default the compression level to mszip
            this.compressionLevel = Cab.CompressionLevel.Mszip;
        }

        /// <summary>
        /// Gets or sets the cabinet template name for this media template row.
        /// </summary>
        /// <value>Cabinet name.</value>
        public string CabinetTemplate
        {
            get { return (string)this.Fields[1].Data; }
            set { this.Fields[1].Data = value; }
        }

        /// <summary>
        /// Gets or sets the disk prompt for this media template row.
        /// </summary>
        /// <value>Disk prompt.</value>
        public string DiskPrompt
        {
            get { return (string)this.Fields[2].Data; }
            set { this.Fields[2].Data = value; }
        }


        /// <summary>
        /// Gets or sets the volume label for this media template row.
        /// </summary>
        /// <value>Volume label.</value>
        public string VolumeLabel
        {
            get { return (string)this.Fields[3].Data; }
            set { this.Fields[3].Data = value; }
        }

        /// <summary>
        /// Gets or sets the maximum uncompressed media size for this media template row.
        /// </summary>
        /// <value>Disk id.</value>
        public int MaximumUncompressedMediaSize
        {
            get { return (int)this.Fields[4].Data; }
            set { this.Fields[4].Data = value; }
        }


        /// <summary>
        /// Gets or sets the compression level for this media template row.
        /// </summary>
        /// <value>Compression level.</value>
        public Cab.CompressionLevel CompressionLevel
        {
            get { return this.compressionLevel; }
            set
            {
                this.compressionLevel = value;
                this.hasExplicitCompressionLevel = true;
            }
        }

        /// <summary>
        /// Gets a value indicating whether the compression level for this media template row has been set.
        /// </summary>
        /// <value>Compression level.</value>
        public bool HasExplicitCompressionLevel
        {
            get { return this.hasExplicitCompressionLevel; }
        }

    }
}
