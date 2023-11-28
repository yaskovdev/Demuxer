namespace Demuxer;

internal static class Program
{
    public static void Main()
    {
        using var demuxer = new Demuxer();
        demuxer.ReadFrame();
    }
}
