namespace PhotoBooth.Core.Cameras;

public sealed record CaptureResult(
    string FileName,
    string ContentType,
    byte[] Bytes,
    DateTimeOffset CapturedAtUtc);

public interface ICamera
{
    Task<CaptureResult> CaptureAsync(CancellationToken cancellationToken);
}

