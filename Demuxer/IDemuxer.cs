namespace Demuxer;

public interface IDemuxer
{
    void WriteMedia(byte[] data);

    byte[] ReadFrame();
}
