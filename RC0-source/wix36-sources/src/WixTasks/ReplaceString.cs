//-------------------------------------------------------------------------------------------------
// <copyright file="ReplaceString.cs" company="Microsoft">
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
// Contains the ReplaceString class.
// </summary>
//-------------------------------------------------------------------------------------------------
namespace Microsoft.Tools.WindowsInstallerXml.Build.Tasks
{
    using System;
    using Microsoft.Build.Framework;
    using Microsoft.Build.Utilities;

    /// <summary>
    /// Replaces occurances of OldValues with NewValues in String.
    /// </summary>
    public class ReplaceString : Task
    {
        /// <summary>
        /// Text to operate on.
        /// </summary>
        [Output]
        [Required]
        public string Text { get; set; }

        /// <summary>
        /// List of old values to replace.
        /// </summary>
        [Required]
        public string OldValue { get; set; }

        /// <summary>
        /// List of new values to replace old values with.  If not specified, occurances of OldValue will be removed.
        /// </summary>
        public string NewValue { get; set; }

        /// <summary>
        /// Does the string replacement.
        /// </summary>
        /// <returns></returns>
        public override bool Execute()
        {
            if (String.IsNullOrEmpty(this.Text))
            {
                return true;
            }

            if (String.IsNullOrEmpty(this.OldValue))
            {
                Log.LogError("OldValue must be specified");
                return false;
            }

            this.Text = this.Text.Replace(this.OldValue, this.NewValue);

            return true;
        }
    }
}
