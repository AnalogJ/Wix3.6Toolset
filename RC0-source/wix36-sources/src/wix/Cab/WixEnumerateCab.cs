//-------------------------------------------------------------------------------------------------
// <copyright file="WixEnumerateCab.cs" company="Microsoft">
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
// Wrapper class around interop with wixcab.dll to enumerate files from a cabinet.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Cab
{
    using System;
    using System.Collections;
    using System.Runtime.InteropServices;
    using Microsoft.Tools.WindowsInstallerXml.Cab.Interop;
    using Handle = System.Int32;

    /// <summary>
    /// Wrapper class around interop with wixcab.dll to enumerate files from a cabinet.
    /// </summary>
    public sealed class WixEnumerateCab : IDisposable
    {
        private bool disposed;
        private ArrayList fileInfoList;
        private CabInterop.PFNNOTIFY pfnNotify;

        /// <summary>
        /// Creates a cabinet enumerator.
        /// </summary>
        public WixEnumerateCab()
        {
            this.fileInfoList = new ArrayList();
            this.pfnNotify = new CabInterop.PFNNOTIFY(this.Notify);
            NativeMethods.EnumerateCabBegin();
        }

        /// <summary>
        /// Destructor for cabinet enumeration.
        /// </summary>
        ~WixEnumerateCab()
        {
            this.Dispose();
        }

        /// <summary>
        /// Enumerates all files in a cabinet.
        /// </summary>
        /// <param name="cabinetFile">path to cabinet</param>
        /// <returns>list of CabinetFileInfo</returns>
        public ArrayList Enumerate(string cabinetFile)
        {
            this.fileInfoList.Clear(); // we need to clear the list before starting new one

            // the callback (this.Notify) will populate the list for each file in cabinet
            NativeMethods.EnumerateCab(cabinetFile, this.pfnNotify);

            return this.fileInfoList;
        }

        /// <summary>
        /// Disposes the managed and unmanaged objects in this object.
        /// </summary>
        public void Dispose()
        {
            if (!this.disposed)
            {
                NativeMethods.EnumerateCabFinish();

                GC.SuppressFinalize(this);
                this.disposed = true;
            }
        }

        /// <summary>
        /// Delegate that's called for every file in cabinet.
        /// </summary>
        /// <param name="fdint">NOTIFICATIONTYPE</param>
        /// <param name="pfdin">NOTIFICATION</param>
        /// <returns>System.Int32</returns>
        internal Handle Notify(CabInterop.NOTIFICATIONTYPE fdint, CabInterop.NOTIFICATION pfdin)
        {
            // This is FDI's way of notifying us of how many files total are in the cab, accurate even 
            // if the files are split into multiple folders - use it to allocate the precise size we need
            if (CabInterop.NOTIFICATIONTYPE.ENUMERATE == fdint && 0 == this.fileInfoList.Count)
            {
                this.fileInfoList.Capacity = pfdin.Folder;
            }

            if (fdint == CabInterop.NOTIFICATIONTYPE.COPY_FILE)
            {
                CabinetFileInfo fileInfo = new CabinetFileInfo(pfdin.Psz1, pfdin.Date, pfdin.Time);
                this.fileInfoList.Add(fileInfo);
            }
            return 0; // tell cabinet api to skip this file
        }
    }
}
