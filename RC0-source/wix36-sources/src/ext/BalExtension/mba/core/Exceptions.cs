//-------------------------------------------------------------------------------------------------
// <copyright file="Exceptions.cs" company="Microsoft">
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
// Exceptions used by the managed bootstrapper application classes.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Bootstrapper
{
    using System;
    using System.Runtime.Serialization;

    /// <summary>
    /// Base class for exception returned to the bootstrapper application host.
    /// </summary>
    [Serializable]
    public abstract class BootstrapperException : Exception
    {
        /// <summary>
        /// Creates an instance of the <see cref="BootstrapperException"/> base class with the given HRESULT.
        /// </summary>
        /// <param name="hr">The HRESULT for the exception that is used by the bootstrapper application host.</param>
        public BootstrapperException(int hr)
        {
            this.HResult = hr;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="BootstrapperException"/> class.
        /// </summary>
        /// <param name="message">Exception message.</param>
        public BootstrapperException(string message)
            : base(message)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="BootstrapperException"/> class.
        /// </summary>
        /// <param name="message">Exception message</param>
        /// <param name="innerException">Inner exception associated with this one</param>
        public BootstrapperException(string message, Exception innerException)
            : base(message, innerException)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="BootstrapperException"/> class.
        /// </summary>
        /// <param name="info">Serialization information for this exception</param>
        /// <param name="context">Streaming context to serialize to</param>
        protected BootstrapperException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }

    /// <summary>
    /// The bootstrapper application loaded by the host does not contain exactly one instance of the
    /// <see cref="BootstrapperApplicationAttribute"/> class.
    /// </summary>
    /// <seealso cref="BootstrapperApplicationAttribute"/>
    [Serializable]
    public class MissingAttributeException : BootstrapperException
    {
        /// <summary>
        /// Creates a new instance of the <see cref="MissingAttributeException"/> class.
        /// </summary>
        public MissingAttributeException()
            : base(NativeMethods.E_NOTFOUND)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="MissingAttributeException"/> class.
        /// </summary>
        /// <param name="message">Exception message.</param>
        public MissingAttributeException(string message)
            : base(message)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="MissingAttributeException"/> class.
        /// </summary>
        /// <param name="message">Exception message</param>
        /// <param name="innerException">Inner exception associated with this one</param>
        public MissingAttributeException(string message, Exception innerException)
            : base(message, innerException)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="MissingAttributeException"/> class.
        /// </summary>
        /// <param name="info">Serialization information for this exception</param>
        /// <param name="context">Streaming context to serialize to</param>
        protected MissingAttributeException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }

    /// <summary>
    /// The bootstrapper application specified by the <see cref="BootstrapperApplicationAttribute"/>
    ///  does not extend the <see cref="BootstrapperApplication"/> base class.
    /// </summary>
    /// <seealso cref="BootstrapperApplication"/>
    /// <seealso cref="BootstrapperApplicationAttribute"/>
    [Serializable]
    public class InvalidBootstrapperApplicationException : BootstrapperException
    {
        /// <summary>
        /// Creates a new instance of the <see cref="InvalidBootstrapperApplicationException"/> class.
        /// </summary>
        public InvalidBootstrapperApplicationException()
            : base(NativeMethods.E_UNEXPECTED)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="InvalidBootstrapperApplicationException"/> class.
        /// </summary>
        /// <param name="message">Exception message.</param>
        public InvalidBootstrapperApplicationException(string message)
            : base(message)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="InvalidBootstrapperApplicationException"/> class.
        /// </summary>
        /// <param name="message">Exception message</param>
        /// <param name="innerException">Inner exception associated with this one</param>
        public InvalidBootstrapperApplicationException(string message, Exception innerException)
            : base(message, innerException)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="InvalidBootstrapperApplicationException"/> class.
        /// </summary>
        /// <param name="info">Serialization information for this exception</param>
        /// <param name="context">Streaming context to serialize to</param>
        protected InvalidBootstrapperApplicationException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }
}
