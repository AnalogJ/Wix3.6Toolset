//-------------------------------------------------------------------------------------------------
// <copyright file="MsiHandle.cs" company="Microsoft">
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
// Wrapper for MSI API handles.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Msi
{
    using System;
    using System.ComponentModel;
    using Microsoft.Tools.WindowsInstallerXml.Msi.Interop;
    using System.Threading;
    using System.Diagnostics;

    /// <summary>
    /// Wrapper class for MSI handle.
    /// </summary>
    public class MsiHandle : IDisposable
    {
        private bool disposed;
        private uint handle;
        private int owningThread;
#if DEBUG
        private string creationStack;
#endif

        /// <summary>
        /// MSI handle destructor.
        /// </summary>
        ~MsiHandle()
        {
            this.Dispose(false);
        }

        /// <summary>
        /// Gets or sets the MSI handle.
        /// </summary>
        /// <value>The MSI handle.</value>
        internal uint Handle
        {
            get
            {
                if (this.disposed)
                {
                    throw new ObjectDisposedException("MsiHandle");
                }

                return this.handle;
            }

            set
            {
                if (this.disposed)
                {
                    throw new ObjectDisposedException("MsiHandle");
                }

                this.handle = value;
                this.owningThread = Thread.CurrentThread.ManagedThreadId;
#if DEBUG
                this.creationStack = Environment.StackTrace;
#endif
            }
        }

        /// <summary>
        /// Close the MSI handle.
        /// </summary>
        public void Close()
        {
            this.Dispose();
        }

        /// <summary>
        /// Disposes the managed and unmanaged objects in this object.
        /// </summary>
        public void Dispose()
        {
            this.Dispose(true);
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Disposes the managed and unmanaged objects in this object.
        /// </summary>
        /// <param name="disposing">true to dispose the managed objects.</param>
        protected virtual void Dispose(bool disposing)
        {
            if (!this.disposed)
            {
                if (0 != this.handle)
                {
                    if (Thread.CurrentThread.ManagedThreadId == this.owningThread)
                    {
                        int error = MsiInterop.MsiCloseHandle(this.handle);
                        if (0 != error)
                        {
                            throw new Win32Exception(error);
                        }
                        this.handle = 0;
                    }
                    else
                    {
                        // Don't try to close the handle on a different thread than it was opened.
                        // This will occasionally cause MSI to AV.
                        string message = String.Format("Leaked msi handle {0} created on thread {1} by type {2}.  This handle cannot be closed on thread {3}",
                            this.handle, this.owningThread, this.GetType(), Thread.CurrentThread.ManagedThreadId);
#if DEBUG
                        throw new InvalidOperationException(String.Format("{0}.  Created {1}", message, this.creationStack));
#else
                        Debug.WriteLine(message);
#endif
                    }
                }

                this.disposed = true;
            }
        }
    }
}
