//-------------------------------------------------------------------------------------------------
// <copyright file="LuxLogger.cs" company="Microsoft">
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
// The Lux unit-test framework logging classes.
// </summary>
//-------------------------------------------------------------------------------------------------


namespace Microsoft.Tools.WindowsInstallerXml.Lux.CustomActions
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using Microsoft.Deployment.WindowsInstaller;

    /// <summary>
    /// Logging class for Lux CAs.
    /// </summary>
    public class LuxLogger
    {
        private Session session;

        /// <summary>
        /// Initializes a new instance of the LuxLogger class.
        /// </summary>
        /// <param name="session">MSI session handle</param>
        public LuxLogger(Session session)
        {
            this.session = session;
        }

        /// <summary>
        /// Logs a message to the MSI log using the MSI error table.
        /// </summary>
        /// <param name="id">Error message id</param>
        /// <param name="args">Arguments to the error message (will be inserted into the formatted error message).</param>
        public void Log(int id, params string[] args)
        {
            // field 0 is free, field 1 is the id, fields 2..n are arguments
            using (Record rec = new Record(1 + args.Length))
            {
                rec.SetInteger(1, id);
                for (int i = 0; i < args.Length; i++)
                {
                    rec.SetString(2 + i, args[i]);
                }

                this.session.Message(InstallMessage.User, rec);
            }
        }
    }
}
