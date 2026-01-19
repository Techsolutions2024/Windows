using SixLabors.Fonts;
using SixLabors.ImageSharp;
using SixLabors.ImageSharp.PixelFormats;
using SixLabors.ImageSharp.Drawing.Processing;
using SixLabors.ImageSharp.Processing;

namespace PhotoBooth.Core.Cameras;

public sealed class MockCamera : ICamera
{
    public async Task<CaptureResult> CaptureAsync(CancellationToken cancellationToken)
    {
        // Simulate capture latency.
        await Task.Delay(250, cancellationToken);

        var capturedAt = DateTimeOffset.UtcNow;
        using var image = new Image<Rgba32>(1920, 1280);
        image.Mutate(ctx =>
        {
            ctx.Fill(new LinearGradientBrush(
                new PointF(0, 0),
                new PointF(image.Width, image.Height),
                GradientRepetitionMode.None,
                new ColorStop(0, Color.DeepSkyBlue),
                new ColorStop(1, Color.HotPink)));

            var font = SystemFonts.CreateFont("DejaVu Sans", 48);
            ctx.DrawText($"Mock Capture\n{capturedAt:O}", font, Color.White, new PointF(60, 60));
        });

        await using var ms = new MemoryStream();
        await image.SaveAsJpegAsync(ms, cancellationToken);

        return new CaptureResult(
            FileName: $"capture-{capturedAt:yyyyMMdd-HHmmss}.jpg",
            ContentType: "image/jpeg",
            Bytes: ms.ToArray(),
            CapturedAtUtc: capturedAt);
    }
}
