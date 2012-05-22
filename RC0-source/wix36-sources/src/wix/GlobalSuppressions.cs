//-------------------------------------------------------------------------------------------------
// <copyright file="GlobalSuppressions.cs" company="Microsoft">
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
// Global suppression of inappropriate code analysis tools.
// </summary>
//-------------------------------------------------------------------------------------------------

// MSI-defined types
[assembly: System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Design", "CA1020:AvoidNamespacesWithFewTypes", Scope = "namespace", Target = "Microsoft.Tools.WindowsInstallerXml.Cab")]
[assembly: System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Design", "CA1008:EnumsShouldHaveZeroValue", Scope = "type", Target = "Microsoft.Tools.WindowsInstallerXml.Msi.InstallMessage")]
[assembly: System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Design", "CA1008:EnumsShouldHaveZeroValue", Scope = "type", Target = "Microsoft.Tools.WindowsInstallerXml.Msi.InstallUILevels")]
[assembly: System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Design", "CA1027:MarkEnumsWithFlags", Scope = "type", Target = "Microsoft.Tools.WindowsInstallerXml.Msi.OpenDatabase")]
[assembly: System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Design", "CA1032:ImplementStandardExceptionConstructors")]
[assembly: System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Design", "CA1036:OverrideMethodsOnComparableTypes")]

[assembly: System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Maintainability", "CA1502:AvoidExcessiveComplexity")]

// .NET 2.0 requirement
[assembly: System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Reliability", "CA2000:DisposeObjectsBeforeLosingScope", Scope = "member", Target = "Microsoft.Tools.WindowsInstallerXml.Intermediate.Load(System.String,Microsoft.Tools.WindowsInstallerXml.TableDefinitionCollection,System.Boolean,System.Boolean):Microsoft.Tools.WindowsInstallerXml.Intermediate")]
[assembly: System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Reliability", "CA2000:DisposeObjectsBeforeLosingScope", Scope = "member", Target = "Microsoft.Tools.WindowsInstallerXml.Library.Load(System.IO.Stream,System.Uri,Microsoft.Tools.WindowsInstallerXml.TableDefinitionCollection,System.Boolean,System.Boolean):Microsoft.Tools.WindowsInstallerXml.Library")]
[assembly: System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Reliability", "CA2000:DisposeObjectsBeforeLosingScope", Scope = "member", Target = "Microsoft.Tools.WindowsInstallerXml.Localization.Load(System.IO.Stream,System.Uri,Microsoft.Tools.WindowsInstallerXml.TableDefinitionCollection,System.Boolean):Microsoft.Tools.WindowsInstallerXml.Localization")]
[assembly: System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Reliability", "CA2000:DisposeObjectsBeforeLosingScope", Scope = "member", Target = "Microsoft.Tools.WindowsInstallerXml.Output.Load(System.IO.Stream,System.Uri,System.Boolean,System.Boolean):Microsoft.Tools.WindowsInstallerXml.Output")]

// .NET 2.0 requirement
[assembly: System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Reliability", "CA2006:UseSafeHandleToEncapsulateNativeResources", Scope = "member", Target = "Microsoft.Tools.WindowsInstallerXml.Cab.WixCreateCab.handle")]

[assembly: System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", MessageId = "Msi", Scope = "namespace", Target = "Microsoft.Tools.WindowsInstallerXml.Msi")]
[assembly: System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", MessageId = "wix")]
[assembly: System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1709:IdentifiersShouldBeCasedCorrectly", MessageId = "wix")]
[assembly: System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", MessageId = "wx")]
