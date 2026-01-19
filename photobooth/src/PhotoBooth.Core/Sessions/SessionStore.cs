using System.Collections.Concurrent;

namespace PhotoBooth.Core.Sessions;

public sealed class SessionStore
{
    private readonly ConcurrentDictionary<string, PhotoSession> _sessions = new(StringComparer.Ordinal);

    public PhotoSession Create(DateTimeOffset nowUtc, string? templateId)
    {
        var session = new PhotoSession
        {
            Id = SessionId.New(),
            CreatedAtUtc = nowUtc,
            TemplateId = templateId,
            Status = SessionStatus.Created,
        };

        _sessions[session.Id.Value] = session;
        return session;
    }

    public bool TryGet(SessionId id, out PhotoSession? session)
        => _sessions.TryGetValue(id.Value, out session);

    public PhotoSession GetRequired(SessionId id)
    {
        if (!_sessions.TryGetValue(id.Value, out var session) || session is null)
        {
            throw new KeyNotFoundException($"Session not found: {id.Value}");
        }

        return session;
    }

    public IEnumerable<PhotoSession> List() => _sessions.Values.OrderByDescending(s => s.CreatedAtUtc);

    public bool TryRemove(SessionId id) => _sessions.TryRemove(id.Value, out _);
}

