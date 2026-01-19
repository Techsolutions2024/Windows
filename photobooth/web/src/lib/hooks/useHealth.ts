import { useEffect, useState } from 'react';
import { health } from '../api';

export function useHealth() {
  const [status, setStatus] = useState<'unknown' | 'ok' | 'down'>('unknown');
  const [message, setMessage] = useState<string | null>(null);

  useEffect(() => {
    let cancelled = false;

    const tick = async () => {
      try {
        const h = await health();
        if (cancelled) return;
        setStatus(h.ok ? 'ok' : 'down');
        setMessage(h.ts);
      } catch (e) {
        if (cancelled) return;
        setStatus('down');
        setMessage(String(e));
      }
    };

    void tick();
    const id = window.setInterval(tick, 4000);
    return () => {
      cancelled = true;
      window.clearInterval(id);
    };
  }, []);

  return { status, message };
}
