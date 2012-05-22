//-------------------------------------------------------------------------------------------------
// <copyright file="Session.cs" company="Microsoft">
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
// Controls the installation process.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Msi
{
    using System;
    using System.ComponentModel;
    using System.Globalization;
    using Microsoft.Tools.WindowsInstallerXml.Msi.Interop;

    /// <summary>
    /// Controls the installation process.
    /// </summary>
    internal sealed class Session : MsiHandle
    {
        /// <summary>
        /// Instantiate a new Session.
        /// </summary>
        /// <param name="database">The database to open.</param>
        public Session(Database database)
        {
            string packagePath = String.Format(CultureInfo.InvariantCulture, "#{0}", (uint)database.Handle);

            uint handle = 0;
            int error = MsiInterop.MsiOpenPackage(packagePath, out handle);
            if (0 != error)
            {
                throw new MsiException(error);
            }
            this.Handle = handle;
        }

        /// <summary>
        /// Executes a built-in action, custom action, or user-interface wizard action.
        /// </summary>
        /// <param name="action">Specifies the action to execute.</param>
        public void DoAction(string action)
        {
            int error = MsiInterop.MsiDoAction(this.Handle, action);
            if (0 != error)
            {
                throw new MsiException(error);
            }
        }
    }
}
