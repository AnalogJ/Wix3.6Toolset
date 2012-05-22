//-------------------------------------------------------------------------------------------------
// <copyright file="NewsItem.cs" company="Microsoft">
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
// The model for an individual news item.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.UX
{
    using System;

    /// <summary>
    /// The model for an individual news item.
    /// </summary>
    public class NewsItem
    {
        public string Author { get; set; }
        public string Title { get; set; }
        public string Url { get; set; }
        public string Snippet { get; set; }
        public DateTime Updated { get; set; }
    }
}
