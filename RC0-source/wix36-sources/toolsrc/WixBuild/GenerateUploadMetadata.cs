//-------------------------------------------------------------------------------------------------
// <copyright file="GenerateUploadMetadata.cs" company="Microsoft">
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
// Contains the GenerateUploadMetadata class.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.WixBuild
{
    using System;
    using System.IO;
    using System.Text;
    using Microsoft.Build.Framework;
    using Microsoft.Build.Utilities;

    /// <summary>
    /// MSBuild task for creating the upload metadata for a release.
    /// </summary>
    public class GenerateUploadMetadata : Task
    {
        /// <summary>
        /// Gets and sets the version for the upload metadata.
        /// </summary>
        [Required]
        public string Version
        {
            get;
            set;
        }

        /// <summary>
        /// Gets and sets the set of files that will be uploaded.
        /// </summary>
        [Required]
        public ITaskItem[] UploadFiles
        {
            get;
            set;
        }

        /// <summary>
        /// Gets and sets the output file name.
        /// </summary>
        [Output]
        public ITaskItem OutputFile
        {
            get;
            set;
        }

        /// <summary>
        /// Executes the task by converting upload items into metadata.
        /// </summary>
        /// <returns><see langword="true"/> if the task successfully executed; otherwise, <see langword="false"/>.</returns>
        public override bool Execute()
        {
            ////{
            ////    "id" : "v3.6.2517.0",
            ////    "downloadable" : true,
            ////    "date" : "2012/1/17",
            ////    "roots" : [ "~" ],
            ////    "files" :
            ////    [
            ////      { "name" : "wix36.exe", "contentType" : "application/octet-stream", "show" : true },
            ////      { "name" : "data/wix36.msi", "contentType" : "application/octet-stream"  },
            ////      { "name" : "data/wix36.cab", "contentType" : "application/octet-stream"  },
            ////      { "name" : "wix36-binaries.zip", "contentType" : "application/octet-stream", "show" : true, "protected" : true }
            ////    ]
            ////}
            StringBuilder json = new StringBuilder();
            json.AppendLine("{");
            json.AppendFormat(" \"id\":  \"v{0}\",\r\n", this.Version);
            json.AppendLine(" \"downloadable\":  true,");
            json.AppendFormat(" \"date\":  \"{0}\",\r\n", DateTime.Now.ToString("yyyy-MM-dd"));
            json.AppendLine(" \"roots\": [ \"~\"],");
            json.AppendLine(" \"files\":");
            json.AppendLine(" [");
            json.AppendLine(this.UploadFilesToString());
            json.AppendLine(" ]");
            json.AppendLine("}");

            using (StreamWriter stream = File.CreateText(this.OutputFile.ItemSpec))
            {
                stream.Write(json.ToString());
            }

            return true;
        }

        private string UploadFilesToString()
        {
            string[] files = new string[this.UploadFiles.Length];

            for (int i = 0; i < this.UploadFiles.Length; ++i)
            {
                ITaskItem item = this.UploadFiles[i];
                string filename = Path.GetFileName(item.ItemSpec);

                StringBuilder sb = new StringBuilder();
                sb.Append("  {");
                sb.AppendFormat(" \"name\" : \"{0}\"", Path.Combine(item.GetMetadata("relativefolder"), filename)).Replace("\\", "/");
                sb.AppendFormat(", \"contentType\" : \"{0}\"", String.IsNullOrEmpty(item.GetMetadata("contenttype")) ? this.GuessContentType(Path.GetExtension(item.ItemSpec)) : item.GetMetadata("contenttype"));

                if (!String.IsNullOrEmpty(item.GetMetadata("title")))
                {
                    sb.AppendFormat(", \"title\" : \"{0}\"", item.GetMetadata("title").Replace("\\", "\\\\"));
                }

                bool promoted;
                if (Boolean.TryParse(item.GetMetadata("promoted"), out promoted) && promoted)
                {
                    sb.Append(", \"promoted\" : true");
                }

                bool show;
                if (Boolean.TryParse(item.GetMetadata("show"), out show) && show)
                {
                    sb.Append(", \"show\" : true");
                }

                bool protectedItem;
                if (Boolean.TryParse(item.GetMetadata("protected"), out protectedItem) && protectedItem)
                {
                    sb.Append(", \"protected\" : true");
                }

                files[i] = sb.Append(" }").ToString();
            }

            return String.Join(",\r\n", files);
        }

        private string GuessContentType(string extension)
        {
            switch (extension.ToLowerInvariant())
            {
                case ".cab":
                    return "application/vnd.ms-cab-compressed";

                case ".zip":
                    return "application/zip";

                default:
                    return "application/octet-stream";
            }
        }
    }
}
