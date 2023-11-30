namespace Demuxer;

using System.Runtime.InteropServices;

// ReSharper disable once UnassignedGetOnlyAutoProperty
[StructLayout(LayoutKind.Sequential)]
public struct FrameMetadata
{
    public FrameType type;

    public int timestamp;
}

internal static class NativeDemuxerApi
{
    [DllImport("NativeDemuxer.dll", EntryPoint = "create_demuxer")]
    internal static extern IntPtr CreateDemuxer(Callback callback);

    [DllImport("NativeDemuxer.dll", EntryPoint = "read_frame")]
    internal static extern int ReadFrame(IntPtr demuxer, byte[] data, ref FrameMetadata metadata);

    [DllImport("NativeDemuxer.dll", EntryPoint = "delete_demuxer")]
    internal static extern void DeleteDemuxer(IntPtr demuxer);
}
