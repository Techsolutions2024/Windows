export function StatusBadge(props: {
  label: string;
  tone: 'good' | 'bad' | 'neutral';
  title?: string;
}) {
  const cls =
    props.tone === 'good'
      ? 'bg-emerald-500/15 text-emerald-200 ring-1 ring-emerald-500/30'
      : props.tone === 'bad'
        ? 'bg-rose-500/15 text-rose-200 ring-1 ring-rose-500/30'
        : 'bg-white/10 text-slate-200 ring-1 ring-white/15';

  return (
    <span
      className={`inline-flex items-center gap-2 rounded-full px-3 py-1 text-xs font-semibold ${cls}`}
      title={props.title}
    >
      <span className="h-1.5 w-1.5 rounded-full bg-current opacity-80" />
      {props.label}
    </span>
  );
}
