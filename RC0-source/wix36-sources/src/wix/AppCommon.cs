//-------------------------------------------------------------------------------------------------
// <copyright file="AppCommon.cs" company="Microsoft">
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
// Common utilities for Wix applications.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.Collections;
    using System.Collections.Specialized;
    using System.Configuration;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Text;
    using System.Threading;
    using System.Reflection;

    /// <summary>
    /// Common utilities for Wix applications.
    /// </summary>
    public static class AppCommon
    {
        /// <summary>
        /// Get a set of files that possibly have a search pattern in the path (such as '*').
        /// </summary>
        /// <param name="searchPath">Search path to find files in.</param>
        /// <param name="fileType">Type of file; typically "Source".</param>
        /// <returns>An array of files matching the search path.</returns>
        /// <remarks>
        /// This method is written in this verbose way because it needs to support ".." in the path.
        /// It needs the directory path isolated from the file name in order to use Directory.GetFiles
        /// or DirectoryInfo.GetFiles.  The only way to get this directory path is manually since
        /// Path.GetDirectoryName does not support ".." in the path.
        /// </remarks>
        /// <exception cref="WixFileNotFoundException">Throws WixFileNotFoundException if no file matching the pattern can be found.</exception>
        public static string[] GetFiles(string searchPath, string fileType)
        {
            if (null == searchPath)
            {
                throw new ArgumentNullException("searchPath");
            }

            // convert alternate directory separators to the standard one
            string filePath = searchPath.Replace(Path.AltDirectorySeparatorChar, Path.DirectorySeparatorChar);
            int lastSeparator = filePath.LastIndexOf(Path.DirectorySeparatorChar);
            string[] files = null;

            try
            {
                if (0 > lastSeparator)
                {
                    files = Directory.GetFiles(".", filePath);
                }
                else // found directory separator
                {
                    files = Directory.GetFiles(filePath.Substring(0, lastSeparator + 1), filePath.Substring(lastSeparator + 1));
                }
            }
            catch (DirectoryNotFoundException)
            {
                // don't let this function throw the DirectoryNotFoundException. (this exception
                // occurs for non-existant directories and invalid characters in the searchPattern)
            }
            catch (ArgumentException)
            {
                // don't let this function throw the ArgumentException. (this exception
                // occurs in certain situations such as when passing a malformed UNC path)
            }
            catch (IOException)
            {
                throw new WixFileNotFoundException(searchPath, fileType);
            }

            // file could not be found or path is invalid in some way
            if (null == files || 0 == files.Length)
            {
                throw new WixFileNotFoundException(searchPath, fileType);
            }

            return files;
        }

        /// <summary>
        /// Read the configuration file (*.exe.config).
        /// </summary>
        /// <param name="extensions">Extensions to load.</param>
        public static void ReadConfiguration(StringCollection extensions)
        {
            if (null == extensions)
            {
                throw new ArgumentNullException("extensions");
            }

            // Don't use the default AppSettings reader because
            // the tool may be called from within another process.
            // Instead, read the .exe.config file from the tool location.
            string toolPath = Assembly.GetCallingAssembly().Location;
            Configuration config = ConfigurationManager.OpenExeConfiguration(toolPath);
            if (config.HasFile)
            {
                KeyValueConfigurationElement configVal = config.AppSettings.Settings["extensions"];
                if (configVal != null)
                {
                    string extensionTypes = configVal.Value;
                    foreach (string extensionType in extensionTypes.Split(";".ToCharArray()))
                    {
                        extensions.Add(extensionType);
                    }
                }
            }
        }

        /// <summary>
        /// Prepares the console for localization.
        /// </summary>
        public static void PrepareConsoleForLocalization()
        {
            Thread.CurrentThread.CurrentUICulture = CultureInfo.CurrentUICulture.GetConsoleFallbackUICulture();
            if ((Console.OutputEncoding.CodePage != Encoding.UTF8.CodePage) &&
                (Console.OutputEncoding.CodePage != Thread.CurrentThread.CurrentUICulture.TextInfo.OEMCodePage) &&
                (Console.OutputEncoding.CodePage != Thread.CurrentThread.CurrentUICulture.TextInfo.ANSICodePage))
            {
                Thread.CurrentThread.CurrentUICulture = new CultureInfo("en-US");
            }
        }

        /// <summary>
        /// Creates and returns the string for CreatingApplication field (MSI Summary Information Stream).
        /// </summary>
        /// <remarks>It reads the AssemblyProductAttribute and AssemblyVersionAttribute of executing assembly
        /// and builds the CreatingApplication string of the form "[ProductName] ([ProductVersion])".</remarks>
        /// <returns>Returns value for PID_APPNAME."</returns>
        public static string GetCreatingApplicationString()
        {
            string applicationCreatorString = "Windows Installer XML";
            Assembly executingAssembly = Assembly.GetExecutingAssembly();
            FileVersionInfo fileVersion = FileVersionInfo.GetVersionInfo(executingAssembly.Location);

            object[] customAttributes = executingAssembly.GetCustomAttributes(typeof(AssemblyProductAttribute), false);
            if (null != customAttributes && 0 < customAttributes.Length)
            {
                AssemblyProductAttribute assemblyProduct = customAttributes[0] as AssemblyProductAttribute;
                if (null != assemblyProduct)
                {
                    applicationCreatorString = assemblyProduct.Product;
                }
            }

            return String.Format(CultureInfo.CurrentUICulture, "{0} ({1})", applicationCreatorString, fileVersion.FileVersion);
        }

        /// <summary>
        /// Displays help message header on Console for caller tool.
        /// </summary>
        public static void DisplayToolHeader()
        {
            string toolDescription = "Toolset";
            Assembly executingAssembly = Assembly.GetCallingAssembly();
            FileVersionInfo fileVersion = FileVersionInfo.GetVersionInfo(executingAssembly.Location);

            object[] customAttributes = executingAssembly.GetCustomAttributes(typeof(AssemblyDescriptionAttribute), false);
            if (null != customAttributes && 0 < customAttributes.Length)
            {
                AssemblyDescriptionAttribute assemblyDescription = customAttributes[0] as AssemblyDescriptionAttribute;
                if (null != assemblyDescription)
                {
                    toolDescription = assemblyDescription.Description;
                }
            }

            Console.WriteLine(String.Format(CultureInfo.CurrentUICulture, WixDistributionSpecificStrings.ToolsetHelpHeader, toolDescription, fileVersion.FileVersion));
        }

        /// <summary>
        /// Displays help message header on Console for caller tool.
        /// </summary>
        public static void DisplayToolFooter()
        {
            Console.Write(WixDistributionSpecificStrings.ToolsetHelpFooter);
        }
    }
}
