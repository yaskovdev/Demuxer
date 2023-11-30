﻿namespace Demuxer;

using System.Diagnostics.CodeAnalysis;
using System.Runtime.InteropServices;

[SuppressMessage("ReSharper", "UnassignedGetOnlyAutoProperty", Justification = "Set up in native code")]
public struct FrameMetadata
{
    public FrameType Type { get; }

    public long Timestamp { get; }
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
