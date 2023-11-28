namespace Demuxer;

public interface IDemuxer
{
    void WritePacket(byte[] packet);

    byte[] ReadFrame();
}
