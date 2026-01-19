using PhotoBooth.Core.Cameras;
using PhotoBooth.Core.Imaging;
using PhotoBooth.Core.Printing;
using PhotoBooth.Core.Sessions;

namespace PhotoBooth.Core;

public sealed class PhotoBoothService
{
    private readonly SessionStore _store;
    private readonly ICamera _camera;
    private readonly IImageRenderer _renderer;
    private readonly IPrinter _printer;
    private readonly TimeProvider _time;

    public PhotoBoothService(
        SessionStore store,
        ICamera camera,
        IImageRenderer renderer,
        IPrinter printer,
        TimeProvider time)
    {
        _store = store;
        _camera = camera;
        _renderer = renderer;
        _printer = printer;
        _time = time;
    }

    public PhotoSession CreateSession(string? templateId)
        => _store.Create(_time.GetUtcNow(), templateId);

    public PhotoSession GetSession(SessionId id) => _store.GetRequired(id);

    public IEnumerable<PhotoSession> ListSessions() => _store.List();

    public async Task<SessionPhoto> CaptureAsync(SessionId id, CancellationToken cancellationToken)
    {
        var session = _store.GetRequired(id);
        session.Status = SessionStatus.Capturing;

        try
        {
            var result = await _camera.CaptureAsync(cancellationToken);
            var photo = new SessionPhoto(result.FileName, result.ContentType, result.Bytes.LongLength, result.CapturedAtUtc);
            session.Photos.Add(photo);
            session.Status = SessionStatus.Ready;

            // Persist bytes in a predictable folder under temp.
            var dir = GetSessionDir(id);
            Directory.CreateDirectory(dir);
            var path = Path.Combine(dir, result.FileName);
            await File.WriteAllBytesAsync(path, result.Bytes, cancellationToken);

            return photo;
        }
        catch (Exception ex)
        {
            session.Status = SessionStatus.Error;
            session.ErrorMessage = ex.Message;
            throw;
        }
    }

    public async Task<SessionRender> RenderAsync(SessionId id, CancellationToken cancellationToken)
    {
        var session = _store.GetRequired(id);
        if (session.Photos.Count == 0)
        {
            throw new InvalidOperationException("Session has no photos");
        }

        var dir = GetSessionDir(id);
        Directory.CreateDirectory(dir);
        var photoBytes = session.Photos
            .Select(p => File.ReadAllBytes(Path.Combine(dir, p.FileName)))
            .ToArray();

        var request = new RenderRequest(session.TemplateId, photoBytes, _time.GetUtcNow());
        var result = await _renderer.RenderAsync(request, cancellationToken);

        var renderPath = Path.Combine(dir, result.FileName);
        await File.WriteAllBytesAsync(renderPath, result.Bytes, cancellationToken);

        var render = new SessionRender(result.FileName, result.ContentType, result.Bytes.LongLength, result.RenderedAtUtc);
        session.Render = render;
        return render;
    }

    public async Task<PrintJob> PrintAsync(SessionId id, CancellationToken cancellationToken)
    {
        var session = _store.GetRequired(id);
        if (session.Render is null)
        {
            throw new InvalidOperationException("Session has no rendered output");
        }

        var dir = GetSessionDir(id);
        var renderPath = Path.Combine(dir, session.Render.FileName);
        var bytes = await File.ReadAllBytesAsync(renderPath, cancellationToken);

        var printResult = await _printer.PrintAsync(
            new PrintRequest(session.Render.FileName, session.Render.ContentType, bytes, _time.GetUtcNow()),
            cancellationToken);

        var printJob = new PrintJob(printResult.FileName, printResult.PrintedAtUtc);
        session.PrintJob = printJob;
        session.Status = SessionStatus.Printed;
        return printJob;
    }

    public static string GetSessionDir(SessionId id)
        => Path.Combine(Path.GetTempPath(), "photobooth", "sessions", id.Value);
}

