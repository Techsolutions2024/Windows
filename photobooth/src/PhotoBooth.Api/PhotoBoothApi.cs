using PhotoBooth.Core;
using PhotoBooth.Core.Sessions;

namespace PhotoBooth.Api;

public static class PhotoBoothApi
{
    public static void MapPhotoBoothApi(this IEndpointRouteBuilder app)
    {
        var api = app.MapGroup("/api");

        api.MapPost("/sessions", (CreateSessionRequest req, PhotoBoothService booth) =>
        {
            var session = booth.CreateSession(req.TemplateId);
            return Results.Ok(new SessionDto(session));
        });

        api.MapGet("/sessions", (PhotoBoothService booth) =>
            booth.ListSessions().Select(s => new SessionDto(s)));

        api.MapGet("/sessions/{id}", (string id, PhotoBoothService booth) =>
        {
            var session = booth.GetSession(new SessionId(id));
            return Results.Ok(new SessionDto(session));
        });

        api.MapPost("/sessions/{id}/capture", async (string id, PhotoBoothService booth, CancellationToken ct) =>
        {
            var photo = await booth.CaptureAsync(new SessionId(id), ct);
            return Results.Ok(photo);
        });

        api.MapPost("/sessions/{id}/render", async (string id, PhotoBoothService booth, CancellationToken ct) =>
        {
            var render = await booth.RenderAsync(new SessionId(id), ct);
            return Results.Ok(render);
        });

        api.MapPost("/sessions/{id}/print", async (string id, PhotoBoothService booth, CancellationToken ct) =>
        {
            var printJob = await booth.PrintAsync(new SessionId(id), ct);
            return Results.Ok(printJob);
        });

        api.MapGet("/sessions/{id}/files/{fileName}", (string id, string fileName) =>
        {
            var dir = PhotoBoothService.GetSessionDir(new SessionId(id));
            var path = Path.Combine(dir, fileName);
            if (!File.Exists(path)) return Results.NotFound();
            var contentType = fileName.EndsWith(".jpg", StringComparison.OrdinalIgnoreCase)
                ? "image/jpeg"
                : "application/octet-stream";
            return Results.File(path, contentType);
        });
    }

    public sealed record CreateSessionRequest(string? TemplateId);

    public sealed record SessionDto(
        string Id,
        DateTimeOffset CreatedAtUtc,
        string Status,
        string? TemplateId,
        IReadOnlyList<SessionPhoto> Photos,
        SessionRender? Render,
        PrintJob? PrintJob,
        string? ErrorMessage)
    {
        public SessionDto(PhotoSession session) : this(
            session.Id.Value,
            session.CreatedAtUtc,
            session.Status.ToString(),
            session.TemplateId,
            session.Photos,
            session.Render,
            session.PrintJob,
            session.ErrorMessage)
        {
        }
    }
}

