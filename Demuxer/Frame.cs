namespace Demuxer;

public class Frame
{
    public FrameType Type { get; }

    public long Timestamp { get; }

    public ArraySegment<byte> Data { get; }

    public Frame(FrameType type, ulong size, long timestamp, byte[] data)
    {
        Type = type;
        Timestamp = timestamp;
        Data = new ArraySegment<byte>(data, 0, (int)size);
    }
}
