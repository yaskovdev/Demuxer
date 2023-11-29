namespace Demuxer;

public class Frame
{
    public FrameType Type { get; }

    public byte[] Data { get; }

    public Frame(FrameType type, byte[] data)
    {
        Type = type;
        Data = data;
    }
}
