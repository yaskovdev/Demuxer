namespace Demuxer;

using System.Runtime.InteropServices;

internal static class NativeDemuxerApi
{
    [DllImport("NativeDemuxer.dll", EntryPoint = "create_demuxer")]
    internal static extern IntPtr CreateDemuxer(Callback callback);

    [DllImport("NativeDemuxer.dll", EntryPoint = "read_frame")]
    internal static extern int ReadFrame(IntPtr demuxer, byte[] data, ref int isVideo);

    [DllImport("NativeDemuxer.dll", EntryPoint = "delete_demuxer")]
    internal static extern void DeleteDemuxer(IntPtr demuxer);
}
