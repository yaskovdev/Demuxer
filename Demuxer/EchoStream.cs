namespace Demuxer;

using System.Collections.Concurrent;

public class EchoStream
{
    private readonly ConcurrentQueue<byte[]> _buffers = new ConcurrentQueue<byte[]>();
    private readonly ManualResetEvent _hasBuffers = new ManualResetEvent(false);

    public void Write(byte[] buffer)
    {
        _buffers.Enqueue(buffer);
        _hasBuffers.Set();
    }
}
