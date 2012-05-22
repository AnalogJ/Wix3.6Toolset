//-------------------------------------------------------------------------------------------------
// <copyright file="UpdateViewModel.cs" company="Microsoft">
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
// The model of the update view.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.UX
{
    using System;
    using System.ComponentModel;
    using System.Linq;
    using System.Net;
    using System.ServiceModel.Syndication;
    using System.Windows.Input;
    using System.Xml;

    /// <summary>
    /// The states of the update view model.
    /// </summary>
    public enum UpdateState
    {
        Unknown,
        Initializing,
        Checking,
        Current,
        Available,
        Failed,
    }

    /// <summary>
    /// The model of the update view.
    /// </summary>
    public class UpdateViewModel : PropertyNotifyBase
    {
        private RootViewModel root;

        private readonly string AppSyndicationNamespace = "http://appsyndication.org/2006/appsyn";

        private UpdateState state;
        private string updateUrl;
        private BackgroundWorker worker;

        private ICommand checkCommand;
        private ICommand launchCommand;

        public UpdateViewModel(RootViewModel root)
        {
            this.root = root;
            this.root.PropertyChanged += new System.ComponentModel.PropertyChangedEventHandler(this.RootPropertyChanged);

            this.State = UpdateState.Unknown;

            this.worker = new BackgroundWorker();
            this.worker.DoWork += new DoWorkEventHandler(worker_DoWork);
        }

        public ICommand CheckCommand
        {
            get
            {
                if (this.checkCommand == null)
                {
                    this.checkCommand = new RelayCommand(param => this.Refresh(), param => this.State == UpdateState.Unknown);
                }

                return this.checkCommand;
            }
        }

        public bool CheckEnabled
        {
            get { return this.CheckCommand.CanExecute(this); }
        }

        public bool CheckVisible
        {
            get { return this.CheckCommand.CanExecute(this) || this.State == UpdateState.Initializing; }
        }

        public bool IsUpToDate
        {
            get { return this.State == UpdateState.Current; }
        }

        public ICommand LaunchCommand
        {
            get
            {
                if (this.launchCommand == null)
                {
                    this.launchCommand = new RelayCommand(param => WixBA.LaunchUrl(this.UpdateUrl), param => this.State == UpdateState.Available && !String.IsNullOrEmpty(this.UpdateUrl));
                }

                return this.launchCommand;
            }
        }

        public bool LaunchEnabled
        {
            get { return this.LaunchCommand.CanExecute(this); }
        }

        public bool CheckingEnabled
        {
            get { return this.State == UpdateState.Checking; }
        }

        /// <summary>
        /// Gets and sets the state of the update view model.
        /// </summary>
        public UpdateState State
        {
            get
            {
                return this.state;
            }

            set
            {
                if (this.state != value)
                {
                    this.state = value;
                    base.OnPropertyChanged("State");
                    base.OnPropertyChanged("Title");
                    base.OnPropertyChanged("CheckVisible");
                    base.OnPropertyChanged("CheckEnabled");
                    base.OnPropertyChanged("CheckingEnabled");
                    base.OnPropertyChanged("IsUpToDate");
                    base.OnPropertyChanged("LaunchEnabled");
                }
            }
        }

        /// <summary>
        /// Gets and sets the title of the update view model.
        /// </summary>
        public string Title
        {
            get
            {
                switch (this.state)
                {
                    case UpdateState.Initializing:
                        return "Initializing update detection...";

                    case UpdateState.Checking:
                        return "Checking for updates...";

                    case UpdateState.Current:
                        return "Up to date";

                    case UpdateState.Available:
                        return "Newer version available";

                    case UpdateState.Failed:
                        return "Failed to check for updates";

                    case UpdateState.Unknown:
                        return "Check for updates.";

                    default:
                        return "Unexpected state";
                }
            }
        }

        /// <summary>
        /// Gets and sets the update URL.
        /// </summary>
        public string UpdateUrl
        {
            get
            {
                return this.updateUrl;
            }

            set
            {
                if (this.updateUrl != value)
                {
                    this.updateUrl = value;
                    base.OnPropertyChanged("UpdateUrl");
                    base.OnPropertyChanged("LaunchEnabled");
                }
            }
        }

        void RootPropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            if ("State" == e.PropertyName)
            {
            }
        }

        /// <summary>
        /// Causes the update view to check updates again.
        /// </summary>
        public void Refresh()
        {
            // If we're already checking for updates, skip the refresh.
            if (this.State == UpdateState.Checking)
            {
                return;
            }

            this.State = UpdateState.Checking;
            this.UpdateUrl = null;

            this.worker.RunWorkerAsync();
        }

        /// <summary>
        /// Worker thread to check for updates.
        /// </summary>
        /// <param name="sender">Sender.</param>
        /// <param name="e">Arguments.</param>
        private void worker_DoWork(object sender, DoWorkEventArgs e)
        {
            bool succeeded = false;
            try
            {
                HttpWebRequest request = WixBA.Model.CreateWebRequest("http://wix.sourceforge.net/releases/wix3.6.feed");
                HttpWebResponse response = (HttpWebResponse)request.GetResponse();

                if (response.StatusCode == HttpStatusCode.OK)
                {
                    SyndicationFeed feed;
                    using (XmlReader reader = XmlReader.Create(response.GetResponseStream()))
                    {
                        feed = SyndicationFeed.Load(reader);
                    }

                    var updates = from entry in feed.Items
                                  from link in entry.Links
                                  from extension in entry.ElementExtensions
                                  where String.Equals(link.RelationshipType, "enclosure", StringComparison.Ordinal) &&
                                        String.Equals(extension.OuterNamespace, this.AppSyndicationNamespace, StringComparison.Ordinal) &&
                                        String.Equals(extension.OuterName, "version", StringComparison.Ordinal)
                                  select new Update()
                                  {
                                      Url = link.Uri.AbsoluteUri,
                                      Version = new Version(extension.GetObject<string>())
                                  };

                    Update update = updates.Where(u => u.Version > WixBA.Model.Version).OrderByDescending(u => u.Version).FirstOrDefault();
                    if (update == null)
                    {
                        this.UpdateUrl = null;
                        this.State = UpdateState.Current;
                    }
                    else
                    {
                        this.UpdateUrl = update.Url;
                        this.State = UpdateState.Available;
                    }

                    succeeded = true;
                }
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
            catch (XmlException)
            {
            }

            if (!succeeded)
            {
                this.State = UpdateState.Failed;
                this.UpdateUrl = null;
            }
        }

        /// <summary>
        /// Helper class to store AppSyndication URLs associated with their version.
        /// </summary>
        private class Update
        {
            public string Url { get; set; }
            public Version Version { get; set; }
        }
    }
}
