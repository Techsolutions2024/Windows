namespace PhotoBooth.Core.Sessions;

public sealed record SessionId(string Value)
{
    public static SessionId New() => new(Guid.NewGuid().ToString("n"));
}

public enum SessionStatus
{
    Created = 0,
    Capturing = 1,
    Ready = 2,
    Printed = 3,
    Error = 4,
}

public sealed record SessionPhoto(
    string FileName,
    string ContentType,
    long SizeBytes,
    DateTimeOffset CapturedAtUtc);

public sealed record SessionRender(
    string FileName,
    string ContentType,
    long SizeBytes,
    DateTimeOffset RenderedAtUtc);

public sealed record PrintJob(
    string FileName,
    DateTimeOffset PrintedAtUtc);

public sealed class PhotoSession
{
    public required SessionId Id { get; init; }
    public required DateTimeOffset CreatedAtUtc { get; init; }
    public SessionStatus Status { get; set; } = SessionStatus.Created;
    public string? TemplateId { get; set; }
    public List<SessionPhoto> Photos { get; } = new();
    public SessionRender? Render { get; set; }
    public PrintJob? PrintJob { get; set; }
    public string? ErrorMessage { get; set; }
}

