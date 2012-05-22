//-------------------------------------------------------------------------------------------------
// <copyright file="Constants.cs" company="Microsoft">
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
// Definitions shared across Lux projects.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Lux
{
    /// <summary>
    /// Constants shared across Lux projects.
    /// </summary>
    public class Constants
    {
        public const string LuxNamespace = "http://schemas.microsoft.com/wix/2009/Lux";
        public const string LuxCustomActionName = "WixRunImmediateUnitTests";
        public const string LuxMutationRunningProperty = "WIXLUX_RUNNING_MUTATION";
        public const string LuxTableName = "WixUnitTest";

        /// <summary>
        /// Error table ids.
        /// </summary>
        public const int TestIdMinimumSuccess = 27110;
        public const int TestPassedExpressionTrue = 27110;
        public const int TestPassedPropertyValueMatch = 27111;
        public const int TestSkipped = 27112;
        public const int TestIdMaximumSuccess = 27112;

        public const int TestIdMinimumFailure = 27120;
        public const int TestUnknownResult = 27120;
        public const int TestUnknownOperation = 27121;
        public const int TestNotCreated = 27122;
        public const int TestFailedExpressionFalse = 27123;
        public const int TestFailedExpressionSyntaxError = 27124;
        public const int TestFailedPropertyValueMismatch = 27125;
        public const int TestFailedIndexOutOfBounds = 27126;
        public const int TestFailedExpectedEvenNameValueContent = 27127;
        public const int TestFailedIndexUnknown = 27128;
        public const int TestIdMaximumFailure = 27128;
    }
}