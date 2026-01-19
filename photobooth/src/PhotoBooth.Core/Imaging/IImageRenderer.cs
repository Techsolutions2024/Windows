namespace PhotoBooth.Core.Imaging;

public interface IImageRenderer
{
    Task<RenderResult> RenderAsync(RenderRequest request, CancellationToken cancellationToken);
}

