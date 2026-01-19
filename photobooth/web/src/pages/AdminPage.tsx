import { useEffect, useMemo, useState } from 'react';
import * as api from '../lib/api';
import type { SessionDto } from '../lib/types';

function formatUtc(s: string) {
  const d = new Date(s);
  if (Number.isNaN(d.getTime())) return s;
  return d.toLocaleString();
}

function statusTone(status: SessionDto['status']) {
  if (status === 'Printed') return 'rgba(16,185,129,.25)';
  if (status === 'Error') return 'rgba(244,63,94,.25)';
  if (status === 'Capturing') return 'rgba(59,130,246,.25)';
  return 'rgba(255,255,255,.10)';
}

export function AdminPage() {
  const [sessions, setSessions] = useState<SessionDto[]>([]);
  const [selected, setSelected] = useState<SessionDto | null>(null);
  const [poll, setPoll] = useState(true);
  const [templateId, setTemplateId] = useState<string>('');
  const [error, setError] = useState<string | null>(null);

  const selectedId = selected?.id ?? null;

  const refresh = async () => {
    const list = await api.listSessions();
    setSessions(list);
    if (selectedId) {
      const s = list.find((x) => x.id === selectedId);
      if (s) setSelected(s);
    }
  };

  useEffect(() => {
    let cancelled = false;
    const tick = async () => {
      try {
        await refresh();
        if (!cancelled) setError(null);
      } catch (e) {
        if (!cancelled) setError(String(e));
      }
    };

    void tick();
    if (!poll) return;

    const id = window.setInterval(tick, 2000);
    return () => {
      cancelled = true;
      window.clearInterval(id);
    };
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [poll, selectedId]);

  const selectedDetails = useMemo(() => {
    if (!selected) return null;
    return sessions.find((s) => s.id === selected.id) ?? selected;
  }, [selected, sessions]);

  const create = async () => {
    try {
      setError(null);
      const s = await api.createSession(templateId.trim() || null);
      setSelected(s);
      await refresh();
    } catch (e) {
      setError(String(e));
    }
  };

  const action = async (kind: 'capture' | 'render' | 'print') => {
    if (!selectedDetails) return;
    try {
      setError(null);
      if (kind === 'capture') await api.capture(selectedDetails.id);
      if (kind === 'render') await api.render(selectedDetails.id);
      if (kind === 'print') await api.printJob(selectedDetails.id);
      await refresh();
    } catch (e) {
      setError(String(e));
    }
  };

  return (
    <div className="section-grid">
      <section className="card">
        <div className="flex items-center justify-between gap-3">
          <div>
            <div className="text-xs uppercase tracking-widest text-slate-400">Admin</div>
            <h1 style={{ margin: '6px 0 0 0', fontSize: 20 }}>Sessions</h1>
          </div>
          <button className="btn" type="button" onClick={() => setPoll((v) => !v)}>
            Poll: {poll ? 'On' : 'Off'}
          </button>
        </div>

        <div style={{ height: 12 }} />

        {error ? (
          <div
            className="mono"
            style={{
              padding: 12,
              borderRadius: 12,
              border: '1px solid rgba(244,63,94,.35)',
              background: 'rgba(244,63,94,.08)',
              color: 'rgba(255,255,255,.95)',
              marginBottom: 12,
            }}
          >
            {error}
          </div>
        ) : null}

        <div className="flex" style={{ gap: 10 }}>
          <input
            className="input"
            placeholder="Template ID (optional)"
            value={templateId}
            onChange={(e) => setTemplateId(e.target.value)}
          />
          <button className="btn btn-primary" type="button" onClick={create}>
            Create
          </button>
        </div>

        <div style={{ height: 14 }} />

        <div style={{ display: 'grid', gap: 10 }}>
          {sessions.length === 0 ? (
            <div className="text-slate-400">No sessions yet.</div>
          ) : (
            sessions
              .slice()
              .sort((a, b) => (a.createdAtUtc < b.createdAtUtc ? 1 : -1))
              .map((s) => (
                <button
                  key={s.id}
                  type="button"
                  onClick={() => setSelected(s)}
                  className="btn"
                  style={{
                    textAlign: 'left',
                    display: 'grid',
                    gap: 8,
                    background: selectedId === s.id ? 'rgba(255,255,255,.14)' : undefined,
                  }}
                >
                  <div className="flex items-center justify-between gap-3">
                    <div className="mono" style={{ fontSize: 12 }}>
                      {s.id}
                    </div>
                    <span
                      className="mono"
                      style={{
                        padding: '4px 10px',
                        borderRadius: 999,
                        background: statusTone(s.status),
                        border: '1px solid rgba(255,255,255,.10)',
                        fontSize: 12,
                      }}
                    >
                      {s.status}
                    </span>
                  </div>
                  <div className="text-xs text-slate-400">
                    {formatUtc(s.createdAtUtc)} · {s.photos.length} photos
                  </div>
                </button>
              ))
          )}
        </div>
      </section>

      <section className="card">
        <div className="flex items-center justify-between gap-3">
          <div>
            <div className="text-xs uppercase tracking-widest text-slate-400">Details</div>
            <h2 style={{ margin: '6px 0 0 0', fontSize: 20 }}>Selected</h2>
          </div>
          <div className="flex" style={{ gap: 10, flexWrap: 'wrap' }}>
            <button
              className="btn"
              type="button"
              onClick={() => action('capture')}
              disabled={!selectedDetails}
            >
              Capture
            </button>
            <button
              className="btn"
              type="button"
              onClick={() => action('render')}
              disabled={!selectedDetails || selectedDetails.photos.length === 0}
            >
              Render
            </button>
            <button
              className="btn"
              type="button"
              onClick={() => action('print')}
              disabled={!selectedDetails || !selectedDetails.render}
            >
              Print
            </button>
          </div>
        </div>

        <div style={{ height: 12 }} />

        {!selectedDetails ? (
          <div className="text-slate-400">Select a session to inspect.</div>
        ) : (
          <>
            <div className="kv">
              <div>ID</div>
              <div className="mono">{selectedDetails.id}</div>
              <div>Status</div>
              <div>{selectedDetails.status}</div>
              <div>Created</div>
              <div>{formatUtc(selectedDetails.createdAtUtc)}</div>
              <div>Template</div>
              <div className="mono">{selectedDetails.templateId ?? '-'}</div>
              <div>Photos</div>
              <div>{selectedDetails.photos.length}</div>
              <div>Render</div>
              <div className="mono">{selectedDetails.render?.fileName ?? '-'}</div>
              <div>Print</div>
              <div className="mono">{selectedDetails.printJob?.fileName ?? '-'}</div>
            </div>

            <div style={{ height: 14 }} />

            <div className="gallery">
              {selectedDetails.photos.map((p) => (
                <img
                  key={p.fileName}
                  className="img"
                  src={api.sessionFileUrl(selectedDetails.id, p.fileName)}
                  alt={p.fileName}
                  loading="lazy"
                />
              ))}
              {selectedDetails.render ? (
                <img
                  className="img"
                  src={api.sessionFileUrl(selectedDetails.id, selectedDetails.render.fileName)}
                  alt={selectedDetails.render.fileName}
                  loading="lazy"
                />
              ) : null}
            </div>
          </>
        )}
      </section>
    </div>
  );
}
