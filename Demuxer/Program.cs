namespace Demuxer;

internal static class Program
{
    public static void Main()
    {
        using var demuxer = new Demuxer();
        var bytes = File.ReadAllBytes(@"c:\dev\experiment3\capture.webm");
        demuxer.WritePacket(bytes);
        while (true)
        {
            var frame = demuxer.ReadFrame();
            if (frame.Data.Length == 0)
            {
                break;
            }
            Console.WriteLine($"Received {frame.Type} frame with {frame.Data.Length} bytes");
        }
    }
}
