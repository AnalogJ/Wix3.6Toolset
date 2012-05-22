//-------------------------------------------------------------------------------------------------
// <copyright file="Inscriber.cs" company="Microsoft">
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
// Converts a wixout representation of an MSM database into a ComponentGroup the form of WiX source.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.CodeDom.Compiler;
    using System.Collections;
    using System.Collections.Generic;
    using System.Globalization;
    using System.IO;
    using System.Runtime.InteropServices;
    using System.Security.Cryptography.X509Certificates;
    using System.Text;
    using Microsoft.Tools.WindowsInstallerXml.Msi;

    /// <summary>
    /// Converts a wixout representation of an MSM database into a ComponentGroup the form of WiX source.
    /// </summary>
    public sealed class Inscriber : IMessageHandler
    {
        private TempFileCollection tempFiles;
        private TableDefinitionCollection tableDefinitions;
        private bool encounteredError;

        /// <summary>
        /// Event for messages.
        /// </summary>
        public event MessageEventHandler MessageHandler;

        public Inscriber()
        {
            this.tableDefinitions = Installer.GetTableDefinitions();
        }

        /// <summary>
        /// Gets whether the inscriber has encountered an error while processing.
        /// </summary>
        /// <value>Flag if inscriber encountered an error during processing.</value>
        public bool EncounteredError
        {
            get { return this.encounteredError; }
        }

        /// <summary>
        /// Gets or sets the temp files collection.
        /// </summary>
        /// <value>The temp files collection.</value>
        public TempFileCollection TempFiles
        {
            get { return this.tempFiles; }
            set { this.tempFiles = value; }
        }

        /// <summary>
        /// Gets or sets the path to the temp files location.
        /// </summary>
        /// <value>The path to the temp files location.</value>
        public string TempFilesLocation
        {
            get
            {
                if (null == this.tempFiles)
                {
                    return null;
                }
                else
                {
                    return this.tempFiles.BasePath;
                }
            }
            set
            {
                this.DeleteTempFiles();

                if (null == value)
                {
                    this.tempFiles = new TempFileCollection();
                }
                else
                {
                    this.tempFiles = new TempFileCollection(value);
                }

                // ensure the base path exists
                Directory.CreateDirectory(this.tempFiles.BasePath);
            }
        }

        /// <summary>
        /// Extracts engine from attached container and updates engine with detached container signatures.
        /// </summary>
        /// <param name="bundleFile">Bundle with attached container.</param>
        /// <param name="outputFile">Bundle engine only.</param>
        /// <returns>True if bundle was updated.</returns>
        public bool InscribeBundleEngine(string bundleFile, string outputFile)
        {
            string tempFile = Path.Combine(this.TempFilesLocation, "bundle_engine_unsigned.exe");

            using (BurnReader reader = BurnReader.Open(bundleFile, this))
            using (FileStream writer = File.Open(tempFile, FileMode.Create, FileAccess.Write, FileShare.Read | FileShare.Delete))
            {
                reader.Stream.Seek(0, SeekOrigin.Begin);

                byte[] buffer = new byte[4 * 1024];
                int total = 0;
                int read = 0;
                do
                {
                    read = Math.Min(buffer.Length, (int)reader.EngineSize - total);

                    read = reader.Stream.Read(buffer, 0, read);
                    writer.Write(buffer, 0, read);

                    total += read;
                } while (total < reader.EngineSize && 0 < read);

                if (total != reader.EngineSize)
                {
                    throw new InvalidOperationException("Failed to copy engine out of bundle.");
                }

                // TODO: update writer with detached container signatures.
            }

            Directory.CreateDirectory(Path.GetDirectoryName(outputFile));
            if (File.Exists(outputFile))
            {
                File.Delete(outputFile);
            }
            File.Move(tempFile, outputFile);

            return true;
        }

        /// <summary>
        /// Updates engine with attached container information and adds attached container again.
        /// </summary>
        /// <param name="bundleFile">Bundle with attached container.</param>
        /// <param name="signedEngineFile">Signed bundle engine.</param>
        /// <param name="outputFile">Signed engine with attached container.</param>
        /// <returns>True if bundle was updated.</returns>
        public bool InscribeBundle(string bundleFile, string signedEngineFile, string outputFile)
        {
            bool inscribed = false;
            string tempFile = Path.Combine(this.TempFilesLocation, "bundle_engine_signed.exe");

            using (BurnReader reader = BurnReader.Open(bundleFile, this))
            {
                File.Copy(signedEngineFile, tempFile, true);

                // If there was an attached container on the original (unsigned) bundle, put it back.
                if (reader.AttachedContainerSize > 0)
                {
                    reader.Stream.Seek(reader.AttachedContainerAddress, SeekOrigin.Begin);

                    using (BurnWriter writer = BurnWriter.Open(tempFile, this))
                    {
                        writer.RememberThenResetSignature();
                        writer.AppendContainer(reader.Stream, reader.AttachedContainerSize, BurnCommon.Container.Attached);
                        inscribed = true;
                    }
                }
            }

            Directory.CreateDirectory(Path.GetDirectoryName(outputFile));
            if (File.Exists(outputFile))
            {
                File.Delete(outputFile);
            }
            File.Move(tempFile, outputFile);

            return inscribed;
        }

        /// <summary>
        /// Updates database with signatures from external cabinets.
        /// </summary>
        /// <param name="databaseFile">Path to MSI database.</param>
        /// <param name="outputFile">Ouput for updated MSI database.</param>
        /// <param name="tidy">Clean up files.</param>
        /// <returns>True if database is updated.</returns>
        public bool InscribeDatabase(string databaseFile, string outputFile, bool tidy)
        {
            // Keeps track of whether we've encountered at least one signed cab or not - we'll throw a warning if no signed cabs were encountered
            bool foundUnsignedExternals = false;
            bool shouldCommit = false;

            FileAttributes attributes = File.GetAttributes(databaseFile);
            if (FileAttributes.ReadOnly == (attributes & FileAttributes.ReadOnly))
            {
                this.OnMessage(WixErrors.ReadOnlyOutputFile(databaseFile));
                return shouldCommit;
            }

            using (Database database = new Database(databaseFile, OpenDatabase.Transact))
            {
                // Just use the English codepage, because the tables we're importing only have binary streams / MSI identifiers / other non-localizable content
                int codepage = 1252;

                // reset list of certificates seen for this database
                Dictionary<string, object> certificates = new Dictionary<string, object>();

                // Reset the in-memory tables for this new database
                Table digitalSignatureTable = new Table(null, this.tableDefinitions["MsiDigitalSignature"]);
                Table digitalCertificateTable = new Table(null, this.tableDefinitions["MsiDigitalCertificate"]);

                using (View mediaView = database.OpenExecuteView("SELECT * FROM Media"))
                {
                    while (true)
                    {
                        using (Record mediaRecord = mediaView.Fetch())
                        {
                            if (null == mediaRecord)
                            {
                                break;
                            }

                            X509Certificate2 cert2 = null;
                            Row digitalSignatureRow = null;

                            string cabName = mediaRecord.GetString(4); // get the name of the cab
                            // If there is no cabinet or it's an internal cab, skip it.
                            if (String.IsNullOrEmpty(cabName) || cabName.StartsWith("#", StringComparison.Ordinal))
                            {
                                continue;
                            }

                            string cabId = mediaRecord.GetString(1); // get the ID of the cab
                            string cabPath = Path.Combine(Path.GetDirectoryName(databaseFile), cabName);

                            // If the cabs aren't there, throw an error but continue to catch the other errors
                            if (!File.Exists(cabPath))
                            {
                                this.OnMessage(WixErrors.WixFileNotFound(cabPath));
                                continue;
                            }

                            try
                            {
                                // Get the certificate from the cab
                                X509Certificate signedFileCert = X509Certificate.CreateFromSignedFile(cabPath);
                                cert2 = new X509Certificate2(signedFileCert);
                            }
                            catch (System.Security.Cryptography.CryptographicException e)
                            {
                                uint HResult = unchecked((uint)Marshal.GetHRForException(e));

                                // If the file has no cert, continue, but flag that we found at least one so we can later give a warning
                                if (0x80092009 == HResult) // CRYPT_E_NO_MATCH
                                {
                                    foundUnsignedExternals = true;
                                    continue;
                                }

                                // todo: exactly which HRESULT corresponds to this issue?
                                // If it's one of these exact platforms, warn the user that it may be due to their OS.
                                if ((5 == Environment.OSVersion.Version.Major && 2 == Environment.OSVersion.Version.Minor) || // W2K3
                                        (5 == Environment.OSVersion.Version.Major && 1 == Environment.OSVersion.Version.Minor)) // XP
                                {
                                    this.OnMessage(WixErrors.UnableToGetAuthenticodeCertOfFileDownlevelOS(cabPath, String.Format(CultureInfo.InvariantCulture, "HRESULT: 0x{0:x8}", HResult)));
                                }
                                else // otherwise, generic error
                                {
                                    this.OnMessage(WixErrors.UnableToGetAuthenticodeCertOfFile(cabPath, String.Format(CultureInfo.InvariantCulture, "HRESULT: 0x{0:x8}", HResult)));
                                }
                            }

                            // If we haven't added this cert to the MsiDigitalCertificate table, set it up to be added
                            if (!certificates.ContainsKey(cert2.Thumbprint))
                            {
                                // Add it to our "add to MsiDigitalCertificate" table dictionary
                                Row digitalCertificateRow = digitalCertificateTable.CreateRow(null);
                                digitalCertificateRow[0] = cert2.Thumbprint;

                                // Export to a file, because the MSI API's require us to provide a file path on disk
                                string certPath = Path.Combine(this.TempFilesLocation, "MsiDigitalCertificate");
                                Directory.CreateDirectory(certPath);
                                certPath = Path.Combine(certPath, cert2.Thumbprint + ".cer");
                                File.Delete(certPath);

                                using (BinaryWriter writer = new BinaryWriter(File.Open(certPath, FileMode.Create)))
                                {
                                    writer.Write(cert2.RawData);
                                    writer.Close();
                                }

                                // Now set the file path on disk where this binary stream will be picked up at import time
                                digitalCertificateRow[1] = cert2.Thumbprint + ".cer";

                                certificates.Add(cert2.Thumbprint, certPath);
                            }

                            digitalSignatureRow = digitalSignatureTable.CreateRow(null);

                            digitalSignatureRow[0] = "Media";
                            digitalSignatureRow[1] = cabId;
                            digitalSignatureRow[2] = cert2.Thumbprint;
                        }
                    }
                }

                if (digitalSignatureTable.Rows.Count > 0)
                {
                    database.ImportTable(codepage, (IMessageHandler)this, digitalSignatureTable, this.TempFilesLocation, true);
                    shouldCommit = true;
                }

                if (digitalCertificateTable.Rows.Count > 0)
                {
                    database.ImportTable(codepage, (IMessageHandler)this, digitalCertificateTable, this.TempFilesLocation, true);
                    shouldCommit = true;
                }

                certificates = null;

                // If we did find external cabs but none of them were signed, give a warning
                if (foundUnsignedExternals)
                {
                    this.OnMessage(WixWarnings.ExternalCabsAreNotSigned(databaseFile));
                }

                if (shouldCommit)
                {
                    database.Commit();
                }
            }

            return shouldCommit;
        }

        /// <summary>
        /// Cleans up the temp files used by the Inscriber.
        /// </summary>
        /// <returns>True if all files were deleted, false otherwise.</returns>
        public bool DeleteTempFiles()
        {
            if (null == this.tempFiles)
            {
                return true; // no work to do
            }
            else
            {
                bool deleted = Common.DeleteTempFiles(this.TempFilesLocation, this);

                if (deleted)
                {
                    ((IDisposable)this.tempFiles).Dispose();
                    this.tempFiles = null; // temp files have been deleted, no need to remember this now
                }

                return deleted;
            }
        }

        /// <summary>
        /// Sends a message to the message delegate if there is one.
        /// </summary>
        /// <param name="mea">Message event arguments.</param>
        public void OnMessage(MessageEventArgs e)
        {
            WixErrorEventArgs errorEventArgs = e as WixErrorEventArgs;

            if (null != errorEventArgs)
            {
                this.encounteredError = true;
            }

            if (null != this.MessageHandler)
            {
                this.MessageHandler(this, e);
                if (MessageLevel.Error == e.Level)
                {
                    this.encounteredError = true;
                }
            }
            else if (null != errorEventArgs)
            {
                throw new WixException(errorEventArgs);
            }
        }
    }
}
