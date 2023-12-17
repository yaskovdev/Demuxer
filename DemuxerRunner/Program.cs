namespace DemuxerRunner;

using System.Runtime.InteropServices;
using Demuxer;

internal static class Program
{
    public static async Task Main()
    {
        var buffer = new BlockingCircularBuffer(512 * 1024 * 1024);
        using var demuxer = new Demuxer(buffer);
        var producer = new Thread(() =>
        {
            var simulator = new BrowserSimulator(buffer);
            simulator.StartProducingMedia();
        });
        producer.Start();

        await using var outputVideoStream = File.Create(@"c:\dev\experiment3\capture.video");
        await using var outputAudioStream = File.Create(@"c:\dev\experiment3\capture.audio");
        while (true)
        {
            var frame = demuxer.ReadFrame();
            if (frame.Data == IntPtr.Zero)
            {
                break;
            }
            Console.WriteLine($"Extracted frame of type {frame.Type} with size {frame.Size} and timestamp {frame.Timestamp}");
            var data = new byte[frame.Size];
            Marshal.Copy(frame.Data, data, 0, data.Length);
            await PickStreamFor(frame, outputVideoStream, outputAudioStream).WriteAsync(data);
        }
        producer.Join();
    }

    private static Stream PickStreamFor(AbstractFrame frame, Stream videoStream, Stream audioStream) =>
        frame.Type switch
        {
            FrameType.Video => videoStream,
            FrameType.Audio => audioStream,
            _ => Stream.Null
        };
}
