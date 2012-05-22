//-------------------------------------------------------------------------------------------------
// <copyright file="CompilerExtension.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
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
// The base compiler extension.  Any of these methods can be overridden to change
// the behavior of the compiler.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics.CodeAnalysis;
    using System.IO;
    using System.Reflection;
    using System.Xml;
    using System.Xml.Schema;

    /// <summary>
    /// Base class for creating a compiler extension.
    /// </summary>
    public abstract class CompilerExtension
    {
        public enum ComponentKeypathType
        {
            None,
            File,
            Directory,
            ODBCDataSource,
            Registry,
            RegistryFormatted
        }

        private CompilerCore compilerCore;

        /// <summary>
        /// Gets or sets the compiler core for the extension.
        /// </summary>
        /// <value>Compiler core for the extension.</value>
        public CompilerCore Core
        {
            get { return this.compilerCore; }
            set { this.compilerCore = value; }
        }

        /// <summary>
        /// Gets the schema for this extension.
        /// </summary>
        /// <value>Schema for this extension.</value>
        public abstract XmlSchema Schema
        {
            get;
        }

        /// <summary>
        /// Called at the beginning of the compilation of a source file.
        /// </summary>
        public virtual void InitializeCompile()
        {
        }

        /// <summary>
        /// Processes an attribute for the Compiler.
        /// </summary>
        /// <param name="sourceLineNumbers">Source line number for the parent element.</param>
        /// <param name="parentElement">Parent element of attribute.</param>
        /// <param name="attribute">Attribute to process.</param>
        [SuppressMessage("Microsoft.Design", "CA1059:MembersShouldNotExposeCertainConcreteTypes")]
        public virtual void ParseAttribute(SourceLineNumberCollection sourceLineNumbers, XmlElement parentElement, XmlAttribute attribute)
        {
            this.Core.UnexpectedAttribute(sourceLineNumbers, attribute);
        }

        /// <summary>
        /// Processes an attribute for the Compiler.
        /// </summary>
        /// <param name="sourceLineNumbers">Source line number for the parent element.</param>
        /// <param name="parentElement">Parent element of attribute.</param>
        /// <param name="attribute">Attribute to process.</param>
        /// <param name="contextValues">Extra information about the context in which this element is being parsed.</param>
        [SuppressMessage("Microsoft.Design", "CA1059:MembersShouldNotExposeCertainConcreteTypes")]
        public virtual void ParseAttribute(SourceLineNumberCollection sourceLineNumbers, XmlElement parentElement, XmlAttribute attribute, Dictionary<string, string> contextValues)
        {
            this.Core.UnexpectedAttribute(sourceLineNumbers, attribute);
        }

        /// <summary>
        /// Processes an element for the Compiler.
        /// </summary>
        /// <param name="sourceLineNumbers">Source line number for the parent element.</param>
        /// <param name="parentElement">Parent element of element to process.</param>
        /// <param name="element">Element to process.</param>
        /// <param name="contextValues">Extra information about the context in which this element is being parsed.</param>
        [SuppressMessage("Microsoft.Design", "CA1059:MembersShouldNotExposeCertainConcreteTypes")]
        public virtual void ParseElement(SourceLineNumberCollection sourceLineNumbers, XmlElement parentElement, XmlElement element, params string[] contextValues)
        {
            this.Core.UnexpectedElement(parentElement, element);
        }

        /// <summary>
        /// Processes an element for the Compiler, with the ability to supply a component keypath.
        /// </summary>
        /// <param name="sourceLineNumbers">Source line number for the parent element.</param>
        /// <param name="parentElement">Parent element of element to process.</param>
        /// <param name="element">Element to process.</param>
        /// <param name="keyPath">Explicit key path.</param>
        /// <param name="contextValues">Extra information about the context in which this element is being parsed.</param>
        [SuppressMessage("Microsoft.Design", "CA1059:MembersShouldNotExposeCertainConcreteTypes")]
        public virtual ComponentKeypathType ParseElement(SourceLineNumberCollection sourceLineNumbers, XmlElement parentElement, XmlElement element, ref string keyPath, params string[] contextValues)
        {
            this.ParseElement(sourceLineNumbers, parentElement, element, contextValues);
            return ComponentKeypathType.None;
        }

        /// <summary>
        /// Called at the end of the compilation of a source file.
        /// </summary>
        public virtual void FinalizeCompile()
        {
        }

        /// <summary>
        /// Helper for loading an xml schema from an embedded resource.
        /// </summary>
        /// <param name="assembly">The assembly containing the embedded resource.</param>
        /// <param name="resourceName">The name of the embedded resource being requested.</param>
        /// <returns>The loaded xml schema.</returns>
        protected static XmlSchema LoadXmlSchemaHelper(Assembly assembly, string resourceName)
        {
            using (Stream resourceStream = assembly.GetManifestResourceStream(resourceName))
            {
                return XmlSchema.Read(resourceStream, null);
            }
        }
    }
}
