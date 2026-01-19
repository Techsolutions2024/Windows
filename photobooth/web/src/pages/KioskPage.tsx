import { useEffect, useMemo, useState } from 'react';
import * as api from '../lib/api';
import type { SessionDto } from '../lib/types';
import { HttpError } from '../lib/http';

type Step = 'idle' | 'countdown' | 'capturing' | 'review';

function sleep(ms: number) {
  return new Promise((r) => setTimeout(r, ms));
}

function formatUtc(s?: string | null) {
  if (!s) return '-';
  const d = new Date(s);
  if (Number.isNaN(d.getTime())) return s;
  return d.toLocaleString();
}

export function KioskPage() {
  const [session, setSession] = useState<SessionDto | null>(null);
  const [step, setStep] = useState<Step>('idle');
  const [countdown, setCountdown] = useState<number>(0);
  const [busy, setBusy] = useState(false);
  const [error, setError] = useState<string | null>(null);

  const photosCount = session?.photos?.length ?? 0;

  const canStart = !busy;
  const canCapture = !!session && !busy;
  const canRender = !!session && photosCount > 0 && !busy;
  const canPrint = !!session?.render && !busy;

  const primaryLabel = useMemo(() => {
    if (step === 'idle') return 'Start';
    if (step === 'review') return 'New Session';
    return '…';
  }, [step]);

  useEffect(() => {
    const keyHandler = (e: KeyboardEvent) => {
      if (e.key === 'Enter' && canStart && step === 'idle') {
        void startFlow();
      }
    };
    window.addEventListener('keydown', keyHandler);
    return () => window.removeEventListener('keydown', keyHandler);
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [canStart, step, session]);

  async function refreshSession(id: string) {
    const s = await api.getSession(id);
    setSession(s);
  }

  function showError(e: unknown) {
    if (e instanceof HttpError) {
      setError(`Request failed: ${e.message}`);
      return;
    }
    setError(String(e));
  }

  async function newSession() {
    setError(null);
    setBusy(true);
    try {
      const s = await api.createSession(null);
      setSession(s);
      setStep('idle');
    } finally {
      setBusy(false);
    }
  }

  async function startFlow() {
    setError(null);
    setBusy(true);
    try {
      let active = session;
      if (!active) {
        active = await api.createSession(null);
        setSession(active);
      }

      setStep('countdown');
      for (let i = 3; i >= 1; i -= 1) {
        setCountdown(i);
        await sleep(750);
      }
      setCountdown(0);

      setStep('capturing');
      await api.capture(active.id);
      await refreshSession(active.id);

      setStep('review');
    } catch (e) {
      setStep('idle');
      showError(e);
    } finally {
      setBusy(false);
    }
  }

  async function doCapture() {
    if (!session) return;
    setError(null);
    setBusy(true);
    try {
      await api.capture(session.id);
      await refreshSession(session.id);
    } catch (e) {
      showError(e);
    } finally {
      setBusy(false);
    }
  }

  async function doRender() {
    if (!session) return;
    setError(null);
    setBusy(true);
    try {
      await api.render(session.id);
      await refreshSession(session.id);
    } catch (e) {
      showError(e);
    } finally {
      setBusy(false);
    }
  }

  async function doPrint() {
    if (!session) return;
    setError(null);
    setBusy(true);
    try {
      await api.printJob(session.id);
      await refreshSession(session.id);
    } catch (e) {
      showError(e);
    } finally {
      setBusy(false);
    }
  }

  async function reset() {
    setSession(null);
    setStep('idle');
    setCountdown(0);
    setError(null);
  }

  return (
    <div className="section-grid">
      <section className="card">
        <div className="flex items-center justify-between gap-3">
          <div>
            <div className="text-xs uppercase tracking-widest text-slate-400">Kiosk</div>
            <h1 style={{ margin: '6px 0 0 0', fontSize: 20 }}>Session Flow</h1>
          </div>
          <button className="btn" type="button" onClick={reset} disabled={busy}>
            Reset
          </button>
        </div>

        <div style={{ height: 12 }} />

        <div className="kv">
          <div>Session</div>
          <div className="mono">{session?.id ?? '(none)'}</div>
          <div>Status</div>
          <div>{session?.status ?? '-'}</div>
          <div>Created</div>
          <div>{formatUtc(session?.createdAtUtc)}</div>
          <div>Photos</div>
          <div>{photosCount}</div>
          <div>Render</div>
          <div className="mono">{session?.render?.fileName ?? '-'}</div>
          <div>Print</div>
          <div className="mono">{session?.printJob?.fileName ?? '-'}</div>
        </div>

        <div style={{ height: 14 }} />

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

        <div className="flex" style={{ flexWrap: 'wrap', gap: 10 }}>
          <button className="btn btn-primary" type="button" onClick={startFlow} disabled={!canStart}>
            {primaryLabel}
          </button>
          <button className="btn" type="button" onClick={newSession} disabled={busy}>
            New Session
          </button>
          <button className="btn" type="button" onClick={doCapture} disabled={!canCapture}>
            Capture
          </button>
          <button className="btn" type="button" onClick={doRender} disabled={!canRender}>
            Render
          </button>
          <button className="btn" type="button" onClick={doPrint} disabled={!canPrint}>
            Print
          </button>
        </div>

        <div style={{ height: 14 }} />

        {step === 'countdown' ? (
          <div
            style={{
              fontSize: 72,
              fontWeight: 800,
              letterSpacing: '-.06em',
              textAlign: 'center',
              padding: 24,
              borderRadius: 18,
              border: '1px solid rgba(255,255,255,.10)',
              background: 'rgba(255,255,255,.03)',
            }}
          >
            {countdown}
          </div>
        ) : null}
      </section>

      <section className="card">
        <div className="flex items-center justify-between gap-3">
          <div>
            <div className="text-xs uppercase tracking-widest text-slate-400">Preview</div>
            <h2 style={{ margin: '6px 0 0 0', fontSize: 20 }}>Images</h2>
          </div>
          <div className="text-xs text-slate-400">Tap to fullscreen (browser)</div>
        </div>

        <div style={{ height: 12 }} />

        <div className="gallery">
          {(session?.photos ?? []).map((p) => (
            <img
              key={p.fileName}
              className="img"
              src={api.sessionFileUrl(session.id, p.fileName)}
              alt={p.fileName}
              loading="lazy"
            />
          ))}

          {session?.render ? (
            <img
              className="img"
              src={api.sessionFileUrl(session.id, session.render.fileName)}
              alt={session.render.fileName}
              loading="lazy"
            />
          ) : null}
        </div>
      </section>
    </div>
  );
}
