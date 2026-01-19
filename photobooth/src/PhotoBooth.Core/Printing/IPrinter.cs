namespace PhotoBooth.Core.Printing;

public sealed record PrintRequest(
    string FileName,
    string ContentType,
    byte[] Bytes,
    DateTimeOffset NowUtc);

public sealed record PrintResult(
    string FileName,
    DateTimeOffset PrintedAtUtc);

public interface IPrinter
{
    Task<PrintResult> PrintAsync(PrintRequest request, CancellationToken cancellationToken);
}

