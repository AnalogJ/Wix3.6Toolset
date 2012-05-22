//-------------------------------------------------------------------------------------------------
// <copyright file="SummaryInfoRowCollection.cs" company="Microsoft">
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
// Array collection of rows.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.Collections;
    using System.Collections.ObjectModel;

    /// <summary>
    /// Indexed container class for summary information rows.
    /// </summary>
    internal sealed class SummaryInfoRowCollection : KeyedCollection<int, Row>
    {
        /// <summary>
        /// Creates the keyed collection from existing rows in a table.
        /// </summary>
        /// <param name="table">The summary information table to index.</param>
        internal SummaryInfoRowCollection(Table table)
        {
            if (0 != String.CompareOrdinal("_SummaryInformation", table.Name))
            {
                string message = string.Format(WixStrings.EXP_UnsupportedTable, table.Name);
                throw new ArgumentException(message, "table");
            }

            foreach (Row row in table.Rows)
            {
                this.Add(row);
            }
        }

        /// <summary>
        /// Gets the summary property ID for the <paramref name="row"/>.
        /// </summary>
        /// <param name="row">The row to index.</param>
        /// <returns>The summary property ID for the <paramref name="row"/>.
        protected override int GetKeyForItem(Row row)
        {
            return (int)row[0];
        }
    }
}
