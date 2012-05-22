//-------------------------------------------------------------------------------------------------
// <copyright file="AssemblyInfo.cs" company="Microsoft">
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
// The assembly information for the Windows Installer XML Toolset Visual Studio Extension.
// </summary>
//-------------------------------------------------------------------------------------------------

using System;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

using Microsoft.Tools.WindowsInstallerXml;
using Microsoft.Tools.WindowsInstallerXml.Extensions;
using Microsoft.Tools.WindowsInstallerXml.Tools;

[assembly: AssemblyTitle("WiX Toolset VS Extension")]
[assembly: AssemblyDescription("Windows Installer XML Toolset Visual Studio Extension")]
[assembly: AssemblyCulture("")]
[assembly: CLSCompliant(true)]
[assembly: ComVisible(false)]
[assembly: AssemblyDefaultWixExtension(typeof(VSExtension))]
[assembly: AssemblyDefaultHeatExtension(typeof(VSHeatExtension))]
