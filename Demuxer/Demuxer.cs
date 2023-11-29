namespace Demuxer;

using System.Runtime.InteropServices;

public class Demuxer : IDemuxer, IDisposable
{
    // ReSharper disable once PrivateFieldCanBeConvertedToLocalVariable
    private readonly Callback _callback;
    private readonly byte[] _source;
    private readonly IntPtr _demuxer;
    private int _offset;

    public Demuxer(byte[] source)
    {
        _callback = Callback;
        _source = source;
        _demuxer = NativeDemuxerApi.CreateDemuxer(_callback);
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
        var numberOfBytesToCopy = Math.Min(_source.Length - _offset, size);
        Console.WriteLine($"Callback called with buffer {buffer} and size {size}");
        if (numberOfBytesToCopy == 0)
        {
            return -1;
        }
        Marshal.Copy(_source, _offset, buffer, numberOfBytesToCopy);
        _offset += numberOfBytesToCopy;
        return numberOfBytesToCopy;
    }
}
