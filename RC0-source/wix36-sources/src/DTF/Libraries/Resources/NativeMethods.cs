//---------------------------------------------------------------------
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
// <summary>
// Part of the Deployment Tools Foundation project.
// </summary>
//---------------------------------------------------------------------

namespace Microsoft.Deployment.Resources
{
    using System;
    using System.IO;
    using System.Text;
    using System.Reflection;
    using System.Collections;
    using System.Globalization;
    using System.Runtime.InteropServices;

    internal static class NativeMethods
    {
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        internal static extern IntPtr LoadLibraryEx(string fileName, IntPtr hFile, uint flags);
        internal const uint LOAD_LIBRARY_AS_DATAFILE = 2;
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)] [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool FreeLibrary(IntPtr module);
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)] [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool EnumResourceTypes(IntPtr module, EnumResTypesProc enumFunc, IntPtr param);
        [return: MarshalAs(UnmanagedType.Bool)]
        internal delegate bool EnumResTypesProc(IntPtr module, IntPtr type, IntPtr param);
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)] [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool EnumResourceNames(IntPtr module, IntPtr type, EnumResNamesProc enumFunc, IntPtr param);
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)] [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool EnumResourceNames(IntPtr module, string type, EnumResNamesProc enumFunc, IntPtr param);
        [return: MarshalAs(UnmanagedType.Bool)]
        internal delegate bool EnumResNamesProc(IntPtr module, IntPtr type, IntPtr name, IntPtr param);
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)] [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool EnumResourceLanguages(IntPtr module, IntPtr type, IntPtr name, EnumResLangsProc enumFunc, IntPtr param);
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)] [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool EnumResourceLanguages(IntPtr module, string type, string name, EnumResLangsProc enumFunc, IntPtr param);
        [return: MarshalAs(UnmanagedType.Bool)]
        internal delegate bool EnumResLangsProc(IntPtr module, IntPtr type, IntPtr name, ushort langId, IntPtr param);
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        internal static extern IntPtr FindResourceEx(IntPtr module, string type, string name, ushort langId);
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        internal static extern IntPtr LoadResource(IntPtr module, IntPtr resourceInfo);
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        internal static extern IntPtr LockResource(IntPtr resourceData);
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        internal static extern uint SizeofResource(IntPtr module, IntPtr resourceInfo);
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        internal static extern IntPtr BeginUpdateResource(string fileName, [MarshalAs(UnmanagedType.Bool)] bool deleteExistingResources);
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)] [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool UpdateResource(IntPtr updateHandle, IntPtr type, IntPtr name, ushort lcid, IntPtr data, uint dataSize);
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)][return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool UpdateResource(IntPtr updateHandle, IntPtr type, string name, ushort lcid, IntPtr data, uint dataSize);
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)] [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool UpdateResource(IntPtr updateHandle, string type, string name, ushort lcid, IntPtr data, uint dataSize);
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)] [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool EndUpdateResource(IntPtr updateHandle, [MarshalAs(UnmanagedType.Bool)] bool discardChanges);
    }
}
