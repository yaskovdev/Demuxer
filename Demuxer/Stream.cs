namespace Demuxer;

public class Stream
{
    private const int MaxSize = 512 * 1024 * 1024;

    private readonly object _lock = new();
    private readonly byte[] _buffer = new byte[MaxSize];
    private int _size;
    private int _offset;

    public void Write(byte[] bytes)
    {
        lock (_lock)
        {
            if (_size + bytes.Length > MaxSize)
            {
                Monitor.Wait(_lock);
            }
            Array.Copy(_buffer, _offset, _buffer, 0, _size);
            _offset = 0;
            Array.Copy(bytes, 0, _buffer, _size, bytes.Length);
            _size += bytes.Length;
            Monitor.PulseAll(_lock);
        }
    }

    public byte[] Read(int size)
    {
        lock (_lock)
        {
            if (_size - _offset == 0)
            {
                Monitor.Wait(_lock); // TODO: do not wait if disposed
            }
            var numberOfBytesToCopy = Math.Min(_size - _offset, size);
            var result = new byte[numberOfBytesToCopy];
            Array.Copy(_buffer, _offset, result, 0, numberOfBytesToCopy);
            _offset += numberOfBytesToCopy;
            Monitor.PulseAll(_lock);
            // Console.WriteLine($"Remaining buffer size is {_size - _offset}");
            return result;
        }
    }
}
