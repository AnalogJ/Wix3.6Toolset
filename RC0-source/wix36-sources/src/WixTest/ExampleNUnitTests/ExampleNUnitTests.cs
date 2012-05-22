//-----------------------------------------------------------------------
// <copyright file="ExampleTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// <summary>Contains example tests for WixTest</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.IO;
    using System.Text;

    using Microsoft.Tools.WindowsInstallerXml.Test;
    using NUnit.Framework;

    /// <summary>
    /// Contains example tests for WixTest
    /// </summary>
    [TestFixture]
    public class ExampleTests
    {
        /// <summary>
        /// Perform some initialization before the tests are run
        /// </summary>
        [SetUp]
        public void AssemblyInitialize()
        {
            Settings.WixToolDirectory = @"%SystemDrive%\delivery\Target\wix\x86\debug\lang-neutral";
        }

        /// <summary>
        /// An example test that verifies an ICE violation is caught by smoke
        /// </summary>
        [Test]
        public void ExampleTest1()
        {
            string testDirectory = Environment.ExpandEnvironmentVariables(@"%WIX%\examples\test\extraice.0");

            // Build the MSI that will be run against Smoke.
            // Pass the -sval argument so that light does not fail due to ICE violations - it will be up to Smoke to catch them.
            string outputDirectory = Builder.BuildPackage(testDirectory, "product.wxs", "product.msi", false, null, "-sval");
            string databaseFile = Path.Combine(outputDirectory, "product.msi");

            // Run Smoke
            Smoke smoke = new Smoke(testDirectory);
            smoke.DatabaseFiles.Add(databaseFile);
            smoke.CubFiles.Add(@"%WIX%\examples\data\test.cub");
            smoke.ExpectedWixWarnings.Add(1076);
            smoke.ExpectedOutputStrings.Add("ICE1000");

            Result smokeResult = smoke.Run();
        }

        /// <summary>
        /// An example test that verifies an MSI is built correctly
        /// </summary>
        [Test]
        public void ExampleTest2()
        {
            string testDirectory = Environment.ExpandEnvironmentVariables(@"%WIX%\examples\test\componentfloating");

            string outputDirectory = Builder.BuildPackage(testDirectory, "product.wxs", "product.msi", false);

            string expectedMSI = Path.Combine(outputDirectory, "product.msi");
            string actualMSI = Path.Combine(outputDirectory, "product.msi");
            ArrayList differences = Verifier.CompareResults(expectedMSI, actualMSI);

            if (0 != differences.Count)
            {
                foreach (string difference in differences)
                {
                    Console.WriteLine(difference);
                }

                Assert.Fail("Expected output did not match actual output");
            }
        }

        /// <summary>
        /// An example test that queries the MSI for a particular value
        /// </summary>
        [Test]
        public void ExampleTest3()
        {
            string testDirectory = Environment.ExpandEnvironmentVariables(@"%WIX%\examples\test\componentfloating");

            string outputDirectory = Builder.BuildPackage(testDirectory, "product.wxs", "product.msi", false);

            string msi = Path.Combine(outputDirectory, "product.msi");
            string sql = "SELECT `Component_` FROM `File` WHERE `File` = 'TestFloatProductFile'";

            Assert.AreEqual("FeatureFloat", Verifier.Query(msi, sql));
        }
    }
}
