import type { ReactNode } from 'react';
import { cls } from '../lib/cls';
import { StatusBadge } from './StatusBadge';
import { useHealth } from '../lib/hooks/useHealth';

export function AppShell(props: {
  mode: 'kiosk' | 'admin';
  children: ReactNode;
  onGoKiosk: () => void;
  onGoAdmin: () => void;
}) {
  const health = useHealth();

  return (
    <div className="min-h-screen bg-slate-950 text-slate-50">
      <header className="sticky top-0 z-10 border-b border-white/10 bg-slate-950/70 backdrop-blur">
        <div className="mx-auto flex max-w-6xl items-center justify-between px-4 py-3">
          <div className="flex items-center gap-3">
            <div className="h-9 w-9 rounded-xl bg-white/10" />
            <div>
              <div className="text-xs uppercase tracking-widest text-slate-400">
                PhotoBooth
              </div>
              <div className="text-sm font-semibold">
                {props.mode === 'admin' ? 'Admin Console' : 'Kiosk'}
              </div>
            </div>
          </div>

          <div className="flex items-center gap-3">
            <StatusBadge
              label={
                health.status === 'ok'
                  ? 'API OK'
                  : health.status === 'down'
                    ? 'API Down'
                    : 'Checking…'
              }
              tone={
                health.status === 'ok'
                  ? 'good'
                  : health.status === 'down'
                    ? 'bad'
                    : 'neutral'
              }
              title={health.message ?? undefined}
            />

            <button
              className={cls(
                'rounded-xl px-3 py-2 text-sm font-semibold',
                props.mode === 'kiosk'
                  ? 'bg-white/10 text-white'
                  : 'bg-white/5 text-slate-200 hover:bg-white/10',
              )}
              onClick={props.onGoKiosk}
              type="button"
            >
              Kiosk
            </button>
            <button
              className={cls(
                'rounded-xl px-3 py-2 text-sm font-semibold',
                props.mode === 'admin'
                  ? 'bg-white/10 text-white'
                  : 'bg-white/5 text-slate-200 hover:bg-white/10',
              )}
              onClick={props.onGoAdmin}
              type="button"
            >
              Admin
            </button>
          </div>
        </div>
      </header>

      <main className="mx-auto max-w-6xl px-4 py-6">{props.children}</main>

      <footer className="border-t border-white/10 py-6">
        <div className="mx-auto flex max-w-6xl items-center justify-between px-4 text-xs text-slate-500">
          <div>© {new Date().getFullYear()} PhotoBooth</div>
          <div className="flex items-center gap-2">
            <span className="rounded-full bg-white/10 px-2 py-1">v0.1</span>
            <span className="text-slate-600">Production skeleton</span>
          </div>
        </div>
      </footer>
    </div>
  );
}
