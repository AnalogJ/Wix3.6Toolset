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

[assembly: AssemblyDescription("LINQ extensions for Windows Installer classes")]
[assembly: ComVisible(false)]
[assembly: CLSCompliant(true)]

[assembly: SuppressMessage("Microsoft.Design", "CA1004:GenericMethodsShouldProvideTypeParameter", Scope = "member", Target = "Microsoft.Deployment.WindowsInstaller.Linq.QTable`1.System.Linq.IQueryable<TRecord>.CreateQuery(System.Linq.Expressions.Expression):System.Linq.IQueryable`1<TElement>")]
[assembly: SuppressMessage("Microsoft.Design", "CA1004:GenericMethodsShouldProvideTypeParameter", Scope = "member", Target = "Microsoft.Deployment.WindowsInstaller.Linq.QTable`1.System.Linq.IQueryable<TRecord>.Execute(System.Linq.Expressions.Expression):TResult")]
