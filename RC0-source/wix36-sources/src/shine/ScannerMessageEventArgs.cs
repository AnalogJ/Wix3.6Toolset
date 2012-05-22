using System;

namespace Microsoft.Tools.WindowsInstallerXml
{
    public enum ScannerMessageType
    {
        Normal,
        Verbose,
        Warning,
        Error,
    }

    public delegate void ScannerMessageEventHandler(object sender, ScannerMessageEventArgs e);

    public class ScannerMessageEventArgs : EventArgs
    {
        public string Message { get; set; }

        public ScannerMessageType Type { get; set; }
    }
}
