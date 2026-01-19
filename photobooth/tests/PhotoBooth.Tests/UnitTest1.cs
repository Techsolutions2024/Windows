using PhotoBooth.Core.Imaging;
using SixLabors.ImageSharp.Drawing.Processing;
using SixLabors.ImageSharp.Processing;
using SixLabors.ImageSharp;

namespace PhotoBooth.Tests;

public class ImageRendererTests
{
    [Fact]
    public async Task RenderAsync_ReturnsJpegBytes()
    {
        var renderer = new ImageRenderer();
        var req = new RenderRequest(
            TemplateId: "basic-2x2",
            Photos: new[] { CreateTestPhotoBytes() },
            NowUtc: DateTimeOffset.UtcNow);

        var result = await renderer.RenderAsync(req, CancellationToken.None);

        Assert.NotNull(result);
        Assert.Equal("image/jpeg", result.ContentType);
        Assert.True(result.Bytes.Length > 1024);
    }

    private static byte[] CreateTestPhotoBytes()
    {
        using var img = new SixLabors.ImageSharp.Image<SixLabors.ImageSharp.PixelFormats.Rgba32>(800, 600);
        img.Mutate(ctx => ctx.Fill(SixLabors.ImageSharp.Color.Aquamarine));
        using var ms = new MemoryStream();
        img.SaveAsJpeg(ms);
        return ms.ToArray();
    }
}
