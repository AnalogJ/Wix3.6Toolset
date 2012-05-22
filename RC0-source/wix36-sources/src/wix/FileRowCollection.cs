//-------------------------------------------------------------------------------------------------
// <copyright file="FileRowCollection.cs" company="Microsoft">
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
// A collection of file rows.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.Collections;

    /// <summary>
    /// A collection of file rows.
    /// </summary>
    public sealed class FileRowCollection : ICollection
    {
        private Hashtable hashedCollection;
        private SortedList sortedCollection;

        /// <summary>
        /// Instantiate a new RowCollection class.
        /// </summary>
        public FileRowCollection()
        {
            this.hashedCollection = new Hashtable();
            this.sortedCollection = new SortedList();
        }

        /// <summary>
        /// Instantiate a new RowCollection class.
        /// </summary>
        public FileRowCollection(bool duplicateFileIds)
        {
            this.hashedCollection = duplicateFileIds ? null : new Hashtable();
            this.sortedCollection = new SortedList();
        }

        /// <summary>
        /// Gets the number of items in the collection.
        /// </summary>
        /// <value>Number of items in collection.</value>
        public int Count
        {
            get { return this.sortedCollection.Count; }
        }

        /// <summary>
        /// Gets if the collection has been synchronized.
        /// </summary>
        /// <value>True if the collection has been synchronized.</value>
        public bool IsSynchronized
        {
            get { return this.sortedCollection.IsSynchronized; }
        }

        /// <summary>
        /// Gets the object used to synchronize the collection.
        /// </summary>
        /// <value>Oject used the synchronize the collection.</value>
        public object SyncRoot
        {
            get { return this; }
        }

        /// <summary>
        /// Gets a file row by file id.
        /// </summary>
        /// <param name="fileId">File identifier of file row to locate.</param>
        public FileRow this[string fileId]
        {
            get 
            {
                if (this.hashedCollection == null)
                {
                    throw new InvalidOperationException(WixStrings.EXP_CannotIndexIntoFileRowCollection);
                }
                return (FileRow)this.hashedCollection[fileId];
            }
        }

        /// <summary>
        /// Adds a FileRow to the end of the collection.
        /// </summary>
        /// <param name="fileRow">The FileRow to be added to the end of the collection.</param>
        public void Add(FileRow fileRow)
        {
            if (this.hashedCollection != null)
            {
                this.hashedCollection.Add(fileRow.File, fileRow);
            }
            this.sortedCollection.Add(fileRow, null);
        }

        /// <summary>
        /// Adds the rows in the RowCollection to the end of this collection.
        /// </summary>
        /// <param name="rowCollection">The RowCollection to add.</param>
        public void AddRange(RowCollection rowCollection)
        {
            if (null != this.hashedCollection && this.hashedCollection.Count == 0)
            {
                this.hashedCollection = new Hashtable(rowCollection.Count);
            }

            foreach (FileRow fileRow in rowCollection)
            {
                this.Add(fileRow);
            }
        }

        /// <summary>
        /// Copies the collection into an array.
        /// </summary>
        /// <param name="array">Array to copy the collection into.</param>
        /// <param name="index">Index to start copying from.</param>
        public void CopyTo(System.Array array, int index)
        {
            this.sortedCollection.Keys.CopyTo(array, index);
        }

        /// <summary>
        /// Gets enumerator for the collection.
        /// </summary>
        /// <returns>Enumerator for the collection.</returns>
        public IEnumerator GetEnumerator()
        {
            return this.sortedCollection.Keys.GetEnumerator();
        }
    }
}
