namespace Demuxer;

public class Frame
{
    public FrameType Type { get; }

    public int Timestamp { get; }

    public byte[] Data { get; }

    public Frame(FrameType type, int timestamp, byte[] data)
    {
        Type = type;
        Timestamp = timestamp;
        Data = data;
    }
}
