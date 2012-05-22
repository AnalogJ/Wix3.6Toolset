//-------------------------------------------------------------------------------------------------
// <copyright file="DocCompiler.cs" company="Microsoft">
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
// Compiles various things into documentation.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.DocCompiler
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.IO;
    using System.Reflection;
    using System.Text;
    using System.Xml;
    using System.Xml.Schema;

    /// <summary>
    /// Compiles various things into documentation.
    /// </summary>
    public class DocCompiler
    {
        internal const string DocCompilerNamespace = "http://schemas.microsoft.com/wix/2005/DocCompiler";
        internal const string XhtmlNamespace = "http://www.w3.org/1999/xhtml";

        private string hhcFile;
        private XmlNamespaceManager namespaceManager;
        private string versionNumber;
        private Version version;
        private string outputDir;
        private string outputFileName;
        private bool showHelp;
        private string tocFile;
        private bool chm = false;
        private bool web = false;

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        /// <param name="args">The command line arguments.</param>
        /// <returns>The error code for the application.</returns>
        [STAThread]
        public static int Main(string[] args)
        {
            DocCompiler docCompiler = new DocCompiler();
            return docCompiler.Run(args);
        }

        /// <summary>
        /// Run the application.
        /// </summary>
        /// <param name="args">The command line arguments.</param>
        /// <returns>The error code for the application.</returns>
        private int Run(string[] args)
        {
            try
            {
                this.ParseCommandline(args);

                // get the assemblies
                Assembly docCompilerAssembly = this.GetType().Assembly;
                FileVersionInfo fv = FileVersionInfo.GetVersionInfo(docCompilerAssembly.Location);
                this.versionNumber = fv.FileVersion;
                this.version = new Version(fv.FileMajorPart, fv.FileMinorPart, fv.FileBuildPart, fv.FilePrivatePart);

                if (this.showHelp)
                {
                    Console.WriteLine("Microsoft (R) Documentation Compiler version {0}", fv.FileVersion);
                    Console.WriteLine("Copyright (C) Microsoft Corporation. All rights reserved.");
                    Console.WriteLine();
                    Console.WriteLine(" usage: DocCompiler [-?] {-c:hhc.exe|-w} tableOfContents.xml outputChmOrDir");
                    Console.WriteLine();
                    Console.WriteLine("    c - path to HTML Help Compiler to create output CHM");
                    Console.WriteLine("    w - creates Web HTML Manual to output directory");

                    return 0;
                }

                // ensure the directory containing the html files exists
                Directory.CreateDirectory(Path.Combine(this.outputDir, "html"));

                // load the schema
                XmlReader schemaReader = null;
                XmlSchemaCollection schemas = null;
                try
                {
                    schemaReader = new XmlTextReader(docCompilerAssembly.GetManifestResourceStream("Microsoft.Tools.DocCompiler.Xsd.docCompiler.xsd"));
                    schemas = new XmlSchemaCollection();
                    schemas.Add(DocCompilerNamespace, schemaReader);
                }
                finally
                {
                    schemaReader.Close();
                }

                // load the table of contents
                XmlTextReader reader = null;
                try
                {
                    reader = new XmlTextReader(this.tocFile);
                    XmlValidatingReader validatingReader = new XmlValidatingReader(reader);
                    validatingReader.Schemas.Add(schemas);

                    // load the xml into a DOM
                    XmlDocument doc = new XmlDocument();
                    doc.Load(validatingReader);

                    // create a namespace manager
                    this.namespaceManager = new XmlNamespaceManager(doc.NameTable);
                    this.namespaceManager.AddNamespace("doc", DocCompilerNamespace);
                    this.namespaceManager.AddNamespace("xhtml", XhtmlNamespace);
                    this.namespaceManager.PushScope();

                    this.ProcessCopyDirectories(doc);
                    this.ProcessCopyFiles(doc);
                    this.ProcessTopics(doc);
                    this.ProcessSchemas(doc);

                    if (this.chm)
                    {
                        this.CompileChm(doc);
                    }
                    if (this.web)
                    {
                        this.BuildWeb(doc);
                    }
                }
                finally
                {
                    if (reader != null)
                    {
                        reader.Close();
                    }
                }
            }
            catch (XmlException xe)
            {
                Console.WriteLine("{0}({1},{2}) : fatal error DCMP0002 : {3}", xe.SourceUri, xe.LineNumber, xe.LinePosition, xe.Message);
                Console.WriteLine();
                Console.WriteLine("Stack Trace:");
                Console.WriteLine(xe.StackTrace);
                return 2;
            }
            catch (Exception e)
            {
                Console.WriteLine("DocCompiler.exe : fatal error DCMP0001: {0}", e.Message);
                Console.WriteLine();
                Console.WriteLine("Stack Trace:");
                Console.WriteLine(e.StackTrace);

                if (e is NullReferenceException)
                {
                    throw;
                }

                return 1;
            }

            return 0;
        }

        /// <summary>
        /// Process directories to copy.
        /// </summary>
        /// <param name="doc">The documentation compiler xml document.</param>
        private void ProcessCopyDirectories(XmlDocument doc)
        {
            XmlNodeList copyDirectoryNodes = doc.SelectNodes("//doc:CopyDirectory", this.namespaceManager);

            foreach (XmlElement copyDirectoryElement in copyDirectoryNodes)
            {
                DirectoryInfo sourceDirectory = new DirectoryInfo(copyDirectoryElement.GetAttribute("Source"));
                string destinationDirectory = copyDirectoryElement.GetAttribute("Destination");

                destinationDirectory = Path.Combine(this.outputDir, destinationDirectory);

                CopyDirectory(sourceDirectory, destinationDirectory);
            }
        }

        /// <summary>
        /// Recursively copy a directory.
        /// </summary>
        /// <param name="sourceDirectory">DirectoryInfo object representing the source directory.</param>
        /// <param name="destinationDirectory">String representing the fully qualified path to the
        /// destination directory.</param>
        private void CopyDirectory(DirectoryInfo sourceDirectory, string destinationDirectory)
        {
            // create the destination directory if it does not yet exist
            if (!Directory.Exists(destinationDirectory))
            {
                Directory.CreateDirectory(destinationDirectory);
            }

            // copy all files in the directory
            foreach (FileInfo fileToCopy in sourceDirectory.GetFiles())
            {
                string destinationFile = Path.Combine(destinationDirectory, fileToCopy.Name);
                fileToCopy.CopyTo(destinationFile, true);

                // remove the read-only attribute
                File.SetAttributes(destinationFile, File.GetAttributes(destinationFile) & ~FileAttributes.ReadOnly);
            }

            // recursively copy all sub-directories in the directory
            foreach (DirectoryInfo directoryToCopy in sourceDirectory.GetDirectories())
            {
                CopyDirectory(directoryToCopy, Path.Combine(destinationDirectory, directoryToCopy.Name));
            }
        }

        /// <summary>
        /// Process files to copy.
        /// </summary>
        /// <param name="doc">The documentation compiler xml document.</param>
        private void ProcessCopyFiles(XmlDocument doc)
        {
            XmlNodeList copyFileNodes = doc.SelectNodes("//doc:CopyFile", this.namespaceManager);

            foreach (XmlElement copyFileElement in copyFileNodes)
            {
                string sourceFile = copyFileElement.GetAttribute("Source");
                string destinationFile = copyFileElement.GetAttribute("Destination");

                destinationFile = Path.Combine(this.outputDir, destinationFile);

                File.Copy(sourceFile, destinationFile, true);

                // remove the read-only attribute
                File.SetAttributes(destinationFile, File.GetAttributes(destinationFile) & ~FileAttributes.ReadOnly);
            }
        }

        /// <summary>
        /// Process the xml schemas.
        /// </summary>
        /// <param name="doc">The documentation compiler xml document.</param>
        private void ProcessSchemas(XmlDocument doc)
        {
            XmlNodeList schemaNodes = doc.SelectNodes("//doc:XmlSchema", this.namespaceManager);

            XmlSchemaCompiler schemaCompiler = new XmlSchemaCompiler(this.outputDir, this.versionNumber);
            schemaCompiler.CompileSchemas(schemaNodes);
        }

        /// <summary>
        /// Process the topics.
        /// </summary>
        /// <param name="doc">The documentation compiler xml document.</param>
        private void ProcessTopics(XmlDocument doc)
        {
            XmlNodeList topicNodes = doc.SelectNodes("//doc:Topic", this.namespaceManager);

            foreach (XmlElement topicElement in topicNodes)
            {
                string sourceFile = topicElement.GetAttribute("SourceFile");

                if (sourceFile.Length > 0)
                {
                    // get the title from the HTML file and save it as an attribute for later processing
                    string title = this.GetTopicTitle(sourceFile);
                    topicElement.SetAttribute("Title", title);

                    string htmlDir = Path.Combine(this.outputDir, "html");
                    string destinationFile = Path.Combine(htmlDir, Path.GetFileName(sourceFile));

                    // save the relative path to the destination file
                    string relDestinationFile = Path.Combine("html", Path.GetFileName(sourceFile));
                    topicElement.SetAttribute("DestinationFile", relDestinationFile);

                    CopyWithExpansions(sourceFile, destinationFile);

                    // copy the relative destination file path to the child Index nodes
                    XmlNodeList indexNodes = topicElement.SelectNodes("doc:Index", this.namespaceManager);
                    foreach (XmlElement indexElement in indexNodes)
                    {
                        indexElement.SetAttribute("DestinationFile", relDestinationFile);
                    }
                }
            }
        }

        /// <summary>
        /// Inspect the given topic XHTML file to determine its title.
        /// </summary>
        /// <param name="htmlFile">The XHTML file to inspect.</param>
        /// <returns>The topic title.</returns>
        private string GetTopicTitle(string xhtmlFile)
        {
            XmlReaderSettings settings = new XmlReaderSettings();
            settings.XmlResolver = null;
            settings.ProhibitDtd = false;
            
            using (XmlReader reader = XmlReader.Create(xhtmlFile, settings))
            {
                if (!reader.ReadToFollowing("html", XhtmlNamespace) || !reader.ReadToFollowing("head", XhtmlNamespace) || !reader.ReadToFollowing("title", XhtmlNamespace))
                {
                    return String.Format("***Couldn't read title from topic {0}", xhtmlFile);
                }
                else
                {
                    return reader.ReadString().Trim();
                }
            }
        }

        /// <summary>
        /// Copies a file, performing in-place expansions (for version numbers, etc.)
        /// </summary>
        /// <param name="sourceFile">Source file to copy.</param>
        /// <param name="destinationFile">Destination file to write.</param>
        private void CopyWithExpansions(string sourceFile, string destinationFile)
        {
            // Copying the entire file to memory is relateively safe because it's a
            // hand-authored file which isn't too large.
            string contents = File.ReadAllText(sourceFile);

            if (contents.Contains("[["))
            {
                contents = contents.Replace("[[Version]]", this.version.ToString());
                contents = contents.Replace("[[Version.Major]]", this.version.Major.ToString());
                contents = contents.Replace("[[Version.Minor]]", this.version.Minor.ToString());
                contents = contents.Replace("[[Version.Build]]", this.version.Build.ToString());
                contents = contents.Replace("[[Version.Revision]]", this.version.Revision.ToString());
            }

            File.WriteAllText(destinationFile, contents, Encoding.ASCII);
        }

        /// <summary>
        /// Compile the documentation into a chm file.
        /// </summary>
        /// <param name="doc">The documentation compiler xml document.</param>
        private void CompileChm(XmlDocument doc)
        {
            XmlElement defaultTopicNode = (XmlElement)doc.SelectSingleNode("//doc:Topic", this.namespaceManager);
            string defaultTopicFile = defaultTopicNode.GetAttribute("DestinationFile");
            string defaultTopicTitle = defaultTopicNode.GetAttribute("Title");

            // create the project file
            string projectFile = Path.Combine(this.outputDir, "project.hhp");
            using (StreamWriter sw = File.CreateText(projectFile))
            {
                sw.WriteLine("[OPTIONS]");
                sw.WriteLine("Compatibility=1.1 or later");
                sw.WriteLine(String.Format("Compiled file={0}", this.outputFileName));
                sw.WriteLine("Contents file=toc.hhc");
                sw.WriteLine("Index file=idx.hhk");
                sw.WriteLine("Default Window=Main");
                sw.WriteLine(String.Format("Default topic={0}", defaultTopicFile));
                sw.WriteLine("Display compile progress=No");
                sw.WriteLine("Error log file=log.txt");
                sw.WriteLine("Full-text search=Yes");
                sw.WriteLine("Language=0x409 English (United States)");
                sw.WriteLine(String.Format("Title={0}", defaultTopicTitle));
                sw.WriteLine("");
                sw.WriteLine("[WINDOWS]");
                sw.WriteLine("Main=,\"toc.hhc\",\"idx.hhk\",\"{0}\",\"{0}\",,,,,0x63520,,0x384e,,,,,,,,0", defaultTopicFile);
            }

            // create the index file
            string indexFile = Path.Combine(this.outputDir, "idx.hhk");
            using (StreamWriter sw = File.CreateText(indexFile))
            {
                sw.WriteLine("<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML//EN\">");
                sw.WriteLine("<HTML>");
                sw.WriteLine("<HEAD>");
                sw.WriteLine("<META NAME=\"GENERATOR\" CONTENT=\"A tool\">");
                sw.WriteLine("</HEAD>");
                sw.WriteLine("<BODY>");
                sw.WriteLine("<OBJECT TYPE=\"text/site properties\">");
                sw.WriteLine("	<PARAM NAME=\"FrameName\" VALUE=\"TEXT\">");
                sw.WriteLine("</OBJECT>");
                sw.WriteLine("<UL>");

                XmlNodeList topicNodes = doc.SelectNodes("//doc:Topic|//doc:Index", this.namespaceManager);
                foreach (XmlElement topicElement in topicNodes)
                {
                    string title = topicElement.GetAttribute("Title");
                    string destinationFile = topicElement.GetAttribute("DestinationFile");

                    if (destinationFile.Length > 0)
                    {
                        sw.WriteLine("\t<LI> <OBJECT type=\"text/sitemap\">");
                        sw.WriteLine(String.Format("\t\t<param name=\"Keyword\" value=\"{0}\">", title));
                        sw.WriteLine(String.Format("\t\t<param name=\"Name\" value=\"{0}\">", title));
                        sw.WriteLine(String.Format("\t\t<param name=\"Local\" value=\"{0}\">", destinationFile));
                        sw.WriteLine("\t\t</OBJECT>");
                    }
                }

                sw.WriteLine("</UL>");
                sw.WriteLine("</BODY>");
                sw.WriteLine("</HTML>");
            }

            // create the table of contents file
            string tocFile = Path.Combine(this.outputDir, "toc.hhc");
            using (StreamWriter sw = File.CreateText(tocFile))
            {
                sw.WriteLine("<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML//EN\">");
                sw.WriteLine("<HTML>");
                sw.WriteLine("<HEAD>");
                sw.WriteLine("<meta name=\"GENERATOR\" content=\"Microsoft&reg; HTML Help Workshop 4.1\">");
                sw.WriteLine("<!-- Sitemap 1.0 -->");
                sw.WriteLine("</HEAD><BODY>");
                sw.WriteLine("<OBJECT type=\"text/site properties\">");
                sw.WriteLine("\t<param name=\"ImageType\" value=\"Folder\">");
                sw.WriteLine("</OBJECT>");
                sw.WriteLine("<UL>");

                XmlNodeList topicNodes = doc.SelectNodes("//doc:TableOfContents/doc:Topic", this.namespaceManager);
                foreach (XmlNode topicNode in topicNodes)
                {
                    this.WriteTopic((XmlElement)topicNode, sw);
                }

                sw.WriteLine("</UL>");
                sw.WriteLine("</BODY></HTML>");
            }

            // call the help compiler
            Process hhcProcess = new Process();
            hhcProcess.StartInfo.FileName = this.hhcFile;
            hhcProcess.StartInfo.Arguments = String.Concat("\"", projectFile, "\"");
            hhcProcess.StartInfo.CreateNoWindow = true;
            hhcProcess.StartInfo.UseShellExecute = false;
            hhcProcess.StartInfo.RedirectStandardOutput = true;
            hhcProcess.Start();

            // wait for the process to terminate
            hhcProcess.WaitForExit();

            // check for errors
            if (hhcProcess.ExitCode != 1)
            {
                throw new InvalidOperationException("The help compiler failed.");
            }
        }

        /// <summary>
        /// Write a single topic and its children to the HTMLHelp table of contents file.
        /// </summary>
        /// <param name="topicElement">The topic element to write.</param>
        /// <param name="sw">Writer for the table of contents.</param>
        private void WriteTopic(XmlElement topicElement, StreamWriter sw)
        {
            string destinationFile = topicElement.GetAttribute("DestinationFile");
            string title = topicElement.GetAttribute("Title");

            sw.WriteLine("\t<LI> <OBJECT type=\"text/sitemap\">");
            sw.WriteLine(String.Format("\t\t<param name=\"Name\" value=\"{0}\">", title));
            if (destinationFile.Length > 0)
            {
                sw.WriteLine(String.Format("\t\t<param name=\"Local\" value=\"{0}\">", destinationFile));
            }
            sw.WriteLine("\t\t</OBJECT>");

            XmlNodeList topicNodes = topicElement.SelectNodes("doc:Topic", this.namespaceManager);
            if (topicNodes.Count > 0)
            {
                sw.WriteLine("<UL>");
            }

            foreach (XmlNode topicNode in topicNodes)
            {
                this.WriteTopic((XmlElement)topicNode, sw);
            }

            if (topicNodes.Count > 0)
            {
                sw.WriteLine("</UL>");
            }
        }

        /// <summary>
        /// Build web manual pages.
        /// </summary>
        /// <param name="doc">The documentation compiler xml document.</param>
        private void BuildWeb(XmlDocument doc)
        {
            XmlNodeList topicNodes = doc.SelectNodes("//doc:TableOfContents/doc:Topic", this.namespaceManager);

            List<XmlNode> parentTopics = new List<XmlNode>();

            foreach (XmlNode topicNode in topicNodes)
            {
                this.WriteWebTopic((XmlElement)topicNode, parentTopics);
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="topicElement">The topic element to write.</param>
        /// <param name="parentTopics"></param>
        private void WriteWebTopic(XmlElement topicElement, List<XmlNode> parentTopics)
        {
            string file = Path.Combine(this.outputDir, topicElement.GetAttribute("DestinationFile"));
            string title = topicElement.GetAttribute("Title");

            if (String.IsNullOrEmpty(file))
                return;

            string outputFile = Path.Combine(this.outputDir, Path.ChangeExtension(file, ".xml"));
            XmlTextWriter w = new XmlTextWriter(outputFile, Encoding.UTF8);

            w.WriteStartElement("ManualPage");
            w.WriteAttributeString("File", Path.GetFullPath(file));
            w.WriteAttributeString("Title", title);

            IEnumerator<XmlNode> parentTopicEnumerator = parentTopics.GetEnumerator();
            if (parentTopicEnumerator.MoveNext())
            {
                w.WriteStartElement("ParentTopics");
                this.WriteWebParentTopic(parentTopicEnumerator, w);
                w.WriteEndElement();
            }

            w.WriteEndElement();
            w.Close();

            XmlNodeList topicNodes = topicElement.SelectNodes("doc:Topic", this.namespaceManager);
            if (topicNodes.Count > 0)
            {
                parentTopics.Add(topicElement);

                foreach (XmlNode topicNode in topicNodes)
                {
                    this.WriteWebTopic((XmlElement)topicNode, parentTopics);
                }

                parentTopics.RemoveAt(parentTopics.Count - 1);
            }
        }

        /// <summary>
        /// Writes "breadcrumb" trail of links to parent topic elements.
        /// </summary>
        /// <param name="topicEnumerator"></param>
        /// <param name="w"></param>
        private void WriteWebParentTopic(IEnumerator<XmlNode> topicEnumerator, XmlWriter w)
        {
            XmlElement topicElement = (XmlElement)topicEnumerator.Current;

            string destinationFile = Path.GetFileName(topicElement.GetAttribute("DestinationFile"));
            string title = topicElement.GetAttribute("Title");

            w.WriteStartElement("Topic");
            w.WriteAttributeString("File", destinationFile);
            w.WriteAttributeString("Title", title);

            if (topicEnumerator.MoveNext())
            {
                WriteWebParentTopic(topicEnumerator, w);
            }

            w.WriteEndElement();
        }

        /// Parse the command line arguments.
        /// </summary>
        /// <param name="args">Command line arguments.</param>
        private void ParseCommandline(string[] args)
        {
            foreach (string arg in args)
            {
                if (arg.StartsWith("-") || arg.StartsWith("/"))
                {
                    if (arg.Length > 1)
                    {
                        switch (arg[1])
                        {
                            case '?':
                                this.showHelp = true;
                                break;
                            case 'c':
                                this.hhcFile = arg.Substring(3);
                                this.chm = true;
                                break;
                            case 'w':
                                this.web = true;
                                break;
                            default:
                                throw new ArgumentException(String.Format("Unrecognized commandline parameter '{0}'.", arg));
                        }
                    }
                    else
                    {
                        throw new ArgumentException(String.Format("Unrecognized commandline parameter '{0}'.", arg));
                    }
                }
                else if (this.tocFile == null)
                {
                    this.tocFile = arg;
                }
                else if (this.outputFileName == null)
                {
                    if (this.chm)
                    {
                        this.outputFileName = Path.GetFileName(arg);
                        this.outputDir = Path.GetDirectoryName(arg);
                    }
                    if (this.web)
                    {
                        this.outputDir = arg;
                    }
                }
                else
                {
                    throw new ArgumentException(String.Format("Unrecognized argument '{0}'.", arg));
                }
            }

            // check for missing mandatory arguments
            if (!this.showHelp && (this.outputDir == null || (!this.chm && !this.web) || (this.chm && this.hhcFile == null)))
            {
                throw new ArgumentException("Missing mandatory argument.");
            }
        }
    }
}
