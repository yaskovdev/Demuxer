namespace Demuxer;

using System.Runtime.InteropServices;

public class Demuxer : IDemuxer, IDisposable
{
    private readonly Stream _stream = new Stream();

    // ReSharper disable once PrivateFieldCanBeConvertedToLocalVariable
    private readonly Callback _callback;
    private readonly IntPtr _demuxer;

    public Demuxer()
    {
        _callback = Callback;
        _demuxer = NativeDemuxerApi.CreateDemuxer(_callback);
    }

    public void WritePacket(byte[] packet)
    {
        _stream.Write(packet);
    }

    /// <summary>
    /// If the returned array is empty, then there is not enough source data. Write more packets.
    /// </summary>
    public Frame ReadFrame()
    {
        var data = new byte[1920 * 1080 * 3 / 2]; // TODO: check the size and do not hardcode
        var isVideo = 0;
        var status = NativeDemuxerApi.ReadFrame(_demuxer, data, ref isVideo);
        return status == 0 ? new Frame(isVideo == 0 ? FrameType.Audio : FrameType.Video, data) : new Frame(FrameType.Audio, Array.Empty<byte>());
    }

    public void Dispose()
    {
        NativeDemuxerApi.DeleteDemuxer(_demuxer);
    }

    private int Callback(IntPtr buffer, int size)
    {
        var bytes = _stream.Read(size);
        Marshal.Copy(bytes, 0, buffer, bytes.Length);
        return bytes.Length;
    }
}
