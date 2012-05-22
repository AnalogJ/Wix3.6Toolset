using System;

namespace Microsoft.Tools.WindowsInstallerXml
{
    public class ScannedSymbolSymbolReference : IComparable
    {
        public ScannedSymbol SourceSymbol { get; set; }

        public ScannedSymbol TargetSymbol { get; set; }

        public int CompareTo(object obj)
        {
            ScannedSymbolSymbolReference r = (ScannedSymbolSymbolReference)obj;
            int result = this.SourceSymbol.Key.CompareTo(r.SourceSymbol.Key);
            if (result == 0)
            {
                result = this.TargetSymbol.Key.CompareTo(r.TargetSymbol.Key);
            }

            return result;
        }
    }
}
