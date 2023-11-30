namespace Demuxer;

internal static class Program
{
    public static async Task Main()
    {
        using var demuxer = new Demuxer();
        var producer = new Thread(() =>
        {
            var simulator = new BrowserSimulator(demuxer);
            simulator.StartProducingMedia();
        });
        producer.Start();

        await using var fileStream = File.Create(@"c:\dev\experiment3\capture.video");
        while (true)
        {
            var frame = demuxer.ReadFrame();
            if (frame.Data.Length == 0)
            {
                break;
            }
            Console.WriteLine($"Extracted frame of type {frame.Type} with timestamp {frame.Timestamp}");
            if (frame.Type == FrameType.Video)
            {
                await fileStream.WriteAsync(frame.Data);
            }
        }
        producer.Join();
    }
}
