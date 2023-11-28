namespace Demuxer;

public class Demuxer : IDemuxer, IDisposable
{
    private readonly IntPtr _demuxer = NativeDemuxerApi.CreateDemuxer();

    public void WritePacket(byte[] packet)
    {
    }

    public byte[] ReadFrame()
    {
        NativeDemuxerApi.ReadFrame(_demuxer);
        return Array.Empty<byte>();
    }

    public void Dispose()
    {
        NativeDemuxerApi.DeleteDemuxer(_demuxer);
    }
}
