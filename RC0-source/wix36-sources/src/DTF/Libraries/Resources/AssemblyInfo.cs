//---------------------------------------------------------------------
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
// <summary>
// Part of the Deployment Tools Foundation project.
// </summary>
//---------------------------------------------------------------------

using System;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Diagnostics.CodeAnalysis;
using System.Security;
using System.Security.Permissions;

[assembly: AssemblyDescription("Classes for reading and writing resource data in executable files")]
[assembly: ComVisible(false)]
[assembly: CLSCompliant(true)]

// SECURITY: The UnmanagedCode assertions in the resource classes are safe, because
// appropriate demands are made for file I/O permissions before reading/writing files.
[assembly: SecurityPermission(SecurityAction.RequestMinimum, Assertion = true, UnmanagedCode = true)]

// SECURITY: Review carefully!
// This assembly is designed so that partially trusted callers should be able to
// read and write file version info in a path where they have limited
// file I/O permission.
[assembly: AllowPartiallyTrustedCallers]


[assembly: SuppressMessage("Microsoft.Design", "CA1033:InterfaceMethodsShouldBeCallableByChildTypes", Scope = "member", Target = "Microsoft.Deployment.Resources.ResourceCollection.#System.Collections.Generic.ICollection`1<Microsoft.Deployment.Resources.Resource>.IsReadOnly")]
