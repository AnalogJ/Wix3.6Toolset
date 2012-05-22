//-------------------------------------------------------------------------------------------------
// <copyright file="AutoMediaAssigner.cs" company="Microsoft">
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
// AutoMediaAssigner assigns files to cabs depending on Media or MediaTemplate.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.Collections;
    using System.Reflection;
    using System.IO;
    using System.Globalization;
    using System.Collections.Generic;

    /// <summary>
    /// AutoMediaAssigner assigns files to cabs depending on Media or MediaTemplate.
    /// </summary>
    public class AutoMediaAssigner
    {
        private BinderCore core;
        private bool filesCompressed;
        private string cabinetNameTemplate;

        private Hashtable cabinets;
        private MediaRowCollection mediaRows;
        private FileRowCollection uncompressedFileRows;

        private Output output;

        /// <summary>
        /// Gets cabinets 
        /// </summary>
        public virtual Hashtable Cabinets
        {
            get { return cabinets; }
        }

        /// <summary>
        /// Get media rows
        /// </summary>
        /// <value>MediaRowCollection</value>
        public virtual MediaRowCollection MediaRows
        {
            get { return mediaRows; }
        }

        /// <summary>
        /// Get uncompressed file rows. This will contain file rows of File elements that are marked with compression=no.
        /// This contains all the files when Package element is marked with compression=no
        /// </summary>
        /// <value>FileRowCollection</value>
        public FileRowCollection UncompressedFileRows
        {
            get { return uncompressedFileRows; }
            set { uncompressedFileRows = value; }
        }

        /// <summary>
        /// Constructor for auto media assigner.
        /// </summary>
        /// <param name="output">Output</param>
        /// <param name="core">Binder core.</param>
        /// <param name="filesCompressed">True if files are compressed by default. </param>
        public AutoMediaAssigner(Output output, BinderCore core, bool filesCompressed)
        {
            this.output = output;
            this.core = core;
            this.filesCompressed = filesCompressed;
            this.cabinetNameTemplate = "Cab{0}.cab";

            uncompressedFileRows = new FileRowCollection();
            mediaRows = new MediaRowCollection();
            cabinets = new Hashtable();
        }

        /// <summary>
        /// Assigns the file rows to media rows based on Media or MediaTemplate authoring. Updates uncompressed files collection.
        /// </summary>
        /// <param name="fileRows">FileRowCollection</param>
        public void AssignFiles(FileRowCollection fileRows)
        {
            bool autoAssign = false;
            MediaRow mergeModuleMediaRow = null;
            Table mediaTable = this.output.Tables["Media"];
            Table mediaTemplateTable = this.output.Tables["WixMediaTemplate"];

            // If both tables are authored, it is an error.
            if ((mediaTemplateTable != null && mediaTemplateTable.Rows.Count > 0) &&( mediaTable != null && mediaTable.Rows.Count > 1))
            {
                throw new WixException(WixErrors.MediaTableCollision(null));
            }

            autoAssign = mediaTemplateTable != null && OutputType.Module != this.output.Type ? true : false;

            // When building merge module, all the files go to "#MergeModule.CABinet"
            if (OutputType.Module == this.output.Type)
            {
                Table mergeModuleMediaTable = new Table(null, this.core.TableDefinitions["Media"]);
                mergeModuleMediaRow = (MediaRow)mergeModuleMediaTable.CreateRow(null);
                mergeModuleMediaRow.Cabinet = "#MergeModule.CABinet";

                this.cabinets.Add(mergeModuleMediaRow, new FileRowCollection());
            }

            if (autoAssign)
            {
                this.AutoAssignFiles(mediaTable, fileRows);
            }
            else
            {
                this.ManuallyAssignFiles(mediaTable, mergeModuleMediaRow, fileRows);
            }
        }

        /// <summary>
        /// Assigne files to cabinets based on MediaTemplate authoring.
        /// </summary>
        /// <param name="fileRows">FileRowCollection</param>
        private void AutoAssignFiles(Table mediaTable, FileRowCollection fileRows)
        {
            const int MaxCabIndex = 999;

            ulong currentPreCabSize = 0;
            ulong maxPreCabSizeInBytes;
            int maxPreCabSizeInMB = 0;
            int currentCabIndex = 0;

            MediaRow currentMediaRow = null;

            Table mediaTemplateTable = this.output.Tables["WixMediaTemplate"];

            // Auto assign files to cabinets based on maximum uncompressed media size
            mediaTable.Rows.Clear();
            WixMediaTemplateRow mediaTemplateRow = (WixMediaTemplateRow)mediaTemplateTable.Rows[0];

            if (!String.IsNullOrEmpty(mediaTemplateRow.CabinetTemplate))
            {
                this.cabinetNameTemplate = mediaTemplateRow.CabinetTemplate;
            }

            string mumsString = Environment.GetEnvironmentVariable("WIX_MUMS");

            try
            {
                // Override authored mums value if environment variable is authored.
                if (!String.IsNullOrEmpty(mumsString))
                {
                    maxPreCabSizeInMB = Int32.Parse(mumsString);
                }
                else
                {
                    maxPreCabSizeInMB = mediaTemplateRow.MaximumUncompressedMediaSize;
                }

                maxPreCabSizeInBytes = (ulong)maxPreCabSizeInMB * 1024 * 1024;
            }
            catch (FormatException)
            {
                throw new WixException(WixErrors.IllegalEnvironmentVariable("WIX_MUMS", mumsString));
            }
            catch (OverflowException)
            {
                throw new WixException(WixErrors.MaximumUncompressedMediaSizeTooLarge(null, maxPreCabSizeInMB));
            }

            foreach (FileRow fileRow in fileRows)
            {
                // When building a product, if the current file is not to be compressed or if 
                // the package set not to be compressed, don't cab it.
                if (OutputType.Product == output.Type &&
                    (YesNoType.No == fileRow.Compressed ||
                    (YesNoType.NotSet == fileRow.Compressed && !this.filesCompressed)))
                {
                    uncompressedFileRows.Add(fileRow);
                    continue;
                }

                FileInfo fileInfo = null;

                // Get the file size
                try
                {
                    fileInfo = new FileInfo(fileRow.Source);
                }
                catch (ArgumentException)
                {
                    this.core.OnMessage(WixErrors.InvalidFileName(fileRow.SourceLineNumbers, fileRow.Source));
                }
                catch (PathTooLongException)
                {
                    this.core.OnMessage(WixErrors.InvalidFileName(fileRow.SourceLineNumbers, fileRow.Source));
                }
                catch (NotSupportedException)
                {
                    this.core.OnMessage(WixErrors.InvalidFileName(fileRow.SourceLineNumbers, fileRow.Source));
                }

                if (fileInfo.Exists)
                {
                    if (fileInfo.Length > Int32.MaxValue)
                    {
                        throw new WixException(WixErrors.FileTooLarge(fileRow.SourceLineNumbers, fileRow.Source));
                    }

                    fileRow.FileSize = Convert.ToInt32(fileInfo.Length, CultureInfo.InvariantCulture);
                }

                if (currentCabIndex == MaxCabIndex)
                {
                    // Associate current file with last cab (irrespective of the size) and cab index is not incremented anymore.
                    FileRowCollection cabinetFileRow = (FileRowCollection)this.cabinets[currentMediaRow];
                    fileRow.DiskId = currentCabIndex;
                    cabinetFileRow.Add(fileRow);
                    continue;
                }

                // Update current cab size.
                currentPreCabSize += (ulong)fileRow.FileSize;

                if (currentPreCabSize > maxPreCabSizeInBytes)
                {
                    // Overflow due to current file
                    if (currentPreCabSize == (ulong)fileRow.FileSize)
                    {
                        // Only one file in this cab.
                        currentMediaRow = AddMediaRow(mediaTable, ++currentCabIndex);

                        FileRowCollection cabinetFileRow = (FileRowCollection)cabinets[currentMediaRow];
                        fileRow.DiskId = currentCabIndex;
                        cabinetFileRow.Add(fileRow);
                        currentPreCabSize = 0;
                    }
                    else
                    {
                        currentMediaRow = this.AddMediaRow(mediaTable, ++currentCabIndex);

                        // Finish current media row and create new one.
                        FileRowCollection cabinetFileRow = (FileRowCollection)this.cabinets[currentMediaRow];
                        fileRow.DiskId = currentCabIndex;
                        cabinetFileRow.Add(fileRow);
                        currentPreCabSize = (ulong)fileRow.FileSize;
                    }
                }
                else
                {
                    // File fits in the current cab.
                    if (currentMediaRow == null)
                    {
                        // Create new cab and MediaRow
                        currentMediaRow = AddMediaRow(mediaTable, ++currentCabIndex);
                    }

                    // Associate current file with current cab.
                    FileRowCollection cabinetFileRow = (FileRowCollection)this.cabinets[currentMediaRow];
                    fileRow.DiskId = currentCabIndex;
                    cabinetFileRow.Add(fileRow);
                }
            }

            // If there are uncompressed files and no MediaRow, create a default one.
            if (uncompressedFileRows.Count > 0 && mediaTable.Rows.Count == 0 )
            {
                 MediaRow defaultMediaRow = (MediaRow)mediaTable.CreateRow(null);
                 defaultMediaRow.DiskId = 1;
                 mediaRows.Add(defaultMediaRow);
            }
        }

        /// <summary>
        /// Assign files to cabinets based on Media authoring.
        /// </summary>
        /// <param name="mediaTable"></param>
        /// <param name="mergeModuleMediaRow"></param>
        /// <param name="fileRows"></param>
        private void ManuallyAssignFiles(Table mediaTable, MediaRow mergeModuleMediaRow, FileRowCollection fileRows)
        {
            if (OutputType.Module != this.output.Type)
            {
                if (null != mediaTable)
                {
                    Dictionary<string, MediaRow> cabinetMediaRows = new Dictionary<string, MediaRow>(StringComparer.InvariantCultureIgnoreCase);
                    foreach (MediaRow mediaRow in mediaTable.Rows)
                    {
                        // If the Media row has a cabinet, make sure it is unique across all Media rows.
                        if (!String.IsNullOrEmpty(mediaRow.Cabinet))
                        {
                            MediaRow existingRow;
                            if (cabinetMediaRows.TryGetValue(mediaRow.Cabinet, out existingRow))
                            {
                                this.core.OnMessage(WixErrors.DuplicateCabinetName(mediaRow.SourceLineNumbers, mediaRow.Cabinet));
                                this.core.OnMessage(WixErrors.DuplicateCabinetName2(existingRow.SourceLineNumbers, existingRow.Cabinet));
                            }
                            else
                            {
                                cabinetMediaRows.Add(mediaRow.Cabinet, mediaRow);
                            }
                        }

                        this.mediaRows.Add(mediaRow);
                    }
                }

                foreach (MediaRow mediaRow in this.mediaRows)
                {
                    if (null != mediaRow.Cabinet)
                    {
                        this.cabinets.Add(mediaRow, new FileRowCollection());
                    }
                }
            }

            foreach (FileRow fileRow in fileRows)
            {
                if (OutputType.Module == output.Type)
                {
                    ((FileRowCollection)this.cabinets[mergeModuleMediaRow]).Add(fileRow);
                }
                else
                {
                    MediaRow mediaRow = this.mediaRows[fileRow.DiskId];

                    // When building a product, if the current file is not to be compressed or if 
                    // the package set not to be compressed, don't cab it.
                    if (OutputType.Product == output.Type &&
                        (YesNoType.No == fileRow.Compressed ||
                        (YesNoType.NotSet == fileRow.Compressed && !this.filesCompressed)))
                    {
                        uncompressedFileRows.Add(fileRow);
                    }
                    else // file in a Module or marked compressed
                    {
                        FileRowCollection cabinetFileRow = (FileRowCollection)this.cabinets[mediaRow];

                        if (null != cabinetFileRow)
                        {
                            cabinetFileRow.Add(fileRow);
                        }
                        else
                        {
                            this.core.OnMessage(WixErrors.ExpectedMediaCabinet(fileRow.SourceLineNumbers, fileRow.File, fileRow.DiskId));
                        }
                    }
                }
            }
        }

        /// <summary>
        /// Adds a row to the media table with cab name template filled in.
        /// </summary>
        /// <param name="mediaTable"></param>
        /// <param name="cabIndex"></param>
        /// <returns></returns>
        private MediaRow AddMediaRow(Table mediaTable, int cabIndex)
        {
            MediaRow currentMediaRow = (MediaRow)mediaTable.CreateRow(null);
            currentMediaRow.DiskId = cabIndex;
            mediaRows.Add(currentMediaRow);
            currentMediaRow.Cabinet = String.Format(this.cabinetNameTemplate, cabIndex);
            cabinets.Add(currentMediaRow, new FileRowCollection());
            return currentMediaRow;
        }
    }
}
