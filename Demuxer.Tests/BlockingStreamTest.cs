namespace Demuxer.Tests;

[TestClass]
public class BlockingStreamTest
{
    [TestMethod]
    public void ShouldReadWhatYouWrote()
    {
        var instanceUnderTest = new BlockingStream();
        instanceUnderTest.Write(new byte[] { 1, 2, 3 });
        instanceUnderTest.Write(new byte[] { 4 });
        instanceUnderTest.Read(2).ShouldBe(new byte[] { 1, 2 });
        instanceUnderTest.Read(2).ShouldBe(new byte[] { 3, 4 });
    }
}
