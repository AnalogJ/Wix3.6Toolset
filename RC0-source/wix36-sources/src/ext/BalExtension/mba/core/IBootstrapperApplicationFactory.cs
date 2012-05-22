//-------------------------------------------------------------------------------------------------
// <copyright file="IBootstrapperApplicationFactory.cs" company="Microsoft">
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
// Class interface for the BootstrapperApplicationFactory class.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Bootstrapper
{
    using System;
    using System.CodeDom.Compiler;
    using System.Runtime.InteropServices;

    [ComVisible(true)]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    [Guid("2965A12F-AC7B-43A0-85DF-E4B2168478A4")]
    [GeneratedCodeAttribute("Microsoft.Tools.WindowsInstallerXml.Bootstrapper.InteropCodeGenerator", "1.0.0.0")]
    public interface IBootstrapperApplicationFactory
    {
        IBootstrapperApplication Create(
            [MarshalAs(UnmanagedType.Interface)] IBootstrapperEngine pEngine,
            ref Command command
            );
    }

    /// <summary>
    /// Command information passed from the engine for the user experience to perform.
    /// </summary>
    [Serializable]
    [StructLayout(LayoutKind.Sequential)]
    [GeneratedCodeAttribute("Microsoft.Tools.WindowsInstallerXml.Bootstrapper.InteropCodeGenerator", "1.0.0.0")]
    public struct Command
    {
        [MarshalAs(UnmanagedType.U4)] private readonly LaunchAction action;
        [MarshalAs(UnmanagedType.U4)] private readonly Display display;
        [MarshalAs(UnmanagedType.U4)] private readonly Restart restart;
        [MarshalAs(UnmanagedType.LPWStr)] private readonly string wzCommandLine;
        [MarshalAs(UnmanagedType.I4)] private readonly int nCmdShow;
        [MarshalAs(UnmanagedType.U4)] private readonly ResumeType resume;
        private readonly IntPtr hwndSplashScreen;
        [MarshalAs(UnmanagedType.I4)] private readonly RelationType relation;
        [MarshalAs(UnmanagedType.LPWStr)]
        private readonly string wzLayoutDirectory;

        /// <summary>
        /// Gets the action for the user experience to perform.
        /// </summary>
        public LaunchAction Action
        {
            get { return this.action; }
        }

        /// <summary>
        /// Gets the display level for the user experience.
        /// </summary>
        public Display Display
        {
            get { return this.display; }
        }

        /// <summary>
        /// Gets the action to perform if a reboot is required.
        /// </summary>
        public Restart Restart
        {
            get { return this.restart; }
        }

        /// <summary>
        /// Gets command line arguments.
        /// </summary>
        public string CommandLine
        {
            get { return this.wzCommandLine; }
        }

        /// <summary>
        /// Gets layout directory.
        /// </summary>
        public string LayoutDirectory
        {
            get { return this.wzLayoutDirectory; }
        }

        /// <summary>
        /// Gets the method of how the engine was resumed from a previous installation step.
        /// </summary>
        public ResumeType Resume
        {
            get { return this.resume; }
        }

        /// <summary>
        /// Gets the handle to the splash screen window. If no splash screen was displayed this value will be IntPtr.Zero.
        /// </summary>
        public IntPtr SplashScreen
        {
            get { return this.hwndSplashScreen; }
        }

        /// <summary>
        /// If this was run from a related bundle, specifies the relation type
        /// </summary>
        public RelationType Relation
        {
            get { return this.relation; }
        }
    }
}
