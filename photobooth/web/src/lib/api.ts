import { httpJson } from './http';
import type { SessionDto } from './types';

export async function health(): Promise<{ ok: boolean; ts: string }> {
  return httpJson('/api/health', { method: 'GET' });
}

export async function createSession(templateId?: string | null): Promise<SessionDto> {
  return httpJson('/api/sessions', {
    method: 'POST',
    body: JSON.stringify({ templateId: templateId ?? null }),
  });
}

export async function listSessions(): Promise<SessionDto[]> {
  return httpJson('/api/sessions', { method: 'GET' });
}

export async function getSession(id: string): Promise<SessionDto> {
  return httpJson(`/api/sessions/${id}`, { method: 'GET' });
}

export async function capture(id: string): Promise<unknown> {
  return httpJson(`/api/sessions/${id}/capture`, { method: 'POST' });
}

export async function render(id: string): Promise<unknown> {
  return httpJson(`/api/sessions/${id}/render`, { method: 'POST' });
}

export async function printJob(id: string): Promise<unknown> {
  return httpJson(`/api/sessions/${id}/print`, { method: 'POST' });
}

export function sessionFileUrl(sessionId: string, fileName: string) {
  return `/api/sessions/${sessionId}/files/${encodeURIComponent(fileName)}`;
}
