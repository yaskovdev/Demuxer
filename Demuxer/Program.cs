namespace Demuxer;

internal static class Program
{
    public static async Task Main()
    {
        using var demuxer = new Demuxer();
        var bytes = await File.ReadAllBytesAsync(@"c:\dev\experiment3\capture.webm");
        await using var fileStream = File.Create(@"c:\dev\experiment3\capture.video");
        demuxer.WritePacket(bytes);
        while (true)
        {
            var frame = demuxer.ReadFrame();
            if (frame.Data.Length == 0)
            {
                break;
            }
            Console.WriteLine($"Received {frame.Type} frame with {frame.Data.Length} bytes");
            if (frame.Type == FrameType.Video)
            {
                await fileStream.WriteAsync(frame.Data);
            }
        }
    }
}
