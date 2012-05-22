//-------------------------------------------------------------------------------------------------
// <copyright file="RootViewModel.cs" company="Microsoft">
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
// The model of the view for the WixBA.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.UX
{
    using System;
    using System.Diagnostics;
    using System.Reflection;
    using System.Windows;
    using System.Windows.Input;
    using Microsoft.Tools.WindowsInstallerXml.Bootstrapper;

    /// <summary>
    /// The errors returned from the engine
    /// </summary>
    public enum Error
    {
        UserCancelled = 1223,
    }

    /// <summary>
    /// The model of the root view in WixBA.
    /// </summary>
    public class RootViewModel : PropertyNotifyBase
    {
        private ICommand cancelCommand;
        private ICommand closeCommand;
        private ICommand refreshCommand;

        private bool canceled;
        private InstallationState state;

        /// <summary>
        /// Creates a new model of the root view.
        /// </summary>
        public RootViewModel()
        {
            this.InstallationViewModel = new InstallationViewModel(this);
            this.ProgressViewModel = new ProgressViewModel(this);
            this.UpdateViewModel = new UpdateViewModel(this);
        }

        public InstallationViewModel InstallationViewModel { get; private set; }
        public ProgressViewModel ProgressViewModel { get; private set; }
        public UpdateViewModel UpdateViewModel { get; private set; }

        public ICommand CloseCommand
        {
            get
            {
                if (this.closeCommand == null)
                {
                    this.closeCommand = new RelayCommand(param => WixBA.View.Close());
                }

                return this.closeCommand;
            }
        }

        public ICommand RefreshCommand
        {
            get
            {
                if (this.refreshCommand == null)
                {
                    this.refreshCommand = new RelayCommand(param => this.Refresh(), param => false); // TODO: enable this command
                }

                return this.refreshCommand;
            }
        }

        public ICommand CancelCommand
        {
            get
            {
                if (this.cancelCommand == null)
                {
                    this.cancelCommand = new RelayCommand(param =>
                    {
                        lock (this)
                        {
                            this.Canceled = (MessageBoxResult.Yes == MessageBox.Show(WixBA.View, "Are you sure you want to cancel?", "WiX Toolset", MessageBoxButton.YesNo, MessageBoxImage.Error));
                        }
                    },
                    param => this.State == InstallationState.Applying);
                }

                return this.cancelCommand;
            }
        }

        public bool CancelEnabled
        {
            get { return this.CancelCommand.CanExecute(this); }
        }

        public bool Canceled
        {
            get
            {
                return this.canceled;
            }

            set
            {
                if (this.canceled != value)
                {
                    this.canceled = value;
                    base.OnPropertyChanged("Canceled");
                }
            }
        }

        /// <summary>
        /// Gets and sets the state of the view's model.
        /// </summary>
        public InstallationState State
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

                    // Notify all the properties derived from the state that the state changed.
                    base.OnPropertyChanged("State");
                    base.OnPropertyChanged("CancelEnabled");
                }
            }
        }

        /// <summary>
        /// Gets and sets the state of the view's model before apply begins in order to return to that state if cancel or rollback occurs.
        /// </summary>
        public InstallationState PreApplyState { get; set; }

        /// <summary>
        /// Instructs the various child models to refresh. Called directly via
        /// the UX *once* to initialize all the models. After that, only called
        /// when the RefreshCommand is executed.
        /// </summary>
        public void Refresh()
        {
            this.InstallationViewModel.Refresh();
            this.UpdateViewModel.Refresh();
        }
    }
}
