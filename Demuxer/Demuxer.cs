namespace Demuxer;

public class Demuxer : IDemuxer, IDisposable
{
    private readonly IntPtr _demuxer = NativeDemuxerApi.CreateDemuxer();

    public void WriteMedia(byte[] data)
    {
    }

    public byte[] ReadFrame() => Array.Empty<byte>();

    public void Dispose()
    {
        NativeDemuxerApi.DeleteDemuxer(_demuxer);
    }
}
