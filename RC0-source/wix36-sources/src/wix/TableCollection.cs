//-------------------------------------------------------------------------------------------------
// <copyright file="TableCollection.cs" company="Microsoft">
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
// Hash table collection for tables.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.Collections;

    /// <summary>
    /// Hash table collection for tables.
    /// </summary>
    public sealed class TableCollection : ICollection
    {
        private SortedList collection;

        /// <summary>
        /// Instantiate a new TableCollection class.
        /// </summary>
        public TableCollection()
        {
            this.collection = new SortedList();
        }

        /// <summary>
        /// Gets the number of items in the collection.
        /// </summary>
        /// <value>Number of items in collection.</value>
        public int Count
        {
            get { return this.collection.Count; }
        }

        /// <summary>
        /// Gets if the collection has been synchronized.
        /// </summary>
        /// <value>True if the collection has been synchronized.</value>
        public bool IsSynchronized
        {
            get { return this.collection.IsSynchronized; }
        }

        /// <summary>
        /// Gets the object used to synchronize the collection.
        /// </summary>
        /// <value>Oject used the synchronize the collection.</value>
        public object SyncRoot
        {
            get { return this.collection.SyncRoot; }
        }

        /// <summary>
        /// Gets a table by name.
        /// </summary>
        /// <param name="tableName">Name of table to locate.</param>
        public Table this[string tableName]
        {
            get { return (Table)this.collection[tableName]; }
        }

        /// <summary>
        /// Adds a table to the collection.
        /// </summary>
        /// <param name="table">Table to add to the collection.</param>
        /// <remarks>Indexes the table by name.</remarks>
        public void Add(Table table)
        {
            if (null == table)
            {
                throw new ArgumentNullException("table");
            }

            this.collection.Add(table.Name, table);
        }

        /// <summary>
        /// Remove a table from the collection.
        /// </summary>
        /// <param name="tableName">Table to remove from the collection.</param>
        public void Remove(string tableName)
        {
            if (null == tableName)
            {
                throw new ArgumentNullException("tableName");
            }

            this.collection.Remove(tableName);
        }

        /// <summary>
        /// Copies the collection into an array.
        /// </summary>
        /// <param name="array">Array to copy the collection into.</param>
        /// <param name="index">Index to start copying from.</param>
        public void CopyTo(System.Array array, int index)
        {
            this.collection.CopyTo(array, index);
        }

        /// <summary>
        /// Ensure this TableCollection contains a particular table.
        /// </summary>
        /// <param name="section">Section containing the new table.</param>
        /// <param name="tableDefinition">Definition of the table that should exist.</param>
        /// <returns>The table in this collection.</returns>
        public Table EnsureTable(Section section, TableDefinition tableDefinition)
        {
            Table table = this[tableDefinition.Name];

            if (null == table)
            {
                table = new Table(section, tableDefinition);
                this.Add(table);
            }

            return table;
        }

        /// <summary>
        /// Gets enumerator for the collection.
        /// </summary>
        /// <returns>Enumerator for the collection.</returns>
        public IEnumerator GetEnumerator()
        {
            return this.collection.Values.GetEnumerator();
        }
    }
}
