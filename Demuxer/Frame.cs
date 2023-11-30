namespace Demuxer;

public class Frame
{
    public FrameType Type { get; }

    public long Timestamp { get; }

    public byte[] Data { get; }

    public Frame(FrameType type, long timestamp, byte[] data)
    {
        Type = type;
        Timestamp = timestamp;
        Data = data;
    }
}
