namespace Demuxer;

public class Demuxer : IDemuxer, IDisposable
{
    private readonly IntPtr _demuxer = NativeDemuxerApi.CreateDemuxer();

    public void WritePacket(byte[] packet)
    {
        NativeDemuxerApi.WritePacket(_demuxer, packet, packet.Length);
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
}
