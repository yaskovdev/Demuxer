namespace Demuxer;

using System.Runtime.InteropServices;

internal static class NativeDemuxerApi
{
    [DllImport("NativeDemuxer.dll", EntryPoint = "fill_chroma_and_luma")]
    internal static extern int FillChromaAndLuma(IntPtr chromaPtr, IntPtr lumaPtr, byte numberOfBytesInEachPlane);
}
