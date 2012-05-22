//---------------------------------------------------------------------
// <copyright file="GroupIconInfo.cs" company="Microsoft">
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
// <summary>
// Part of the Deployment Tools Foundation project.
// </summary>
//---------------------------------------------------------------------

namespace Microsoft.Deployment.Resources
{
    using System;
    using System.IO;
    using System.Text;
    using System.Reflection;
    using System.Collections;
    using System.Globalization;
    using System.Diagnostics.CodeAnalysis;

    internal enum GroupIconType
    {
        Unknown,
        Icon,
        Cursor,
    }

    internal struct GroupIconDirectoryInfo
    {
        public byte width;
        public byte height;
        public byte colors;
        public byte reserved;
        public ushort planes;
        public ushort bitsPerPixel;
        public uint imageSize;
        public uint imageOffset; // only valid when icon group is read from .ico file.
        public ushort imageIndex; // only valid when icon group is read from PE resource.
    }

    internal class GroupIconInfo
    {
        private ushort reserved;
        private GroupIconType type;
        private GroupIconDirectoryInfo[] images;

        public GroupIconInfo()
        {
            this.images = new GroupIconDirectoryInfo[0];
        }

        public GroupIconDirectoryInfo[] DirectoryInfo { get { return this.images; } }

        public void ReadFromFile(Stream stream)
        {
            BinaryReader reader = new BinaryReader(stream);
            this.Read(reader, true);
        }

        public void ReadFromResource(byte[] data)
        {
            using (BinaryReader reader = new BinaryReader(new MemoryStream(data, false)))
            {
                this.Read(reader, false);
            }
        }

        public byte[] GetResourceData()
        {
            byte[] data = null;

            using (MemoryStream stream = new MemoryStream())
            {
                BinaryWriter writer = new BinaryWriter(stream);
                writer.Write(this.reserved);
                writer.Write((ushort)this.type);
                writer.Write((ushort)this.images.Length);
                for (int i = 0; i < this.images.Length; ++i)
                {
                    writer.Write(this.images[i].width);
                    writer.Write(this.images[i].height);
                    writer.Write(this.images[i].colors);
                    writer.Write(this.images[i].reserved);
                    writer.Write(this.images[i].planes);
                    writer.Write(this.images[i].bitsPerPixel);
                    writer.Write(this.images[i].imageSize);
                    writer.Write(this.images[i].imageIndex);
                }

                data = new byte[stream.Length];
                stream.Seek(0, SeekOrigin.Begin);
                stream.Read(data, 0, data.Length);
            }

            return data;
        }

        private void Read(BinaryReader reader, bool readFromFile)
        {
            this.reserved = reader.ReadUInt16();
            this.type = (GroupIconType)reader.ReadUInt16();

            int imageCount = reader.ReadUInt16();
            this.images = new GroupIconDirectoryInfo[imageCount];
            for (int i = 0; i < imageCount; ++i)
            {
                this.images[i].width = reader.ReadByte();
                this.images[i].height = reader.ReadByte();
                this.images[i].colors = reader.ReadByte();
                this.images[i].reserved = reader.ReadByte();
                this.images[i].planes = reader.ReadUInt16();
                this.images[i].bitsPerPixel = reader.ReadUInt16();
                this.images[i].imageSize = reader.ReadUInt32();
                if (readFromFile)
                {
                    this.images[i].imageOffset = reader.ReadUInt32();
                    this.images[i].imageIndex = (ushort)(i + 1);
                }
                else
                {
                    this.images[i].imageIndex = reader.ReadUInt16();
                }
            }
        }

    }
}
