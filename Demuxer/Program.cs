namespace Demuxer;

using System.Runtime.InteropServices;

internal static class Program
{
    public static void Main()
    {
        Console.WriteLine("With IntPtr:");
        const byte resultingFrameLength = 6;
        var resultingFrame = new byte[resultingFrameLength];
        var resultingFrameHandle = GCHandle.Alloc(resultingFrame, GCHandleType.Pinned);
        var resultingFramePtr = resultingFrameHandle.AddrOfPinnedObject();
        try
        {
            var chromaPtr = resultingFramePtr;
            var lumaPtr = resultingFramePtr + resultingFrameLength / 2;
            var status = NativeDemuxerApi.FillChromaAndLuma(chromaPtr, lumaPtr, resultingFrameLength / 2);
            Console.WriteLine($"Finished, status is {status}");
            DisplayFrame(resultingFramePtr, resultingFrameLength);
        }
        finally
        {
            resultingFrameHandle.Free();
        }
    }

    private static void DisplayFrame(IntPtr framePtr, int length)
    {
        Console.Write("Going to display frame: ");
        // Just printing the frame bytes for simplicity:
        for (var i = 0; i < length; ++i)
        {
            if (i > 0) Console.Write(", ");
            Console.Write(Marshal.ReadByte(framePtr + i));
        }
        Console.WriteLine();
    }
}
