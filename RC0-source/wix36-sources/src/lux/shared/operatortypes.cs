//-------------------------------------------------------------------------------------------------
// <copyright file="operatortypes.cs" company="Microsoft">
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
// The Lux unit-test framework test operator types enum.
// </summary>
//-------------------------------------------------------------------------------------------------



namespace Microsoft.Tools.WindowsInstallerXml.Lux
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using Microsoft.Deployment.WindowsInstaller;

    /// <summary>
    /// The allowed operators for the LuxUnitTest MSI table 'op' column.
    /// </summary>
    public enum LuxOperator
    {
        /// <summary>No value specified.</summary>
        NotSet = 0,

        /// <summary>Equality comparison.</summary>
        Equal,

        /// <summary>Inequality comparison.</summary>
        NotEqual,

        /// <summary>Case-insensitive equality comparison.</summary>
        CaseInsensitiveEqual,

        /// <summary>Case-insensitive inequality comparison.</summary>
        CaseInsensitiveNotEqual,
    }    
}
