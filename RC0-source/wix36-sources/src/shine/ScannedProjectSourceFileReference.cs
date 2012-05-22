using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Microsoft.Tools.WindowsInstallerXml
{
    public class ScannedProjectSourceFileReference : IComparable
    {
        public ScannedProject SourceProject { get; set; }

        public ScannedSourceFile TargetSourceFile { get; set; }

        public int CompareTo(object obj)
        {
            ScannedProjectSourceFileReference r = (ScannedProjectSourceFileReference)obj;
            int result = this.SourceProject.Key.CompareTo(r.SourceProject.Key);
            if (result == 0)
            {
                result = this.TargetSourceFile.Key.CompareTo(r.TargetSourceFile.Key);
            }

            return result;
        }
    }
}
