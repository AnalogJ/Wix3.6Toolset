//-------------------------------------------------------------------------------------------------
// <copyright file="IBootstrapperEngine.cs" company="Microsoft">
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
// COM interop interface for IBootstrapperEngine.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Bootstrapper
{
    using System;
    using System.CodeDom.Compiler;
    using System.Runtime.InteropServices;
    using System.Text;

    /// <summary>
    /// Allows calls into the bootstrapper engine.
    /// </summary>
    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    [Guid("6480D616-27A0-44D7-905B-81512C29C2FB")]
    [GeneratedCodeAttribute("Microsoft.Tools.WindowsInstallerXml.Bootstrapper.InteropCodeGenerator", "1.0.0.0")]
    public interface IBootstrapperEngine
    {
        void GetPackageCount(
            [MarshalAs(UnmanagedType.U4)] out int pcPackages
            );

        [PreserveSig]
        int GetVariableNumeric(
            [MarshalAs(UnmanagedType.LPWStr)] string wzVariable,
            out long pllValue
            );

        [PreserveSig]
        int GetVariableString(
            [MarshalAs(UnmanagedType.LPWStr)] string wzVariable,
            [MarshalAs(UnmanagedType.LPWStr), Out] StringBuilder wzValue,
            [MarshalAs(UnmanagedType.U4)] ref int pcchValue
            );

        [PreserveSig]
        int GetVariableVersion(
            [MarshalAs(UnmanagedType.LPWStr)] string wzVariable,
            [MarshalAs(UnmanagedType.U8)] out long pqwValue
            );

        [PreserveSig]
        int FormatString(
            [MarshalAs(UnmanagedType.LPWStr)] string wzIn,
            [MarshalAs(UnmanagedType.LPWStr), Out] StringBuilder wzOut,
            [MarshalAs(UnmanagedType.U4)] ref int pcchOut
            );

        [PreserveSig]
        int EscapeString(
            [MarshalAs(UnmanagedType.LPWStr)] string wzIn,
            [MarshalAs(UnmanagedType.LPWStr), Out] StringBuilder wzOut,
            [MarshalAs(UnmanagedType.U4)] ref int pcchOut
            );

        void EvaluateCondition(
            [MarshalAs(UnmanagedType.LPWStr)] string wzCondition,
            [MarshalAs(UnmanagedType.Bool)] out bool pf
            );

        void Log(
            [MarshalAs(UnmanagedType.U4)] LogLevel level,
            [MarshalAs(UnmanagedType.LPWStr)] string wzMessage
            );

        void SendEmbeddedError(
            [MarshalAs(UnmanagedType.U4)] int dwErrorCode,
            [MarshalAs(UnmanagedType.LPWStr)] string wzMessage,
            [MarshalAs(UnmanagedType.U4)] int dwUIHint,
            [MarshalAs(UnmanagedType.I4)] out int pnResult
            );

        void SendEmbeddedProgress(
            [MarshalAs(UnmanagedType.U4)] int dwProgressPercentage,
            [MarshalAs(UnmanagedType.U4)] int dwOverallProgressPercentage,
            [MarshalAs(UnmanagedType.I4)] out int pnResult
            );

        void SetLocalSource(
            [MarshalAs(UnmanagedType.LPWStr)] string wzPackageOrContainerId,
            [MarshalAs(UnmanagedType.LPWStr)] string wzPayloadId,
            [MarshalAs(UnmanagedType.LPWStr)] string wzPath
            );

        void SetDownloadSource(
            [MarshalAs(UnmanagedType.LPWStr)] string wzPackageOrContainerId,
            [MarshalAs(UnmanagedType.LPWStr)] string wzPayloadId,
            [MarshalAs(UnmanagedType.LPWStr)] string wzUrl,
            [MarshalAs(UnmanagedType.LPWStr)] string wzUser,
            [MarshalAs(UnmanagedType.LPWStr)] string wzPassword
            );

        void SetVariableNumeric(
            [MarshalAs(UnmanagedType.LPWStr)] string wzVariable,
            long llValue
            );

        void SetVariableString(
            [MarshalAs(UnmanagedType.LPWStr)] string wzVariable,
            [MarshalAs(UnmanagedType.LPWStr)] string wzValue
            );

        void SetVariableVersion(
            [MarshalAs(UnmanagedType.LPWStr)] string wzVariable,
            [MarshalAs(UnmanagedType.U8)] long qwValue
            );

        void CloseSplashScreen();

        void Detect();

        void Plan(
            [MarshalAs(UnmanagedType.U4)] LaunchAction action
            );

        [PreserveSig]
        int Elevate(
            IntPtr hwndParent
            );

        void Apply(
            IntPtr hwndParent
            );

        void Quit(
            [MarshalAs(UnmanagedType.U4)] int dwExitCode
            );
    }

    /// <summary>
    /// The installation action for the bundle or current package.
    /// </summary>
    public enum ActionState
    {
        None,
        Uninstall,
        Install,
        AdminInstall,
        Modify,
        Repair,
        MinorUpgrade,
        MajorUpgrade,
        Patch,
    }

    /// <summary>
    /// The action for the UX to perform.
    /// </summary>
    public enum LaunchAction
    {
        Unknown,
        Help,
        Layout,
        Uninstall,
        Install,
        Modify,
        Repair,
    }

    /// <summary>
    /// The message log level.
    /// </summary>
    public enum LogLevel
    {
        /// <summary>
        /// No logging level (generic).
        /// </summary>
        None,

        /// <summary>
        /// User messages.
        /// </summary>
        Standard,

        /// <summary>
        /// Verbose messages.
        /// </summary>
        Verbose,

        /// <summary>
        /// Messages for debugging.
        /// </summary>
        Debug,

        /// <summary>
        /// Error messages.
        /// </summary>
        Error,
    }

    /// <summary>
    /// Describes the state of an installation package.
    /// </summary>
    public enum PackageState
    {
        Unknown,
        Obsolete,
        Absent,
        Cached,
        Present,
        Superseded,
    }

    /// <summary>
    /// Indicates the state desired for an installation package.
    /// </summary>
    public enum RequestState
    {
        None,
        Absent,
        Cache,
        Present,
        Repair,
    }

    /// <summary>
    /// Indicates the state of a feature.
    /// </summary>
    public enum FeatureState
    {
        Unknown,
        Absent,
        Advertised,
        Local,
        Source,
    }

    /// <summary>
    /// Indicates the action for a feature.
    /// </summary>
    public enum FeatureAction
    {
        None,
        AddLocal,
        AddSource,
        AddDefault,
        Reinstall,
        Advertise,
        Remove,
    }
}
