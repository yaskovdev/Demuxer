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
    public byte[] ReadFrame()
    {
        var status = NativeDemuxerApi.ReadFrame(_demuxer);
        return status == 0 ? new byte[] { 0 } : Array.Empty<byte>(); // TODO: return real frame if status is 0
    }

    public void Dispose()
    {
        NativeDemuxerApi.DeleteDemuxer(_demuxer);
    }
}
