namespace Demuxer;

internal static class Program
{
    public static void Main()
    {
        using var demuxer = new Demuxer();
        var bytes = File.ReadAllBytes(@"c:\dev\experiment3\capture.webm");
        demuxer.WritePacket(bytes);
        demuxer.ReadFrame();
    }
}
