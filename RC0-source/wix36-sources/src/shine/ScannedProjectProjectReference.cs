using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Microsoft.Tools.WindowsInstallerXml
{
    public class ScannedProjectProjectReference : IComparable
    {
        public ScannedProject SourceProject { get; set; }

        public ScannedProject TargetProject { get; set; }

        public int CompareTo(object obj)
        {
            ScannedProjectProjectReference r = (ScannedProjectProjectReference)obj;
            int result = this.SourceProject.Key.CompareTo(r.SourceProject.Key);
            if (result == 0)
            {
                result = this.TargetProject.Key.CompareTo(r.TargetProject.Key);
            }

            return result;
        }
    }
}
