namespace Demuxer;

using System.Runtime.InteropServices;

internal static class NativeDemuxerApi
{
    [DllImport("NativeDemuxer.dll", EntryPoint = "create_demuxer")]
    internal static extern IntPtr CreateDemuxer();

    [DllImport("NativeDemuxer.dll", EntryPoint = "delete_demuxer")]
    internal static extern IntPtr DeleteDemuxer(IntPtr demuxer);
}
