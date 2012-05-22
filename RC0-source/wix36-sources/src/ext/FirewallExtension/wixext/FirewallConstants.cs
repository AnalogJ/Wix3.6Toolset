//-------------------------------------------------------------------------------------------------
// <copyright file="FirewallConstants.cs" company="Microsoft">
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
// Constants used by FirewallExtension
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Extensions
{
    using System;
    using System.Collections.Generic;
    using System.Text;

    static class FirewallConstants
    {
        // from icftypes.h
        public const int NET_FW_IP_PROTOCOL_TCP = 6;
        public const int NET_FW_IP_PROTOCOL_UDP = 17;

        // from icftypes.h
        public const int NET_FW_PROFILE2_DOMAIN = 0x0001;
        public const int NET_FW_PROFILE2_PRIVATE = 0x0002;
        public const int NET_FW_PROFILE2_PUBLIC = 0x0004;
        public const int NET_FW_PROFILE2_ALL = 0x7FFFFFFF;
    }
}
