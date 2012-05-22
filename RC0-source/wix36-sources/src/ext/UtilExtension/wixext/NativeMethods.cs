//-------------------------------------------------------------------------------------------------
// <copyright file="NativeMethods.cs" company="Microsoft">
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
// Native methods for util extension.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Extensions
{
    using System;
    using System.Runtime.InteropServices;

    /// <summary>
    /// Native methods for Util extension.
    /// </summary>
    internal sealed class NativeMethods
    {
        /// <summary>
        /// Creates a self-extracting installation package.
        /// </summary>
        /// <param name="wzSetupStub">Path to the input setup stub exe.</param>
        /// <param name="rgPackages">Array of packages to put in setup stub exe.</param>
        /// <param name="cPackages">Count of packages to put in stub.</param>
        /// <param name="wzOutput">Path to the output setup exe.</param>
        /// <returns>HRESULT from function.</returns>
        [DllImport("setupbuilder.dll", EntryPoint = "CreateSetup", CharSet = CharSet.Unicode, ExactSpelling = true)]
        public static extern int CreateSetup([MarshalAs(UnmanagedType.LPWStr)]string wzSetupStub, CREATE_SETUP_PACKAGE[] rgPackages, int cPackages, [MarshalAs(UnmanagedType.LPWStr)]string wzOutput);

        /// <summary>
        /// Creates a self-extracting installation package.
        /// </summary>
        /// <param name="wzSetupStub">Path to the input setup stub exe.</param>
        /// <param name="wzMsi">Path to the input MSI.</param>
        /// <param name="wzOutput">Path to the output setup exe.</param>
        /// <returns>HRESULT from function.</returns>
        [DllImport("setupbuilder.dll", EntryPoint = "CreateSimpleSetup", CharSet = CharSet.Unicode, ExactSpelling = true)]
        public static extern int CreateSimpleSetup([MarshalAs(UnmanagedType.LPWStr)]string wzSetupStub, [MarshalAs(UnmanagedType.LPWStr)]string wzMsi, [MarshalAs(UnmanagedType.LPWStr)]string wzOutput);

        /// <summary>
        /// Contains parameters for the CreateSetup function and receives information about the folder selected by the user.
        /// </summary>
        [StructLayout(LayoutKind.Sequential)]
        public struct CREATE_SETUP_PACKAGE
        {
            public string wzSourcePath;
            public bool fPrivileged;
            public bool fCache;
        }
    }
}
