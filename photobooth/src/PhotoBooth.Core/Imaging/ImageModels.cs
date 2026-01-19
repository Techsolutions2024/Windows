namespace PhotoBooth.Core.Imaging;

public sealed record RenderRequest(
    string? TemplateId,
    IReadOnlyList<byte[]> Photos,
    DateTimeOffset NowUtc);

public sealed record RenderResult(
    string FileName,
    string ContentType,
    byte[] Bytes,
    DateTimeOffset RenderedAtUtc);

