namespace DemuxerRunner;

using Demuxer;

public class BrowserSimulator
{
    private readonly IBlockingBuffer _buffer;

    public BrowserSimulator(IBlockingBuffer buffer)
    {
        _buffer = buffer;
    }

    public void StartProducingMedia()
    {
        for (var i = 0; i < 43; i++)
        {
            var bytes = File.ReadAllBytes(@$"c:\dev\experiment3\chunks\chunk_{i:D7}");
            _buffer.Write(Encode(bytes));
            Thread.Sleep(100);
        }
    }

    private static string Encode(byte[] source) => new(source.Select(it => (char)it).ToArray());
}
