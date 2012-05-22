//-------------------------------------------------------------------------------------------------
// <copyright file="FileSystemScraper.cs" company="Microsoft">
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
// Filesystem scraping class. Static methods only.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.ApplicationModel
{
    using System;
    using System.Collections;
    using System.Globalization;
    using IO = System.IO;
    using Microsoft.Tools.WindowsInstallerXml.Serialize;

    /// <summary>
    /// Filesystem scraping class. Static methods only.
    /// </summary>
    internal class FileSystemScraper
    {
        private static int directoryId = 0;
        private static int fileId = 0;

        /// <summary>
        /// Scrapes the file system from a given directory path.
        /// </summary>
        /// <param name="path">Path from which to start scraping.</param>
        /// <returns>Directory representing the subtree starting at path.</returns>
        public static Directory ScrapeFileSystem(string path)
        {
            Directory rootDirectory = new Directory();
            directoryId++;
            rootDirectory.Id = String.Format(CultureInfo.InvariantCulture, "Dir{0}", directoryId);
            rootDirectory.LongName = IO.Path.GetFileName(path);
            rootDirectory.Name = rootDirectory.Id;
            rootDirectory.FileSource = path;
            ScrapeDirectory(rootDirectory);

            return rootDirectory;
        }

        /// <summary>
        /// Scrapes a directory subtree.
        /// </summary>
        /// <param name="target">Directory currently being scraped.</param>
        private static void ScrapeDirectory(Directory target)
        {
            ArrayList subItems = new ArrayList();
            foreach (string directory in IO.Directory.GetDirectories(target.FileSource))
            {
                Directory scrapedDirectory = new Directory();
                directoryId++;
                scrapedDirectory.Id = String.Format(CultureInfo.InvariantCulture, "Dir{0}", directoryId);
                scrapedDirectory.LongName = IO.Path.GetFileName(directory);
                scrapedDirectory.Name = scrapedDirectory.Id;
                scrapedDirectory.FileSource = directory;
                target.AddChild(scrapedDirectory);
                ScrapeDirectory(scrapedDirectory);
            }

            foreach (string file in IO.Directory.GetFiles(target.FileSource))
            {
                File scrapedFile = new File();
                scrapedFile.LongName = IO.Path.GetFileName(file);
                scrapedFile.Source = file;
                fileId++;
                string fileExtension = IO.Path.GetExtension(file);
                if (fileExtension.Length > 4)
                {
                    scrapedFile.Id = String.Format(CultureInfo.InvariantCulture, "Fil{0}{1}", fileId, fileExtension.Substring(0, 4));
                }
                else
                {
                    scrapedFile.Id = String.Format(CultureInfo.InvariantCulture, "Fil{0}{1}", fileId, fileExtension);
                }
                scrapedFile.Name = scrapedFile.Id;

                Component fileComponent = new Component();
                fileComponent.Id = String.Format(CultureInfo.InvariantCulture, "Comp{0}", scrapedFile.Name);
                fileComponent.DiskId = 1;
                fileComponent.Guid = Guid.NewGuid().ToString();
                fileComponent.AddChild(scrapedFile);
                target.AddChild(fileComponent);
            }
        }
    }
}
