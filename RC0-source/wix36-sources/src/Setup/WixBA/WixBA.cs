//-------------------------------------------------------------------------------------------------
// <copyright file="WixBA.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
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
// The WiX toolset user experience.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.UX
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.IO;
    using System.Net;
    using System.Text;
    using System.Windows.Input;
    using Microsoft.Tools.WindowsInstallerXml.Bootstrapper;
    using Threading = System.Windows.Threading;

    /// <summary>
    /// The WiX toolset user experience.
    /// </summary>
    public class WixBA : BootstrapperApplication
    {
        /// <summary>
        /// Gets the global model.
        /// </summary>
        static public Model Model { get; private set; }

        /// <summary>
        /// Gets the global view.
        /// </summary>
        static public RootView View { get; private set; }
        // TODO: We should refactor things so we dont have a global View.

        /// <summary>
        /// Gets the global dispatcher.
        /// </summary>
        static public Threading.Dispatcher Dispatcher { get; private set; }

        /// <summary>
        /// Launches the default web browser to the provided URI.
        /// </summary>
        /// <param name="uri">URI to open the web browser.</param>
        public static void LaunchUrl(string uri)
        {
            // Switch the wait cursor since shellexec can take a second or so.
            Cursor cursor = WixBA.View.Cursor;
            WixBA.View.Cursor = Cursors.Wait;

            try
            {
                Process process = new Process();
                process.StartInfo.FileName = uri;
                process.StartInfo.UseShellExecute = true;
                process.StartInfo.Verb = "open";

                process.Start();
            }
            finally
            {
                WixBA.View.Cursor = cursor; // back to the original cursor.
            }
        }

        /// <summary>
        /// Thread entry point for WiX Toolset UX.
        /// </summary>
        protected override void Run()
        {
            this.Engine.Log(LogLevel.Verbose, "Running the WiX BA.");
            WixBA.Model = new Model(this);
            WixBA.Dispatcher = Threading.Dispatcher.CurrentDispatcher;
            RootViewModel viewModel = new RootViewModel();

            // Populate the view models with the latest data. This is where Detect is called.
            viewModel.Refresh();

            // Create a Window to show UI.
            if (WixBA.Model.Command.Display == Display.Passive ||
                WixBA.Model.Command.Display == Display.Full)
            {
                this.Engine.Log(LogLevel.Verbose, "Creating a UI.");
                WixBA.View = new RootView(viewModel);
                WixBA.View.Show();
            }

            Threading.Dispatcher.Run();

            this.PostTelemetry();
            this.Engine.Quit(WixBA.Model.Result);
        }

        private void PostTelemetry()
        {
            string result = String.Concat("0x", WixBA.Model.Result.ToString("x"));

            StringBuilder telemetryData = new StringBuilder();
            foreach (KeyValuePair<string, string> kvp in WixBA.Model.Telemetry)
            {
                telemetryData.AppendFormat("{0}={1}+", kvp.Key, kvp.Value);
            }
            telemetryData.AppendFormat("Result={0}", result);

            byte[] data = Encoding.UTF8.GetBytes(telemetryData.ToString());

            try
            {
                HttpWebRequest post = WixBA.Model.CreateWebRequest(String.Format("http://wixtoolset.org/telemetry/v{0}/?r={1}", WixBA.Model.Version.ToString(), result));
                post.Method = "POST";
                post.ContentType = "application/x-www-form-urlencoded";
                post.ContentLength = data.Length;

                using (Stream postStream = post.GetRequestStream())
                {
                    postStream.Write(data, 0, data.Length);
                }

                HttpWebResponse response = (HttpWebResponse)post.GetResponse();
            }
            catch (ArgumentException)
            {
            }
            catch (FormatException)
            {
            }
            catch (OverflowException)
            {
            }
            catch (WebException)
            {
            }
        }
    }
}
