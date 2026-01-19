import { useEffect, useState } from 'react';
import * as api from '../api';
import type { SessionDto } from '../types';

export function useSessionPoll(sessionId: string | null, enabled: boolean, intervalMs: number) {
  const [session, setSession] = useState<SessionDto | null>(null);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    let cancelled = false;

    const tick = async () => {
      if (!sessionId) {
        setSession(null);
        return;
      }
      try {
        const s = await api.getSession(sessionId);
        if (!cancelled) {
          setSession(s);
          setError(null);
        }
      } catch (e) {
        if (!cancelled) setError(String(e));
      }
    };

    void tick();
    if (!enabled) return;

    const id = window.setInterval(tick, intervalMs);
    return () => {
      cancelled = true;
      window.clearInterval(id);
    };
  }, [sessionId, enabled, intervalMs]);

  return { session, error, setSession };
}
