//-------------------------------------------------------------------------------------------------
// <copyright file="Symbol.cs" company="Microsoft">
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
// Symbol representing a single row in a database.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.Text;

    /// <summary>
    /// Symbol representing a single row in a database.
    /// </summary>
    public sealed class Symbol
    {
        private Row row;

        /// <summary>
        /// Creates a symbol for a row.
        /// </summary>
        /// <param name="row">Row for the symbol</param>
        public Symbol(Row row)
        {
            this.row = row;
        }

        /// <summary>
        /// Gets the name of the symbol.
        /// </summary>
        /// <value>Name of the symbol.</value>
        public string Name
        {
            get
            {
                StringBuilder sb = new StringBuilder();

                sb.Append(this.row.TableDefinition.Name);
                sb.Append(":");
                sb.Append(this.row.GetPrimaryKey('/'));

                return sb.ToString();
            }
        }

        /// <summary>
        /// Gets the section for the symbol.
        /// </summary>
        /// <value>Section for the symbol.</value>
        public Section Section
        {
            get { return (null == this.row.Table) ? null : this.row.Table.Section; }
        }

        /// <summary>
        /// Gets the row for this symbol.
        /// </summary>
        /// <value>Row for this symbol.</value>
        public Row Row
        {
            get { return this.row; }
        }
    }
}
