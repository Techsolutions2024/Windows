export function cls(...parts: Array<string | undefined | false | null>) {
  return parts.filter(Boolean).join(' ');
}
