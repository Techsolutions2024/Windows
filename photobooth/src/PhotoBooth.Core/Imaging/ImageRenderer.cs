using SixLabors.Fonts;
using SixLabors.ImageSharp;
using SixLabors.ImageSharp.Drawing.Processing;
using SixLabors.ImageSharp.Formats.Jpeg;
using SixLabors.ImageSharp.PixelFormats;
using SixLabors.ImageSharp.Processing;

namespace PhotoBooth.Core.Imaging;

/// <summary>
/// Simple renderer producing a 2x2 strip with a header/footer.
/// Can be extended to support templates/overlays/QR.
/// </summary>
public sealed class ImageRenderer : IImageRenderer
{
    private static readonly JpegEncoder JpegEncoder = new() { Quality = 90 };

    public async Task<RenderResult> RenderAsync(RenderRequest request, CancellationToken cancellationToken)
    {
        var now = request.NowUtc;
        var photos = request.Photos;
        if (photos.Count == 0)
        {
            throw new InvalidOperationException("No photos to render");
        }

        // Output canvas approximates 4x6" @ 300DPI -> 1200x1800.
        const int width = 1200;
        const int height = 1800;
        const int padding = 40;
        const int headerHeight = 160;
        const int footerHeight = 140;

        using var canvas = new Image<Rgba32>(width, height, Color.White);

        canvas.Mutate(ctx =>
        {
            ctx.Fill(Color.White);

            // Header
            ctx.Fill(Color.Black, new Rectangle(0, 0, width, headerHeight));
            var titleFont = SystemFonts.CreateFont("DejaVu Sans", 56, FontStyle.Bold);
            ctx.DrawText("PHOTOBOOTH", titleFont, Color.White, new PointF(padding, 40));

            // Footer
            ctx.Fill(Color.Black, new Rectangle(0, height - footerHeight, width, footerHeight));
            var footerFont = SystemFonts.CreateFont("DejaVu Sans", 32);
            ctx.DrawText($"{now:yyyy-MM-dd HH:mm} UTC", footerFont, Color.White, new PointF(padding, height - footerHeight + 50));

            // Photo grid
        });

        var gridTop = headerHeight + padding;
        var gridBottom = height - footerHeight - padding;
        var gridHeight = gridBottom - gridTop;
        var cellWidth = (width - padding * 3) / 2;
        var cellHeight = (gridHeight - padding) / 2;

        for (var i = 0; i < 4; i++)
        {
            var photoIndex = i % photos.Count;
            await using var photoStream = new MemoryStream(photos[photoIndex], writable: false);
            using var photo = await Image.LoadAsync(photoStream, cancellationToken);
            photo.Mutate(ctx => ctx.AutoOrient());

            var resized = photo.Clone(ctx =>
                ctx.Resize(new ResizeOptions
                {
                    Size = new Size(cellWidth, cellHeight),
                    Mode = ResizeMode.Crop,
                    Position = AnchorPositionMode.Center,
                }));

            var col = i % 2;
            var row = i / 2;
            var x = padding + col * (cellWidth + padding);
            var y = gridTop + row * (cellHeight + padding);

            canvas.Mutate(ctx => ctx.DrawImage(resized, new Point(x, y), 1f));
            resized.Dispose();
        }

        // Apply a mild contrast/saturation to pop.
        canvas.Mutate(ctx =>
        {
            ctx.Contrast(1.05f);
            ctx.Saturate(1.05f);
        });

        await using var ms = new MemoryStream();
        await canvas.SaveAsJpegAsync(ms, JpegEncoder, cancellationToken);
        var bytes = ms.ToArray();

        return new RenderResult(
            FileName: $"render-{now:yyyyMMdd-HHmmss}.jpg",
            ContentType: "image/jpeg",
            Bytes: bytes,
            RenderedAtUtc: now);
    }
}
