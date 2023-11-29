namespace Demuxer;

internal static class Program
{
    public static async Task Main()
    {
        var bytes = await File.ReadAllBytesAsync(@"c:\dev\experiment3\capture.webm");
        using var demuxer = new Demuxer(bytes);
        await using var fileStream = File.Create(@"c:\dev\experiment3\capture.video");
        demuxer.WritePacket(bytes);
        while (true)
        {
            var frame = demuxer.ReadFrame();
            if (frame.Data.Length == 0)
            {
                break;
            }
            if (frame.Type == FrameType.Video)
            {
                await fileStream.WriteAsync(frame.Data);
            }
        }
    }
}
